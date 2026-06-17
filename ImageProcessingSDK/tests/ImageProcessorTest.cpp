#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <QImage>
#include <QColor>
#include "IOperation.h"
#include "ImageProcessingSDK.h"
#include "ClientApp/src/QImageAdapter.h"

using ::testing::_;
using ::testing::Invoke;

// Mock IOperation interface
class MockOperation : public IOperation {
public:
    MOCK_METHOD(void, apply, (const SDK_Image_t* src, SDK_Image_t* dst), (const, override));
};

TEST(GMockTest, MockIOperation) {
    MockOperation mockOp;

    // Create test images
    SDK_Image_t src;
    ipsdk_init_image(&src);
    src.width = 10;
    src.height = 10;
    src.channels = 3;
    src.data = (uint8_t*)malloc(10 * 10 * 3);

    SDK_Image_t dst;
    ipsdk_init_image(&dst);

    // apply
    EXPECT_CALL(mockOp, apply(&src, &dst))
        .Times(1)
        .WillOnce(Invoke([](const SDK_Image_t* s, SDK_Image_t* d) {
            d->width = s->width;
            d->height = s->height;
            d->channels = s->channels;
        }));

    // Call the mock
    mockOp.apply(&src, &dst);

    // Verify the mock
    EXPECT_EQ(dst.width, 10);
    EXPECT_EQ(dst.height, 10);

    // Cleanup
    ipsdk_free_image(&src);
    ipsdk_free_image(&dst);
}


TEST(PictureTest, LoadAndConvert) {
    const QString path = "/tmp/ipsdk_test_image.png";

   
    QImage original(4, 3, QImage::Format_RGBA8888);
    original.fill(QColor(10, 20, 30, 255));
    original.setPixelColor(0, 0, QColor(200, 100, 50, 255));
    ASSERT_TRUE(original.save(path, "PNG"));

    // Load it back the same way the app does.
    QImage loaded;
    ASSERT_TRUE(loaded.load(path));
    loaded = loaded.convertToFormat(QImage::Format_RGBA8888);

    EXPECT_EQ(loaded.width(), 4);
    EXPECT_EQ(loaded.height(), 3);

   
    SDK_Image_t sdk = QtAdapter::qImageToSDK(loaded);
    EXPECT_EQ(sdk.width, 4);
    EXPECT_EQ(sdk.height, 3);
    EXPECT_EQ(sdk.channels, 4);
    ASSERT_NE(sdk.data, nullptr);

    EXPECT_EQ(sdk.data[0], 200);
    EXPECT_EQ(sdk.data[1], 100);
    EXPECT_EQ(sdk.data[2], 50);
    EXPECT_EQ(sdk.data[3], 255);


    std::free(sdk.data);
    sdk.data = nullptr;
    ipsdk_free_image(&sdk);

    std::remove(path.toStdString().c_str());
}
