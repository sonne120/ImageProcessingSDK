#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include "ImageProcessingSDK.h"


static void makeImage(SDK_Image_t* img, int w, int h, int ch, unsigned char fill) {
    ipsdk_init_image(img);
    img->width = w;
    img->height = h;
    img->channels = ch;
    img->data = static_cast<unsigned char*>(malloc(static_cast<size_t>(w) * h * ch));
    memset(img->data, fill, static_cast<size_t>(w) * h * ch);
}


TEST(ProcessBlurTest, NullImageReturnsError) {
    EXPECT_EQ(ipsdk_process_blur(nullptr, 2, 2, 2), -1);
}
TEST(ProcessBlurTest, NullDataReturnsError) {
    SDK_Image_t img;
    ipsdk_init_image(&img);
    img.width = 10;
    img.height = 10;
    img.channels = 3;
    // img.data stays nullptr
    EXPECT_EQ(ipsdk_process_blur(&img, 2, 2, 2), -1);
}

TEST(ProcessBlurTest, BlurPreservesDimensions) {
    SDK_Image_t img;
    makeImage(&img, 16, 16, 3, 128);

    EXPECT_EQ(ipsdk_process_blur(&img, 2, 2, 2), 0);

    // Blur does not resize
    EXPECT_EQ(img.width, 16);
    EXPECT_EQ(img.height, 16);
    EXPECT_EQ(img.channels, 3);
    ASSERT_NE(img.data, nullptr);

    ipsdk_free_image(&img);
}

TEST(ProcessBlurTest, BlurOfUniformImageIsUnchanged) {
    const unsigned char fill = 200;
    SDK_Image_t img;
    makeImage(&img, 12, 12, 3, fill);

    EXPECT_EQ(ipsdk_process_blur(&img, 3, 3, 3), 0);

    ASSERT_NE(img.data, nullptr);
    const size_t bytes = static_cast<size_t>(img.width) * img.height * img.channels;
    bool allEqual = true;
    for (size_t i = 0; i < bytes; ++i) {
        if (img.data[i] != fill) { allEqual = false; break; }
    }
    EXPECT_TRUE(allEqual) << "Blurring a constant image must leave every pixel unchanged";

    ipsdk_free_image(&img);
}

TEST(ProcessBlurTest, SingleRegionSucceeds) {
    SDK_Image_t img;
    makeImage(&img, 8, 8, 3, 64);

    EXPECT_EQ(ipsdk_process_blur(&img, 1, 1, 1), 0);

    EXPECT_EQ(img.width, 8);
    EXPECT_EQ(img.height, 8);
    ASSERT_NE(img.data, nullptr);

    ipsdk_free_image(&img);
}


TEST(ProcessResizeTest, NullImageReturnsError) {
    EXPECT_EQ(ipsdk_process_resize(nullptr, 2, 2), -1);
}

TEST(ProcessResizeTest, NullDataReturnsError) {
    SDK_Image_t img;
    ipsdk_init_image(&img);
    img.width = 10;
    img.height = 10;
    img.channels = 3;
    // img.data stays nullptr
    EXPECT_EQ(ipsdk_process_resize(&img, 2, 2), -1);
}

TEST(ProcessResizeTest, ResizesToTarget) {
    SDK_Image_t img;
    makeImage(&img, 16, 16, 3, 128);

    EXPECT_EQ(ipsdk_process_resize(&img, 2, 2), 0);

    // Resize now produces the 300x300 target (channels are preserved).
    EXPECT_EQ(img.width, 300);
    EXPECT_EQ(img.height, 300);
    EXPECT_EQ(img.channels, 3);
    ASSERT_NE(img.data, nullptr);

    ipsdk_free_image(&img);
}

TEST(ProcessResizeTest, UniformImageIsUnchanged) {
    const unsigned char fill = 77;
    SDK_Image_t img;
    makeImage(&img, 12, 12, 3, fill);

    EXPECT_EQ(ipsdk_process_resize(&img, 3, 3), 0);

    ASSERT_NE(img.data, nullptr);
    const size_t bytes = static_cast<size_t>(img.width) * img.height * img.channels;
    bool allEqual = true;
    for (size_t i = 0; i < bytes; ++i) {
        if (img.data[i] != fill) { allEqual = false; break; }
    }
    EXPECT_TRUE(allEqual) << "Resizing a constant image must leave every pixel unchanged";

    ipsdk_free_image(&img);
}


TEST(ProcessCompositeTest, NullImageReturnsError) {
    EXPECT_EQ(ipsdk_process_composite(nullptr, 2, 2), -1);
}

TEST(ProcessCompositeTest, NullDataReturnsError) {
    SDK_Image_t img;
    ipsdk_init_image(&img);
    img.width = 10;
    img.height = 10;
    img.channels = 3;
    // img.data stays nullptr
    EXPECT_EQ(ipsdk_process_composite(&img, 2, 2), -1);
}

TEST(ProcessCompositeTest, ResizesToTarget) {
    SDK_Image_t img;
    makeImage(&img, 16, 16, 3, 128);

    EXPECT_EQ(ipsdk_process_composite(&img, 2, 2), 0);

    // Composite (Blur -> Resize) produces the 300x300 target.
    EXPECT_EQ(img.width, 300);
    EXPECT_EQ(img.height, 300);
    EXPECT_EQ(img.channels, 3);
    ASSERT_NE(img.data, nullptr);

    ipsdk_free_image(&img);
}

TEST(ProcessCompositeTest, UniformImageIsUnchanged) {
    const unsigned char fill = 150;
    SDK_Image_t img;
    makeImage(&img, 12, 12, 3, fill);

    EXPECT_EQ(ipsdk_process_composite(&img, 3, 3), 0);

    ASSERT_NE(img.data, nullptr);
    const size_t bytes = static_cast<size_t>(img.width) * img.height * img.channels;
    bool allEqual = true;
    for (size_t i = 0; i < bytes; ++i) {
        if (img.data[i] != fill) { allEqual = false; break; }
    }
    EXPECT_TRUE(allEqual) << "Blur+resize of a constant image must leave every pixel unchanged";

    ipsdk_free_image(&img);
}
