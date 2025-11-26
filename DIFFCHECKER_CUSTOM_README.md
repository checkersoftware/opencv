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
- Docker installed and running

### Build Command

```bash
docker run --rm --platform linux/amd64 \
  -e CMAKE_BUILD_PARALLEL_LEVEL=$(getconf _NPROCESSORS_ONLN) \
  -v "$(pwd)":/src -u $(id -u):$(id -g) emscripten/emsdk:2.0.10 \
  emcmake python3 ./platforms/js/build_js.py build_js \
    --config platforms/js/diffchecker_image_align_custom_opencv_js.config.py \
    --disable_single_file \
    --build_flags "-s ASSERTIONS=0 -s SAFE_HEAP=0 -s ENVIRONMENT=web" \
    --cmake_option="-DBUILD_LIST=core,imgproc,features2d,calib3d,objdetect,video,dnn,photo" \
    --cmake_option="-DCMAKE_BUILD_TYPE=MinSizeRel" \
    --cmake_option="-DBUILD_TESTS=OFF" \
    --cmake_option="-DBUILD_PERF_TESTS=OFF" \
    --clean_build_dir
```

### Output

After a successful build, the output files will be in `build_js/bin/`:
- `opencv.js` - The main JavaScript file
- `opencv.wasm` - The WebAssembly binary

---

## Adding Your Own Custom Bindings

If you need to expose additional OpenCV functions to JavaScript:

### Step 1: Create a binding file

Create a new `.cpp` file in `modules/js/src/` with your Emscripten bindings:

```cpp
#include <opencv2/your_module.hpp>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace cv;

val yourFunction_js(/* params */) {
    // Convert JS types to C++ types
    // Call OpenCV function
    // Return results as val objects
}

EMSCRIPTEN_BINDINGS(your_bindings) {
    function("yourFunction_js", &yourFunction_js);
}
```

### Step 2: Register in CMakeLists.txt

Add to `modules/js/CMakeLists.txt`:

```cmake
target_sources(${the_module} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/your_bindings.cpp
)
target_link_libraries(${the_module} PRIVATE opencv_your_module)
```

### Step 3: Update the config (optional)

If you also want the auto-generated bindings for certain functions, add them to your config file's whitelist.

### Step 4: Rebuild

Run the Docker build command above.

---

## Why Custom Bindings?

The standard OpenCV.js build auto-generates JavaScript bindings, but:
- Not all functions are exposed
- Some functions have complex signatures that don't translate well
- You may want a simpler JavaScript-friendly API

Custom Emscripten bindings let you:
- Expose any OpenCV function
- Design a cleaner API for JavaScript consumers
- Handle type conversions explicitly
