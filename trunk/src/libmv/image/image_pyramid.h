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

#ifndef LIBMV_IMAGE_IMAGE_PYRAMID_H
#define LIBMV_IMAGE_IMAGE_PYRAMID_H

#include <vector>

#include "libmv/image/image.h"

namespace libmv {

class ImagePyramid {
 public:
  ImagePyramid() {
  }

  ImagePyramid(const FloatImage &image, int num_levels, double sigma = 0.9) {
    Init(image, num_levels, sigma);
  }

  ~ImagePyramid() {
  }

  void Init(const FloatImage &image, int num_levels, double sigma = 0.9);

  const FloatImage &Level(int i) const {
    assert(0 <= i && i < NumLevels());
    return levels_[i];
  }
  const FloatImage &GradientX(int i) const {
    assert(0 <= i && i < NumLevels());
    return gradient_x_[i];
  }
  const FloatImage &GradientY(int i) const {
    assert(0 <= i && i < NumLevels());
    return gradient_y_[i];
  }

  int NumLevels() const {
    return levels_.size();
  }

  double Sigma() const {
    return sigma_;
  }

 private:
  void ComputeLevel(int l);

 private:
  double sigma_;
  std::vector<FloatImage> levels_;
  std::vector<FloatImage> gradient_x_;
  std::vector<FloatImage> gradient_y_;
};

}  // namespace libmv

#endif  // LIBMV_IMAGE_IMAGE_PYRAMID_H