// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <limits>
#include "support/TestSupport.hpp"
using namespace refractor; using namespace rt;

static void busy(Engine& e) {
  neutral(e); e.setParameter(ParamId::Pitch, 0.8f); e.setParameter(ParamId::Primary, 0.9f); e.setParameter(ParamId::Secondary, 0.6f);
  e.setParameter(ParamId::Tracking, 0.3f); e.setParameter(ParamId::Tone, 0.6f); e.setParameter(ParamId::Magic, 0.7f);
  e.setParameter(ParamId::MagicEngaged, 1.0f);
}

// T-018 [integration] Channel handling per Engine contract. AMENDED (TC-3): final block is clamped to the samples that remain. Enforces: SC-6, D-014, D-017.
TEST_CASE("T-018 channel configurations", "[operational][T-018]") {
  const double fs = 48000; const Vec a = sine(440, fs, 0.5), b = noise(a.size(), 5u);
  { Engine e; e.prepare(fs, 512); busy(e); e.reset(); Vec o0(a.size()), o1(a.size());
    const float* in[2] = { a.data(), b.data() }; float* out[2] = { o0.data(), o1.data() };
    for (size_t p = 0; p < a.size(); p += 128) { const float* i[2] = { in[0] + p, in[1] + p }; float* o[2] = { out[0] + p, out[1] + p }; e.process(i, o, 2, 2, int(std::min<size_t>(128, a.size() - p))); }
    CHECK(maxAbsDiff(wetOf(o0, a), wetOf(o1, b)) <= 1e-6f); }
  { Engine e; e.prepare(fs, 512); busy(e); e.reset(); Vec o0(a.size()), o1(a.size());
    for (size_t p = 0; p < a.size(); p += 128) { const float* i[1] = { a.data() + p }; float* o[2] = { o0.data() + p, o1.data() + p }; e.process(i, o, 1, 2, int(std::min<size_t>(128, a.size() - p))); }
    CHECK(maxAbsDiff(o0, o1) == 0.0f); }
  { Engine e; e.prepare(fs, 512); neutral(e); e.reset(); Vec o0(a.size());
    for (size_t p = 0; p < a.size(); p += 128) { const float* i[2] = { a.data() + p, b.data() + p }; float* o[1] = { o0.data() + p }; e.process(i, o, 2, 1, int(std::min<size_t>(128, a.size() - p))); }
    Vec mean(a.size()); for (size_t i = 0; i < a.size(); ++i) mean[i] = 0.5f * (a[i] + b[i]);
    CHECK(maxAbsDiff(o0, mean) <= 1e-6f); }
  { Engine e; e.prepare(fs, 512); busy(e); e.reset(); Vec x = noise(a.size(), 11u), ref = x;   // in-place
    for (size_t p = 0; p < x.size(); p += 128) { float* io[1] = { x.data() + p }; e.process(io, io, 1, 1, int(std::min<size_t>(128, x.size() - p))); }
    Engine r; r.prepare(fs, 512); busy(r); r.reset(); CHECK(maxAbsDiff(x, run(r, ref)) == 0.0f); }
}

// T-020 [operational] Determinism and reset(). Enforces: D-014.
TEST_CASE("T-020 determinism and reset", "[operational][T-020]") {
  const double fs = 48000; const Vec x = noise(size_t(2 * fs), 21u), junk = noise(size_t(fs), 22u, 0.9f);
  Engine a, b; a.prepare(fs, 512); b.prepare(fs, 512); busy(a); busy(b); a.reset(); b.reset();
  CHECK(maxAbsDiff(run(a, x), run(b, x)) == 0.0f);
  Engine c; c.prepare(fs, 512); busy(c); c.reset(); (void)run(c, junk); c.reset();
  Engine d; d.prepare(fs, 512); busy(d); d.reset();
  CHECK(maxAbsDiff(run(c, x), run(d, x)) == 0.0f);
}

// T-021 [operational] Output independent of block partitioning (per-sample smoothing). Enforces: D-014.
TEST_CASE("T-021 block-size invariance", "[operational][T-021]") {
  const double fs = 48000; const Vec x = noise(size_t(fs), 31u);
  Engine ref; ref.prepare(fs, 4096); busy(ref); ref.reset(); const Vec y = run(ref, x, 128);
  for (int bs : {1, 7, 64, 4096}) { Engine e; e.prepare(fs, 4096); busy(e); e.reset(); INFO("block " << bs); CHECK(maxAbsDiff(run(e, x, bs), y) <= 1e-6f); }
  Engine e; e.prepare(fs, 4096); busy(e); e.reset(); Vec out(x.size()); uint32_t s = 77u; size_t p = 0;
  while (p < x.size()) { s = s * 1664525u + 1013904223u; const int n = int(std::min<size_t>(1 + (s >> 20) % 4096, x.size() - p));
    const float* i[1] = { x.data() + p }; float* o[1] = { out.data() + p }; e.process(i, o, 1, 1, n); p += size_t(n); }
  CHECK(maxAbsDiff(out, y) <= 1e-6f);
}

// T-023 [security/operational] Non-finite input treated as 0; output always finite. Enforces: D-014, R-006.
TEST_CASE("T-023 non-finite input robustness", "[operational][security][T-023]") {
  const double fs = 48000; Vec x = sine(330, fs, 1.5); Vec clean = x; uint32_t s = 5u;
  const float bad[3] = { std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };
  for (size_t i = 0; i < size_t(0.5 * fs); i += 37) { s = s * 1664525u + 1013904223u; x[i] = bad[(s >> 16) % 3]; clean[i] = 0.0f; }
  Engine a, b; a.prepare(fs, 512); b.prepare(fs, 512); busy(a); busy(b); a.reset(); b.reset();
  const Vec ya = run(a, x), yb = run(b, clean);
  bool finite = true; for (float v : ya) finite &= std::isfinite(v);
  CHECK(finite); CHECK(maxAbsDiff(ya, yb) == 0.0f);
}

// T-024 [operational] Decaying state flushes to exact zero (no denormal tails). Enforces: D-014, R-007.
TEST_CASE("T-024 silence flushes to zero", "[operational][T-024]") {
  const double fs = 48000; Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, 0.75f); e.setParameter(ParamId::Primary, calib::kUnityKnob); e.setParameter(ParamId::Magic, 0.5f);
  e.setParameter(ParamId::MagicEngaged, 1.0f); e.reset();
  Vec x(size_t(61 * fs), 0.0f); const Vec b = sine(300, fs, 0.2); std::copy(b.begin(), b.end(), x.begin());
  const Vec y = run(e, x);
  bool zero = true; for (size_t i = size_t(60 * fs); i < y.size(); ++i) zero &= (y[i] == 0.0f);
  CHECK(zero);
}
