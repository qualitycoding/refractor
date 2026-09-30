#include "dsp/ClockDomain.hpp"
#include <algorithm>
#include <cmath>
#include "dsp/Calibration.hpp"
namespace refractor {
void ClockDomain::prepare(double hostRate, int) {
  hostRate_ = hostRate > 0.0 ? hostRate : 48000.0;
  smoothA_ = 1.0 - std::exp(-1.0 / (calib::kSmoothingSeconds * hostRate_));
  reset();
}
void ClockDomain::reset() { scale_ = target_; acc_ = 0.0; accInt_ = 0.0; elapsed_ = 0.0; prevIn_ = lastOut_ = prevOut_ = 0.0f; }
void ClockDomain::setClockScale(double scale) { target_ = std::clamp(std::isfinite(scale) ? scale : 1.0, 0.01, 1.0); }
double ClockDomain::internalRate() const { return calib::kNominalClockHz * scale_; }
}  // namespace refractor
