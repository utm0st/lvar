#pragma once

#include "lvar_shader.h"
#include "lvar_common.h"

#include <unordered_map>
#include <utility>
#include <memory>

namespace lvar {
  namespace resource {

    // @NOTE: cursed code to be able to use the unordered_map with the pair
    class pair_hash final {
    public:
      std::size_t operator()(const std::pair<unsigned int, char const*>& arg) const
      {
        // yoooo, these {} create an instance of the hash for the types in <>!
        const auto hash1 = std::hash<unsigned int>{}(arg.first);
        const auto hash2 = std::hash<char const*>{}(arg.second);
        return hash1 ^ (hash2 << 1);
      }
    };

    // this class is expected to be omoi
    class manager final {
    public:
      // all resources will be loaded in the constructor
      manager() noexcept;
      ~manager() noexcept;
    public:
      auto error() const noexcept { return err; }
      auto get_shader(char const* sid) const noexcept { return shaders.at(fnv1a(sid)).get(); }
      unsigned int getUniformLocation(unsigned int const id, char const* uniname)
      {
        const auto key = std::make_pair(id, uniname);
        const auto it  = uniforms.find(key);
        if(it == uniforms.end()) { // not found
          auto const location = glGetUniformLocation(id, uniname);
          uniforms[key] = location;
          return location;
        }
        return it->second;
      }
      auto use_shader(unsigned int const id) const noexcept
      {
        glUseProgram(id);
      }
      auto set_uni_mat4(unsigned int const id, char const* uniname, m4 const& m)
      {
        glUniformMatrix4fv(getUniformLocation(id, uniname), 1, false, &m.get(0, 0));
      }
      auto set_uni_vec3(unsigned int const id, char const* uniname, v3 const& value)
      {
        glUniform3f(getUniformLocation(id, uniname), value.x, value.y, value.z);
      }
    private:
      bool shader_compilation_has_errors(unsigned int const program, shader_type const type) const noexcept;
      // load shader files, compile and link them, return the id of the gend program
      unsigned int compile_link_shaders(char const* vertpath, char const* fragpath) noexcept;
      shader create_shader(char const* path_vertex_sh,
                           char const* path_frag_sh,
                           void* vertex_data,
                           std::size_t const vertex_sz,
                           unsigned int const num_attrs,
                           unsigned int const elem_per_attr,
                           std::size_t const stride_sz) noexcept;
    private:
      std::unordered_map<int, std::unique_ptr<shader>> shaders;
      std::unordered_map<std::pair<unsigned int, char const*>, unsigned int, pair_hash> uniforms;
      bool err;
    };

  };
};
