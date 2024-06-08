#pragma once

#include "lvar_math.h"

#include <iostream>

namespace lvar {

  inline void print(v3 const& v)
  {
    std::clog << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
  }

}

