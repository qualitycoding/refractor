#include <cstdio>
#include "support/TestSupport.hpp"
using namespace refractor; using namespace rt;
int main() {
  const double fs = 48000; const Vec in = sine(440, fs, 1.5);
  for (float k : {0.0f, 0.1f, 0.25f, 0.4f, 0.6f, 0.75f, 0.9f, 1.0f}) {
    Engine e; e.prepare(fs, 512); neutral(e);
    e.setParameter(ParamId::Pitch, k); e.setParameter(ParamId::Primary, 0.0f); e.setParameter(ParamId::Secondary, 1.0f); e.reset();
    const Vec w = wetOf(run(e, in), in);
    const double oct = k > 0.5f ? 2.0 : 0.5, want = 440.0 * oct * ratioOf(expectedSemitones(k));
    const double f = dominantHz(w, fs, size_t(0.5 * fs));
    // also report the strongest peaks within +-15% of expectation to see sidebands
    std::printf("knob %.2f want %.1f got %.1f err %+.2f%%  ratioVoice=%.3f\n", k, want, f, 100*(f/want-1), oct*ratioOf(expectedSemitones(k)));
  }
}
