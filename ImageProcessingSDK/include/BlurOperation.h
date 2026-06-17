#pragma once
#include "IOperation.h"

class BlurOperation : public IOperation {
private:
    int radius;
public:
    explicit BlurOperation(int r);
    void apply(const SDK_Image_t* src, SDK_Image_t* dst) const override;
};
