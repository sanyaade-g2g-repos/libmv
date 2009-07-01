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

#include <QMenuBar>
#include <QFileDialog>
#include <QtGui>

#include "libmv/base/vector.h"
#include "libmv/image/array_nd.h"
#include "libmv/image/surf.h"
#include "libmv/image/surf_descriptor.h"
#include "libmv/multiview/projection.h"
#include "libmv/multiview/fundamental.h"
#include "libmv/multiview/focal_from_fundamental.h"
#include "libmv/multiview/nviewtriangulation.h"
#include "libmv/multiview/bundle.h"
#include "libmv/logging/logging.h"
#include "ui/tvr/main_window.h"

TvrMainWindow::TvrMainWindow(QWidget *parent)
  : QMainWindow(parent) {
  
  CreateActions();
  CreateMenus();

  InvalidateTextures();

  viewers_area_ = new QMdiArea();
  setCentralWidget(viewers_area_);
  Show2DView();
  viewers_area_->currentSubWindow()->showMaximized();
  
  setWindowTitle("TVR");
}

TvrMainWindow::~TvrMainWindow() {
}

void TvrMainWindow::CreateActions() {
  open_images_action_ = new QAction(tr("&Open Images..."), this);
  open_images_action_->setShortcut(tr("Ctrl+O"));
  open_images_action_->setStatusTip(tr("Open an image pair"));
  connect(open_images_action_, SIGNAL(triggered()),
          this, SLOT(OpenImages()));

  view_2d_action_ = new QAction(tr("&2D View"), this);
  view_2d_action_->setShortcut(tr("2"));
  view_2d_action_->setStatusTip(tr("Show a 2D view of the matches"));
  connect(view_2d_action_, SIGNAL(triggered()),
          this, SLOT(Show2DView()));

  view_3d_action_ = new QAction(tr("&3D View"), this);
  view_3d_action_->setShortcut(tr("3"));
  view_3d_action_->setStatusTip(tr("Show a 3D view of the reconstruction"));
  connect(view_3d_action_, SIGNAL(triggered()),
          this, SLOT(Show3DView()));

  save_blender_action_ = new QAction(tr("&Save as Blender..."), this);
  save_blender_action_->setShortcut(tr("Ctrl+S"));
  save_blender_action_->setStatusTip(tr("Save Scene as a Blender Script"));
  connect(save_blender_action_, SIGNAL(triggered()),
          this, SLOT(SaveBlender()));
  
  compute_features_action_ = new QAction(tr("&Compute Features"), this);
  compute_features_action_->setStatusTip(tr("Compute Surf Features"));
  connect(compute_features_action_, SIGNAL(triggered()),
          this, SLOT(ComputeFeatures()));
  
  compute_candidate_matches_action_ =
      new QAction(tr("&Compute Candidate Matches"), this);
  compute_candidate_matches_action_->setStatusTip(
      tr("Compute Candidate Matches"));
  connect(compute_candidate_matches_action_, SIGNAL(triggered()),
          this, SLOT(ComputeCandidateMatches()));
  
  compute_robust_matches_action_ = new QAction(tr("&Compute Robust Matches"),
                                               this);
  compute_robust_matches_action_->setStatusTip(tr("Compute Robust Matches"));
  connect(compute_robust_matches_action_, SIGNAL(triggered()),
          this, SLOT(ComputeRobustMatches()));

  focal_from_fundamental_action_ = new QAction(tr("&Focal from Fundamental"),
                                               this);
  focal_from_fundamental_action_->setStatusTip(
      tr("Compute Focal Distance from the Fundamental MatrixRobust Matrix"));
  connect(focal_from_fundamental_action_, SIGNAL(triggered()),
          this, SLOT(FocalFromFundamental()));

  metric_reconstruction_action_ = new QAction(tr("&Metric Reconstruction"),
                                              this);
  metric_reconstruction_action_->setStatusTip(tr("Compute a metric "
      "reconstrution given the current focal length estimates"));
  connect(metric_reconstruction_action_, SIGNAL(triggered()),
          this, SLOT(MetricReconstruction()));

  metric_bundle_action_ = new QAction(tr("&Bundle Adjust"),
                                      this);
  metric_bundle_action_->setStatusTip(tr("Perform non-linear "
      "optimization of camera parameters and points."));
  connect(metric_bundle_action_, SIGNAL(triggered()),
          this, SLOT(MetricBundle()));
}

