#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace AudioPlugin {

class InharmonicController : public Steinberg::Vst::EditControllerEx1,
                             public Steinberg::Vst::IMidiMapping {
public:
  InharmonicController() = default;
  ~InharmonicController() SMTG_OVERRIDE = default;

  static Steinberg::FUnknown *createInstance(void *) {
    return (Steinberg::Vst::IEditController *)new InharmonicController;
  }

  // from IPluginBase
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *context)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

  // from EditController
  Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;
  Steinberg::IPlugView *PLUGIN_API createView(Steinberg::FIDString name)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;

  // from EditController
  virtual Steinberg::tresult PLUGIN_API getMidiControllerAssignment(
      Steinberg::int32 busIndex, Steinberg::int16 channel,
      Steinberg::Vst::CtrlNumber midiControllerNumber,
      Steinberg::Vst::ParamID &tag) SMTG_OVERRIDE;

  // Interface
  OBJ_METHODS(InharmonicController, EditController)
  DEFINE_INTERFACES
  // Here you can add more supported VST3 interfaces
  // DEF_INTERFACE (Vst::IXXX)
  DEF_INTERFACE(IMidiMapping)
  END_DEFINE_INTERFACES(EditController)
  DELEGATE_REFCOUNT(EditController)

protected:
};

} // namespace AudioPlugin
