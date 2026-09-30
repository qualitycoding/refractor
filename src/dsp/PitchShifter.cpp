#include "dsp/PitchShifter.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>
namespace refractor {
namespace {
constexpr double kPi = std::numbers::pi;
constexpr double kTwoPi = 2.0 * std::numbers::pi;
inline double wrapPi(double x) { return x - kTwoPi * std::round(x / kTwoPi); }
}  // namespace

void PhaseVocoderShifter::prepare(int fftSize, int overlap) {
  n_ = 16; while (n_ < fftSize) n_ <<= 1;
  overlap = std::max(overlap, 4);
  hop_ = n_ / overlap; half_ = n_ / 2;
  scale_ = 8.0 / (3.0 * overlap);                       // undoes the sum of squared periodic-Hann windows
  window_.assign(static_cast<size_t>(n_), 0.0);
  for (int i = 0; i < n_; ++i) window_[static_cast<size_t>(i)] = 0.5 - 0.5 * std::cos(kTwoPi * i / n_);
  tw_.assign(static_cast<size_t>(half_), cd{});
  for (int k = 0; k < half_; ++k) tw_[static_cast<size_t>(k)] = std::polar(1.0, -kTwoPi * k / n_);
  bitrev_.assign(static_cast<size_t>(n_), 0);
  for (int i = 1, j = 0; i < n_; ++i) {
    int bit = n_ >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; bitrev_[static_cast<size_t>(i)] = j;
  }
  const auto h = static_cast<size_t>(half_ + 1);
  prevAna_.assign(h, 0.0); synPrev_.assign(h, 0.0); mag_.assign(h, 0.0); ymag_.assign(h, 0.0);
  peaks_.assign(h, 0);
  outAccum_.assign(static_cast<size_t>(n_), 0.0);
  inRing_.assign(static_cast<size_t>(n_), 0.0f);
  outFifo_.assign(static_cast<size_t>(hop_), 0.0f);
  x_.assign(static_cast<size_t>(n_), cd{}); y_.assign(static_cast<size_t>(n_), cd{});
  reset();
}
void PhaseVocoderShifter::reset() {
  std::fill(prevAna_.begin(), prevAna_.end(), 0.0); std::fill(synPrev_.begin(), synPrev_.end(), 0.0);
  std::fill(outAccum_.begin(), outAccum_.end(), 0.0); std::fill(inRing_.begin(), inRing_.end(), 0.0f);
  std::fill(outFifo_.begin(), outFifo_.end(), 0.0f);
  inPos_ = 0; hopCount_ = 0; outIdx_ = 0; ratio_ = 1.0f;
}
void PhaseVocoderShifter::setRatio(float r) { ratio_ = std::isfinite(r) && r > 0.0f ? r : 1.0f; }

float PhaseVocoderShifter::processSample(float in) {
  inRing_[static_cast<size_t>(inPos_)] = in;
  if (++inPos_ == n_) inPos_ = 0;
  const float out = outFifo_[static_cast<size_t>(outIdx_)];
  if (++outIdx_ == hop_) outIdx_ = 0;
  if (++hopCount_ == hop_) { hopCount_ = 0; processFrame(); outIdx_ = 0; }
  return out;
}

void PhaseVocoderShifter::fft(cd* a, bool inverse) const {
  const int n = n_;
  if (inverse) for (int i = 0; i < n; ++i) a[i] = std::conj(a[i]);
  for (int i = 1; i < n; ++i) { const int j = bitrev_[static_cast<size_t>(i)]; if (i < j) std::swap(a[i], a[j]); }
  for (int len = 2; len <= n; len <<= 1) {
    const int stride = n / len, hl = len / 2;
    for (int i = 0; i < n; i += len)
      for (int j = 0; j < hl; ++j) {
        const cd u = a[i + j], v = a[i + j + hl] * tw_[static_cast<size_t>(j * stride)];
        a[i + j] = u + v; a[i + j + hl] = u - v;
      }
  }
  if (inverse) for (int i = 0; i < n; ++i) a[i] = std::conj(a[i]) / static_cast<double>(n);
}

void PhaseVocoderShifter::processFrame() {
  const int N = n_, H = hop_, half = half_;
  const double r = static_cast<double>(ratio_);
  for (int i = 0; i < N; ++i) {   // oldest sample is at inPos_
    int idx = inPos_ + i; if (idx >= N) idx -= N;
    x_[static_cast<size_t>(i)] = cd(static_cast<double>(inRing_[static_cast<size_t>(idx)]) * window_[static_cast<size_t>(i)], 0.0);
  }
  fft(x_.data(), false);
  double maxMag = 0.0;
  for (int k = 0; k <= half; ++k) { mag_[static_cast<size_t>(k)] = std::abs(x_[static_cast<size_t>(k)]); maxMag = std::max(maxMag, mag_[static_cast<size_t>(k)]); }
  std::fill(y_.begin(), y_.end(), cd{});
  std::fill(ymag_.begin(), ymag_.end(), 0.0);
  if (maxMag > 1e-12) {
    // --- peaks: 5-bin local maxima above -60 dB of the frame maximum ---
    int np = 0; const double thr = 1e-3 * maxMag;
    for (int k = 1; k < half; ++k) {
      const double m = mag_[static_cast<size_t>(k)];
      if (m < thr || m <= mag_[static_cast<size_t>(k - 1)] || m < mag_[static_cast<size_t>(k + 1)]) continue;
      if (k >= 2 && m <= mag_[static_cast<size_t>(k - 2)]) continue;
      if (k + 2 <= half && m < mag_[static_cast<size_t>(k + 2)]) continue;
      peaks_[static_cast<size_t>(np++)] = k;
    }
    if (np == 0) { int kb = 0; for (int k = 0; k <= half; ++k) if (mag_[static_cast<size_t>(k)] > mag_[static_cast<size_t>(kb)]) kb = k; peaks_[0] = kb; np = 1; }
    for (int p = 0; p < np; ++p) {
      const int kp = peaks_[static_cast<size_t>(p)];
      const int lo = p == 0 ? 0 : [&] { int b = peaks_[static_cast<size_t>(p - 1)], pk = b;
                                        for (int k = b; k <= kp; ++k) if (mag_[static_cast<size_t>(k)] < mag_[static_cast<size_t>(pk)]) pk = k;
                                        return pk + 1; }();
      const int hi = p == np - 1 ? half : [&] { int b = peaks_[static_cast<size_t>(p + 1)], pk = kp;
                                               for (int k = kp; k <= b; ++k) if (mag_[static_cast<size_t>(k)] < mag_[static_cast<size_t>(pk)]) pk = k;
                                               return pk; }();
      // true frequency of the partial from the phase advance of its peak bin
      const double ph = std::arg(x_[static_cast<size_t>(kp)]);
      const double dev = wrapPi(ph - prevAna_[static_cast<size_t>(kp)] - kTwoPi * kp * H / N);
      const double omega = std::clamp(kTwoPi * kp / N + dev / H, 0.0, kPi);
      const double omega2 = r * omega;
      // shift the whole region by the rounded frequency DIFFERENCE (so ratio 1 is exactly the identity)
      const int dk = static_cast<int>(std::lround((omega2 - omega) * N / kTwoPi));
      const int kdest = kp + dk;
      // synthesis phase of the destination peak: previous phase of that bin + exact phase advance
      auto foldBin = [&](int kk, bool& conj) { int m = kk % N; if (m < 0) m += N; conj = false; if (m > half) { m = N - m; conj = true; } return m; };
      bool pf; const int pm = foldBin(kdest, pf);
      const double prev = pf ? -synPrev_[static_cast<size_t>(pm)] : synPrev_[static_cast<size_t>(pm)];
      const double psi = prev + omega2 * H;
      const cd rot = std::polar(1.0, psi - ph);
      for (int k = lo; k <= hi; ++k) {
        bool cj; const int m = foldBin(k + dk, cj);
        cd v = x_[static_cast<size_t>(k)] * rot; if (cj) v = std::conj(v);
        const double vm = mag_[static_cast<size_t>(k)];
        if (vm > ymag_[static_cast<size_t>(m)]) { ymag_[static_cast<size_t>(m)] = vm; y_[static_cast<size_t>(m)] = v; }
      }
    }
    y_[0] = cd(y_[0].real(), 0.0); y_[static_cast<size_t>(half)] = cd(y_[static_cast<size_t>(half)].real(), 0.0);
    for (int k = 1; k < half; ++k) y_[static_cast<size_t>(N - k)] = std::conj(y_[static_cast<size_t>(k)]);
  }
  for (int k = 0; k <= half; ++k) {
    prevAna_[static_cast<size_t>(k)] = maxMag > 1e-12 ? std::arg(x_[static_cast<size_t>(k)]) : 0.0;
    synPrev_[static_cast<size_t>(k)] = maxMag > 1e-12 ? std::arg(y_[static_cast<size_t>(k)]) : 0.0;
  }
  if (maxMag > 1e-12) {
    fft(y_.data(), true);
    for (int i = 0; i < N; ++i) outAccum_[static_cast<size_t>(i)] += y_[static_cast<size_t>(i)].real() * window_[static_cast<size_t>(i)] * scale_;
  }
  for (int i = 0; i < H; ++i) outFifo_[static_cast<size_t>(i)] = static_cast<float>(outAccum_[static_cast<size_t>(i)]);
  std::copy(outAccum_.begin() + H, outAccum_.end(), outAccum_.begin());
  std::fill(outAccum_.end() - H, outAccum_.end(), 0.0);
}
}  // namespace refractor
