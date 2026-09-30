#include <chrono>
#include <cstdio>
#include <cmath>
#include "support/TestSupport.hpp"
#include "dsp/PitchShifter.hpp"
using namespace rt;
int main() {
  const double fs = 32768.0;
  // 1) Chord: three partials, each must appear at f*r with ~ input amplitude; little energy elsewhere.
  for (double r : {0.749, 1.26, 2.0}) {
    const double fr[3] = {196.0, 247.0, 294.0};  // G major triad (guitar range)
    Vec x(size_t(2.5 * fs), 0.0f);
    for (double f : fr) { const Vec s = sine(f, fs, 2.5, 0.15f); for (size_t i = 0; i < x.size(); ++i) x[i] += s[i]; }
    refractor::PhaseVocoderShifter sh; sh.prepare(1024, 4); sh.setRatio(float(r));
    Vec y(x.size()); for (size_t i = 0; i < x.size(); ++i) y[i] = sh.processSample(x[i]);
    const size_t b = size_t(0.8 * fs); double tot = bandEnergy(y, fs, 20, 16000, b), inband = 0; std::printf("chord r=%.3f:", r);
    for (double f : fr) {
      const double e = bandEnergy(y, fs, 0.97 * f * r, 1.03 * f * r, b), ein = bandEnergy(x, fs, 0.97 * f, 1.03 * f, b);
      inband += e; std::printf("  %.0f->%.1f Hz gain %+.1f dB", f, f * r, 10 * std::log10(e / ein));
    }
    std::printf("   out-of-partial energy %.1f dB\n", 10 * std::log10(std::max(tot - inband, 1e-30) / tot));
  }
  // 2) Lag by energy centroid of a Hann-windowed noise burst (dry vs wet), ratio 1 and 1.26
  for (double r : {1.0, 1.26}) for (int N : {512, 1024, 2048}) {
    const size_t len = size_t(0.4 * fs), L = size_t(0.15 * fs), t0 = size_t(0.1 * fs);
    Vec x(len, 0.0f); const Vec nz = noise(L, 5u, 0.5f);
    for (size_t i = 0; i < L; ++i) x[t0 + i] = nz[i] * float(0.5 - 0.5 * std::cos(2 * M_PI * i / (L - 1)));
    refractor::PhaseVocoderShifter sh; sh.prepare(N, 4); sh.setRatio(float(r)); Vec y(len);
    for (size_t i = 0; i < len; ++i) y[i] = sh.processSample(x[i]);
    auto cen = [&](const Vec& v) { double a = 0, b = 0; for (size_t i = 0; i < v.size(); ++i) { a += double(v[i]) * v[i] * i; b += double(v[i]) * v[i]; } return a / b; };
    std::printf("lag N=%d r=%.2f: centroid lag %.1f samples (latency=%d)  energy gain %.2f\n", N, r, cen(y) - cen(x), sh.latencySamples(), std::sqrt(rms(y)*rms(y)/ (rms(x)*rms(x))));
  }
  // 3) CPU: ns per sample
  { refractor::PhaseVocoderShifter sh; sh.prepare(1024, 4); sh.setRatio(1.26f); const Vec x = noise(size_t(10 * fs), 9u); float acc = 0;
    const auto t0 = std::chrono::steady_clock::now(); for (float v : x) acc += sh.processSample(v);
    const double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / double(x.size());
    std::printf("cost: %.0f ns/sample (noise input, N=1024)  -> one voice at 32768 Hz = %.2f %% of a core  (acc %g)\n", ns, ns * 32768 / 1e7, acc); }
}
