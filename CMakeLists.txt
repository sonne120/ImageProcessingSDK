cmake_minimum_required(VERSION 3.16)
project(ImageProcessingSDK VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory" FORCE)
endif()

enable_testing()

add_subdirectory(ImageProcessingSDK)
add_subdirectory(ClientApp)

