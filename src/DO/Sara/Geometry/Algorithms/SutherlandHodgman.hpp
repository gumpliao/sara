// ========================================================================== //
// This file is part of Sara, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2014-2016 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

#ifndef DO_SARA_GEOMETRY_ALGORITHMS_SUTHERLANDHODGMAN_HPP
#define DO_SARA_GEOMETRY_ALGORITHMS_SUTHERLANDHODGMAN_HPP

#include <DO/Sara/Defines.hpp>

#include <DO/Sara/Core/EigenExtension.hpp>

#include <DO/Sara/Geometry/Tools/Utilities.hpp>
#include <DO/Sara/Geometry/Objects/Polygon.hpp>


namespace DO { namespace Sara {

  /*!
    Intersection test between lines.
    'u' is the intersection point if it exists.
   */
  bool intersection(const P2::Line& line1, const P2::Line& line2,
                    Vector2d& u);

  /*!
   Simple implementation of Sutherland-Hodgman algorithm.
   - Polygon 'subject' must be a simple polygon, i.e., without holes.
   - Polygon 'clip' must be a convex polygon.
   */
  DO_SARA_EXPORT
  std::vector<Point2d> sutherland_hodgman(const std::vector<Point2d>& subject,
                                          const std::vector<Point2d>& clip);

} /* namespace Sara */
} /* namespace DO */


#endif /* DO_SARA_GEOMETRY_ALGORITHMS_SUTHERLANDHODGMAN_HPP */
