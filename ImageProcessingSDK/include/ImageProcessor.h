#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <algorithm>
#include <memory>
#include <functional>
#include <cstdlib>
#include <cstring>
#include "ImageProcessingSDK.h"
#include "IOperation.h"
#include "SpscRingBuffer.h"

namespace ip {

// Qt-independent rectangle structure
struct Rect {
    int x;
    int y;
    int width;
    int height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}
};

struct ImageRegion {
    int id;
    int parentImageId;
    Rect rect;
    std::vector<uint8_t> pixelData;
    int width;
    int height;
    int channels;
};

class CompositeTask {
public:
    ImageRegion region;
    std::vector<std::shared_ptr<IOperation>> pipeline;
    std::shared_ptr<std::atomic<size_t>> globalCounter;
    // Destination slot in the orchestrator's results[] vector. The task is
    // copied by value into the worker's ring buffer, so the processed pixels
    // must be written back here for reassembleImage() to find them.
    ImageRegion* output = nullptr;

    void execute() {
        if (pipeline.empty()) {
            if (globalCounter) globalCounter->fetch_sub(1, std::memory_order_acq_rel);
            return;
        }

        SDK_Image_t current_buffer;
        ipsdk_init_image(&current_buffer);
        current_buffer.width = region.width;
        current_buffer.height = region.height;
        current_buffer.channels = region.channels;
        int size = region.width * region.height * region.channels;
        current_buffer.data = (unsigned char*)std::malloc(size);
        std::memcpy(current_buffer.data, region.pixelData.data(), size);

        SDK_Image_t next_buffer;
        ipsdk_init_image(&next_buffer);

        for (auto& op : pipeline) {
            op->apply(&current_buffer, &next_buffer);

            SDK_Image_t temp = current_buffer;
            current_buffer = next_buffer;
            next_buffer = temp;
        }

        if (current_buffer.data && current_buffer.width > 0 && current_buffer.height > 0) {
            int out_size = current_buffer.width * current_buffer.height * current_buffer.channels;
            // Publish the processed pixels into the orchestrator's results slot
            // (falling back to the local region if no slot was wired up). The
            // rect/id were already set by the orchestrator, so leave them.
            ImageRegion* dst = output ? output : &region;
            dst->pixelData.assign(current_buffer.data, current_buffer.data + out_size);
            dst->width = current_buffer.width;
            dst->height = current_buffer.height;
            dst->channels = current_buffer.channels;
        }

        ipsdk_free_image(&current_buffer);
        ipsdk_free_image(&next_buffer);

        if (globalCounter) {
            globalCounter->fetch_sub(1, std::memory_order_acq_rel);
        }
    }
};

class ImageProcessor {
public:
    ImageProcessor(int numWorkers = 0);
    ~ImageProcessor();

    // Composite operation: Blur -> Resize
    void process(SDK_Image_t* image, int rows, int cols);

    // Single operations
    void processBlur(SDK_Image_t* image, int radius, int rows, int cols);
    void processResize(SDK_Image_t* image, int rows, int cols);
    void processGrayscale(SDK_Image_t* image, int rows, int cols);
    void processSharpen(SDK_Image_t* image, double amount, double sigma, int threshold, int rows, int cols);

private:
    using PipelineBuilder = std::function<void(CompositeTask&, ImageRegion&, int, int)>;

    
    void executeParallelPipeline(SDK_Image_t* image, int rows, int cols, const PipelineBuilder& buildPipeline,
                                 int targetW = 0, int targetH = 0);
    std::vector<ImageRegion> splitIntoRegions(const SDK_Image_t* source, int rows, int cols) const;
    void reassembleImage(SDK_Image_t* image, const std::vector<ImageRegion>& results, int targetWidth, int targetHeight) const;
    void workerLoop(int workerId);

    std::unique_ptr<SpscRingBuffer<CompositeTask, 64>[]> buffers;
    std::vector<std::thread> workers;
    std::atomic<bool> isRunning;
    int numWorkers;
};

} 
