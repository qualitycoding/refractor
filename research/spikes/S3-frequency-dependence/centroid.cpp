#include <cstdio>
#include <cmath>
#include "support/TestSupport.hpp"
#include "dsp/PitchShifter.hpp"
using namespace rt;
// Three independent pitch estimates on the bare shifter: dominant FFT peak, power-weighted spectral centroid
// (+-40% band around target), and mean zero-crossing rate.
int main() {
  const double fs = 32768.0; const int W = 1024;
  for (double f0 : {440.0, 300.0, 220.0}) for (double r : {0.75, 0.5, 0.375, 1.26, 2.52}) {
    refractor::DelayLineShifter s; s.prepare(W); s.setRatio(float(r));
    const Vec x = sine(f0, fs, 2.0); Vec y(x.size());
    for (size_t i = 0; i < x.size(); ++i) y[i] = s.processSample(x[i]);
    const size_t b = size_t(0.5 * fs), N = size_t(1) << 18; const double want = f0 * r;
    const double peak = dominantHz(y, fs, b);
    // centroid
    std::vector<std::complex<double>> a(N); const size_t L = y.size() - b;
    for (size_t i = 0; i < L; ++i) a[i] = y[b+i] * (0.5 - 0.5*std::cos(2*M_PI*i/double(L-1)));
    fft(a); double num = 0, den = 0;
    for (size_t k = size_t(0.6*want*N/fs); k < size_t(1.4*want*N/fs); ++k) { const double p = std::norm(a[k]); num += p * (double(k)*fs/N); den += p; }
    // zero crossings
    size_t zc = 0; for (size_t i = b+1; i < y.size(); ++i) if ((y[i-1] < 0) != (y[i] < 0)) ++zc;
    const double zcf = 0.5 * double(zc) / (double(y.size() - b) / fs);
    std::printf("f0=%5.0f r=%.3f want %7.1f | peak %+6.2f%%  centroid %+6.2f%%  zero-cross %+6.2f%%\n", f0, r, want,
                100*(peak/want-1), 100*(num/den/want-1), 100*(zcf/want-1));
  }
}
