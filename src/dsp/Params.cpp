#include "dsp/Params.hpp"
#include "dsp/Errors.hpp"
namespace refractor {
std::span<const ParamSpec> parameterSpecs() { throw NotImplemented("parameterSpecs"); }
const ParamSpec& spec(ParamId) { throw NotImplemented("spec"); }
}
