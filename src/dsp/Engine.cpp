#include "dsp/Engine.hpp"
#include "dsp/Errors.hpp"
namespace refractor {
struct Engine::Impl {};
Engine::Engine() { throw NotImplemented("Engine::Engine"); }
Engine::~Engine() = default;
void Engine::prepare(double, int) { throw NotImplemented("Engine::prepare"); }
void Engine::reset() { throw NotImplemented("Engine::reset"); }
void Engine::setParameter(ParamId, float) { throw NotImplemented("Engine::setParameter"); }
float Engine::getParameter(ParamId) const { throw NotImplemented("Engine::getParameter"); }
void Engine::process(const float* const*, float* const*, int, int, int) { throw NotImplemented("Engine::process"); }
double Engine::nominalWetLagSeconds() const { throw NotImplemented("Engine::nominalWetLagSeconds"); }
}
