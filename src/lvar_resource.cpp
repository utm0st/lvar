#include "lvar_resource.h"
#include "lvar_opengl_gnulinux.h"
#include "lvar_shaders_paths.h"

#include <fstream>
#include <sstream>
#include <array>
#include <string>

// @TEMP: this is temporary, objects will be .obj files most of the time, but for simple demos you'll use
// cubes, so put them in here.
static float cube_vertices[] = {
  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f,  0.5f, -0.5f,
  0.5f,  0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,

  -0.5f, -0.5f,  0.5f,
  0.5f, -0.5f,  0.5f,
  0.5f,  0.5f,  0.5f,
  0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f,
  -0.5f, -0.5f,  0.5f,

  -0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f,

  0.5f,  0.5f,  0.5f,
  0.5f,  0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, -0.5f,  0.5f,
  0.5f,  0.5f,  0.5f,

  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, -0.5f,  0.5f,
  0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f, -0.5f,

  -0.5f,  0.5f, -0.5f,
  0.5f,  0.5f, -0.5f,
  0.5f,  0.5f,  0.5f,
  0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f, -0.5f,
};

namespace lvar {
  namespace resource {

    manager::manager() noexcept
      : err{ false }
    {
      // initialise all resources here, maybe it's better to have a function to load at demand
      shaders[fnv1a(SHADER_VERT_LIGHTING_COLOURS)] =
        std::make_unique<shader>(create_shader(SHADER_VERT_LIGHTING_COLOURS,
                                               SHADER_FRAG_LIGHTING_COLOURS,
                                               cube_vertices,
                                               sizeof(cube_vertices),
                                               1,
                                               3,
                                               sizeof(float) * 3));
      if(err) {
        return;
      }
      shaders[fnv1a(SHADER_VERT_LIGHT_CUBE)] =
        std::make_unique<shader>(create_shader(SHADER_VERT_LIGHT_CUBE,
                                               SHADER_FRAG_LIGHT_CUBE,
                                               cube_vertices,
                                               sizeof(cube_vertices),
                                               1,
                                               3,
                                               sizeof(float) * 3));
    }

    manager::~manager() noexcept
    {
      // cleanup shaders
    }

    bool manager::shader_compilation_has_errors(unsigned int const prg, shader_type const type) const noexcept
    {
      int success{ 0 };
      std::array<char, 512> log{ '\0' };
      switch(type) {
      case shader_type::vertex:
        glGetShaderiv(prg, GL_COMPILE_STATUS, &success);
        if(success != GL_TRUE) {
          glGetShaderInfoLog(prg, log.size(), nullptr, log.data());
          std::cerr << __FUNCTION__ << ": couldn't compile vertex shader:" << log.data() << '\n';
          return true;
        }
        break;
      case shader_type::fragment:
        glGetShaderiv(prg, GL_COMPILE_STATUS, &success);
        if(success != GL_TRUE) {
          glGetShaderInfoLog(prg, log.size(), nullptr, log.data());
          std::cerr << __FUNCTION__ << ": couldn't compile fragment shader: " << log.data() << '\n';
          return true;
        }
        break;
      case shader_type::program:
        glGetProgramiv(prg, GL_LINK_STATUS, &success);
        if(success != GL_TRUE) {
          glGetProgramInfoLog(prg, log.size(), nullptr, log.data());
          std::cerr << __FUNCTION__ << ": couldn't link program: " << log.data() << '\n';
          return true;
        }
        break;
      }
      return false;
    }

    unsigned int manager::compile_link_shaders(char const* vertpath, char const* fragpath) noexcept
    {
      std::ifstream vertfs(vertpath), fragfs(fragpath);
      if(!vertfs) {
        std::cerr << __FUNCTION__ << ": couldn't open vertex file: " << vertpath << '\n';
        err = true;
        return 0;
      }
      if(!fragfs) {
        std::cerr << __FUNCTION__ << ": couldn't open fragment file: " << fragpath << '\n';
        err = true;
        return 0;
      }
      std::stringstream vertss, fragss;
      vertss << vertfs.rdbuf();
      fragss << fragfs.rdbuf();
      unsigned int id_vert, id_frag;
      // @NOTE: this step is necessary
      std::string const vertcode{ vertss.str() };
      std::string const fragcode{ fragss.str() };
      // you only care about the c str, really
      char const* vertcodec{ vertcode.c_str() };
      char const* fragcodec{ fragcode.c_str() };
      id_vert = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(id_vert, 1, &vertcodec, nullptr);
      glCompileShader(id_vert);
      if(shader_compilation_has_errors(id_vert, shader_type::vertex)) {
        glDeleteShader(id_vert);
        err = true;
        return 0;
      }
      id_frag = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(id_frag, 1, &fragcodec, nullptr);
      glCompileShader(id_frag);
      if(shader_compilation_has_errors(id_frag, shader_type::fragment)) {
        glDeleteShader(id_vert);
        glDeleteShader(id_frag);
        err = true;
        return 0;
      }
      unsigned int id_prg{ glCreateProgram() };
      glAttachShader(id_prg, id_vert);
      glAttachShader(id_prg, id_frag);
      glLinkProgram(id_prg);
      if(shader_compilation_has_errors(id_prg, shader_type::program)) {
        glDeleteShader(id_vert);
        glDeleteShader(id_frag);
        err = true;
        return 0;
      }
      glDeleteShader(id_vert);
      glDeleteShader(id_frag);
      return id_prg;
    }

    shader manager::create_shader(char const* path_vertex_sh,
                                  char const* path_frag_sh,
                                  void* vertex_data,
                                  std::size_t const vertex_sz,
                                  unsigned int const num_attrs,
                                  unsigned int const elem_per_attr,
                                  std::size_t const stride_sz) noexcept
    {
      unsigned int id{ compile_link_shaders(path_vertex_sh, path_frag_sh) };
      if(id == 0) {
        return shader{ 0, 0, 0, 0, 0 };
      }
      unsigned int vao, vbo, ubo;
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ubo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertex_sz, vertex_data, GL_STATIC_DRAW); // @TODO: parametrise draw type!
      glBindBuffer(GL_UNIFORM_BUFFER, ubo);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(uni_buff_obj), nullptr, GL_STATIC_DRAW);
      auto const uni_block_idx = glGetUniformBlockIndex(id, "matrices");
      unsigned int const unused_bind_point{ 1 };
      glUniformBlockBinding(id, uni_block_idx, unused_bind_point);
      glBindBufferBase(GL_UNIFORM_BUFFER, unused_bind_point, ubo);
      glBindVertexArray(vao);
      for(unsigned int i{ 0 }; i < num_attrs; ++i) {
        glVertexAttribPointer(i, elem_per_attr, GL_FLOAT, GL_FALSE, stride_sz, reinterpret_cast<void*>(i * stride_sz));
        glEnableVertexAttribArray(i);
      }
      return shader{ id, vao, vbo, 0, ubo };
    }
  };
};
