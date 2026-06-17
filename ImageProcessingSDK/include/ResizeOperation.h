#pragma once
#include "IOperation.h"

class ResizeOperation : public IOperation {
private:
    int new_w;
    int new_h;
public:
    ResizeOperation(int w, int h);
    void apply(const SDK_Image_t* src, SDK_Image_t* dst) const override;
};
