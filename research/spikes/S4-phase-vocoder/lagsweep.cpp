#include <cstdio>
#include <numbers>
#include "support/TestSupport.hpp"
#include "dsp/Mapping.hpp"
using namespace refractor; using namespace rt;
static double lagFor(double fs, float t, uint32_t seed, size_t Lsamp, double tone = 0) {
  Engine e; e.prepare(fs, 512); neutral(e); e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Tracking, t); e.reset();
  const double lagS = e.nominalWetLagSeconds(); const size_t t0 = size_t(0.05 * fs), L = Lsamp, total = t0 + L + size_t((2.5 * lagS + 0.1) * fs);
  Vec in(total, 0.0f); const Vec nz = tone > 0 ? sine(tone, fs, double(L) / fs, 0.5f) : noise(L, seed, 0.5f);
  for (size_t i = 0; i < L; ++i) in[t0 + i] = nz[i] * float(0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * double(i) / double(L - 1)));
  const Vec w = wetOf(run(e, in), in);
  auto c = [](const Vec& v) { double a = 0, b = 0; for (size_t i = 0; i < v.size(); ++i) { a += double(v[i]) * v[i] * double(i); b += double(v[i]) * v[i]; } return a / b; };
  return (c(w) - c(in)) / fs;
}
int main() {
  const double fs = 48000; double worst = 0;
  for (float t : {1.0f, 0.75f, 0.5f, 0.25f, 0.0f}) {
    const double fInt = calib::kNominalClockHz * mapping::trackingToClockScale(t), exp = calib::kWindowSamples / fInt;
    std::printf("tracking %.2f expected %7.2f ms; tone-burst error (ms) at 220/330/440/660/880 Hz:", t, 1e3 * exp);
    for (double f : {220.0, 330.0, 440.0, 660.0, 880.0}) { const double e = 1e3 * (lagFor(fs, t, 0, size_t(0.3 * fs), f) - exp); worst = std::max(worst, std::abs(e)); std::printf(" %+6.2f", e); }
    std::printf("\n");
  }
  std::printf("worst |error| = %.2f ms\n", worst);
}
