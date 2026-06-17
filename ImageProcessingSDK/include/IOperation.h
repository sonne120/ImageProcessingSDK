#pragma once
#include "ImageProcessingSDK.h"

class IOperation {
public:
    virtual ~IOperation() = default;
    virtual void apply(const SDK_Image_t* src, SDK_Image_t* dst) const = 0;
};
