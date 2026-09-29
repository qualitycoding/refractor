#include "dsp/Mapping.hpp"
#include "dsp/Errors.hpp"
namespace refractor::mapping {
float pitchKnobToSemitones(float) { throw NotImplemented("pitchKnobToSemitones"); }
float semitonesToRatio(float) { throw NotImplemented("semitonesToRatio"); }
float levelKnobToGain(float) { throw NotImplemented("levelKnobToGain"); }
double trackingToClockScale(float) { throw NotImplemented("trackingToClockScale"); }
float toneToCutoffHz(float) { throw NotImplemented("toneToCutoffHz"); }
float magicToFeedback(float) { throw NotImplemented("magicToFeedback"); }
}