void TvrMainWindow::CreateMenus() {
  file_menu_ = menuBar()->addMenu(tr("&File"));
  file_menu_->addAction(open_images_action_);
  file_menu_->addAction(save_blender_action_);
  view_menu_ = menuBar()->addMenu(tr("&View"));
  view_menu_->addAction(view_2d_action_);
  view_menu_->addAction(view_3d_action_);
  matching_menu_ = menuBar()->addMenu(tr("&Matching"));
  matching_menu_->addAction(compute_features_action_);
  matching_menu_->addAction(compute_candidate_matches_action_);
  matching_menu_->addAction(compute_robust_matches_action_);
  calibration_menu_ = menuBar()->addMenu(tr("&Calibration"));
  calibration_menu_->addAction(focal_from_fundamental_action_);
  calibration_menu_->addAction(metric_reconstruction_action_);
  calibration_menu_->addAction(metric_bundle_action_);
}

void TvrMainWindow::OpenImages() {
  QStringList filenames = QFileDialog::getOpenFileNames(this,
      "Select Two Images", "",
      "Image Files (*.png *.jpg *.bmp *.ppm *.pgm *.xpm)");

  if (filenames.size() == 2) {
    for (int i = 0; i < 2; ++i) {
      QImage q;
      q.load(filenames[i]);
      document_.images[i] = q.rgbSwapped();
    }
    InitTextures();
    if (!viewers_area_->currentSubWindow())
      Show2DView();
    UpdateViewers();
  } else if (filenames.size() != 0) {
    QMessageBox::information(this, tr("TVR"), tr("Please select 2 images."));
    OpenImages();
  }
}

void TvrMainWindow::SaveBlender() {
  QString filename = QFileDialog::getSaveFileName(this,
      "Save as Blender Script", "", "Blender Python Script (*.py)");
  if(filename.isNull())
    return;
  document_.SaveAsBlender(filename.toAscii().data());
}

void TvrMainWindow::Show2DView() {
  MatchViewer *viewer = new MatchViewer(&context_, textures_, this);
  viewer->SetDocument(&document_);
  viewers_area_->addSubWindow(viewer);
  viewer->show();
}

void TvrMainWindow::Show3DView() {
  Viewer3D *viewer = new Viewer3D(&context_, textures_, this);
  viewer->SetDocument(&document_);
  viewers_area_->addSubWindow(viewer);
  viewer->show();
}

void TvrMainWindow::InvalidateTextures() {
  textures_[0].textureID = 0;
  textures_[1].textureID = 0;
}

void TvrMainWindow::InitTextures() {
  for (int i = 0; i < 2; ++i) {
    InitTexture(i);
  }
  emit TextureChanged();
}

void TvrMainWindow::InitTexture(int index) {
  QImage &im = document_.images[index];

  if (im.isNull()) return;

  glGenTextures(1, &textures_[index].textureID);
  
  textures_[index].width = im.width();
  textures_[index].height = im.height();

  // Select our current texture.
  glBindTexture(GL_TEXTURE_2D, textures_[index].textureID);
  // Select modulate to mix texture with color for shading.
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  // When texture area is small, bilinear filter the closest mipmap.
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST_MIPMAP_NEAREST);
  // When texture area is large, enlarge the pixels but don't upsample.
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // Wrap the texture at the edges (repeat).
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // build our texture mipmaps
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, textures_[index].width,
      textures_[index].height, GL_RGBA, GL_UNSIGNED_BYTE, im.bits());
}

void TvrMainWindow::ComputeFeatures() {
  for (int i = 0; i < 2; ++i) {
    ComputeFeatures(i);
  }
}
  
