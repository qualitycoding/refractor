#include "plugin/PluginEditor.h"
#include <cmath>
#include "dsp/Calibration.hpp"
#include "dsp/DualModeSwitch.hpp"
#include "dsp/Mapping.hpp"
#include "dsp/Params.hpp"
#include "plugin/PluginProcessor.h"

namespace refractor {
namespace {
const juce::Colour kBg{0xff12161a}, kPanel{0xff1c232a}, kAccent{0xff3cc7b7}, kWarm{0xffe3a93b}, kText{0xffdbe4ec}, kDim{0xff7b8894};
double nowSeconds() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }

juce::String pitchText(double v) {
  const double st = std::round(static_cast<double>(mapping::pitchKnobToSemitones(static_cast<float>(v))) * 10.0) / 10.0;
  return juce::String(st == 0.0 ? 0.0 : st, 1) + " st";   // never prints "-0.0"
}
juce::String levelText(double v) {
  const float g = mapping::levelKnobToGain(static_cast<float>(v));
  return g <= 1.0e-4f ? juce::String("off") : juce::String(20.0 * std::log10(static_cast<double>(g)), 1) + " dB";
}
juce::String lagText(double v) {
  const double fInt = calib::kNominalClockHz * mapping::trackingToClockScale(static_cast<float>(v));
  return juce::String(juce::roundToInt(1000.0 * calib::kWindowSamples / fInt)) + " ms";
}
juce::String toneText(double v) {
  const float hz = mapping::toneToCutoffHz(static_cast<float>(v));
  return std::isinf(hz) ? juce::String("open") : juce::String(hz / 1000.0f, 1) + " kHz";
}
juce::String magicText(double v) { return juce::String(juce::roundToInt(v * 100.0)) + " %"; }
}  // namespace

// Footswitch: tap toggles, hold acts momentarily (D-013). `invert` maps "engaged" to the opposite parameter state (bypass).
struct RefractorEditor::Foot : public juce::Component {
  Foot(juce::RangedAudioParameter* p, bool invertParam, juce::String caption, juce::Colour ledColour)
      : param(p), invert(invertParam), text(std::move(caption)), led(ledColour), sw(paramEngaged()) {}
  bool paramEngaged() const { return param != nullptr && ((param->getValue() >= 0.5f) != invert); }
  void apply() { if (param != nullptr) param->setValueNotifyingHost((sw.engaged() != invert) ? 1.0f : 0.0f); }
  void sync() { if (!down && sw.engaged() != paramEngaged()) { sw.set(paramEngaged()); repaint(); } }
  void mouseDown(const juce::MouseEvent&) override {
    if (param == nullptr) return;
    param->beginChangeGesture(); down = true; sw.press(nowSeconds()); apply(); repaint();
  }
  void mouseUp(const juce::MouseEvent&) override {
    if (param == nullptr || !down) return;
    sw.release(nowSeconds()); down = false; apply(); param->endChangeGesture(); repaint();
  }
  void paint(juce::Graphics& g) override {
    const auto b = getLocalBounds().toFloat();
    const float d = juce::jmin(b.getWidth(), b.getHeight() * 0.62f), cx = b.getCentreX();
    const auto ledBox = juce::Rectangle<float>(0, 0, d * 0.22f, d * 0.22f).withCentre({cx, b.getY() + d * 0.14f});
    g.setColour(sw.engaged() ? led : kDim.withAlpha(0.35f)); g.fillEllipse(ledBox);
    const auto pad = juce::Rectangle<float>(0, 0, d, d).withCentre({cx, b.getY() + d * 0.14f + d * 0.6f});
    g.setColour(kPanel.brighter(down ? 0.35f : 0.12f)); g.fillEllipse(pad);
    g.setColour(kDim); g.drawEllipse(pad, 2.0f);
    g.setColour(kText); g.setFont(juce::Font(juce::FontOptions(b.getHeight() * 0.15f, juce::Font::bold)));
    g.drawText(text, getLocalBounds().removeFromBottom(static_cast<int>(b.getHeight() * 0.2f)), juce::Justification::centred);
  }
  juce::RangedAudioParameter* param; bool invert; juce::String text; juce::Colour led; DualModeSwitch sw; bool down = false;
};

