#include "BlurOperation.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <algorithm>


BlurOperation::BlurOperation(int r) : radius(r) {}


void BlurOperation::apply(const SDK_Image_t* src, SDK_Image_t* dst) const {
    if (!src || !src->data || src->width <= 0 || src->height <= 0 || !dst) return;

    const int w = src->width;
    const int h = src->height;
    const int c = src->channels;
    const int r = radius;
    const int size = w * h * c;

    dst->width = w;
    dst->height = h;
    dst->channels = c;
    if (dst->data) std::free(dst->data);
    dst->data = static_cast<unsigned char*>(std::malloc(size));
    if (!dst->data) return;

    // Temporary buffer 
    std::vector<unsigned char> tmp(size);

    auto blur = [&](const unsigned char* in, unsigned char* out, 
                      int len, int lines, int stride, int line_stride) {
        for (int line = 0; line < lines; ++line) {
            int sums[4] = {0}; 
            int count = 0;
            
            const unsigned char* in_line = in + line * line_stride;
            unsigned char* out_line = out + line * line_stride;

            // Initialize window 
            int initial_r = std::min(r, len - 1);
            for (int i = 0; i <= initial_r; ++i) {
                for (int ch = 0; ch < c; ++ch) sums[ch] += in_line[i * stride + ch];
                count++;
            }

            // Slide window across the line
            for (int i = 0; i < len; ++i) {
                for (int ch = 0; ch < c; ++ch) out_line[i * stride + ch] = sums[ch] / count;

                int edge_next = i + r + 1;
                int edge_prev = i - r;

                if (edge_next < len) {
                    for (int ch = 0; ch < c; ++ch) sums[ch] += in_line[edge_next * stride + ch];
                    count++;
                }
                if (edge_prev >= 0) {
                    for (int ch = 0; ch < c; ++ch) sums[ch] -= in_line[edge_prev * stride + ch];
                    count--;
                }
            }
        }
    };

    // horizontal blur 
    blur(src->data, tmp.data(), w, h, c, w * c);

    // vertical blur 
    blur(tmp.data(), dst->data, h, w, w * c, c);
}

