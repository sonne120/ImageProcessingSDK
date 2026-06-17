cmake_minimum_required(VERSION 3.16)
project(ImageProcessingSDK VERSION 1.0.0 LANGUAGES CXX)  

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

add_library(ImageProcessingSDK STATIC
    src/ImageProcessingSDK.cpp
    src/BlurOperation.cpp
    src/ResizeOperation.cpp
    src/GrayscaleOperation.cpp
    src/SharpenOperation.cpp
    src/ImageProcessor.cpp
    include/ImageProcessingSDK.h
    include/ImageProcessor.h
    include/SpscRingBuffer.h
)

target_link_libraries(ImageProcessingSDK PUBLIC Qt6::Core Qt6::Gui)

target_include_directories(ImageProcessingSDK PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS ImageProcessingSDK
    EXPORT ImageProcessingSDKTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT ImageProcessingSDKTargets
    FILE ImageProcessingSDKTargets.cmake
    NAMESPACE ImageProcessingSDK::
    DESTINATION lib/cmake/ImageProcessingSDK
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ImageProcessingSDKConfig.cmake
    INSTALL_DESTINATION lib/cmake/ImageProcessingSDK
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ImageProcessingSDKConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ImageProcessingSDKConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ImageProcessingSDKConfigVersion.cmake
    DESTINATION lib/cmake/ImageProcessingSDK
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
