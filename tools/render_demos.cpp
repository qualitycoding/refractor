// Renders the G-003 listening bundle: deterministic synthetic inputs (Karplus-Strong plucks) through the DSP engine with
// eight presets. Usage: render_demos <output-dir>. Writes <dir>/input/*.wav, <dir>/*.wav (48 kHz, 24-bit mono) and
// <dir>/PRESETS.md. Not part of the plugin; outside src/ so the no-I/O policy (T-051) does not apply.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "dsp/Engine.hpp"
#include "dsp/Mapping.hpp"

namespace {
using Vec = std::vector<float>;
constexpr double kFs = 48000.0;

// Deterministic Karplus-Strong pluck mixed into `out` at time `t0` seconds.
void pluck(Vec& out, double hz, double t0, double seconds, float amp, uint32_t seed) {
  const int len = std::max(2, static_cast<int>(std::lround(kFs / hz)));
  std::vector<float> line(static_cast<size_t>(len));
  uint32_t s = seed;
  for (auto& v : line) { s = s * 1664525u + 1013904223u; v = (static_cast<float>(s >> 8) / 8388608.0f - 1.0f) * amp; }
  const size_t start = static_cast<size_t>(t0 * kFs), n = static_cast<size_t>(seconds * kFs);
  int idx = 0;
  for (size_t i = 0; i < n && start + i < out.size(); ++i) {
    const int nxt = (idx + 1) % len;
    const float y = line[static_cast<size_t>(idx)];
    line[static_cast<size_t>(idx)] = 0.4985f * (y + line[static_cast<size_t>(nxt)]);   // decay + damping
    out[start + i] += y; idx = nxt;
  }
}
void writeWav(const std::filesystem::path& p, const Vec& x) {
  std::ofstream f(p, std::ios::binary);
  auto w32 = [&](uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
  auto w16 = [&](uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };
  const uint32_t dataBytes = static_cast<uint32_t>(x.size() * 3);
  f.write("RIFF", 4); w32(36 + dataBytes); f.write("WAVEfmt ", 8); w32(16); w16(1); w16(1); w32(48000); w32(48000 * 3); w16(3); w16(24);
  f.write("data", 4); w32(dataBytes);
  for (float v : x) {
    const int32_t q = static_cast<int32_t>(std::lround(std::clamp(static_cast<double>(v), -1.0, 1.0) * 8388607.0));
    const char b[3] = {static_cast<char>(q & 0xff), static_cast<char>((q >> 8) & 0xff), static_cast<char>((q >> 16) & 0xff)};
    f.write(b, 3);
  }
}
double peakDb(const Vec& x) { float pk = 0; for (float v : x) pk = std::max(pk, std::fabs(v)); return pk > 0 ? 20.0 * std::log10(static_cast<double>(pk)) : -200.0; }

struct Preset { const char* name; const char* input; float pitch, primary, secondary, tracking, tone, magic; bool magicOn; double tail; const char* note; };
}  // namespace

int main(int argc, char** argv) {
  using refractor::ParamId;
  const std::filesystem::path dir = argc > 1 ? argv[1] : "demo";
  std::filesystem::create_directories(dir / "input");
  // ---- inputs ----
  Vec phrase(static_cast<size_t>(7.0 * kFs), 0.0f), chord(static_cast<size_t>(5.0 * kFs), 0.0f), bass(static_cast<size_t>(6.0 * kFs), 0.0f), note(static_cast<size_t>(1.5 * kFs), 0.0f);
  const double pn[] = {110.0, 164.81, 220.0, 261.63, 329.63, 261.63, 220.0, 164.81, 110.0, 146.83, 196.0, 246.94};
  for (int i = 0; i < 12; ++i) pluck(phrase, pn[i], 0.15 + 0.5 * i, 1.4, 0.35f, 100u + static_cast<uint32_t>(i));
  const double cn[] = {98.0, 196.0, 246.94, 392.0};
  for (int i = 0; i < 4; ++i) pluck(chord, cn[i], 0.1 + 0.035 * i, 4.0, 0.25f, 200u + static_cast<uint32_t>(i));
  const double bn[] = {55.0, 55.0, 65.41, 73.42, 82.41, 73.42, 65.41, 55.0};
  for (int i = 0; i < 8; ++i) pluck(bass, bn[i], 0.1 + 0.7 * i, 1.2, 0.45f, 300u + static_cast<uint32_t>(i));
  pluck(note, 220.0, 0.05, 1.4, 0.4f, 400u);
  writeWav(dir / "input" / "pluck_phrase.wav", phrase); writeWav(dir / "input" / "chord_G.wav", chord);
  writeWav(dir / "input" / "bass_line.wav", bass);     writeWav(dir / "input" / "single_note_A3.wav", note);
  const struct { const char* n; const Vec* v; } inputs[] = {{"pluck_phrase", &phrase}, {"chord_G", &chord}, {"bass_line", &bass}, {"single_note_A3", &note}};

  const Preset presets[] = {
    {"01_noon_chorus",          "pluck_phrase",   0.5f, 0.7f, 0.0f, 0.8f, 1.0f, 0.0f, false, 1.0, "unison: the always-on detune makes noon a chorus"},
    {"02_slapback",             "pluck_phrase",   0.5f, 0.7f, 0.0f, 0.2f, 1.0f, 0.0f, false, 1.0, "Tracking 0.2 = long lag, clearly separate wet copy"},
    {"03_fourth_down_harmony",  "chord_G",        0.0f, 0.7f, 0.0f, 0.5f, 1.0f, 0.0f, false, 1.0, "Pitch fully CCW (-5 st) on a chord"},
    {"04_third_up_plus_octave", "pluck_phrase",   1.0f, 0.7f, 0.5f, 0.5f, 1.0f, 0.0f, false, 1.0, "Pitch CW (+4 st) with Secondary adding the octave above"},
    {"05_magic_low_repeats",    "pluck_phrase",   0.5f, 0.7f, 0.0f, 0.5f, 1.0f, 0.35f, true, 3.0, "Magic low at noon: a few regenerating repeats"},
    {"06_magic_ascending",      "single_note_A3", 1.0f, 0.7f, 0.0f, 0.5f, 1.0f, 0.6f,  true, 4.0, "Magic with Pitch above noon: trails climb"},
    {"07_magic_descending",     "single_note_A3", 0.0f, 0.7f, 0.0f, 0.5f, 1.0f, 0.6f,  true, 4.0, "Magic with Pitch below noon: trails fall"},
    {"08_max_self_oscillation", "single_note_A3", 0.8f, 1.0f, 1.0f, 0.3f, 0.6f, 1.0f,  true, 8.0, "Everything up: bounded self-oscillation (loud by design)"},
    {"09_bass_fourth_down",     "bass_line",      0.0f, 0.7f, 0.0f, 0.3f, 0.7f, 0.0f, false, 1.5, "Low notes are where a short window smears most: listen for clarity"},
  };
  std::ofstream md(dir / "PRESETS.md");
  md << "# Demo presets (rendered by tools/render_demos)\n\n48 kHz, 24-bit mono WAV. Output = dry + wet exactly as the plugin produces it; nothing is normalised.\n"
        "Knob values are 0-1 plugin parameter values; semitones, gain and lag are what they map to.\n\n"
        "| file | input | Pitch | Primary | Secondary | Tracking (lag) | Tone | Magic | Magic on | peak | note |\n|---|---|---|---|---|---|---|---|---|---|---|\n";
  for (const auto& pr : presets) {
    const Vec* in = nullptr; for (const auto& i : inputs) if (std::string(i.n) == pr.input) in = i.v;
    refractor::Engine e; e.prepare(kFs, 512);
    e.setParameter(ParamId::Pitch, pr.pitch); e.setParameter(ParamId::Primary, pr.primary); e.setParameter(ParamId::Secondary, pr.secondary);
    e.setParameter(ParamId::Tracking, pr.tracking); e.setParameter(ParamId::Tone, pr.tone); e.setParameter(ParamId::Magic, pr.magic);
    e.setParameter(ParamId::MagicEngaged, pr.magicOn ? 1.0f : 0.0f); e.reset();
    Vec x = *in; x.resize(x.size() + static_cast<size_t>(pr.tail * kFs), 0.0f); Vec y(x.size());
    for (size_t p = 0; p < x.size(); p += 256) {
      const int n = static_cast<int>(std::min<size_t>(256, x.size() - p)); const float* i[1] = {x.data() + p}; float* o[1] = {y.data() + p};
      e.process(i, o, 1, 1, n);
    }
    writeWav(dir / (std::string(pr.name) + ".wav"), y);
    char row[768];
    std::snprintf(row, sizeof row, "| %s.wav | %s | %.2f (%+.1f st) | %.2f (%.1f dB) | %.2f | %.2f (%.0f ms) | %.2f | %.2f | %s | %.1f dBFS | %s |\n", pr.name, pr.input,
                  pr.pitch, refractor::mapping::pitchKnobToSemitones(pr.pitch), pr.primary, 20.0 * std::log10(refractor::mapping::levelKnobToGain(pr.primary)),
                  pr.secondary, pr.tracking, 1000.0 * e.nominalWetLagSeconds(), pr.tone, pr.magic, pr.magicOn ? "yes" : "no", peakDb(y), (std::string(pr.note) + (peakDb(y) > 0.0 ? " **[peak above 0 dBFS: this WAV is hard-clipped; the plugin output is not]**" : "")).c_str());
    md << row;
    std::printf("%s.wav  peak %.1f dBFS\n", pr.name, peakDb(y));
  }
  return 0;
}
