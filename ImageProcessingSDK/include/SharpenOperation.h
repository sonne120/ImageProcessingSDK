#pragma once
#include "IOperation.h"

class SharpenOperation : public IOperation {
private:
    double amount;     // Sharpening strength
    double sigma;      // Gaussian blur radius of the mask
    int    threshold;  // Minimum contrast (0..255) before sharpening
public:
    SharpenOperation(double amount, double sigma, int threshold);
    void apply(const SDK_Image_t* src, SDK_Image_t* dst) const override;
};
