#pragma once

#include <vector>

namespace lvar {
  namespace obj {

    class vertex final {
    public:
      float x;
      float y;
      float z;
    };

    class face final {
    public:
      unsigned int idx_x;
      unsigned int idx_y;
      unsigned int idx_z;
    };

    // @TODO: once you have the arena, use it here instead of a vector
    class mesh final {
    public:
      std::vector<vertex> vertices;
      std::vector<face> faces;  // @TODO: why would you need this?
      std::vector<unsigned int> indices;
    };

    bool parse_file(char const* filepath, mesh& o);

  };
};
