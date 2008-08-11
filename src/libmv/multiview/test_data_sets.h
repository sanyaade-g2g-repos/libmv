// Copyright (c) 2007, 2008 libmv authors.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef LIBMV_MULTIVIEW_TEST_DATA_SETS_H_
#define LIBMV_MULTIVIEW_TEST_DATA_SETS_H_

#include <vector>

#include "libmv/numeric/numeric.h"

namespace libmv {

struct TwoViewDataSet {
  Mat3 K1, K2;  // Internal parameters.
  Mat3 R1, R2;  // Rotation.
  Vec3 t1, t2;  // Translation.
  Mat34 P1, P2; // Projection matrix, P = K(R|t)
  Mat3 F;       // Fundamental matrix.
  Mat X;        // 3D points.
  Mat x1, x2;   // Projected points.
};

// Two cameras at (-1,-1,-10) and (2,1,-10) looking approximately towards z+.
TwoViewDataSet TwoRealisticCameras();

} // namespace libmv

#endif  // LIBMV_MULTIVIEW_TEST_DATA_SETS_H_
