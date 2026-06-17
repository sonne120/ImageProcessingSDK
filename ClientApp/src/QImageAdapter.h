#pragma once

#include <QImage>
#include "ImageProcessingSDK.h"
#include <memory>

namespace QtAdapter {

// RAII wrapper for SDK_Image_t
class SDKImageGuard {
public:
    SDKImageGuard() {
        ipsdk_init_image(&img);
    }

    ~SDKImageGuard() {
        if (img.data) {
            std::free(img.data);
        }
        ipsdk_free_image(&img);
    }

    SDK_Image_t* get() { return &img; }
    const SDK_Image_t* get() const { return &img; }

    // Disable copy
    SDKImageGuard(const SDKImageGuard&) = delete;
    SDKImageGuard& operator=(const SDKImageGuard&) = delete;

private:
    SDK_Image_t img;
};

inline SDK_Image_t qImageToSDK(const QImage& qimg) {
    SDK_Image_t result;
    ipsdk_init_image(&result);

    if (qimg.isNull()) {
        return result;
    }

    QImage rgba = qimg.convertToFormat(QImage::Format_RGBA8888);

    result.width = rgba.width();
    result.height = rgba.height();
    result.channels = 4;

    size_t bytes = rgba.sizeInBytes();
    result.data = (unsigned char*)std::malloc(bytes);

    if (result.data) {
        std::memcpy(result.data, rgba.constBits(), bytes);
    }

    return result;
}

// Convert SDK_Image_t to QImage 
inline QImage sdkToQImage(const SDK_Image_t* sdkImg) {
    if (!sdkImg || !sdkImg->data || sdkImg->width <= 0 || sdkImg->height <= 0) {
        return QImage();
    }

    QImage::Format format;
    switch (sdkImg->channels) {
        case 1:
            format = QImage::Format_Grayscale8;
            break;
        case 3:
            format = QImage::Format_RGB888;
            break;
        case 4:
            format = QImage::Format_RGBA8888;
            break;
        default:
            return QImage(); 
    }

    QImage result(sdkImg->data, sdkImg->width, sdkImg->height, format);
    return result.copy(); 
}

} 
