#include "lvar_math.h"

#include <cassert>
#include <iostream>

using namespace lvar;

void test_translate()
{
  m4 move123{ identity() };
  translate(move123, v3{ 1.0f, 2.0f, 3.0f });
  assert(move123.get(3, 0) == 1.0f && move123.get(3, 1) == 2.0f && move123.get(3, 2) == 3.0f );
}

int main()
{
  std::ios::sync_with_stdio(false);
  test_translate();
  std::clog << __FILE__ << "...ok\n";
  return EXIT_SUCCESS;
}
