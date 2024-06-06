#include "lvar_math.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>

using namespace lvar;
using namespace std::chrono_literals; // ms

void test_dot_v4x4()
{
  v4 constexpr a{ 3.14f, 2.17f, -1.31f, 9.99f };
  v4 constexpr b{ 1.2f, 5.77f, 3.09f, 1.06f };
  float constexpr expected{ 22.8304 };
  float constexpr result{ dot(a, b) };
  assert(std::fabs(result - expected) < epsilon);
}

void test_dot_v4x4_speed()
{
  // no need to check results bc that's actually done in other tests, here it's just speed
  v4 a;
  v4 b;
  auto currentAccumulatedTime = 0ns;
  for(int i{ 0 }; i < 10'000'000; ++i) {
    a = { 3.14f + i, 2.44f + i, 1.2f + i, 2.2f + i};
    b = { 5.74f + i, 1.02f + i, 2.4f + i, 3.1f + i};
    auto start = std::chrono::high_resolution_clock::now();
    assert(dot(a, b) > 0.0f);         // do this so it doesn't get optimised away
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    currentAccumulatedTime += duration;
  }
  if(currentAccumulatedTime > 2ms) { // it usually never goes past 1ms
    std::clog << "Duration was: " << currentAccumulatedTime << std::endl;
    assert(false && "4x4 dot product is too slow");
  }
}

void test_dot_v3x3()
{
  v3 constexpr a{ -7.23f, -0.176f, -23.31f };
  v3 constexpr b{ -5.12f, -6.21f, -97.09f };
  float constexpr expected{ 2301.27846f };
  float constexpr result{ dot(a, b) };
  assert(std::fabs(result - expected) < epsilon);
}

void test_dot()
{
  test_dot_v4x4();
  test_dot_v4x4_speed();
  test_dot_v3x3();
}

int main()
{
  std::ios::sync_with_stdio(false);
  test_dot();
  std::clog << __FILE__ << "...ok\n";
  return EXIT_SUCCESS;
}
