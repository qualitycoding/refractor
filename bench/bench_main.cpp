// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
// Worst-case load: both voices, Magic engaged at max, lowest clock. Usage: refractor_bench <sr> <block> <seconds>
// Prints {"realtime_fraction": cpu_seconds / audio_seconds}. Harness for T-060/T-061.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <vector>
#include "dsp/Engine.hpp"
int main(int argc, char** argv) {
  try {
    const double sr = argc > 1 ? std::atof(argv[1]) : 48000.0; const int block = argc > 2 ? std::atoi(argv[2]) : 128;
    const double secs = argc > 3 ? std::atof(argv[3]) : 60.0;
    using refractor::ParamId; refractor::Engine e; e.prepare(sr, block);
    const float vals[] = {0.9f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.0f};
    for (int i = 0; i < int(ParamId::Count); ++i) e.setParameter(ParamId(i), vals[i]);
    e.reset();
    std::vector<float> l(static_cast<size_t>(block)), r(static_cast<size_t>(block)); uint32_t s = 1u;
    const long blocks = long(secs * sr / block);
    const auto t0 = std::chrono::steady_clock::now();
    for (long b = 0; b < blocks; ++b) {
      for (int n = 0; n < block; ++n) { s = s * 1664525u + 1013904223u; l[size_t(n)] = r[size_t(n)] = float(s >> 8) / 16777216.0f - 0.5f; }
      float* io[2] = { l.data(), r.data() }; e.process(io, io, 2, 2, block);
    }
    const double el = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("{\"realtime_fraction\": %.6f}\n", el / secs); return 0;
  } catch (const std::exception& ex) { std::fprintf(stderr, "bench error: %s\n", ex.what()); return 2; }
}
