#pragma once
#include <cmath>
#include "dsp/Calibration.hpp"
namespace refractor {
// Host -> internal: mean of the piecewise-linear host signal over the time since the previous internal tick (area
// sampling; weak low-pass, NOT an anti-alias filter, so aliasing remains — C-013).
// Internal -> host: causal linear interpolation between the last two internal outputs (adds one internal tick of lag).
template <class Fn> float ClockDomain::tick(float hostIn, Fn&& fn) {
  scale_ += smoothA_ * (target_ - scale_);
  if (std::fabs(target_ - scale_) < 1e-9) scale_ = target_;
  const double step = calib::kNominalClockHz * scale_ / hostRate_;   // internal ticks per host sample
  const double v0 = static_cast<double>(prevIn_), dv = static_cast<double>(hostIn) - v0;
  double a = acc_, rem = step, consumed = 0.0, pos = 0.0;
  auto accumulate = [&](double p0, double p1) {
    accInt_ += (p1 - p0) * (v0 + dv * 0.5 * (p0 + p1));
    elapsed_ += (p1 - p0);
  };
  while (a + rem >= 1.0) {
    const double toTick = 1.0 - a;
    consumed += toTick; rem -= toTick; a = 0.0;
    const double np = std::fmin(consumed / step, 1.0);
    accumulate(pos, np); pos = np;
    const double xin = elapsed_ > 1e-12 ? accInt_ / elapsed_ : v0 + dv * np;
    accInt_ = 0.0; elapsed_ = 0.0;
    prevOut_ = lastOut_;
    lastOut_ = fn(static_cast<float>(xin));
  }
  accumulate(pos, 1.0);
  acc_ = a + rem;
  prevIn_ = hostIn;
  return prevOut_ + (lastOut_ - prevOut_) * static_cast<float>(acc_);
}
}  // namespace refractor
