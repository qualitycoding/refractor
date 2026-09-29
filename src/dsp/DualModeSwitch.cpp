#include "dsp/DualModeSwitch.hpp"
#include "dsp/Errors.hpp"
namespace refractor {
DualModeSwitch::DualModeSwitch(bool) { throw NotImplemented("DualModeSwitch::DualModeSwitch"); }
void DualModeSwitch::press(double) { throw NotImplemented("DualModeSwitch::press"); }
void DualModeSwitch::release(double) { throw NotImplemented("DualModeSwitch::release"); }
bool DualModeSwitch::engaged() const { throw NotImplemented("DualModeSwitch::engaged"); }
void DualModeSwitch::set(bool) { throw NotImplemented("DualModeSwitch::set"); }
}
