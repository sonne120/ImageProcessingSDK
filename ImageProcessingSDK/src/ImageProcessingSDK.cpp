#include "ImageProcessingSDK.h"
#include "ImageProcessor.h"
#include "BlurOperation.h"
#include "ResizeOperation.h"
#include <cstdlib>
#include <memory>
#include <exception>

namespace {
    std::unique_ptr<ip::ImageProcessor>& get_processor() {
        static std::unique_ptr<ip::ImageProcessor> processor = std::make_unique<ip::ImageProcessor>();
        return processor;
    }
}

extern "C" {

void ipsdk_init_image(SDK_Image_t* img) {
    if (!img) return;
    img->width = 0;
    img->height = 0;
    img->channels = 0;
    img->data = nullptr;
}

void ipsdk_free_image(SDK_Image_t* img) {
    if (img) {
        std::free(img->data); 

        img->data = nullptr;
        img->width = 0;
        img->height = 0;
        img->channels = 0;
    }
}

int ipsdk_blur_direct(const SDK_Image_t* src, SDK_Image_t* dst, int radius) {
    if (!src || !dst || !src->data) return -1;

    try {
        BlurOperation op(radius);
        op.apply(src, dst);
        return 0;
    } catch (...) {
        return -1;
    }
}

int ipsdk_resize_direct(const SDK_Image_t* src, SDK_Image_t* dst, int new_w, int new_h) {
    if (!src || !dst || !src->data) return -1;

    try {
        ResizeOperation op(new_w, new_h);
        op.apply(src, dst);
        return 0;
    } catch (...) {
        return -1;
    }
}

int ipsdk_process_blur(SDK_Image_t* image, int radius, int rows, int cols) {
    if (!image || !image->data) return -1;

    try {
        get_processor()->processBlur(image, radius, rows, cols);
        return 0;
    } catch (...) {
        return -1;
    }
}

int ipsdk_process_resize(SDK_Image_t* image, int rows, int cols) {
    if (!image || !image->data) return -1;

    try {
        get_processor()->processResize(image, rows, cols);
        return 0;
    } catch (...) {
        return -1;
    }
}

int ipsdk_process_grayscale(SDK_Image_t* image, int rows, int cols) {
    if (!image || !image->data) return -1;

    try {
        get_processor()->processGrayscale(image, rows, cols);
        return 0;
    } catch (...) {
        return -1;
    }
}

int ipsdk_process_sharpen(SDK_Image_t* image, double amount, double sigma, int threshold, int rows, int cols) {
    if (!image || !image->data) return -1;

    try {
        get_processor()->processSharpen(image, amount, sigma, threshold, rows, cols);
        return 0;
    } catch (...) {
        return -1;
    }
}

int ipsdk_process_composite(SDK_Image_t* image, int rows, int cols) {
    if (!image || !image->data) return -1;

    try {
        get_processor()->process(image, rows, cols);
        return 0;
    } catch (...) {
        return -1;
    }
}

} 
