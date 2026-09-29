// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
#include <catch2/catch_test_macros.hpp>
#include "dsp/DualModeSwitch.hpp"
#include "dsp/Calibration.hpp"
using refractor::DualModeSwitch; using refractor::calib::kHoldSeconds;

// T-036 [unit] Dual-mode switch: tap = latch, hold = momentary. Enforces: SC-4, C-008, D-013.
TEST_CASE("T-036 dual-mode switch latching and momentary", "[unit][T-036]") {
  DualModeSwitch f(false);
  CHECK_FALSE(f.engaged());
  f.press(0.0); CHECK(f.engaged()); f.release(0.1); CHECK(f.engaged());            // tap -> latch on
  f.press(1.0); CHECK_FALSE(f.engaged()); f.release(1.1); CHECK_FALSE(f.engaged()); // tap -> latch off
  f.press(2.0); CHECK(f.engaged()); f.release(2.0 + kHoldSeconds + 0.01); CHECK_FALSE(f.engaged()); // hold -> momentary
  f.set(true);
  f.press(3.0); CHECK_FALSE(f.engaged()); f.release(3.0 + kHoldSeconds + 0.2); CHECK(f.engaged()); // momentary off
  f.press(4.0); f.set(true); f.release(5.0); CHECK(f.engaged());                    // set() cancels hold
  f.release(6.0); CHECK(f.engaged());                                                // spurious release ignored
}
