#pragma once
#include <vector>
// Two-tap, triangular-crossfade delay-line pitch shifter operating at the internal clock (D-010, C-020).
namespace refractor {
class DelayLineShifter {
public:
  void prepare(int windowSamples);           // allocates; not real-time safe
  void reset();                               // zero state; real-time safe
  void setRatio(float ratio);                 // pitch ratio > 0
  float processSample(float in);              // real-time safe; returns shifted sample
  float nominalLagSamples() const;            // windowSamples / 2
private:
  std::vector<float> buffer_;
  int window_ = 0, write_ = 0;
  double phase_ = 0.0;
  float ratio_ = 1.0f;
};
}
