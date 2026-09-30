#include "dsp/PitchShifter.hpp"
#include <algorithm>
#include <cmath>
namespace refractor {
void DelayLineShifter::prepare(int windowSamples) {
  window_ = std::max(windowSamples, 16);
  std::size_t n = 1;
  while (n < static_cast<std::size_t>(window_) * 4) n <<= 1;
  buffer_.assign(n, 0.0f);
  mask_ = static_cast<unsigned>(n - 1);
  reset();
}
void DelayLineShifter::reset() {
  std::fill(buffer_.begin(), buffer_.end(), 0.0f);
  write_ = 0; phase_ = 0.0; ratio_ = 1.0f;
}
void DelayLineShifter::setRatio(float ratio) { ratio_ = std::isfinite(ratio) && ratio > 0.0f ? ratio : 1.0f; }

// Two taps half a window apart, triangular crossfade (spike S1, C-020). The just-written sample is read at delay 0,
// so the nominal lag at phase 0 (tap 2 at W/2) is exactly W/2 samples.
float DelayLineShifter::processSample(float in) {
  const unsigned w = static_cast<unsigned>(write_);
  buffer_[w] = in;
  const double W = static_cast<double>(window_);
  const double d1 = phase_ * W;
  const double p2 = phase_ >= 0.5 ? phase_ - 0.5 : phase_ + 0.5;
  const double d2 = p2 * W;
  const float g1 = static_cast<float>(1.0 - std::fabs(2.0 * phase_ - 1.0));
  const float g2 = 1.0f - g1;
  auto rd = [&](double d) {
    const unsigned i0 = static_cast<unsigned>(d);
    const float fr = static_cast<float>(d - static_cast<double>(i0));
    const float a = buffer_[(w - i0) & mask_];
    const float b = buffer_[(w - i0 - 1u) & mask_];
    return a + (b - a) * fr;
  };
  const float out = g1 * rd(d1) + g2 * rd(d2);
  write_ = static_cast<int>((w + 1u) & mask_);
  phase_ += (1.0 - static_cast<double>(ratio_)) / W;
  phase_ -= std::floor(phase_);
  if (!(phase_ >= 0.0 && phase_ < 1.0)) phase_ = 0.0;   // NaN / rounding guard
  return out;
}
float DelayLineShifter::nominalLagSamples() const { return static_cast<float>(window_) * 0.5f; }
}  // namespace refractor
