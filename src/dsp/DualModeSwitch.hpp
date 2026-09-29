#pragma once
// Latching + momentary footswitch logic (D-013, C-008). Times are in seconds from any monotonic clock.
// press(): toggles engaged. release(): if held >= kHoldSeconds, reverts to the pre-press state.
namespace refractor {
class DualModeSwitch {
public:
  explicit DualModeSwitch(bool initiallyEngaged = false);
  void press(double timeSeconds);
  void release(double timeSeconds);
  bool engaged() const;
  void set(bool engaged);   // host automation / state restore; cancels any hold in progress
private:
  bool engaged_ = false, beforePress_ = false, down_ = false;
  double pressTime_ = 0.0;
};
}
