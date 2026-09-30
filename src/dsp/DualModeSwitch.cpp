#include "dsp/DualModeSwitch.hpp"
#include "dsp/Calibration.hpp"
namespace refractor {
DualModeSwitch::DualModeSwitch(bool initiallyEngaged) : engaged_(initiallyEngaged), beforePress_(initiallyEngaged) {}
void DualModeSwitch::press(double t) {
  if (down_) return;                 // ignore key-repeat / double press
  down_ = true; pressTime_ = t; beforePress_ = engaged_; engaged_ = !engaged_;
}
void DualModeSwitch::release(double t) {
  if (!down_) return;                // spurious release
  down_ = false;
  if (t - pressTime_ >= calib::kHoldSeconds) engaged_ = beforePress_;   // held: momentary
}
bool DualModeSwitch::engaged() const { return engaged_; }
void DualModeSwitch::set(bool e) { engaged_ = e; beforePress_ = e; down_ = false; }
}  // namespace refractor
