#include "SharpenOperation.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

SharpenOperation::SharpenOperation(double amount_, double sigma_, int threshold_)
    : amount(amount_), sigma(sigma_), threshold(threshold_) {}

namespace {

inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

std::vector<int> boxesForGauss(double sigma, int n) {
    double wIdeal = std::sqrt((12.0 * sigma * sigma / n) + 1.0);
    int wl = static_cast<int>(std::floor(wIdeal));
    if (wl % 2 == 0) wl--;
    if (wl < 1) wl = 1;
    int wu = wl + 2;

    double mIdeal = (12.0 * sigma * sigma - n * wl * wl - 4.0 * n * wl - 3.0 * n)
                    / (-4.0 * wl - 4.0);
    int m = static_cast<int>(std::lround(mIdeal));

    std::vector<int> sizes;
    sizes.reserve(n);
    for (int i = 0; i < n; ++i) sizes.push_back(i < m ? wl : wu);
    return sizes;
}

void boxBlur1D(const std::vector<float>& in, std::vector<float>& out,
               int lines, int len, int lineStep, int stride, int cc, int r) {
    const float inv = 1.0f / (2 * r + 1);
    for (int line = 0; line < lines; ++line) {
        for (int ch = 0; ch < cc; ++ch) {
            const int base = line * lineStep + ch;

            float sum = in[base] * (r + 1);
            for (int j = 1; j <= r; ++j)
                sum += in[base + clampi(j, 0, len - 1) * stride];

            for (int i = 0; i < len; ++i) {
                out[base + i * stride] = sum * inv;
                sum += in[base + clampi(i + r + 1, 0, len - 1) * stride]
                     - in[base + clampi(i - r,     0, len - 1) * stride];
            }
        }
    }
}

void gaussianBlurRGB(const unsigned char* src, std::vector<unsigned char>& out,
                     int w, int h, int c, double sigma) {
    out.assign(static_cast<size_t>(w) * h * c, 0);

    const int cc = std::min(c, 3);
    std::vector<float> buf(static_cast<size_t>(w) * h * cc);
    std::vector<float> tmp(static_cast<size_t>(w) * h * cc);

    // Load colour channels into a contiguous cc-channel float buffer.
    for (int i = 0; i < w * h; ++i)
        for (int ch = 0; ch < cc; ++ch)
            buf[i * cc + ch] = src[i * c + ch];

    for (int box : boxesForGauss(sigma, 3)) {
        const int r = std::max(1, (box - 1) / 2);
        // Horizontal: lines = rows, len = w, step between pixels = cc.
        boxBlur1D(buf, tmp, h, w, w * cc, cc, cc, r);
        // Vertical: lines = columns, len = h, step between pixels = w * cc.
        boxBlur1D(tmp, buf, w, h, cc, w * cc, cc, r);
    }

    for (int i = 0; i < w * h; ++i) {
        for (int ch = 0; ch < c; ++ch) {
            if (ch < cc) {
                int val = static_cast<int>(std::lround(buf[i * cc + ch]));
                out[i * c + ch] = static_cast<unsigned char>(std::clamp(val, 0, 255));
            } else {
                out[i * c + ch] = src[i * c + ch];
            }
        }
    }
}

}
void SharpenOperation::apply(const SDK_Image_t* src, SDK_Image_t* dst) const {
    if (!src || !src->data || src->width <= 0 || src->height <= 0 || !dst) return;

    const int w = src->width;
    const int h = src->height;
    const int c = src->channels;
    const int size = w * h * c;

    dst->width = w;
    dst->height = h;
    dst->channels = c;
    if (dst->data) std::free(dst->data);
    dst->data = static_cast<unsigned char*>(std::malloc(size));
    if (!dst->data) return;

    std::memcpy(dst->data, src->data, size);

    if (c < 3 || amount <= 0.0 || sigma <= 0.0) {
        return; 
    }

    std::vector<unsigned char> blurred;
    gaussianBlurRGB(src->data, blurred, w, h, c, sigma);

    const int colorCh = std::min(c, 3);
    for (int i = 0; i < w * h; ++i) {
        for (int ch = 0; ch < colorCh; ++ch) {
            const int idx = i * c + ch;
            const int orig = src->data[idx];
            const int diff = orig - static_cast<int>(blurred[idx]);

            if (std::abs(diff) > threshold) {
                int val = orig + static_cast<int>(std::lround(amount * diff));
                dst->data[idx] = static_cast<unsigned char>(std::clamp(val, 0, 255));
            }
        }
    }
}
