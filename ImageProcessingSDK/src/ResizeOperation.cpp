#include "ResizeOperation.h"
#include <cstdlib>

ResizeOperation::ResizeOperation(int w, int h) : new_w(w), new_h(h) {}

void ResizeOperation::apply(const SDK_Image_t* src, SDK_Image_t* dst) const {
    dst->width = new_w;
    dst->height = new_h;
    dst->channels = src->channels;

    int size = dst->width * dst->height * dst->channels;
    if (dst->data) std::free(dst->data);
    dst->data = (unsigned char*)std::malloc(size);
    if (!dst->data) return;

    for (int y = 0; y < new_h; ++y) {
        for (int x = 0; x < new_w; ++x) {
            int src_x = (int)(x * ((float)src->width / new_w));
            int src_y = (int)(y * ((float)src->height / new_h));

            if (src_x >= src->width) src_x = src->width - 1;
            if (src_y >= src->height) src_y = src->height - 1;

            for (int c = 0; c < src->channels; ++c) {
                int src_idx = (src_y * src->width + src_x) * src->channels + c;
                int dst_idx = (y * dst->width + x) * dst->channels + c;
                dst->data[dst_idx] = src->data[src_idx];
            }
        }
    }
}
