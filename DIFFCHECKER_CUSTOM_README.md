# Custom OpenCV.js Build

This fork contains customizations to build OpenCV.js with additional functions that are not exposed in the standard OpenCV.js build.

## What's Customized

### 1. Custom Build Configuration
**File:** `platforms/js/diffchecker_image_align_custom_opencv_js.config.py`

A whitelist configuration that selects specific OpenCV functions for the WebAssembly build, including:
- Core operations (matrix math, bitwise ops, etc.)
- Image processing (blur, threshold, contours, morphology)
- Object detection (ArUco markers, QR codes, barcode, face detection)
- Video analysis (optical flow, background subtraction)
- DNN inference
- Feature detection (ORB, BRISK, AKAZE, etc.)
- Camera calibration (homography, PnP, `estimateAffinePartial2D`)

### 2. Custom C++ Binding for `estimateAffinePartial2D`
**File:** `modules/js/src/affine_partial2d_bindings.cpp`

> **Note:** This custom binding may not have been strictly necessary. The function `estimateAffinePartial2D` is marked `CV_EXPORTS_W` in OpenCV, which means it could potentially be exposed by simply adding `'estimateAffinePartial2D'` to the whitelist in the config file (similar to how `estimateAffine2D` is exposed in the standard `opencv_js.config.py`). However, this custom binding provides a more JavaScript-friendly API that accepts flat arrays and returns plain JS objects. If you want to simplify, you could try removing this custom binding and just adding the function to the whitelist instead.

A custom Emscripten binding that exposes `cv::estimateAffinePartial2D` to JavaScript with a convenient API:

```javascript
const result = cv.estimateAffinePartial2D_js(srcPoints, dstPoints, method, threshold, maxIters, confidence, refineIters);
// result.matrix - Float64Array of 6 values [a, b, tx, c, d, ty]
// result.inliers - Array of 0/1 indicating inlier status
```

**Parameters:**
- `srcPoints` / `dstPoints`: Flat arrays of 2D points `[x0, y0, x1, y1, ...]`
- `method`: 0 = LMEDS, 1 = RANSAC (default)
- `ransacReprojThreshold`: RANSAC threshold (default: 3.0)
- `maxIters`: Maximum iterations (default: 2000)
- `confidence`: Confidence level (default: 0.99)
- `refineIters`: Refinement iterations (default: 10)

### 3. CMakeLists.txt Modification
**File:** `modules/js/CMakeLists.txt`

Added the custom binding source and linked against `opencv_calib3d`.

---

## Building OpenCV.js

### Prerequisites

- Ubuntu 20.04 (tested via Parallels on macOS)
- Emscripten SDK installed and configured

### Build Command

```bash
source /path/to/emsdk/emsdk_env.sh

emcmake python ./platforms/js/build_js.py build_js/ \
  --config platforms/js/diffchecker_image_align_custom_opencv_js.config.py \
  --disable_single_file \
  --build_flags "-s ASSERTIONS=0 -s SAFE_HEAP=0 -s ENVIRONMENT=web" \
  --cmake_option="-DBUILD_LIST=core,imgproc,features2d,calib3d,objdetect,video,dnn,photo,js" \
  --cmake_option="-DCMAKE_BUILD_TYPE=MinSizeRel" \
  --cmake_option="-DBUILD_TESTS=OFF" \
  --cmake_option="-DBUILD_PERF_TESTS=OFF" \
  --clean_build_dir
```

### Build Flags Explained

| Flag | Description |
|------|-------------|
| `--config` | Path to the whitelist config file that specifies which OpenCV functions to expose |
| `--disable_single_file` | Output separate `.js` and `.wasm` files instead of embedding wasm in js |
| `--build_flags` | Emscripten compiler flags passed to em++ |
| `-s ASSERTIONS=0` | Disable runtime assertions (smaller/faster output) |
| `-s SAFE_HEAP=0` | Disable heap safety checks (faster output) |
| `-s ENVIRONMENT=web` | Target web browser environment only |
| `--cmake_option` | Pass options directly to CMake |
| `-DBUILD_LIST=...` | Restrict which OpenCV modules are built (must include `js`) |
| `-DCMAKE_BUILD_TYPE=MinSizeRel` | Optimize for minimal binary size |
| `-DBUILD_TESTS=OFF` | Skip building OpenCV test binaries |
| `-DBUILD_PERF_TESTS=OFF` | Skip building performance test binaries |
| `--clean_build_dir` | Remove previous build artifacts before building |

For more details:
- `build_js.py` flags: `python ./platforms/js/build_js.py --help`
- Emscripten `-s` flags: [Emscripten settings reference](https://emscripten.org/docs/tools_reference/settings_reference.html)
- CMake `-D` options: [CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html)

### Output

After a successful build, the output files will be in `build_js/bin/`:
- `opencv.js` - The main JavaScript file
- `opencv.wasm` - The WebAssembly binary

---

## Alternative Build Methods

For other build configurations (Docker, different platforms, etc.), see the official OpenCV.js documentation:
https://docs.opencv.org/3.4/d4/da1/tutorial_js_setup.html
