#pragma once
#include <memory>
#include "dsp/Params.hpp"
// Top-level effect engine (D-002 signal flow). JUCE-free. Contract (D-003, D-014):
//  * prepare() may allocate; reset()/setParameter()/process() never allocate, lock, or throw once implemented.
//  * process(): numIn in {1,2}, numOut in {1,2}; in-place allowed (in[c]==out[c]).
//  * Dry path is exact: with Primary=Secondary=0 and Magic disengaged, out == in (per channel).
//  * Wet = mono sum (mean) of inputs through the effect; identical wet added to every output channel.
//  * Non-finite input samples are treated as 0; output is always finite.
//  * numOut==1 with numIn==2: dry = mean of inputs.
//  * All smoothing is per-sample (output independent of block partitioning, T-021).
//  * reset() zeroes all state and snaps every smoother to its current target value.
//  * Wet lag = kWindowSamples / f_int (phase-vocoder latency); reset() zeroes every shifter/filter state.
namespace refractor {
class Engine {
public:
  Engine();
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  void prepare(double sampleRate, int maxBlockSize);
  void reset();
  void setParameter(ParamId id, float value);   // clamped to spec range; bools: >=0.5 is true
  float getParameter(ParamId id) const;
  void process(const float* const* in, float* const* out, int numIn, int numOut, int numSamples);
  /// Nominal wet lag for the CURRENT (target) Tracking value, in seconds: kWindowSamples/2 / f_int.
  double nominalWetLagSeconds() const;
  static constexpr double kTailSeconds = 10.0;
private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
}
