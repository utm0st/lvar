#include "lvar_camera.h"
#include "lvar_math.h"
#include "lvar_math_debug.h"

namespace lvar {

  camera::camera(v3 const& position, input::manager& im)
    : up{ v3{ 0.0f, 1.0f, 0.0f } },
      front{ v3{ 0.0f, 0.0f, -1.0f } },
      pos{ position },
      curr_vel{ 0.0f, 0.0f, 0.0f },
      goal_vel{ 0.0f, 0.0f, 0.0f },
      input_manager{ im },
      yaw{ -90.0f },
      pitch{ 0.0f },
      speed{ 2.0f }
  {
  }

  void camera::handle_input_kb()
  {
    if(input_manager.key_held(input::key::w)) {
      goal_vel.z = speed;
    } else if(input_manager.key_release(input::key::w)) {
      goal_vel.z = 0;
    }
    if(input_manager.key_held(input::key::s)) {
      goal_vel.z = -speed;
    } else if(input_manager.key_release(input::key::s)) {
      goal_vel.z = 0;
    }
    if(input_manager.key_held(input::key::a)) {
      goal_vel.x = -speed;
    } else if(input_manager.key_release(input::key::a)) {
      goal_vel.x = 0;
    }
    if(input_manager.key_held(input::key::d)) {
      goal_vel.x = speed;
    } else if(input_manager.key_release(input::key::d)) {
      goal_vel.x = 0;
    }
  }

  void camera::update(float const mouse_x, float const mouse_y, float const dt)
  {
    handle_input_kb();
    // interpolate camera movement to make it feel better
    curr_vel.z = approach(curr_vel.z, goal_vel.z, dt);
    curr_vel.x = approach(curr_vel.x, goal_vel.x, dt);
    curr_vel.y = approach(curr_vel.y, goal_vel.y, dt);
    // apply this new interpolated velocity to the position
    pos = add(pos, scale(front, curr_vel.z));
    pos = add(pos, scale(normalise(cross(front, up)), curr_vel.x));
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
