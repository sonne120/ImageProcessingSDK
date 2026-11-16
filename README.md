[![Downloads](https://img.shields.io/github/downloads/sonne120/ImageProcessingSDK/total)](https://github.com/sonne120/ImageProcessingSDK/releases)
[![Latest](https://img.shields.io/github/downloads/sonne120/ImageProcessingSDK/latest/total)](https://github.com/sonne120/ImageProcessingSDK/releases/latest)

![Language](https://img.shields.io/github/languages/top/sonne120/ImageProcessingSDK)
![Stars](https://img.shields.io/github/stars/sonne120/ImageProcessingSDK)
![Last commit](https://img.shields.io/github/last-commit/sonne120/ImageProcessingSDK)


# image-processing-sdk

A small C++ image-processing library (with a C API) and a Qt GUI client.
It splits an image into a grid of tiles and processes them in parallel on a
background worker pool, then reassembles the result — so filters scale across
CPU cores.

## Filters

- **Blur** — box blur, configurable radius.
- **Resize** — nearest-neighbour scaling (to a 300×300 target in the app).
- **Grayscale** — Rec. 601 luminance; preserves alpha.
- **Sharpen** — unsharp mask with *amount / radius (sigma) / threshold* controls;
  uses a fast 3-pass box-blur Gaussian, so cost is independent of radius.
- **Composite** — blur → resize in one pass.

## Architecture

- **`SDK_Image_t`** — the image: `width`, `height`, `channels`, raw `data` buffer.
- **`IOperation`** — operation interface (`apply(src, dst)`); implemented by
  `BlurOperation`, `ResizeOperation`, `GrayscaleOperation`, `SharpenOperation`.
- **`ImageProcessor`** — owns the worker pool; splits an image into `rows × cols`
  tiles, dispatches a `CompositeTask` per tile, and reassembles the output.
- **`SpscRingBuffer`** — lock-free queue handing tasks to each worker (one per core).
- **C API (`ImageProcessingSDK.h`)** — thin `extern "C"` `ipsdk_*` wrappers.

**Flow:** detect cores → split image into tiles → round-robin tasks onto
per-worker queues (`task i → queue i % workers`) → each worker applies its
pipeline → an atomic counter signals completion → main thread reassembles.

> Note: Sharpen runs whole-image (1×1) by default — its wide Gaussian would
> otherwise leave seams between independently-blurred tiles.

## Build & Run

```bash
cmake -S . -B build
cmake --build build -j8

./build/ClientApp/ClientApp.app/Contents/MacOS/ClientApp        # GUI
./build/ImageProcessingSDK/tests/ImageProcessingTests           # tests
```

Add `-DCMAKE_INSTALL_PREFIX=/path` + `cmake --install build` for a relocatable
install (defaults to `build/install/`).

## C API

```c
void ipsdk_init_image(SDK_Image_t* img);
void ipsdk_free_image(SDK_Image_t* img);

// Direct (single-threaded, allocates dst — free it later):
int ipsdk_blur_direct(const SDK_Image_t* src, SDK_Image_t* dst, int radius);
int ipsdk_resize_direct(const SDK_Image_t* src, SDK_Image_t* dst, int new_w, int new_h);

// Parallel (multi-threaded, in-place; rows × cols tiles):
int ipsdk_process_blur(SDK_Image_t* image, int radius, int rows, int cols);
int ipsdk_process_resize(SDK_Image_t* image, int rows, int cols);          // → 300×300
int ipsdk_process_grayscale(SDK_Image_t* image, int rows, int cols);
int ipsdk_process_sharpen(SDK_Image_t* image, double amount, double sigma, int threshold, int rows, int cols);
int ipsdk_process_composite(SDK_Image_t* image, int rows, int cols);       // blur(20) → resize
```

### Example

```c
#include "ImageProcessingSDK.h"

SDK_Image_t image;
ipsdk_init_image(&image);
image.width = 1920; image.height = 1080; image.channels = 4;
image.data = malloc(1920 * 1080 * 4);

ipsdk_process_blur(&image, 20, 4, 8);                 // 4×8 = 32 tiles
ipsdk_process_sharpen(&image, 2.2, 6.0, 12, 1, 1);    // whole-image sharpen

ipsdk_free_image(&image);
```
