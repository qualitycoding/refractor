#include <cstdio>
#include "support/TestSupport.hpp"
using namespace refractor; using namespace rt;
// Worst-case peak of Engine output vs the delay-line analytic bound, over randomised maximum-regeneration settings.
int main() {
  const double fs = 48000; uint32_t s = 12345u; auto rnd = [&] { s = s * 1664525u + 1013904223u; return float(s >> 8) / 16777216.0f; };
  double worst = 0; int worstCase = -1;
  for (int t = 0; t < 40; ++t) {
    Engine e; e.prepare(fs, 512); neutral(e);
    e.setParameter(ParamId::Pitch, rnd()); e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Secondary, 1.0f);
    e.setParameter(ParamId::Magic, 1.0f); e.setParameter(ParamId::MagicEngaged, 1.0f); e.setParameter(ParamId::Tracking, rnd()); e.reset();
    const float amp = 0.1f + 0.9f * rnd(); Vec in(size_t(6.0 * fs), 0.0f);
    const int kind = t % 3; const double f0 = 80 + 900 * rnd();
    const Vec b = kind == 0 ? sine(f0, fs, 0.2, amp) : kind == 1 ? noise(size_t(0.2 * fs), 100u + t, amp) : [&] { Vec v = sine(f0, fs, 0.2, amp / 3); Vec w = sine(f0 * 1.26, fs, 0.2, amp / 3), z = sine(f0 * 1.5, fs, 0.2, amp / 3); for (size_t i = 0; i < v.size(); ++i) v[i] += w[i] + z[i]; return v; }();
    std::copy(b.begin(), b.end(), in.begin());
    float xmax = 0; for (float v : b) xmax = std::max(xmax, std::abs(v));
    const Vec out = run(e, in); float pk = 0; bool fin = true; for (float v : out) { fin &= std::isfinite(v); pk = std::max(pk, std::abs(v)); }
    const double bound = xmax + 2.0 * (xmax + calib::kMagicMaxFeedback) / calib::kUnityKnob, ratio = pk / bound;
    if (ratio > worst) { worst = ratio; worstCase = t; }
    if (!fin) std::printf("NON-FINITE in case %d\n", t);
  }
  std::printf("worst peak / analytic-bound = %.3f (case %d) over 40 runs\n", worst, worstCase);
}
