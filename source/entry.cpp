#include "cids.h"
#include "controller.h"
#include "processor.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "Inharmonic"

using namespace Steinberg::Vst;
using namespace AudioPlugin;

BEGIN_FACTORY_DEF("mogesystem", "https://mogesystem.net/",
                  "mailto:2096686+Mg32@users.noreply.github.com")

DEF_CLASS2(
    INLINE_UID_FROM_FUID(kInharmonicProcessorUID),
    PClassInfo::kManyInstances,          // cardinality
    kVstAudioEffectClass,                // the component category
    stringPluginName,                    // the plugin name
    Vst::kDistributable,                 //
    InharmonicVST3Category,              // Subcategory for this plugin
    FULL_VERSION_STR,                    // plugin version
    kVstVersionString,                   // the VST 3 SDK version
    InharmonicProcessor::createInstance) // function pointer called when this
                                         // component should be instantiated

DEF_CLASS2(INLINE_UID_FROM_FUID(kInharmonicControllerUID),
           PClassInfo::kManyInstances,    // cardinality
           kVstComponentControllerClass,  // the Controller category
           stringPluginName "Controller", // controller name
           0,                             // not used here
           "",                            // not used here
           FULL_VERSION_STR,              // plugin version
           kVstVersionString,             // the VST 3 SDK version
           InharmonicController::createInstance)

END_FACTORY
