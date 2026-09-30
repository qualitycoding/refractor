#include <cstdio>
#include <cmath>
#include "support/TestSupport.hpp"
#include "dsp/PitchShifter.hpp"
using namespace rt;
// Chord fidelity vs window size and internal clock. "clean" = fraction of output energy sitting on the three expected partials (+-3 %).
int main() {
  struct Chord { const char* name; double f[3]; } chords[] = {{"G triad 196/247/294", {196, 247, 294}}, {"E5 power 82/123/165", {82.4, 123.5, 164.8}}, {"open-ish 110/165/220", {110, 165, 220}}};
  for (double fs : {32768.0, 16384.0, 8192.0, 4915.0}) for (int N : {1024, 2048, 4096}) {
    std::printf("f_int=%5.0f N=%4d (bin %.1f Hz, lag %3.0f ms):", fs, N, fs / N, 1000.0 * N / fs);
    for (auto& c : chords) {
      const double r = 1.26; Vec x(size_t(3.0 * fs), 0.0f);
      for (double f : c.f) { const Vec s = sine(f, fs, 3.0, 0.15f); for (size_t i = 0; i < x.size(); ++i) x[i] += s[i]; }
      refractor::PhaseVocoderShifter sh; sh.prepare(N, 4); sh.setRatio(float(r)); Vec y(x.size());
      for (size_t i = 0; i < x.size(); ++i) y[i] = sh.processSample(x[i]);
      const size_t b = size_t(1.2 * fs); const double tot = bandEnergy(y, fs, 15, fs / 2 * 0.98, b); double on = 0;
      for (double f : c.f) on += bandEnergy(y, fs, 0.97 * f * r, 1.03 * f * r, b);
      std::printf("  %4.0f%%", 100 * on / tot);
    }
    std::printf("   (chords: G triad | E5 power | 110/165/220)\n");
  }
}
