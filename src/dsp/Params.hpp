#pragma once
#include <span>
// PUBLIC INTERFACE (D-003). Parameter keys are persisted in host sessions: never rename/reorder after S-014.
namespace refractor {
enum class ParamId : int { Pitch = 0, PitchExp, ExpEnabled, Primary, Secondary, Tracking, Tone, Magic,
                           MagicEngaged, Bypass, Count };
struct ParamSpec {
  ParamId     id;
  const char* key;      // stable host-facing ID
  const char* name;     // display name
  float       min, max, defaultValue;
  bool        isBool;
};
/// Returns exactly ParamId::Count specs, ordered by ParamId. Values per D-003.
std::span<const ParamSpec> parameterSpecs();
const ParamSpec& spec(ParamId id);
}
