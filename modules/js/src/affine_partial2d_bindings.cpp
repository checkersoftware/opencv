#include <vector>
#include <opencv2/calib3d.hpp>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace cv;

static std::vector<Point2f> toPts(const val& flat) {
  const size_t n = flat["length"].as<size_t>() / 2;
  std::vector<Point2f> out(n);
  for (size_t i = 0; i < n; ++i) {
    out[i].x = flat[2*i].as<float>();
    out[i].y = flat[2*i + 1].as<float>();
  }
  return out;
}

val estimateAffinePartial2D_js(const val& srcFlat,
                               const val& dstFlat,
                               int method /* 0=LMEDS, 1=RANSAC */ = 1,
                               double ransacReprojThreshold = 3.0,
                               int maxIters = 2000,
                               double confidence = 0.99,
                               int refineIters = 10)
{
  std::vector<Point2f> src = toPts(srcFlat);
  std::vector<Point2f> dst = toPts(dstFlat);

  Mat inliers;
  Mat A = estimateAffinePartial2D(src, dst, inliers,
                                  method == 1 ? RANSAC : LMEDS,
                                  ransacReprojThreshold,
                                  maxIters, confidence, refineIters);

  val out = val::object();
  // matrix: flat array length 6 [a,b,tx,c,d,ty]
  val m = val::array();
  if (!A.empty()) {
    for (int r = 0; r < 2; ++r)
      for (int c = 0; c < 3; ++c)
        m.call<void>("push", (A.depth()==CV_64F) ? A.at<double>(r,c)
                                                 : (double)A.at<float>(r,c));
  }
  out.set("matrix", m);

  // inliers: array of 0/1
  val inl = val::array();
  if (!inliers.empty()) {
    for (int i = 0; i < inliers.rows; ++i)
      inl.call<void>("push", (int)inliers.at<uchar>(i,0));
  }
  out.set("inliers", inl);

  return out;
}

EMSCRIPTEN_BINDINGS(affine_partial2d_bindings) {
  function("estimateAffinePartial2D_js", &estimateAffinePartial2D_js);
}
