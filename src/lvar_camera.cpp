#include "lvar_camera.h"
#include "lvar_math.h"

namespace lvar {

  camera::camera(v3 const& position, input::manager& im)
    : up{ v3{ 0.0f, 1.0f, 0.0f } },
      front{ v3{ 0.0f, 0.0f, -1.0f } },
      pos{ position },
      input_manager{ im },
      yaw{ -90.0f },
      pitch{ 0.0f },
      speed{ 2.0f },
      currspeed{ 0.0f }
  {
  }

  void camera::handle_input_kb()
  {
    if(input_manager.key_held(input::key::w)) {
      pos = add(pos, scale(front, currspeed));
    }
    if(input_manager.key_held(input::key::a)) {
      pos = sub(pos, scale(normalise(cross(front, up)), currspeed));
    }
    if(input_manager.key_held(input::key::s)) {
      pos = sub(pos, scale(front, currspeed));
    }
    if(input_manager.key_held(input::key::d)) {
      pos = add(pos, scale(normalise(cross(front, up)), currspeed));
    }
  }

  void camera::update(float const mouse_x, float const mouse_y, float const dt)
  {
    currspeed = speed * dt;
    handle_input_kb();
    yaw += mouse_x;
    pitch += mouse_y;
    if(pitch > 89.0f) {
      pitch = 89.0f;
    } else if(pitch < -89.0f) {
      pitch = -89.0f;
    }
    dir = {
      std::cos(radians(yaw)) * std::cos(radians(pitch)),
      std::sin(radians(pitch)),
      std::sin(radians(yaw)) * std::cos(radians(pitch))
    };
    front = normalise(dir);
    view = look_at(pos, add(pos, front), up);
  }

};
