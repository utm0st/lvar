#include "lvar_math.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace lvar;

void test_cross_basic()
{
  const v3 a{ 5.0f, 2.0f, -2.0f };
  const v3 b{ 1.0f, 10.f, 19.0f };
  const v3 res{ cross(a, b) };
  const v3 expected{ 58.0f, -97.0f, 48.0f };
  assert(std::fabs(expected.x - res.x) <= epsilon);
  assert(std::fabs(expected.y - res.y) <= epsilon);
  assert(std::fabs(expected.z - res.z) <= epsilon);
}

void test_cross_orthogonal()
{
  const v3 a{ 1.0f, 0.0f, 0.0f };
  const v3 b{ 0.0f, 1.0f, 0.0f };
  const v3 res{ cross(a, b) };
  const v3 expected{ 0.0f, 0.0f, 1.0f };
  assert(std::fabs(expected.x - res.x) <= epsilon);
  assert(std::fabs(expected.y - res.y) <= epsilon);
  assert(std::fabs(expected.z - res.z) <= epsilon);
}

void test_cross_parallel()
{
  const v3 a{ 2.0f, 3.0f, -1.0f };
  const v3 b{ 4.0f, 6.0f, -2.0f }; // a but multiplied by 2
  const v3 res{ cross(a, b) };
  const v3 expected{ 0.0f, 0.0f, 0.0f };
  assert(std::fabs(expected.x - res.x) <= epsilon);
  assert(std::fabs(expected.y - res.y) <= epsilon);
  assert(std::fabs(expected.z - res.z) <= epsilon);
}

void test_cross_zero()
{
  const v3 a{ 0.0f, 0.0f, 0.0f };
  const v3 b{ 44.2f, 2.12f, -23.2f };
  const v3 res{ cross(a, b) };
  const v3 expected{ 0.0f, 0.0f, 0.0f };
  assert(std::fabs(expected.x - res.x) <= epsilon);
  assert(std::fabs(expected.y - res.y) <= epsilon);
  assert(std::fabs(expected.z - res.z) <= epsilon);
}

void test_cross_unit()
{
  const v3 a{ 1.0f, 0.0f, 0.0f };
  const v3 b{ 0.0f, 0.0f, 1.0f };
  const v3 res{ cross(a, b) };
  const v3 expected{ 0.0f, -1.0f, 0.0f };
  assert(std::fabs(expected.x - res.x) <= epsilon);
  assert(std::fabs(expected.y - res.y) <= epsilon);
  assert(std::fabs(expected.z - res.z) <= epsilon);
}

void test_cross()
{
  test_cross_basic();
  test_cross_orthogonal();
  test_cross_parallel();
  test_cross_zero();
  test_cross_unit();
}

int main()
{
  std::ios::sync_with_stdio(false);
  test_cross();
  std::clog << __FILE__ << "...ok\n";
  return EXIT_SUCCESS;
}
