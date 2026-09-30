#include <cstdio>
#include <cmath>
#include "support/TestSupport.hpp"
#include "dsp/PitchShifter.hpp"
using namespace rt;
int main(int argc, char** argv) {
  const double fs = 32768.0; const int N = argc > 1 ? std::atoi(argv[1]) : 1024;
  double worst = 0;
  std::printf("N=%d  (peak-estimate error %%, rms gain)\n      f0:", N);
  const double ratios[] = {0.375, 0.5, 0.749, 0.9, 1.0, 1.122, 1.26, 2.0, 2.52};
  for (double r : ratios) std::printf(" r=%-5.3f     ", r); std::printf("\n");
  for (double f0 : {82.0, 110.0, 146.0, 220.0, 300.0, 440.0, 660.0, 880.0, 1200.0}) {
    std::printf("%7.0f:", f0);
    for (double r : ratios) {
      refractor::PhaseVocoderShifter s; s.prepare(N, 4); s.setRatio(float(r));
      const Vec x = sine(f0, fs, 2.5); Vec y(x.size());
      for (size_t i = 0; i < x.size(); ++i) y[i] = s.processSample(x[i]);
      const double want = f0 * r; const size_t b = size_t(0.8 * fs);
      const double f = dominantHz(y, fs, b, SIZE_MAX, 10.0);
      const double e = 100 * (f / want - 1), g = rms(y, b) / rms(x, b);
      if (want < fs / 2 * 0.95) worst = std::max(worst, std::abs(e));
      std::printf(" %+6.2f/%4.2f  ", e, g);
    }
    std::printf("\n");
  }
  std::printf("worst |err| (non-aliased cases) = %.3f %%\n", worst);
}
