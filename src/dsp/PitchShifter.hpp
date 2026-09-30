#pragma once
#include <complex>
#include <vector>
// STFT phase-vocoder pitch shifter with identity phase locking (after Laroche & Dolson 1999), operating at the
// internal clock rate (D-010 rev. 2). Each spectral peak and its region of influence is moved by the pitch ratio, so
// the shift is accurate for any input frequency and for polyphonic input. Partials shifted beyond Nyquist alias back
// into band (intentional "digital remnants", C-013; also keeps Magic regeneration alive).
namespace refractor {
class PhaseVocoderShifter {
public:
  void prepare(int fftSize, int overlap);   // allocates; fftSize a power of two, overlap >= 4 (hop = fftSize/overlap)
  void reset();                              // zero all state; real-time safe
  void setRatio(float ratio);                // pitch ratio > 0; read once per frame
  float processSample(float in);             // real-time safe
  int latencySamples() const { return n_; }  // stationary input-to-output delay, in internal samples
private:
  using cd = std::complex<double>;
  void processFrame();
  void fft(cd* a, bool inverse) const;
  int n_ = 0, hop_ = 0, half_ = 0, inPos_ = 0, hopCount_ = 0, outIdx_ = 0;
  float ratio_ = 1.0f;
  double scale_ = 1.0;
  std::vector<double> window_, prevAna_, synPrev_, mag_, ymag_, outAccum_;
  std::vector<float> inRing_, outFifo_;
  std::vector<cd> x_, y_, tw_;
  std::vector<int> bitrev_, peaks_;
};
}
