#include "dsp/ClockDomain.hpp"
namespace refractor {
void ClockDomain::prepare(double, int) { throw NotImplemented("ClockDomain::prepare"); }
void ClockDomain::reset() { throw NotImplemented("ClockDomain::reset"); }
void ClockDomain::setClockScale(double) { throw NotImplemented("ClockDomain::setClockScale"); }
double ClockDomain::internalRate() const { throw NotImplemented("ClockDomain::internalRate"); }
}
