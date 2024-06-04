#include "lvar_math.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace lvar;

void test_normalise()
{
  const v3 a{ 5.4f, 2.33f, 28.33f };
  const v3 a_norm{ normalise(a) };
  assert(a_norm.x <= 1.0f && a_norm.x > 0.0f);
  assert(a_norm.y <= 1.0f && a_norm.y > 0.0f);
  assert(a_norm.z <= 1.0f && a_norm.z > 0.0f);
  const v3 zero{ 0.0f, 0.0f, 0.0f };
  const v3 zero_norm{ normalise(zero) };
  assert(zero_norm.x == 0.0f);
  assert(zero_norm.y == 0.0f);
  assert(zero_norm.z == 0.0f);
  const v3 neg{ -12.0f, -3.0f, 1.f };
  const v3 neg_norm{ normalise(neg) };
  assert(neg_norm.x <= 0.0f && neg_norm.x >= -1.0f);
  assert(neg_norm.x <= 0.0f && neg_norm.x >= -1.0f);
  assert(neg_norm.z >= 0.0f && neg_norm.z <= 1.0f);
}

int main()
{
  std::ios::sync_with_stdio(false);
  test_normalise();
  std::clog << __FILE__ << "...ok\n";
  return EXIT_SUCCESS;
}
