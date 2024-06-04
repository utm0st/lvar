#include "lvar_math.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace lvar;

void test_mul_identity()
{
  const m4 id{ identity() };
  const m4 random{
    69.0f, 12.3f, - 14.3f,  20.0f,
    52.0f,  2.3f, -114.3f, -30.0f,
    69.0f, 12.3f,   -4.3f,   0.0f,
    69.0f, 12.3f,  - 2.2f,  20.0f,
  };
  // check left to right and right to left
  m4 res{
    mul(id, random)
  };
  for(int i{ 0 }; i < 4; ++i) {
    for(int j{ 0 }; j < 4; ++j) {
      assert(res.get(i, j) == random.get(i, j));
    }
  }
  res = mul(random, id);
  for(int i{ 0 }; i < 4; ++i) {
    for(int j{ 0 }; j < 4; ++j) {
      assert(res.get(i, j) == random.get(i, j));
    }
  }
}

void test_mul_zero()
{
  const m4 zero{
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
  };
  const m4 random{
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
  };
  m4 res{
    mul(zero, random)
  };
  for(int i{ 0 }; i < 4; ++i) {
    for(int j{ 0 }; j < 4; ++j) {
      assert(res.get(i, j) == 0.0f);
    }
  }
  res = mul(random, zero);
  for(int i{ 0 }; i < 4; ++i) {
    for(int j{ 0 }; j < 4; ++j) {
      assert(res.get(i, j) == 0.0f);
    }
  }
}

void test_mul_known()
{
  const m4 a = {
    1.0f,   2.0f,  3.0f,  4.0f,
    5.0f,   6.0f,  7.0f,  8.0f,
    9.0f,  10.0f, 11.0f, 12.0f,
    13.0f, 14.0f, 15.0f, 16.0f,
  };
  const m4 b = {
    16.0f, 15.0f, 14.0f, 13.0f,
    12.0f, 11.0f, 10.0f,  9.0f,
     8.0f,  7.0f,  6.0f,  5.0f,
     4.0f,  3.0f,  2.0f,  1.0f,
  };
  m4 res {
    mul(a, b)
  };
  const m4 expected{
     80.0f,  70.0f,  60.0f,  50.0f,
    240.0f, 214.0f, 188.0f, 162.0f,
    400.0f, 358.0f, 316.0f, 274.0f,
    560.0f, 502.0f, 444.0f, 386.0f,
  };
  for(int i{ 0 }; i < 4; ++i) {
    for(int j{ 0 }; j < 4; ++j) {
      assert(std::fabs(res.get(i, j) - expected.get(i, j)) < epsilon);
    }
  }
  // now test changing order of multiplication to verify the result is different
  m4 res2{
    mul(b, a)
  };
  for(int i{ 0 }; i < 4; ++i) {
    for(int j{ 0 }; j < 4; ++j) {
      assert(std::fabs(res.get(i, j) - res2.get(i, j)) > epsilon);
    }
  }
}

void test_mul_lots()
{
  m4 a = {
    1.0f,   2.0f,  3.0f,  4.0f,
    5.0f,   6.0f,  7.0f,  8.0f,
    9.0f,  10.0f, 11.0f, 12.0f,
    13.0f, 14.0f, 15.0f, 16.0f,
  };
  m4 b = {
    16.0f, 15.0f, 14.0f, 13.0f,
    12.0f, 11.0f, 10.0f,  9.0f,
    8.0f,  7.0f,  6.0f,  5.0f,
    4.0f,  3.0f,  2.0f,  1.0f,
  };
  auto start = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < 10000000; ++i) {
    a.get(0, 2) += + i;
    m4 res {
      mul(a, b)
    };
    assert(res.get(0, 0) != 0.0f);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::clog << "10M matrix mults took: " << duration.count() << "ms." << '\n';
}

void test_mul()
{
  test_mul_identity();
  test_mul_zero();
  test_mul_known();
  // this is just to test performance, atm: ~18ms to compute 10M matrix multiplications
  // test_mul_lots();
}

int main()
{
  std::ios::sync_with_stdio(false);
  test_mul();
  std::clog << __FILE__ << "...ok\n";
  return EXIT_SUCCESS;
}
