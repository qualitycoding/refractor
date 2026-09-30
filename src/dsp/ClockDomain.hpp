#pragma once
#include <vector>
// Variable-rate internal clock domain (D-010). Host-rate input is resampled to f_int = kNominalClockHz * scale
// with linear interpolation and NO anti-alias filter (intentional aliasing, C-013), processed, then
// resampled back to host rate with linear interpolation. Input side uses area (box) averaging over one internal tick
// period rather than point sampling, so transients are never skipped between ticks (DEVIATIONS D-A); aliasing remains.
namespace refractor {
class ClockDomain {
public:
  void prepare(double hostRate, int maxBlock); // allocates
  void reset();
  void setClockScale(double scale);            // (0,1]; smoothed internally
  double internalRate() const;                 // current f_int in Hz
  /// Pushes one host-rate sample; calls fn(internalIn)->internalOut for each internal tick due;
  /// returns the host-rate output sample. Template keeps it allocation-free.
  template <class Fn> float tick(float hostIn, Fn&& fn);
private:
  double hostRate_ = 48000.0, scale_ = 1.0, acc_ = 0.0, target_ = 1.0, smoothA_ = 0.001, accInt_ = 0.0, elapsed_ = 0.0;
  float prevIn_ = 0.0f, lastOut_ = 0.0f, prevOut_ = 0.0f;
};
}
#include "dsp/ClockDomain.ipp"
