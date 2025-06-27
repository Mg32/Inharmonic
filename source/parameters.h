#pragma once
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace AudioPlugin {

// MIDI params
static const Steinberg::Vst::ParamID kTagVolume = 1;
static const Steinberg::Vst::ParamID kTagExpression = 2;
static const Steinberg::Vst::ParamID kTagPitchBend = 3;
static const Steinberg::Vst::ParamID kTagModWheel = 4;
static const Steinberg::Vst::ParamID kTagSustainPedal = 5;
static const Steinberg::Vst::ParamID kTagSostenutoPedal = 6;
static const Steinberg::Vst::ParamID kTagSoftPedal = 7;

// voice params
static const Steinberg::Vst::ParamID kTagOutVol = 100;
static const Steinberg::Vst::ParamID kTagOscMix = 101;
static const Steinberg::Vst::ParamID kTagIsRandomPhase = 102;
static const Steinberg::Vst::ParamID kTagInharmonic = 103;
static const Steinberg::Vst::ParamID kTagInharmonicSubscale = 104;
static const Steinberg::Vst::ParamID kTagInharmKeyFollow = 105;
static const Steinberg::Vst::ParamID kTagAmpEnvA = 106;
static const Steinberg::Vst::ParamID kTagAmpEnvD = 107;
static const Steinberg::Vst::ParamID kTagAmpEnvS = 108;
static const Steinberg::Vst::ParamID kTagAmpEnvR = 109;
static const Steinberg::Vst::ParamID kTagAmpVeloSens = 110;
static const Steinberg::Vst::ParamID kTagVibDelay = 111;
static const Steinberg::Vst::ParamID kTagVibDepth = 112;
static const Steinberg::Vst::ParamID kTagVibSpeed = 113;
static const Steinberg::Vst::ParamID kTagFiltType = 114;
static const Steinberg::Vst::ParamID kTagFiltCutoff = 115;
static const Steinberg::Vst::ParamID kTagFiltReso = 116;
static const Steinberg::Vst::ParamID kTagFiltEnvAmount = 117;
static const Steinberg::Vst::ParamID kTagFiltEnvA = 118;
static const Steinberg::Vst::ParamID kTagFiltEnvD = 119;
static const Steinberg::Vst::ParamID kTagFiltEnvS = 120;
static const Steinberg::Vst::ParamID kTagFiltEnvR = 121;
static const Steinberg::Vst::ParamID kTagFiltKeyFollow = 122;

// effect params
static const Steinberg::Vst::ParamID kTagEqF = 200;
static const Steinberg::Vst::ParamID kTagEqG = 201;
static const Steinberg::Vst::ParamID kTagEqQ = 202;
static const Steinberg::Vst::ParamID kTagChorusTime = 203;
static const Steinberg::Vst::ParamID kTagChorusDepth = 204;
static const Steinberg::Vst::ParamID kTagChorusSpeed = 205;
static const Steinberg::Vst::ParamID kTagChorusAmount = 206;
static const Steinberg::Vst::ParamID kTagSampleDivision = 207;
static const Steinberg::Vst::ParamID kTagReverbTime = 208;
static const Steinberg::Vst::ParamID kTagReverbMix = 209;

struct ParamSet {
  Steinberg::Vst::ParamID tag;
  const Steinberg::Vst::TChar *title;
  Steinberg::int32 stepCount;
  Steinberg::Vst::ParamValue defaultValueNormalized;
  Steinberg::int32 flags;
};

static const ParamSet kAllParameters[] = {
    // midi params
    {kTagVolume, STR16("Volume"), 0, 1.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},
    {kTagExpression, STR16("Expression"), 0, 1.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},
    {kTagPitchBend, STR16("PitchBend"), 0, 0.5,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},
    {kTagModWheel, STR16("ModWheel"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},
    {kTagSustainPedal, STR16("SustainPedal"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},
    {kTagSostenutoPedal, STR16("SostenutoPedal"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},
    {kTagSoftPedal, STR16("SoftPedal"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate |
         Steinberg::Vst::ParameterInfo::kIsProgramChange},

    // voice params
    {kTagOutVol, STR16("OutVol"), 0, 0.5, Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagOscMix, STR16("OscMix"), 0, 0.3, Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagIsRandomPhase, STR16("IsRandomPhase"), 1, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagInharmonic, STR16("Inharmonic"), 0, 0.3,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagInharmonicSubscale, STR16("Subscale"), 0, 0.25,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagInharmKeyFollow, STR16("InharmKeyFollow"), 0, 0.25,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagAmpEnvA, STR16("AmpEnvA"), 0, 0.15,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagAmpEnvD, STR16("AmpEnvD"), 0, 0.7,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagAmpEnvS, STR16("AmpEnvS"), 0, 0.25,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagAmpEnvR, STR16("AmpEnvR"), 0, 0.25,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagAmpVeloSens, STR16("AmpVeloSens"), 0, 0.7,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagVibDelay, STR16("VibDelay"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagVibDepth, STR16("VibDepth"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagVibSpeed, STR16("VibSpeed"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltType, STR16("FiltType"), 5, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltCutoff, STR16("FiltCutoff"), 0, 1.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltReso, STR16("FiltReso"), 0, 0.5,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltEnvAmount, STR16("FiltEnvAmount"), 0, 0.5,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltEnvA, STR16("FiltEnvA"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltEnvD, STR16("FiltEnvD"), 0, 0.5,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltEnvS, STR16("FiltEnvS"), 0, 1.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltEnvR, STR16("FiltEnvR"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagFiltKeyFollow, STR16("FiltKeyFollow"), 0, 1.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},

    // effect params
    {kTagEqF, STR16("EqF"), 0, 0.5, Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagEqG, STR16("EqG"), 0, 0.5, Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagEqQ, STR16("EqQ"), 0, 0.5, Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagChorusTime, STR16("ChorusTime"), 0, 0.75,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagChorusDepth, STR16("ChorusDepth"), 0, 0.125,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagChorusSpeed, STR16("ChorusSpeed"), 0, 0.35,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagChorusAmount, STR16("ChorusAmount"), 0, 0.6,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagSampleDivision, STR16("SampleDivision"), 0, 0.0,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagReverbTime, STR16("ReverbTime"), 0, 0.75,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
    {kTagReverbMix, STR16("ReverbMix"), 0, 0.15,
     Steinberg::Vst::ParameterInfo::kCanAutomate},
};
static const size_t kNumAllParameters =
    sizeof(kAllParameters) / sizeof(kAllParameters[0]);

} // namespace AudioPlugin
