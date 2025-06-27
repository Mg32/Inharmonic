#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace AudioPlugin {
static const Steinberg::FUID kInharmonicProcessorUID(0xCE2677E6, 0x62F35BE2,
                                                     0xAC186EEC, 0x987CC171);
static const Steinberg::FUID kInharmonicControllerUID(0x8E2CBC92, 0x74B45B44,
                                                      0xB7F390F7, 0x89EBF41A);

#define InharmonicVST3Category "Instrument"

} // namespace AudioPlugin
