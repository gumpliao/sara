// ========================================================================== //
// This file is part of Sara, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2013-2016 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

//! @file

#ifndef DO_SARA_IMAGEPROCESSING_GAUSSIANPYRAMID_HPP
#define DO_SARA_IMAGEPROCESSING_GAUSSIANPYRAMID_HPP


#include <DO/Sara/ImageProcessing/ImagePyramid.hpp>
#include <DO/Sara/ImageProcessing/Differential.hpp>
#include <DO/Sara/ImageProcessing/LinearFiltering.hpp>
#include <DO/Sara/ImageProcessing/Scaling.hpp>


namespace DO { namespace Sara {

  /*!
    @ingroup ScaleSpace
    @{
   */

  //! Computes a pyramid of Gaussians.
  template <typename T>
  ImagePyramid<T>
  gaussian_pyramid(const ImageView<T>& image,
                   const ImagePyramidParams& params = ImagePyramidParams())
  {
    using Scalar = typename ImagePyramid<T>::scalar_type;

    // Resize the image with the appropriate factor.
    auto resize_factor = pow(2.f, -params.first_octave_index());
    auto I = enlarge(image, resize_factor);

    // Deduce the new camera sigma with respect to the dilated image.
    auto camera_sigma = Scalar(params.scale_camera())*resize_factor;

    // Blur the image so that its new sigma is equal to the initial sigma.
    auto init_sigma = Scalar(params.scale_initial());
    if (camera_sigma < init_sigma)
    {
      Scalar sigma = sqrt(init_sigma*init_sigma - camera_sigma*camera_sigma);
      I = gaussian(I, sigma);
    }

    // Deduce the maximum number of octaves.
    auto l = std::min(image.width(), image.height());
    auto b = params.image_padding_size();
    // l/2^k > 2b
    // 2^k < l/(2b)
    // k < log(l/(2b))/log(2)
    auto num_octaves = static_cast<int>(log(l/(2.f*b))/log(2.f));

    // Shorten names.
    auto k = Scalar(params.scale_geometric_factor());
    auto num_scales = params.num_scales_per_octave();
    auto downscale_index = int( floor( log(Scalar(2))/log(k)) );

    // Create the image pyramid
    auto G = ImagePyramid<T>{};
    G.reset(num_octaves, num_scales, init_sigma, k);

    for (auto o = 0; o < num_octaves; ++o)
    {
      // Compute the octave scaling factor
      G.octave_scaling_factor(o) =
        (o == 0) ? 1.f/resize_factor : G.octave_scaling_factor(o-1)*2;

      // Compute the gaussians in octave \f$o\f$
      Scalar sigma_s_1 = init_sigma;
      G(0, o) = o == 0 ? I : downscale(G(downscale_index, o - 1), 2);

      for (auto s = 1; s < num_scales; ++s)
      {
        auto sigma = sqrt(k*k*sigma_s_1*sigma_s_1 - sigma_s_1*sigma_s_1);
        G(s,o) = gaussian(G(s-1,o), sigma);
        sigma_s_1 *= k;
      }
    }

    // Done!
    return G;
  }

  //! Computes a pyramid of difference of Gaussians from the Gaussian pyramid.
  template <typename T>
  ImagePyramid<T> difference_of_gaussians_pyramid(const ImagePyramid<T>& gaussians)
  {
    auto D = ImagePyramid<T>{};
    D.reset(gaussians.num_octaves(),
            gaussians.num_scales_per_octave()-1,
            gaussians.scale_initial(),
            gaussians.scale_geometric_factor());

    for (auto o = 0; o < D.num_octaves(); ++o)
    {
      D.octave_scaling_factor(o) = gaussians.octave_scaling_factor(o);
      for (auto s = 0; s < D.num_scales_per_octave(); ++s)
      {
        D(s,o).resize(gaussians(s,o).sizes());
        D(s,o).array() = gaussians(s+1,o).array() - gaussians(s,o).array();
      }
    }
    return D;
  }

