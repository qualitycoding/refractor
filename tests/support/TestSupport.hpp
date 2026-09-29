// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
#pragma once
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <vector>
#include "dsp/Engine.hpp"
#include "dsp/Calibration.hpp"

namespace rt {
using Vec = std::vector<float>;

inline Vec sine(double hz, double fs, double seconds, float amp = 0.25f) {
  Vec v(static_cast<size_t>(seconds * fs));
  for (size_t n = 0; n < v.size(); ++n) v[n] = amp * static_cast<float>(std::sin(2.0 * std::numbers::pi * hz * n / fs));
  return v;
}
/// Deterministic white noise in [-amp, amp] (32-bit LCG, seed fixed per call site).
inline Vec noise(size_t n, uint32_t seed, float amp = 0.25f) {
  Vec v(n); uint32_t s = seed;
  for (auto& x : v) { s = s * 1664525u + 1013904223u; x = amp * (static_cast<float>(s >> 8) / 8388608.0f - 1.0f); }
  return v;
}
inline double rms(const Vec& v, size_t b = 0, size_t e = SIZE_MAX) {
  e = std::min(e, v.size()); double a = 0; for (size_t i = b; i < e; ++i) a += double(v[i]) * v[i];
  return e > b ? std::sqrt(a / double(e - b)) : 0.0;
}
inline float maxAbsDiff(const Vec& a, const Vec& b) {
  float m = 0; for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) m = std::max(m, std::abs(a[i] - b[i])); return m;
}
inline float maxStep(const Vec& v, size_t b, size_t e) {
  float m = 0; for (size_t i = std::max<size_t>(b, 1); i < std::min(e, v.size()); ++i) m = std::max(m, std::abs(v[i] - v[i - 1])); return m;
}
inline void fft(std::vector<std::complex<double>>& a) {
  const size_t n = a.size();
  for (size_t i = 1, j = 0; i < n; ++i) { size_t bit = n >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) std::swap(a[i], a[j]); }
  for (size_t len = 2; len <= n; len <<= 1) {
    const double ang = -2.0 * std::numbers::pi / double(len); const std::complex<double> wl(std::cos(ang), std::sin(ang));
    for (size_t i = 0; i < n; i += len) { std::complex<double> w(1); for (size_t j = 0; j < len / 2; ++j) {
      auto u = a[i + j], v = a[i + j + len / 2] * w; a[i + j] = u + v; a[i + j + len / 2] = u - v; w *= wl; } }
  }
}
/// Dominant frequency (Hz) of v[b,e) above minHz: Hann window, zero-padded to 2^18, parabolic peak interpolation.
inline double dominantHz(const Vec& v, double fs, size_t b = 0, size_t e = SIZE_MAX, double minHz = 20.0) {
  e = std::min(e, v.size()); const size_t N = size_t(1) << 18; std::vector<std::complex<double>> a(N);
  const size_t L = e - b;
  for (size_t i = 0; i < L && i < N; ++i) a[i] = v[b + i] * (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / double(L - 1)));
  fft(a); size_t k0 = std::max<size_t>(1, size_t(minHz * N / fs)), best = k0; double bm = 0;
  for (size_t k = k0; k < N / 2 - 1; ++k) { double m = std::abs(a[k]); if (m > bm) { bm = m; best = k; } }
  const double y0 = std::log(std::abs(a[best - 1]) + 1e-30), y1 = std::log(std::abs(a[best]) + 1e-30), y2 = std::log(std::abs(a[best + 1]) + 1e-30);
  const double d = 0.5 * (y0 - y2) / (y0 - 2 * y1 + y2 + 1e-30);
  return (double(best) + d) * fs / double(N);
}
/// Energy (sum of |X|^2) of v[b,e) in [loHz,hiHz), Hann-windowed, zero-padded to 2^18.
inline double bandEnergy(const Vec& v, double fs, double loHz, double hiHz, size_t b = 0, size_t e = SIZE_MAX) {
  e = std::min(e, v.size()); const size_t N = size_t(1) << 18; std::vector<std::complex<double>> a(N); const size_t L = e - b;
  for (size_t i = 0; i < L && i < N; ++i) a[i] = v[b + i] * (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / double(L - 1)));
  fft(a); double s = 0; for (size_t k = size_t(loHz * N / fs); k < std::min(N / 2, size_t(hiHz * N / fs)); ++k) s += std::norm(a[k]); return s;
}
inline double expectedSemitones(float knob) {
  using namespace refractor::calib;
  return knob <= 0.5f ? kPitchDownSemitones * (1.0 - 2.0 * knob) : kPitchUpSemitones * (2.0 * knob - 1.0);
}
inline double ratioOf(double semis) { return std::pow(2.0, semis / 12.0); }

/// Mono in -> mono out through Engine, fixed block size.
inline Vec run(refractor::Engine& e, const Vec& in, int block = 128) {
  Vec out(in.size());
  for (size_t p = 0; p < in.size(); p += size_t(block)) {
    const int n = int(std::min<size_t>(size_t(block), in.size() - p));
    const float* i[1] = { in.data() + p }; float* o[1] = { out.data() + p };
    e.process(i, o, 1, 1, n);
  }
  return out;
}
inline Vec wetOf(const Vec& out, const Vec& in) { Vec w(out.size()); for (size_t i = 0; i < w.size(); ++i) w[i] = out[i] - in[i]; return w; }

/// Neutral preset: everything off except what the test sets. Call before reset().
inline void neutral(refractor::Engine& e) {
  using refractor::ParamId;
  e.setParameter(ParamId::Pitch, 0.5f); e.setParameter(ParamId::PitchExp, 0.5f); e.setParameter(ParamId::ExpEnabled, 0.0f);
  e.setParameter(ParamId::Primary, 0.0f); e.setParameter(ParamId::Secondary, 0.0f); e.setParameter(ParamId::Tracking, 1.0f);
  e.setParameter(ParamId::Tone, 1.0f); e.setParameter(ParamId::Magic, 0.0f); e.setParameter(ParamId::MagicEngaged, 0.0f);
  e.setParameter(ParamId::Bypass, 0.0f);
}
}
