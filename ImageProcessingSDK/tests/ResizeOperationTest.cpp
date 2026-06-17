#include <gtest/gtest.h>
#include "ResizeOperation.h"
#include "ImageProcessingSDK.h"
#include <cstdlib>

// Parametrized test 
class ResizeOperationTest : public ::testing::TestWithParam<std::pair<int, int>> {};

TEST_P(ResizeOperationTest, CreateResizeOperation) {
    auto params = GetParam();
    int targetWidth = params.first;
    int targetHeight = params.second;

    // verify 
    ResizeOperation resize(targetWidth, targetHeight);

    EXPECT_GT(targetWidth, 0);
    EXPECT_GT(targetHeight, 0);
}

INSTANTIATE_TEST_SUITE_P(
    DifferentSizes,
    ResizeOperationTest,
    ::testing::Values(
        std::make_pair(5, 5),
        std::make_pair(10, 10),
        std::make_pair(15, 15)
    )
);
