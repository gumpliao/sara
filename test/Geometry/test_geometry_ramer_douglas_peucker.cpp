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

#include <gtest/gtest.h>

#include <DO/Sara/Geometry/Algorithms.hpp>

#include "TestPolygon.hpp"

#include "../AssertHelpers.hpp"


using namespace std;
using namespace DO::Sara;


TEST(TestRamerDouglasPeucker, test_squared_distance)
{
  auto a = Point2d{ 0., 0. };
  auto b = Point2d{ 0., 10. };
  auto x = Point2d{ 2., 8. };

  EXPECT_NEAR(detail::orthogonal_distance(a, b, x), 2., 1e-8);
}

TEST(TestRamerDouglasPeucker, test_squared_distance_2)
{
  auto a = Point2d{ 0., 0. };
  auto b = Point2d{ 10., 10. };
  auto x = Point2d{ 5., 3. };

  EXPECT_NEAR(detail::orthogonal_distance(a, b, x), sqrt(2.), 1e-8);
}

TEST(TestRamerDouglasPeucker, test_linesegment_simplification)
{
  const auto line = vector<Point2d>{
    Point2d(191, 639),
    Point2d(192, 639)
  };

  EXPECT_EQ(line, ramer_douglas_peucker(line, 0.1));
}

TEST(TestRamerDouglasPeucker, test_polylines_simplification)
{
  const auto polylines = vector<Point2d>{
    Point2d(0, 0),
    Point2d(1, 0.25),
    Point2d(2, 0.5),
    Point2d(9, 0)
  };

  const auto expected_polylines = vector<Point2d>{
    Point2d(0, 0), Point2d(2, 0.5), Point2d(9, 0)
  };
  EXPECT_EQ(expected_polylines, ramer_douglas_peucker(polylines, 0.1));
}

TEST(TestRamerDouglasPeucker, test_square)
{
  const auto square = vector<Point2d>{
    Point2d(0, 0),
    Point2d(0.25, 0),
    Point2d(0.5, 0),
    Point2d(0.75, 0),
    Point2d(1, 0),
    Point2d(1, 1),
    Point2d(0, 1),
    Point2d(0, 0)
  };

  const auto expected_polygon = vector<Point2d>{
    Point2d(0, 0),
    Point2d(1, 0),
    Point2d(1, 1),
    Point2d(0, 1)
  };

  EXPECT_EQ(expected_polygon, ramer_douglas_peucker(square, 0.1));
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
