#include "dsp/PitchShifter.hpp"
#include "dsp/Errors.hpp"
namespace refractor {
void DelayLineShifter::prepare(int) { throw NotImplemented("DelayLineShifter::prepare"); }
void DelayLineShifter::reset() { throw NotImplemented("DelayLineShifter::reset"); }
void DelayLineShifter::setRatio(float) { throw NotImplemented("DelayLineShifter::setRatio"); }
float DelayLineShifter::processSample(float) { throw NotImplemented("DelayLineShifter::processSample"); }
float DelayLineShifter::nominalLagSamples() const { throw NotImplemented("DelayLineShifter::nominalLagSamples"); }
}
