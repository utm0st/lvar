#pragma once

#include <cassert>
#include <cmath>
#include <initializer_list>
#include <algorithm>
// @TODO: this shit is diff in windows, probably #ifdef? separate file lvar_intrinsics.h?
#include <immintrin.h>          // SSE 4.2

namespace lvar {

  float constexpr PI{ 3.1415926535897f };
  float constexpr epsilon{ 0.001f };

  inline constexpr float radians(float const degrees)
  {
    return degrees * (PI / 180.0f);
  }

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

  class alignas(16) v3i final {
  public:
    int x, y, z;
    int _pad;
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
    inline float& get(int const col, int const row) noexcept
    {
      return elements[col * 4 + row];
    }
    inline const float& get(int const col, int const row) const noexcept
    {
      return elements[col * 4 + row];
    }
  };

  inline constexpr float dot(v4 const& a, v4 const& b)
  {
    // @NOTE: gcc optimises the hell out of this, no need to use intrinsics
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  }

  inline constexpr float dot(v3 const& a, v3 const& b)
  {
    // @NOTE: gcc optimises the hell out of this, no need to use intrinsics
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

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
  inline m4 mul(m4 const& a, m4 const& b)
  {
    m4 res;
    for(int i{ 0 }; i < 4; ++i) {
      __m128 const row = _mm_set_ps(a.get(i, 3), a.get(i, 2), a.get(i, 1), a.get(i, 0));
      for(int j{ 0 }; j < 4; ++j) {
        // these two functions, _mm_set_ps, load floats into in reverse order of the args
        // that you provide! how about that!
        __m128 const col = _mm_set_ps(b.get(3, j), b.get(2, j), b.get(1, j), b.get(0, j));
        // 0xF1 shit is telling the CPU to:
        // 0x1 lower four bits: only the lowest element (value within the register) of the result
        // register is used to store the dot product
        // 0xF higher four bits: all elements from col and row should be multiplied together
        __m128 const dot = _mm_dp_ps(row, col, 0xf1);
        res.get(i, j) = _mm_cvtss_f32(dot);
      }
    }
    return res;
  }

  // this matrix is used to transform from view to clip space. Clip coordinates are between [-1.0, 1.0] range.
  // Everything outside this range will get clipped. FOV -> vertical fov
  m4 perspective(float const fov, float const ratio, float const near, float const far)
  {
    // no need to do simd here bc this function will be called only once or not too many times at least
    assert(fov > 0.0f);
    assert(far > near);
    // basically took it out of the internet, the math is over my head at the moment, see:
    // http://www.songho.ca/opengl/gl_projectionmatrix.html#fov
    float const half_fov_rad{ radians(fov / 2.0f) };
    float const tan_half_fov{ std::tanf(half_fov_rad) };
    float const top{ near * tan_half_fov };
    float const right{ top * ratio };
    m4 result;
    result.get(0, 0) = near / right;
    result.get(1, 1) = near / top;
    result.get(2, 2) = - (far + near) / (far - near);
    result.get(2, 3) = -1.0f;                             // you motherfucker!!!!!!!!!!!!!!!!!
    result.get(3, 2) = - (2 * far * near) / (far - near); // you motherfucker!!!!!!!!!!!!!!!!!
    return result;
  }

  inline void translate(m4& m, v3 const& pos)
  {
    // @NOTE: no need for intrinsics, gcc optimises pretty f well
    m.get(3, 0) += pos.x;
    m.get(3, 1) += pos.y;
    m.get(3, 2) += pos.z;
  }

  inline void scale(m4& m, v3 const& v)
  {
    // @NOTE: no need for intrinsics, gcc optimises pretty f well
    m.get(0, 0) *= v.x;
    m.get(1, 1) *= v.y;
    m.get(2, 2) *= v.z;
  }

  inline v3 scale(v3 const& v, float const s)
  {
    // @NOTE: no need for intrinsics, gcc optimises pretty f well
    return {
      v.x * s,
      v.y * s,
      v.z * s
    };
  }

  // @TODO: comment this for each line because it's hard to read
  inline v3 cross(v3 const& v1, v3 const& v2)
  {
    __m128 const v1vec{ _mm_set_ps(0.0f, v1.z, v1.y, v1.x) };
    __m128 const v2vec{ _mm_set_ps(0.0f, v2.z, v2.y, v2.x) };
    __m128 const v1_yzx{ _mm_shuffle_ps(v1vec, v1vec, _MM_SHUFFLE(3, 0, 2, 1)) };
    __m128 const v2_yzx{ _mm_shuffle_ps(v2vec, v2vec, _MM_SHUFFLE(3, 0, 2, 1)) };
    __m128 const a{ _mm_mul_ps(v1vec, v2_yzx) };
    __m128 const b{ _mm_mul_ps(v1_yzx, v2vec) };
    __m128 resultSimd{ _mm_sub_ps(a, b) };
    resultSimd = _mm_shuffle_ps(resultSimd, resultSimd, _MM_SHUFFLE(3, 0, 2, 1));
    v3 res;
    _mm_store_ss(&res.x, resultSimd);
    _mm_store_ss(&res.y, _mm_shuffle_ps(resultSimd, resultSimd, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&res.z, _mm_shuffle_ps(resultSimd, resultSimd, _MM_SHUFFLE(2, 2, 2, 2)));
    return res;
  }

  inline v3 sub(v3 const& a, v3 const& b)
  {
    // @NOTE: no need for intrinsics, gcc optimises pretty f well
    return {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z
    };
  }

  inline v3 add(v3 const& a, v3 const& b)
  {
    // @NOTE: no need for intrinsics, gcc optimises pretty f well
    return {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z
    };
  }

  inline v3 normalise(v3 const& v)
  {
    // div by zero check so program doesn't blow up
    if(v.x == 0 && v.y == 0 && v.z == 0) {
      return { 0.0f, 0.0f, 0.0f };
    }
    // simd black magic
    __m128 const vsimd{ _mm_set_ps(0.0f, v.z, v.y, v.x) }; // yes, reversed, you know about this already
    __m128 const mul{ _mm_mul_ps(vsimd, vsimd) }; // square each element
    // sum components
    __m128 const sum{ _mm_add_ps(_mm_shuffle_ps(mul, mul, _MM_SHUFFLE(0, 0, 0, 0)),
                                 _mm_add_ps(_mm_shuffle_ps(mul, mul, _MM_SHUFFLE(1, 1, 1, 1)),
                                            _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 2, 2, 2)))) };
    __m128 const sqrt{ _mm_sqrt_ps(sum) }; // sqrt of sum
    __m128 const normalised{ _mm_div_ps(vsimd, sqrt) }; // finally, divide by magnitude and get normalised vec
    v3 res;
    // x is already stored in the lowest single element of the register, so no need to do more black magic
    _mm_store_ss(&res.x, normalised);
    _mm_store_ss(&res.y, _mm_shuffle_ps(normalised, normalised, _MM_SHUFFLE(1, 1, 1, 1)));     // not here tho
    _mm_store_ss(&res.z, _mm_shuffle_ps(normalised, normalised, _MM_SHUFFLE(2, 2, 2, 2)));
    return res;
  }

