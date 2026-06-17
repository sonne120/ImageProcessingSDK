#pragma once
#ifdef __cplusplus

extern "C" {
#endif

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char* data;   
} SDK_Image_t;

void ipsdk_init_image(SDK_Image_t* img);
void ipsdk_free_image(SDK_Image_t* img);

int ipsdk_blur_direct(const SDK_Image_t* src, SDK_Image_t* dst, int radius);
int ipsdk_resize_direct(const SDK_Image_t* src, SDK_Image_t* dst, int new_w, int new_h);

int ipsdk_process_blur(SDK_Image_t* image, int radius, int rows, int cols);

int ipsdk_process_resize(SDK_Image_t* image, int rows, int cols);

int ipsdk_process_grayscale(SDK_Image_t* image, int rows, int cols);

int ipsdk_process_sharpen(SDK_Image_t* image, double amount, double sigma, int threshold, int rows, int cols);

int ipsdk_process_composite(SDK_Image_t* image, int rows, int cols);

#ifdef __cplusplus
}
#endif
