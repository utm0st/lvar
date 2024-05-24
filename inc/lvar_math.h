#pragma once

#include <initializer_list>
#include <algorithm>
#include <cassert>
#include <immintrin.h>          // SSE 4.2

namespace lvar {

  // prepare all these classes for simd
  // this means classes should ideally align with 16 byte boundaries
  class alignas(16) v2 final {
  public:
    float x, y;
    float _pad;
    float _pad2;
  };

  class alignas(16) v3 final {
  public:
    float x, y, z;
    float _pad;
  };

  class alignas(16) v4 final {
  public:
    float x, y, z, w;
  };

  //
  // column-major order because OpenGL uses it and because apparently a lot of operations in
  // graphics programming require you to access columns entries sequentially, so this way of
  // storing data can be cache more friendlier than row-major order.
  //
  // if you were to make this class as row-major order, then every time you needed to set a mat4
  // in OpenGL via a uniform, you'd need to ask OpenGL to transpose the matrix for you (or do it)
  // yourself. This isn't a big deal, really, but it's good to keep in mind
  //
  // also, since C++ uses row-major order, you need to define a function to access its elements in
  // column-major order
  //
  class alignas(16) m4 final {
  private:
    float elements[16];
  public:
    m4() noexcept
      : elements{}
    {
    }
    m4(std::initializer_list<float> a) noexcept
    {
      std::copy(a.begin(), a.end(), elements);
    }
    // allow setting values thru this function bc it's useful
    inline float& get(const int col, const int row) noexcept
    {
      return elements[col * 4 + row];
    }
    inline const float& get(const int col, const int row) const noexcept
    {
      return elements[col * 4 + row];
    }
  };

  // -----
  // -- vector operations
  // -----

  inline float dot(const v4& a, const v4& b)
  {
    // @TODO: simd
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  }

  // -----
  // -- matrix operations
  // -----
  inline m4 identity()
  {
    return {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

  // @NOTE: matrix multiplication is done from right to left, so be careful when
  // you call this function
  inline m4 mul(const m4& a, const m4& b)
  {
    // yeah, this crap is only for intel processors, but want to mess with them, also pretty much all CPUs
    // accept SSE by now...
    m4 res;
    for(int i{ 0 }; i < 4; ++i) {
      __m128 row = _mm_set_ps(a.get(i, 3), a.get(i, 2), a.get(i, 1), a.get(i, 0));
      for(int j{ 0 }; j < 4; ++j) {
        // these two functions, _mm_set_ps, load floats into in reverse order of the args
        // that you provide! how about that!
        __m128 col = _mm_set_ps(b.get(3, j), b.get(2, j), b.get(1, j), b.get(0, j));
        // 0xF1 shit is telling the CPU to:
        // 0x1 lower four bits: only the lowest element (value within the register) of the result
        // register is used to store the dot product
        // 0xF higher four bits: all elements from col and row should be multiplied together
        __m128 dot = _mm_dp_ps(row, col, 0xf1);
        res.get(i, j) = _mm_cvtss_f32(dot);
      }
    }
    return res;
  }

  // -----
  // -- misc
  // -----
  inline m4 perspective()
  {

  }
};