void TvrMainWindow::ComputeFeatures(int image_index) {
  QImage &qimage = document_.images[image_index];
  int width = qimage.width(), height = qimage.height();
  SurfFeatureSet &fs = document_.feature_sets[image_index];
      
  // Convert to gray-scale.
  // TODO(keir): Make a libmv image <-> QImage interop library inside libmv for
  // easy bidirectional exchange of images between Qt and libmv.
  libmv::Array3Df image(height, width, 1);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // TODO(pau): there are better ways to compute intensity.
      //            Implement one and put it on libmv/image.
      int depth = qimage.depth() / 8;
      image(y, x) = 0;
      for (int c = 0; c < depth; ++c) {
        if (c != 3) {  // Skip alpha channel for RGBA.
          image(y, x) = float(qimage.bits()[depth*(y * width + x) + c]);
        }
      }
      image(y, x) /= depth == 4 ? 3 : depth;
    }
  }
  libmv::SurfFeatures(image, 3, 4, &fs.features);

  // Build the kd-tree.
  fs.tree.Build(&fs.features[0], fs.features.size(), 64, 10);

  UpdateViewers();
}

void TvrMainWindow::ComputeCandidateMatches() {
  FindCandidateMatches(document_.feature_sets[0],
                       document_.feature_sets[1],
                       &document_.correspondences);
  UpdateViewers();
}

void TvrMainWindow::ComputeRobustMatches() {
  libmv::Correspondences new_correspondences;

  ComputeFundamental(document_.correspondences,
                     &document_.F, &new_correspondences);
  
  // TODO(pau) Make sure this is not copying too many things.  We could
  //           implement an efficient swap for the biparted graph (just swaping
  //           the maps), or remove outlier tracks from the candidate matches
  //           instead of constructing a new correspondance set.
  std::swap(document_.correspondences, new_correspondences);
  UpdateViewers();
}

static void PointCorrespondencesAsMatrices(libmv::Correspondences &c,
                                           libmv::Mat *x1,
                                           libmv::Mat *x2) {
  using namespace libmv;

  x1->resize(2, c.NumTracks());
  x2->resize(2, c.NumTracks());
  Mat *x[2] = { x1, x2 };
  int i = 0;
  for (Correspondences::TrackIterator t = c.ScanAllTracks();
       !t.Done(); t.Next()) {
    PointCorrespondences::Iterator it =
        PointCorrespondences(&c).ScanFeaturesForTrack(t.track());
    (*x[it.image()])(0, i) = it.feature()->x();
    (*x[it.image()])(1, i) = it.feature()->y();
    it.Next();
    (*x[it.image()])(0, i) = it.feature()->x();
    (*x[it.image()])(1, i) = it.feature()->y();
    i++;
  }
}

  
  
void TvrMainWindow::FocalFromFundamental() {
  libmv::Mat x1, x2;
  PointCorrespondencesAsMatrices(document_.correspondences, &x1, &x2);

  libmv::Vec2 p0((document_.images[0].width() - 1) / 2.,
                 (document_.images[0].height() - 1) / 2.);
  libmv::Vec2 p1((document_.images[1].width() - 1) / 2.,
                 (document_.images[1].height() - 1) / 2.);
  
  bool use_hartleys_method = false;
  if (use_hartleys_method) {
    libmv::FocalFromFundamental(document_.F, p0, p1,
                                &document_.focal_distance[0],
                                &document_.focal_distance[1]);
    LOG(INFO) << "focal 0: " << document_.focal_distance[0]
              << " focal 1: " << document_.focal_distance[1] << "\n";
  } else {
    double fmin_mm = 10;
    double fmax_mm = 150;
    double sensor_width_mm = 36; // Assuming a full-sized sensor 1x equiv.
    double width_pix = document_.images[0].width();
    libmv::FocalFromFundamentalExhaustive(document_.F, p0, x1, x2,
                                          fmin_mm / sensor_width_mm * width_pix,
                                          fmax_mm / sensor_width_mm * width_pix,
                                          100,
                                          &document_.focal_distance[0]);
    document_.focal_distance[1] = document_.focal_distance[0];
    
    LOG(INFO) << "focal: " << document_.focal_distance[0]
              << "pix    35mm equiv: " << document_.focal_distance[0]
                                          * sensor_width_mm / width_pix
              << "\n";
  }
  
  UpdateViewers();
}

