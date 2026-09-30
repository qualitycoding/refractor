#include "dsp/Params.hpp"
#include <array>
namespace refractor {
namespace {
// D-003 — frozen public interface. Keys are persisted in host sessions: never rename or reorder.
constexpr std::array<ParamSpec, static_cast<size_t>(ParamId::Count)> kSpecs{{
    {ParamId::Pitch,        "pitch",       "Pitch",       0.0f, 1.0f, 0.5f, false},
    {ParamId::PitchExp,     "pitch_exp",   "Pitch Exp",   0.0f, 1.0f, 0.5f, false},
    {ParamId::ExpEnabled,   "exp_enabled", "Exp Enabled", 0.0f, 1.0f, 0.0f, true},
    {ParamId::Primary,      "primary",     "Primary",     0.0f, 1.0f, 0.7f, false},
    {ParamId::Secondary,    "secondary",   "Secondary",   0.0f, 1.0f, 0.0f, false},
    {ParamId::Tracking,     "tracking",    "Tracking",    0.0f, 1.0f, 0.8f, false},
    {ParamId::Tone,         "tone",        "Tone",        0.0f, 1.0f, 1.0f, false},
    {ParamId::Magic,        "magic",       "Magic",       0.0f, 1.0f, 0.3f, false},
    {ParamId::MagicEngaged, "magic_on",    "Magic On",    0.0f, 1.0f, 0.0f, true},
    {ParamId::Bypass,       "bypass",      "Bypass",      0.0f, 1.0f, 0.0f, true},
}};
}  // namespace

std::span<const ParamSpec> parameterSpecs() { return {kSpecs.data(), kSpecs.size()}; }
const ParamSpec& spec(ParamId id) { return kSpecs[static_cast<size_t>(id)]; }
}  // namespace refractor
