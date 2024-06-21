#pragma once

namespace lvar {
  namespace resource {

    enum class shader_type {
      vertex,
      fragment,
      program
    };

    // useful class to store shader's id, vao, vbo and ebo once they're created in
    // the resource manager; since every shader is diff (num of attrs, etc), it's
    // not a good idea to define these member functions in here, they should be
    // somewhere else
    class shader final {
    public:
      shader(unsigned int const i,
             unsigned int const va,
             unsigned int const vb,
             unsigned int const e)
        : id{ i },
          vao{ va },
          vbo{ vb },
          ebo{ e }
      {
      }
      ~shader() = default;
    public:
      unsigned int id;
      unsigned int vao;
      unsigned int vbo;
      unsigned int ebo;
    };
  };
};
