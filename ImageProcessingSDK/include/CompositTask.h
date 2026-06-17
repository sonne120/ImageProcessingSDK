#include "BlurOperation.h"
#include "ResizeOperation.h"
#include <cstring>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <atomic>
#include <memory>
#include "ImageProcessor.h"

class CompositeTask {
public:
    ImageRegion region;
    std::vector<std::shared_ptr<IOperation>> pipeline;
    std::shared_ptr<std::atomic<size_t>> globalCounter;
    ImageRegion* output = nullptr;

    void execute() {
        if (pipeline.empty()) {
            if (globalCounter) globalCounter->fetch_sub(1, std::memory_order_acq_rel);
            return;
        }

        SDK_Image_t current_buffer;
        ipsdk_init_image(&current_buffer);
        current_buffer.width = region.width;
        current_buffer.height = region.height;
        current_buffer.channels = region.channels;
        int size = region.width * region.height * region.channels;
        current_buffer.data = (unsigned char*)std::malloc(size);
        std::memcpy(current_buffer.data, region.pixelData.data(), size);

        SDK_Image_t next_buffer;
        ipsdk_init_image(&next_buffer);

        for (auto& op : pipeline) {
            op->apply(&current_buffer, &next_buffer);
            
            SDK_Image_t temp = current_buffer;
            current_buffer = next_buffer;
            next_buffer = temp;
        }

        if (current_buffer.data && current_buffer.width > 0 && current_buffer.height > 0) {
            int out_size = current_buffer.width * current_buffer.height * current_buffer.channels;
            ImageRegion* dst = output ? output : &region;
            dst->pixelData.assign(current_buffer.data, current_buffer.data + out_size);
            dst->width = current_buffer.width;
            dst->height = current_buffer.height;
            dst->channels = current_buffer.channels;
        }

        ipsdk_free_image(&current_buffer);
        ipsdk_free_image(&next_buffer);

        if (globalCounter) {
            globalCounter->fetch_sub(1, std::memory_order_acq_rel);
        }
    }
};