void RefractorEditor::addKnob(Knob& k, const char* key, const char* caption) {
  auto& s = k.slider;
  s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 16);
  s.setColour(juce::Slider::rotarySliderFillColourId, kAccent);
  s.setColour(juce::Slider::thumbColourId, kWarm);
  s.setColour(juce::Slider::textBoxTextColourId, kText);
  s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
  addAndMakeVisible(s);
  k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      static_cast<RefractorProcessor&>(processor).state(), key, s);
  // The attachment installs its own text conversion, so ours must be set afterwards.
  const juce::String k_(key);
  if (k_ == "pitch" || k_ == "pitch_exp") s.textFromValueFunction = pitchText;
  else if (k_ == "primary" || k_ == "secondary") s.textFromValueFunction = levelText;
  else if (k_ == "tracking") s.textFromValueFunction = lagText;
  else if (k_ == "tone") s.textFromValueFunction = toneText;
  else s.textFromValueFunction = magicText;
  s.updateText();
  k.label.setText(caption, juce::dontSendNotification);
  k.label.setJustificationType(juce::Justification::centred);
  k.label.setColour(juce::Label::textColourId, kText);
  addAndMakeVisible(k.label);
}

RefractorEditor::RefractorEditor(RefractorProcessor& p) : juce::AudioProcessorEditor(p) {
  static constexpr const char* keys[7] = {"pitch", "primary", "secondary", "tracking", "tone", "magic", "pitch_exp"};
  static constexpr const char* names[7] = {"Pitch", "Primary", "Secondary", "Tracking", "Tone", "Magic", "Pitch Exp"};
  for (std::size_t i = 0; i < knobs_.size(); ++i) addKnob(knobs_[i], keys[i], names[i]);
  expButton_.setColour(juce::ToggleButton::textColourId, kText);
  expButton_.setColour(juce::ToggleButton::tickColourId, kAccent);
  addAndMakeVisible(expButton_);
  expAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state(), "exp_enabled", expButton_);
  activeFoot_ = std::make_unique<Foot>(p.state().getParameter("bypass"), true, "Active", kAccent);
  magicFoot_ = std::make_unique<Foot>(p.state().getParameter("magic_on"), false, "Magic", kWarm);
  addAndMakeVisible(*activeFoot_); addAndMakeVisible(*magicFoot_);
  setResizable(true, true);
  setResizeLimits(640, 360, 1280, 720);
  if (auto* c = getConstrainer()) c->setFixedAspectRatio(16.0 / 9.0);
  setSize(640, 360);
  startTimerHz(20);   // reflect host automation on the footswitch LEDs
}
RefractorEditor::~RefractorEditor() { stopTimer(); }
void RefractorEditor::timerCallback() { if (activeFoot_) activeFoot_->sync(); if (magicFoot_) magicFoot_->sync(); }

void RefractorEditor::paint(juce::Graphics& g) {
  g.fillAll(kBg);
  auto area = getLocalBounds().toFloat().reduced(getWidth() * 0.012f);
  g.setColour(kPanel); g.fillRoundedRectangle(area, 14.0f);
  g.setColour(kAccent.withAlpha(0.55f)); g.drawRoundedRectangle(area, 14.0f, 1.5f);
  g.setColour(kText); g.setFont(juce::Font(juce::FontOptions(getHeight() * 0.085f, juce::Font::bold)));
  g.drawText("REFRACTOR", getLocalBounds().removeFromTop(static_cast<int>(getHeight() * 0.16f)).withTrimmedLeft(getWidth() / 25), juce::Justification::centredLeft);
  g.setColour(kDim); g.setFont(juce::Font(juce::FontOptions(getHeight() * 0.036f)));
  g.drawText("polyphonic pitch warper", getLocalBounds().removeFromTop(static_cast<int>(getHeight() * 0.16f)).withTrimmedRight(getWidth() / 25), juce::Justification::centredRight);
}

void RefractorEditor::resized() {
  auto r = getLocalBounds().reduced(getWidth() / 40);
  r.removeFromTop(static_cast<int>(getHeight() * 0.09f));
  auto feet = r.removeFromBottom(static_cast<int>(getHeight() * 0.25f));
  const int colW = r.getWidth() / 4;
  auto place = [&](Knob& k, juce::Rectangle<int> cell) {
    k.label.setBounds(cell.removeFromTop(static_cast<int>(getHeight() * 0.05f)));
    k.slider.setBounds(cell);
  };
  auto row1 = r.removeFromTop(r.getHeight() / 2), row2 = r;
  for (int i = 0; i < 4; ++i) place(knobs_[static_cast<std::size_t>(i)], row1.removeFromLeft(colW).reduced(2));
  for (int i = 4; i < 7; ++i) place(knobs_[static_cast<std::size_t>(i)], row2.removeFromLeft(colW).reduced(2));
  expButton_.setBounds(row2.reduced(getWidth() / 60));
  const int fw = feet.getWidth() / 2;
  activeFoot_->setBounds(feet.removeFromLeft(fw).reduced(getWidth() / 40, 0));
  magicFoot_->setBounds(feet.reduced(getWidth() / 40, 0));
}
}  // namespace refractor