void TvrMainWindow::MetricReconstruction() {
  using namespace libmv;

  Vec2 p0((document_.images[0].width() - 1) / 2.,
          (document_.images[0].height() - 1) / 2.);
  Vec2 p1((document_.images[1].width() - 1) / 2.,
          (document_.images[1].height() - 1) / 2.);
  double f0 = document_.focal_distance[0];
  double f1 = document_.focal_distance[1];
  Mat3 K0, K1;
  K0 << f0,  0, p0(0),
         0, f0, p0(1),
         0,  0,     1;
  K1 << f1,  0, p1(0),
         0, f1, p1(1),
         0,  0,     1;
  
  // Compute essential matrix
  Mat3 E;
  EssentialFromFundamental(document_.F, K0, K1, &E);

  // Get matches from the corresponcence structure.
  Mat x0, x1;
  PointCorrespondencesAsMatrices(document_.correspondences, &x0, &x1);
    
  // Recover R, t from E and K
  Mat3 R;
  Vec3 t;
  MotionFromEssentialAndCorrespondence(E, K0, x0.col(0), K1, x1.col(0), &R, &t);

  LOG(INFO) << "R:\n" << R << "\nt:\n" << t;

  document_.K[0] = K0;
  document_.R[0] = Mat3::Identity();
  document_.t[0] = Vec3::Zero();
  document_.K[1] = K1;
  document_.R[1] = R;
  document_.t[1] = t;

  // Triangulate features.
  vector<Mat34> Ps(2);
  P_From_KRt(document_.K[0], document_.R[0], document_.t[0], &Ps[0]);
  P_From_KRt(document_.K[1], document_.R[1], document_.t[1], &Ps[1]);
  
  int n = x0.cols();
  document_.X.resize(n);
  document_.X_colors.resize(n);
  for (int i = 0; i < n; ++i) {
    Mat2X x(2, 2);
    x.col(0) = x0.col(i);
    x.col(1) = x1.col(i);
    Vec4 X;
    NViewTriangulate(x, Ps, &X);
    document_.X[i] = libmv::HomogeneousToEuclidean(X);
    
    // Get 3D point color from first image.
    QRgb rgb = document_.images[0].pixel(int(round(x0(0,i))),
                                         int(round(x0(1,i))));
    document_.X_colors[i] << qBlue(rgb), qGreen(rgb), qRed(rgb);
    document_.X_colors[i] /= 255;
  }
}

void TvrMainWindow::MetricBundle() {
  using namespace libmv;
  
  std::vector<Mat3> K(2);
  std::vector<Mat3> R(2);
  std::vector<Vec3> t(2);
  for (int i = 0; i < 2; ++i) {
    K[i] = document_.K[i];
    R[i] = document_.R[i];
    t[i] = document_.t[i];
  }
  std::vector<Mat2X> x(2);
  Mat xs[2];
  PointCorrespondencesAsMatrices(document_.correspondences, &xs[0], &xs[1]);
  for (int i = 0; i < 2; ++i) {
    x[i] = xs[i];
  }
  
  Mat3X X(3, document_.X.size());
  for (int i = 0; i < X.cols(); ++i) {
    X.col(i) = document_.X[i];
  }
  assert(x[0].cols() == X.cols());
  assert(x[1].cols() == X.cols());
  
  EuclideanBAFull(x, &K, &R, &t, &X);                 // Run BA.
  
  for (int i = 0; i < 2; ++i) {
    document_.K[i] = K[i];
    document_.R[i] = R[i];
    document_.t[i] = t[i];
  }
  for (int i = 0; i < X.cols(); ++i) {
    document_.X[i] = X.col(i); 
  }
}

void TvrMainWindow::UpdateViewers() {
  emit GLUpdateNeeded();
}