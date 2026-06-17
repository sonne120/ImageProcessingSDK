enable_testing()

find_package(GTest REQUIRED)

add_executable(ImageProcessingTests
    test_main.cpp
    ResizeOperationTest.cpp
    ImageProcessorTest.cpp
    ImageProcessingSDKTest.cpp
)

target_include_directories(ImageProcessingTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(ImageProcessingTests
    ImageProcessingSDK
    GTest::gtest
    GTest::gmock
)

add_test(NAME ImageProcessingTests COMMAND ImageProcessingTests)

install(TARGETS ImageProcessingTests
    RUNTIME DESTINATION bin/tests
)
