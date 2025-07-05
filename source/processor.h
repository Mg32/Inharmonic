#pragma once
#include "dsp/effect.h"
#include "dsp/inharmonic.h"

#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

#include <map>

namespace AudioPlugin {

class InharmonicProcessor : public Steinberg::Vst::AudioEffect {
public:
  InharmonicProcessor();
  ~InharmonicProcessor() SMTG_OVERRIDE;

  // Create function
  static Steinberg::FUnknown *createInstance(void * /*context*/) {
    return (Steinberg::Vst::IAudioProcessor *)new InharmonicProcessor;
  }

  // AudioEffect overrides:
  /* Called at first after constructor */
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *context)
      SMTG_OVERRIDE;

  /* Called at the end before destructor */
  Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

  /* Try to set (host => plugin) a wanted bus arrangement */
  Steinberg::tresult PLUGIN_API setBusArrangements(
      Steinberg::Vst::SpeakerArrangement *inputs, Steinberg::int32 numIns,
      Steinberg::Vst::SpeakerArrangement *outputs,
      Steinberg::int32 numOuts) SMTG_OVERRIDE;

  /* Switch the plugin on/off */
  Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

  /* Will be called before any process call */
  Steinberg::tresult PLUGIN_API
  setupProcessing(Steinberg::Vst::ProcessSetup &newSetup) SMTG_OVERRIDE;

  /* Asks if a given sample size is supported see SymbolicSampleSizes */
  Steinberg::tresult PLUGIN_API
  canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

  /* The process call */
  Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData &data)
      SMTG_OVERRIDE;

  /* For persistence */
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;

protected:
  Inharmonic::InharmonicSynth _synth;
  Effect::SampleDivider<float> _divider32;
  Effect::SampleDivider<double> _divider64;
  Effect::BiquadEQ<float> _biquadEQ32;
  Effect::BiquadEQ<double> _biquadEQ64;
  Effect::Chorus<float> _chorus32;
  Effect::Chorus<double> _chorus64;
  Effect::Reverb<float> _reverb32;
  Effect::Reverb<double> _reverb64;

  std::map<Steinberg::Vst::ParamID, Steinberg::Vst::ParamValue> _param = {};
  std::map<Steinberg::int32, Steinberg::Vst::Event> _scheduledEvents = {};

  void applyParameter(Steinberg::Vst::ParamID tag,
                      Steinberg::Vst::ParamValue value);
};

} // namespace AudioPlugin
