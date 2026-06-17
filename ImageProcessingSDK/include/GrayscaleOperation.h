#pragma once
#include "IOperation.h"

class GrayscaleOperation : public IOperation {
public:
    GrayscaleOperation() = default;
    void apply(const SDK_Image_t* src, SDK_Image_t* dst) const override;
};
