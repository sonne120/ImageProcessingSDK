#include "ImageProcessor.h"
#include "BlurOperation.h"
#include "ResizeOperation.h"
#include "GrayscaleOperation.h"
#include "SharpenOperation.h"
#include <iostream>
#include <cstring>

namespace ip {

ImageProcessor::ImageProcessor(int numWorkers_) : isRunning(true) {
    if (numWorkers_ <= 0) {
        numWorkers = std::thread::hardware_concurrency();
    } else {
        numWorkers = numWorkers_;
    }

    buffers = std::make_unique<SpscRingBuffer<CompositeTask, 64>[]>(numWorkers);

    for (int i = 0; i < numWorkers; ++i) {
        workers.emplace_back(&ImageProcessor::workerLoop, this, i);
    }
}

ImageProcessor::~ImageProcessor() {
    isRunning.store(false, std::memory_order_relaxed);
    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }
}

std::vector<ImageRegion>
ImageProcessor::splitIntoRegions(const SDK_Image_t* source, int rows, int cols) const
{
    std::vector<ImageRegion> regions;
    if (!source || !source->data || source->width <= 0 || source->height <= 0 || rows <= 0 || cols <= 0)
        return regions;

    const int w = source->width;
    const int h = source->height;
    const int channels = source->channels;
    const int baseW = w / cols;
    const int baseH = h / rows;

    int currentId = 0;
    regions.reserve(rows * cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int x = c * baseW;
            const int y = r * baseH;
            const int regionW = (c == cols - 1) ? (w - x) : baseW;
            const int regionH = (r == rows - 1) ? (h - y) : baseH;

            ImageRegion reg;
            reg.id = currentId++;
            reg.parentImageId = 1;
            reg.rect = Rect(x, y, regionW, regionH);
            reg.width = regionW;
            reg.height = regionH;
            reg.channels = channels;

            size_t regionBytes = regionW * regionH * channels;
            reg.pixelData.resize(regionBytes);

            for (int row = 0; row < regionH; ++row) {
                const unsigned char* srcRow = source->data + ((y + row) * w + x) * channels;
                unsigned char* dstRow = reg.pixelData.data() + row * regionW * channels;
                std::memcpy(dstRow, srcRow, regionW * channels);
            }

            regions.push_back(std::move(reg));
        }
    }

    return regions;
}

void ImageProcessor::workerLoop(int workerId) {
    CompositeTask task;
    while (isRunning.load(std::memory_order_relaxed)) {
        if (buffers[workerId].pop(task)) {
            task.execute();
        } else {
            std::this_thread::yield();
        }
    }
}


void ImageProcessor::executeParallelPipeline(SDK_Image_t* image, int rows, int cols, const PipelineBuilder& buildPipeline,
                                             int targetW, int targetH) {
    if (!image || !image->data || image->width <= 0 || image->height <= 0) return;

    const int outW = (targetW > 0) ? targetW : image->width;
    const int outH = (targetH > 0) ? targetH : image->height;

    auto regions = splitIntoRegions(image, rows, cols);
    auto counter = std::make_shared<std::atomic<size_t>>(regions.size());
    std::vector<ImageRegion> results(regions.size());

    const int baseW = outW / cols;
    const int baseH = outH / rows;

    for (size_t i = 0; i < regions.size(); ++i) {
        const int r = static_cast<int>(i) / cols;
        const int c = static_cast<int>(i) % cols;

        const int x = c * baseW;
        const int y = r * baseH;
        const int cellW = (c == cols - 1) ? (outW - x) : baseW;
        const int cellH = (r == rows - 1) ? (outH - y) : baseH;

        results[i].id = regions[i].id;
        results[i].parentImageId = regions[i].parentImageId;
        results[i].rect = Rect(x, y, cellW, cellH);

        CompositeTask task;
        task.region = std::move(regions[i]);

        buildPipeline(task, results[i], cellW, cellH);

        task.globalCounter = counter;
        task.output = &results[i];

        size_t targetWorker = i % numWorkers;
        while (!buffers[targetWorker].push(task)) {
            std::this_thread::yield();
        }
    }


    while (counter->load(std::memory_order_acquire) > 0) {
        std::this_thread::yield();
    }

    reassembleImage(image, results, outW, outH);
}

void ImageProcessor::process(SDK_Image_t* image, int rows, int cols) {
    // 300x300 target with Blur(20) and Resize
    executeParallelPipeline(image, rows, cols,
        [](CompositeTask& task, ImageRegion& res, int w, int h) {
            task.pipeline.push_back(std::make_shared<BlurOperation>(20));
            task.pipeline.push_back(std::make_shared<ResizeOperation>(w, h));
        }, 300, 300);
}

void ImageProcessor::processBlur(SDK_Image_t* image, int radius, int rows, int cols) {
    if (!image) return;
    executeParallelPipeline(image, rows, cols, 
        [radius](CompositeTask& task, ImageRegion& res, int w, int h) {
            task.pipeline.push_back(std::make_shared<BlurOperation>(radius));
        });
}

void ImageProcessor::processResize(SDK_Image_t* image, int rows, int cols) {
    executeParallelPipeline(image, rows, cols,
        [](CompositeTask& task, ImageRegion& res, int w, int h) {
            task.pipeline.push_back(std::make_shared<ResizeOperation>(w, h));
        }, 300, 300);
}

void ImageProcessor::processGrayscale(SDK_Image_t* image, int rows, int cols) {
    executeParallelPipeline(image, rows, cols,
        [](CompositeTask& task, ImageRegion& res, int w, int h) {
            task.pipeline.push_back(std::make_shared<GrayscaleOperation>());
        });
}

void ImageProcessor::processSharpen(SDK_Image_t* image, double amount, double sigma, int threshold,
                                    int rows, int cols) {
    executeParallelPipeline(image, rows, cols,
        [amount, sigma, threshold](CompositeTask& task, ImageRegion& res, int w, int h) {
            task.pipeline.push_back(std::make_shared<SharpenOperation>(amount, sigma, threshold));
        });
}

void ImageProcessor::reassembleImage(SDK_Image_t* image, const std::vector<ImageRegion>& results, int targetWidth, int targetHeight) const {
    if (!image || results.empty()) return;

    const int channels = image->channels;
    std::vector<unsigned char> finalBuffer(targetWidth * targetHeight * channels, 0);

    for (const auto& reg : results) {
        if (!reg.pixelData.empty() && reg.width > 0 && reg.height > 0) {
            for (int row = 0; row < reg.height; ++row) {
                int dstY = reg.rect.y + row;
                if (dstY >= targetHeight) continue;

                const unsigned char* srcRow = reg.pixelData.data() + row * reg.width * channels;
                unsigned char* dstRow = finalBuffer.data() + (dstY * targetWidth + reg.rect.x) * channels;

                int copyWidth = std::min(reg.width, targetWidth - reg.rect.x);

                if (copyWidth > 0) {
                    std::memcpy(dstRow, srcRow, copyWidth * channels);
                }
            }
        }
    }

    // Replace original image data with final result
    std::free(image->data);
    image->width = targetWidth;
    image->height = targetHeight;
    image->channels = channels;

    int sz = targetWidth * targetHeight * channels;
    image->data = static_cast<unsigned char*>(std::malloc(sz));
    std::memcpy(image->data, finalBuffer.data(), sz);
}


}
