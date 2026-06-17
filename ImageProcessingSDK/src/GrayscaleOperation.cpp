#include "GrayscaleOperation.h"
#include <cstdlib>
#include <cstring>

void GrayscaleOperation::apply(const SDK_Image_t* src, SDK_Image_t* dst) const {
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

    if (c < 3) {
        std::memcpy(dst->data, src->data, size);
        return;
    }

    for (int i = 0; i < w * h; ++i) {
        const unsigned char* sp = src->data + i * c;
        unsigned char* dp = dst->data + i * c;

        unsigned char gray = static_cast<unsigned char>(
            (77 * sp[0] + 150 * sp[1] + 29 * sp[2]) >> 8);

        dp[0] = gray;
        dp[1] = gray;
        dp[2] = gray;
        for (int ch = 3; ch < c; ++ch) dp[ch] = sp[ch];
    }
}