  inline m4 lookAt(v3 const& pos, v3 const& target, v3 const& up)
  {
    // point from target to pos, Z needs to be positive bc in OpenGL the cam points to
    // towards the neg z axis
    v3 const f{ normalise(sub(pos, target)) }; // direction
    v3 const s{ normalise(cross(up, f)) };     // right
    v3 const u{ cross(f, s) };
    m4 const translation{
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      -pos.x, -pos.y, -pos.z, 1.0f,
    };
    m4 const rotation{
      s.x,  u.x,  f.x,  0.0f,
      s.y,  u.y,  f.y,  0.0f,
      s.z,  u.z,  f.z,  0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
    };
    return mul(translation, rotation);
  }

  // @TODO: support multiple rotations
  // @TODO: use quaternions instead to avoid gimbal lock problem
  void rotate(m4& m, float const degrees, v3i const& axis)
  {
    auto const rad = radians(degrees);
    m4 r;
    if(axis.x == 1) {
      r = m4{
        1.0f, 0.0f,           0.0f,          0.0f,
        0.0f, std::cos(rad), -std::sin(rad), 0.0f,
        0.0f, std::sin(rad),  std::cos(rad), 0.0f,
        0.0f, 0.0f,           0.0f,          1.0f,
      };
    } else if(axis.y == 1) {
      r = m4{
        std::cos(rad),  0.0f, std::sin(rad), 0.0f,
        0.0f,           1.0f, 0.0f,          0.0f,
        -std::sin(rad), 0.0f, std::cos(rad), 0.0f,
        0.0f,           0.0f, 0.0f,          1.0f,
      };
    } else if(axis.z == 1) {
      r = m4{
        std::cos(rad), -std::sin(rad), 0.0f, 0.0f,
        std::sin(rad),  std::cos(rad), 0.0f, 0.0f,
        0.0f,           0.0f,          1.0f, 0.0f,
        0.0f,           0.0f,          0.0f, 1.0f,
      };
    }
    m = mul(m, r);
  }

};
