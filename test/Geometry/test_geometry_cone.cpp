#include <DO/Sara/Geometry/Objects/Cone.hpp>

#include "../AssertHelpers.hpp"

#include "TestPolygon.hpp"


using namespace std;
using namespace DO::Sara;


class TestAffineCone : public TestPolygon
{
protected:
  // Generators of the 2D cone
  Vector2d _alpha;
  Vector2d _beta;

  TestAffineCone() : TestPolygon()
  {
    _alpha << 1, 0;
    _beta << 1, 1;
  }
};


TEST_F(TestAffineCone, test_convex_affine_cone)
{
  AffineCone2 K(_alpha, _beta, _center, AffineCone2::Convex);

  auto convex_predicate = [&](const Point2d& p) {
    return K.contains(p);
  };

  auto convex_ground_truth = [&](const Point2d& p) {
    return
      p.x() > _width/2. &&
      p.y() > _height/2. &&
      p.x() > p.y();
  };

  sweep_check(convex_predicate, convex_ground_truth);
}


TEST_F(TestAffineCone, test_blunt_affine_cone)
{
  AffineCone2 K(_alpha, _beta, _center, AffineCone2::Blunt);

  auto blunt_predicate = [&](const Point2d& p) {
    return K.contains(p);
  };

  auto blunt_ground_truth = [&](const Point2d& p) {
    return
      p.x() >= _width/2. &&
      p.y() >= _height/2. &&
      p.x() >= p.y();
  };

  sweep_check(blunt_predicate, blunt_ground_truth);
}


TEST_F(TestAffineCone, test_convex_pointed_affine_cone)
{
  AffineCone2 K(_alpha, _alpha, _center, AffineCone2::Convex);

  auto convex_pointed_predicate = [&](const Point2d& p) {
    return K.contains(p);
  };

  auto convex_pointed_ground_truth = [&](const Point2d& p) {
    (void) p;
    return false;
  };

  sweep_check(convex_pointed_predicate, convex_pointed_ground_truth);

  AffineCone2 K2 { affine_cone2(0, 0, _center)};
  EXPECT_MATRIX_NEAR(K.basis(), K2.basis(), 1e-6);
  EXPECT_EQ(K.vertex(), K2.vertex());
}


TEST_F(TestAffineCone, test_blunt_pointed_cone)
{
  AffineCone2 K(_alpha, _alpha, _center, AffineCone2::Blunt);

  auto blunt_pointed_predicate = [&](const Point2d& p) {
    return K.contains(p);
  };

  auto blunt_pointed_ground_truth = [&](const Point2d& p) {
    return
      p.x() >= _width/2. &&
      p.y() == _height/2.;
  };

  sweep_check(blunt_pointed_predicate, blunt_pointed_ground_truth);
}


// Degenerate case where the affine cone is actually empty.
TEST_F(TestAffineCone, test_degenerate_convex_affine_cone)
{
  AffineCone2 K(_alpha, -_alpha, _center, AffineCone2::Convex);

  auto convex_predicate = [&](const Point2d& p) {
    return K.contains(p);
  };

  auto convex_ground_truth = [&](const Point2d& p) {
    (void) p;
    return false;
  };

  sweep_check(convex_predicate, convex_ground_truth);
}


// Degenerate case where the affine cone is actually a half-space.
TEST_F(TestAffineCone, test_degenerate_blunt_pointed_affine_cone)
{
  AffineCone2 K(_alpha, -_alpha, _center, AffineCone2::Blunt);
  auto blunt_pointed_predicate = [&](const Point2d& p) {
    return K.contains(p);
  };
  auto blunt_pointed_ground_truth = [&](const Point2d& p) {
    return p.y() == _height/2.;
  };
  sweep_check(blunt_pointed_predicate, blunt_pointed_ground_truth);
}


int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
