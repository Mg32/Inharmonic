#include "controller.h"

#include "cids.h"
#include "parameters.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

namespace AudioPlugin {

tresult PLUGIN_API InharmonicController::initialize(FUnknown *context) {
  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  // register params
  for (size_t i = 0; i < kNumAllParameters; i++) {
    parameters.addParameter(kAllParameters[i].title, STR16("..."),
                            kAllParameters[i].stepCount,
                            kAllParameters[i].defaultValueNormalized,
                            kAllParameters[i].flags, kAllParameters[i].tag);
  }

  return result;
}

tresult PLUGIN_API InharmonicController::getMidiControllerAssignment(
    int32 busIndex, int16 channel, Vst::CtrlNumber midiControllerNumber,
    Vst::ParamID &tag) {

  // associate MIDI event and parameter tag
  switch (midiControllerNumber) {
  case Vst::ControllerNumbers::kCtrlVolume:
    tag = kTagVolume;
    return kResultTrue;
  case Vst::ControllerNumbers::kCtrlExpression:
    tag = kTagExpression;
    return kResultTrue;
  case Vst::ControllerNumbers::kPitchBend:
    tag = kTagPitchBend;
    return kResultTrue;
  case Vst::ControllerNumbers::kCtrlModWheel:
    tag = kTagModWheel;
    return kResultTrue;
  case Vst::ControllerNumbers::kCtrlSustainOnOff:
    tag = kTagSustainPedal;
    return kResultTrue;
  case Vst::ControllerNumbers::kCtrlSustenutoOnOff:
    tag = kTagSostenutoPedal;
    return kResultTrue;
  case Vst::ControllerNumbers::kCtrlSoftPedalOnOff:
    tag = kTagSoftPedal;
    return kResultTrue;
  }

  // no associated parameters
  return kResultFalse;
}

tresult PLUGIN_API InharmonicController::terminate() {
  return EditControllerEx1::terminate();
}

tresult PLUGIN_API InharmonicController::setComponentState(IBStream *state) {
  if (!state)
    return kResultFalse;

  return kResultOk;
}

tresult PLUGIN_API InharmonicController::setState(IBStream *state) {
  IBStreamer streamer(state, kLittleEndian);

  for (size_t i = 0; i < kNumAllParameters; i++) {
    double value = 0.5;
    streamer.readDouble(value);
    auto *param = parameters.getParameter(kAllParameters[i].tag);
    if (param) {
      param->setNormalized(value);
    }
  }

  return kResultTrue;
}

tresult PLUGIN_API InharmonicController::getState(IBStream *state) {
  IBStreamer streamer(state, kLittleEndian);

  for (size_t i = 0; i < kNumAllParameters; i++) {
    auto *param = parameters.getParameter(kAllParameters[i].tag);
    if (param) {
      streamer.writeDouble(param->getNormalized());
    }
  }

  return kResultTrue;
}

IPlugView *PLUGIN_API InharmonicController::createView(FIDString name) {
  // Here the Host wants to open your editor (if you have one)
  if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
    // create your editor here and return a IPlugView ptr of it
    auto *view = new VSTGUI::VST3Editor(this, "view", "editor.uidesc");
    return view;
  }
  return nullptr;
}

} // namespace AudioPlugin
