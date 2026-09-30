#include <cstdio>
#include <cmath>
#include "support/TestSupport.hpp"
#include "dsp/PitchShifter.hpp"
using namespace rt;
int main() {
  const double fs = 32768.0;
  for (int W : {512, 1024, 2048}) {
    std::printf("W=%d\n", W);
    for (double r : {0.75, 0.5, 0.472, 0.375, 1.26, 2.0, 2.52}) {
      refractor::DelayLineShifter s; s.prepare(W); s.setRatio(float(r));
      const Vec x = sine(440, fs, 1.5); Vec y(x.size());
      for (size_t i = 0; i < x.size(); ++i) y[i] = s.processSample(x[i]);
      const double f = dominantHz(y, fs, size_t(0.5 * fs));
      std::printf("  r=%.3f want %.1f got %.1f err %+.2f%%\n", r, 440*r, f, 100*(f/(440*r)-1));
    }
  }
}
