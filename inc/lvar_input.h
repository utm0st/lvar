#pragma once

namespace lvar {
  namespace input {

    enum class key : unsigned int {
      w = 0,
      a,
      s,
      d,
      esc,
      up,
      down,
      left,
      right,
      f1,
      count,
      unknown,
    };

    class manager final {
    public:
      manager() noexcept
      {
        for(unsigned int i{ 0 }; i < static_cast<unsigned int>(key::count); ++i) {
          currkeys[i] = false;
          prevkeys[i] = false;
        }
      }
      ~manager() = default;
    public:
      bool key_pressed(key const k) const noexcept
      {
        return !prevkeys[static_cast<unsigned int>(k)] && currkeys[static_cast<unsigned int>(k)];
      }
      bool key_held(key const k) const noexcept
      {
        return prevkeys[static_cast<unsigned int>(k)] && currkeys[static_cast<unsigned int>(k)];
      }
      // store in prevkeys the contents of currkeys and update currkeys with recent user's input.
      // needed to determine if a key is being pressed or held. if you didn't do that, a single
      // key press would be very hard to get right because it "extends" through multiple frames.
      void begin_frame_kb() noexcept
      {
        for(unsigned int i{ 0 }; i < static_cast<unsigned int>(key::count); ++i) {
          prevkeys[i] = currkeys[i];
        }
      }
      // update key state
      void update_key(key const k, bool const pressed)
      {
        currkeys[static_cast<unsigned int>(k)] = pressed;
      }
    private:
      bool currkeys[static_cast<unsigned int>(key::count)];
      bool prevkeys[static_cast<unsigned int>(key::count)];
    };

  };
};