  //! Computes a pyramid of scale-normalized Laplacians of Gaussians from the
  //! Gaussian pyramid.
  template <typename T>
  ImagePyramid<T> laplacian_pyramid(const ImagePyramid<T>& gaussians)
  {
    auto LoG = ImagePyramid<T>{};
    LoG.reset(gaussians.num_octaves(),
              gaussians.num_scales_per_octave(),
              gaussians.scale_initial(),
              gaussians.scale_geometric_factor());

    for (auto o = 0; o < LoG.num_octaves(); ++o)
    {
      LoG.octave_scaling_factor(o) = gaussians.octave_scaling_factor(o);
      for (auto s = 0; s < LoG.num_scales_per_octave(); ++s)
      {
        LoG(s,o) = laplacian(gaussians(s,o));
        for (auto it = LoG(s, o).begin(); it != LoG(s,o).end(); ++it)
          *it *= pow(gaussians.scale_relative_to_octave(s), 2);
      }
    }
    return LoG;
  }

  //! Computes the gradient vector of \f$I(x,y,\sigma)\f$ at \f$(x,y,\sigma)\f$,
  //! where \f$\sigma = 2^{s/S + o}\f$ where \f$S\f$ is the number of scales per
  //! octave.
  template <typename T>
  Matrix<T, 3, 1> gradient(const ImagePyramid<T>& I, int x, int y, int s, int o)
  {
    // Sanity check
    if (x < 1 || x >= I(s,o).width()-1  ||
        y < 1 || y >= I(s,o).height()-1 ||
        s < 1 || s >= static_cast<int>(I(o).size())-1)
    {
      std::ostringstream msg;
      msg << "Fatal error: gradient out of range!" << std::endl;
      std::cerr << msg.str() << std::endl;
      throw std::out_of_range(msg.str());
    }

    Matrix<T, 3, 1> d;
    d(0) = (I(x+1,y  ,s  ,o) - I(x-1,y  ,s  ,o)) / T(2);
    d(1) = (I(x  ,y+1,s  ,o) - I(x  ,y-1,s  ,o)) / T(2);
    d(2) = (I(x  ,y  ,s+1,o) - I(x  ,y  ,s-1,o)) / T(2);
    return d;
  }

  //! Computes the hessian matrix of \f$I(x,y,\sigma)\f$ at \f$(x,y,\sigma)\f$,
  //! where \f$\sigma = 2^{s/S + o}\f$ where \f$S\f$ is the number of scales
  //! per octave.
  template <typename T>
  Matrix<T, 3, 3> hessian(const ImagePyramid<T>& I, int x, int y, int s, int o)
  {
    // Sanity check
    if (x < 1 || x >= I(s,o).width()-1  ||
        y < 1 || y >= I(s,o).height()-1 ||
        s < 1 || s >= static_cast<int>(I(o).size())-1)
    {
      std::string msg("Fatal error: Hessian out of range!");
      std::cerr << msg << std::endl;
      throw std::out_of_range(msg);
    }

    Matrix<T, 3, 3> H;
    // Ixx
    H(0,0) = I(x+1,y,s,o) - T(2)*I(x,y,s,o) + I(x-1,y,s,o);
    // Iyy
    H(1,1) = I(x,y+1,s,o) - T(2)*I(x,y,s,o) + I(x,y-1,s,o);
    // Iss
    H(2,2) = I(x,y,s+1,o) - T(2)*I(x,y,s,o) + I(x,y,s-1,o);
    // Ixy = Iyx
    H(0,1) = H(1,0) = ( I(x+1,y+1,s,o) - I(x-1,y+1,s,o)
                      - I(x+1,y-1,s,o) + I(x-1,y-1,s,o) ) / T(4);
    // Ixs = Isx
    H(0,2) = H(2,0) = ( I(x+1,y,s+1,o) - I(x-1,y,s+1,o)
                      - I(x+1,y,s-1,o) + I(x-1,y,s-1,o) ) / T(4);
    // Iys = Isy
    H(1,2) = H(2,1) = ( I(x,y+1,s+1,o) - I(x,y-1,s+1,o)
                      - I(x,y+1,s-1,o) + I(x,y-1,s-1,o) ) / T(4);
    // Done!
    return H;
  }

  //! @}

} /* namespace Sara */
} /* namespace DO */


#endif /* DO_SARA_IMAGEPROCESSING_GAUSSIANPYRAMID_HPP */
