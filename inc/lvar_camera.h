#pragma once

#include "lvar_math.h"

namespace lvar {
  namespace input {
    class manager;
  };

  class camera final {
  public:
    camera(v3 const& position, input::manager& im);
  public:
    void update(float const mouse_x, float const mouse_y, float const dt);
    auto& get_view() const noexcept { return view; } // don't fucking modify it
  private:
    void handle_input_kb();
  private:
    m4 view;
    v3 const up;
    v3 front;
    v3 pos;
    v3 dir;
    input::manager& input_manager;
    float yaw; // looking right or left?
    float pitch; // looking up or down?
    float const speed;
    float currspeed;
  };

};
