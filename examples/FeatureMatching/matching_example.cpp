// ========================================================================== //
// This file is part of DO-CV, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2013 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

#include <DO/Sara/FeatureDetectors.hpp>
#include <DO/Sara/FeatureDescriptors.hpp>
#include <DO/Sara/FeatureMatching.hpp>
#include <DO/Sara/Graphics.hpp>
#include <DO/Sara/ImageProcessing.hpp>

using namespace std;
using namespace DO::Sara;

//#define MSER_KEYS
//#define HARRIS_KEYS

string file1 = src_path("../../datasets/All.tif");
string file2 = src_path("../../datasets/GuardOnBlonde.tif");

Set<OERegion, RealDescriptor> compute_sift_keypoints(const Image<float>& image)
{
  // Time everything.
  Timer timer;
  double elapsed = 0.;
  double dog_detection_time;
  double ori_assign_time;
  double sift_description_time;
  double grad_gaussian_time;

  // We describe the work flow of the feature detection and description.
  Set<OERegion, RealDescriptor> keys;
  auto& DoGs = keys.features;
  auto& SIFTDescriptors = keys.descriptors;

  // 1. Feature extraction.
  print_stage("Computing DoG extrema");
  timer.restart();
  ImagePyramidParams pyr_params;//(0);
  ComputeDoGExtrema compute_DoGs(pyr_params, 0.01f);
  auto scale_octave_pairs = vector<Point2i>{};
  DoGs = compute_DoGs(image, &scale_octave_pairs);
  dog_detection_time = timer.elapsed_ms();
  elapsed += dog_detection_time;
  cout << "DoG detection time = " << dog_detection_time << " ms" << endl;
  cout << "DoGs.size() = " << DoGs.size() << endl;

  // 2. Feature orientation.
  // Prepare the computation of gradients on gaussians.
  print_stage("Computing gradients of Gaussians");
  timer.restart();
  auto nabla_G = gradient_polar_coordinates(compute_DoGs.gaussians());
  grad_gaussian_time = timer.elapsed_ms();
  elapsed += grad_gaussian_time;
  cout << "gradient of Gaussian computation time = " << grad_gaussian_time << " ms" << endl;
  cout << "DoGs.size() = " << DoGs.size() << endl;


  // Find dominant gradient orientations.
  print_stage("Assigning (possibly multiple) dominant orientations to DoG extrema");
  timer.restart();
  ComputeDominantOrientations assign_dominant_orientations;
  assign_dominant_orientations(nabla_G, DoGs, scale_octave_pairs);
  ori_assign_time = timer.elapsed_ms();
  elapsed += ori_assign_time;
  cout << "orientation assignment time = " << ori_assign_time << " ms" << endl;
  cout << "DoGs.size() = " << DoGs.size() << endl;


  // 3. Feature description.
  print_stage("Describe DoG extrema with SIFT descriptors");
  timer.restart();
  ComputeSIFTDescriptor<> compute_sift;
  SIFTDescriptors = compute_sift(DoGs, scale_octave_pairs, nabla_G);
  sift_description_time = timer.elapsed_ms();
  elapsed += sift_description_time;
  cout << "description time = " << sift_description_time << " ms" << endl;
  cout << "sifts.size() = " << SIFTDescriptors.size() << endl;

  // Summary in terms of computation time.
  print_stage("Total Detection/Description time");
  cout << "SIFT computation time = " << elapsed << " ms" << endl;

  // 4. Rescale  the feature position and scale $(x,y,\sigma)$ with the octave
  //    scale.
  for (size_t i = 0; i != DoGs.size(); ++i)
  {
    auto octave_scale_factor = nabla_G.octave_scaling_factor(scale_octave_pairs[i](1));
    DoGs[i].center() *= octave_scale_factor;
    DoGs[i].shape_matrix() /= pow(octave_scale_factor, 2);
  }

  return keys;
}

void load(Image<Rgb8>& image1, Image<Rgb8>& image2,
  Set<OERegion, RealDescriptor>& keys1,
  Set<OERegion, RealDescriptor>& keys2,
  vector<Match>& matches)
{
  cout << "Loading images" << endl;
  if (!load(image1, file1))
  {
    cerr << "Error: cannot load image file 1: " << file1 << endl;
    return;
  }
  if (!load(image2, file2))
  {
    cerr << "Error: cannot load image file 2: " << file2 << endl;
    return;
  }

  cout << "Computing/Reading keypoints" << endl;
  Set<OERegion, RealDescriptor> SIFTs1 = compute_sift_keypoints(image1.convert<float>());
  Set<OERegion, RealDescriptor> SIFTs2 = compute_sift_keypoints(image2.convert<float>());
  keys1.append(SIFTs1);
  keys2.append(SIFTs2);
  cout << "Image 1: " << keys1.size() << " keypoints" << endl;
  cout << "Image 2: " << keys2.size() << " keypoints" << endl;

  CHECK(keys1.features.size());
  CHECK(keys1.descriptors.size());
  CHECK(keys2.features.size());
  CHECK(keys2.descriptors.size());

  // Compute/read matches
  cout << "Computing Matches" << endl;
  AnnMatcher matcher(keys1, keys2, 1.0f);
  matches = matcher.compute_matches();
  cout << matches.size() << " matches" << endl;
}


GRAPHICS_MAIN()
{
  // Load images.
  Image<Rgb8> image1, image2;
  Set<OERegion, RealDescriptor> keys1, keys2;
  vector<Match> matches;
  load(image1, image2, keys1, keys2, matches);

  float scale = 1.0f;
  int w = int((image1.width() + image2.width())*scale);
  int h = max(image1.height(), image2.height());
  Point2f off(float(image1.width()), 0.f);

  create_window(w, h);
  set_antialiasing();
  //checkMatches(image1, image2, matches, true, scale);

  for (int i = 0; i < matches.size(); ++i)
  {
    draw_image_pair(image1, image2, off, scale);
    draw_match(matches[i], Red8, off, scale);
    cout << matches[i] << endl;
    get_key();
  }

  return 0;
}