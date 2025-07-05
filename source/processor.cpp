#include "processor.h"

#include "cids.h"
#include "parameters.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace {

static double rangeMap(double x, double xWarp, double yMin, double yMax) {
  const double t = pow(std::max(0.0, std::min(1.0, x)), xWarp);
  return t * (yMax - yMin) + yMin;
}

} // namespace

namespace AudioPlugin {
InharmonicProcessor::InharmonicProcessor() {
  setControllerClass(kInharmonicControllerUID);
}

InharmonicProcessor::~InharmonicProcessor() {}

tresult PLUGIN_API InharmonicProcessor::initialize(FUnknown *context) {
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  addAudioOutput(STR16("Stereo Out"), Vst::SpeakerArr::kStereo);
  addEventInput(STR16("Event In"), 1);

  for (size_t i = 0; i < kNumAllParameters; i++) {
    applyParameter(kAllParameters[i].tag,
                   kAllParameters[i].defaultValueNormalized);
  }

  return kResultOk;
}

tresult PLUGIN_API InharmonicProcessor::terminate() {
  return AudioEffect::terminate();
}

tresult PLUGIN_API InharmonicProcessor::setBusArrangements(
    Vst::SpeakerArrangement *inputs, int32 numIns,
    Vst::SpeakerArrangement *outputs, int32 numOuts) {
  // supported arrangements
  if (numOuts >= 1 && outputs[0] == Vst::SpeakerArr::kStereo) {
    return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
  }

  // unsupported arrangement
  return kResultFalse;
}

tresult PLUGIN_API InharmonicProcessor::setActive(TBool state) {
  return AudioEffect::setActive(state);
}

void InharmonicProcessor::applyParameter(Steinberg::Vst::ParamID tag,
                                         Steinberg::Vst::ParamValue value) {
  _param[tag] = value;
  switch (tag) {
  case kTagVolume:
    _synth.setVolume(value);
    break;
  case kTagExpression:
    _synth.setExpression(value);
    break;
  case kTagPitchBend:
    _synth.setPitchBend(rangeMap(value, 1.0, -1.0, 1.0));
    break;
  case kTagModWheel:
    _synth.setModWheel(rangeMap(value, 1.0, -1.0, 1.0));
    break;
  case kTagSustainPedal:
    _synth.setSustainPedal(value >= 0.99);
    break;
  case kTagSostenutoPedal:
    _synth.setSostenutoPedal(value >= 0.99);
    break;
  case kTagSoftPedal:
    _synth.setSoftPedal(value);
    break;

  case kTagOutVol:
    _synth.setOutVol(0.5 * value * value);
    break;
  case kTagOscMix:
    _synth.setOscMix(value);
    break;
  case kTagIsRandomPhase:
    _synth.setIsRandomPhase(value >= 0.5);
    break;
  case kTagInharmonic:
    _synth.setInharmonic(0.5 * value * value * value * value * value);
    break;
  case kTagInharmonicSubscale:
    _synth.setInharmonicSubscale(value);
    break;
  case kTagInharmKeyFollow:
    _synth.setInharmKeyFollow(value);
    break;
  case kTagAmpEnvA:
    _synth.setAmpEnvA(rangeMap(value, 5.0, 1.0, 5000.0));
    break;
  case kTagAmpEnvD:
    _synth.setAmpEnvD(rangeMap(value, 3.0, 1.0, 5000.0));
    break;
  case kTagAmpEnvS:
    _synth.setAmpEnvS(value);
    break;
  case kTagAmpEnvR:
    _synth.setAmpEnvR(rangeMap(value, 3.0, 1.0, 5000.0));
    break;
  case kTagAmpVeloSens:
    _synth.setAmpVeloSens(value);
    break;
  case kTagVibDelay:
    _synth.setVibDelay(rangeMap(value, 3.0, 0.0, 2000.0));
    break;
  case kTagVibDepth:
    _synth.setVibDepth(rangeMap(value, 3.0, 0.0, 120.0));
    break;
  case kTagVibSpeed:
    _synth.setVibSpeed(rangeMap(value, 3.0, 0.1, 20.0));
    break;
  case kTagFiltType:
    _synth.setFiltType(static_cast<short>(round(value * 5)));
    break;
  case kTagFiltCutoff:
    _synth.setFiltCutoff(exp2(rangeMap(value, 1.0, 6.0, 14.3)));
    break;
  case kTagFiltReso:
    _synth.setFiltReso(rangeMap(value, 2.0, 0.1, 4.0));
    break;
  case kTagFiltEnvAmount:
    _synth.setFiltEnvAmount(rangeMap(value, 1.0, -8.0, 8.0));
    break;
  case kTagFiltEnvA:
    _synth.setFiltEnvA(rangeMap(value, 3.0, 1.0, 5000.0));
    break;
  case kTagFiltEnvD:
    _synth.setFiltEnvD(rangeMap(value, 3.0, 1.0, 5000.0));
    break;
  case kTagFiltEnvS:
    _synth.setFiltEnvS(value);
    break;
  case kTagFiltEnvR:
    _synth.setFiltEnvR(rangeMap(value, 3.0, 1.0, 5000.0));
    break;
  case kTagFiltKeyFollow:
    _synth.setFiltKeyFollow(value);
    break;

  case kTagSampleDivision: {
    size_t div = static_cast<size_t>(round(rangeMap(value, 1.0, 1.0, 8.0)));
    _divider32.setDivision(div);
    _divider64.setDivision(div);
    break;
  }
  case kTagEqF: {
    double f = exp2(rangeMap(value, 1.0, log2(20.0), log2(18000.0)));
    _biquadEQ32.setFrequency(f);
    _biquadEQ64.setFrequency(f);
    break;
  }
  case kTagEqG: {
    double g = rangeMap(value, 1.0, -12.0, 12.0);
    _biquadEQ32.setGain(g);
    _biquadEQ64.setGain(g);
    break;
  }
  case kTagEqQ: {
    double q = rangeMap(value, 3.0, 0.1, 4.0);
    _biquadEQ32.setQ(q);
    _biquadEQ64.setQ(q);
    break;
  }
  case kTagChorusTime: {
    double time = rangeMap(value, 3.0, 0.5, 20.0);
    _chorus32.setDelayTime(time);
    _chorus64.setDelayTime(time);
    break;
  }
  case kTagChorusDepth: {
    double depth = rangeMap(value, 1.0, 0.0, 1.0);
    _chorus32.setDepth(depth);
    _chorus64.setDepth(depth);
    break;
  }
  case kTagChorusSpeed: {
    double speed = rangeMap(value, 3.0, 1.0 / 30.0, 20.0);
    _chorus32.setSpeed(speed);
    _chorus64.setSpeed(speed);
    break;
  }
  case kTagChorusAmount: {
    double mix = rangeMap(value, 1.0, 0.0, 0.707);
    _chorus32.setMix(mix);
    _chorus64.setMix(mix);
    break;
  }
  case kTagReverbTime: {
    double rt = rangeMap(value, 3.0, 0.1, 20.0);
    _reverb32.setTime(rt);
    _reverb64.setTime(rt);
    break;
  }
  case kTagReverbMix: {
    double mix = rangeMap(value, 1.0, 0.0, 1.0);
    _reverb32.setMix(mix);
    _reverb64.setMix(mix);
    break;
  }
  }
}

tresult PLUGIN_API InharmonicProcessor::process(Vst::ProcessData &data) {
  // Parameter processing
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      if (auto *q = data.inputParameterChanges->getParameterData(index)) {
        int32 numPoints = q->getPointCount();
        int32 sampleOffset;
        Vst::ParamValue value;
        if (q->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
          applyParameter(q->getParameterId(), value);
        }
      }
    }
  }

  // Event processing
  _scheduledEvents.clear();
  Vst::IEventList *eventList = data.inputEvents;
  if (eventList != NULL) {
    int32 numEvent = eventList->getEventCount();
    for (int32 i = 0; i < numEvent; i++) {
      Vst::Event event;
      if (eventList->getEvent(i, event) == kResultOk) {
        _scheduledEvents.insert({event.sampleOffset, event});
      }
    }
  }

  // Audio processing
  if (data.numSamples > 0) {
    size_t bufsize = data.numSamples;

    if (data.symbolicSampleSize == Vst::kSample32) {
      bufsize *= sizeof(Vst::Sample32);
      Vst::Sample32 *outL = data.outputs[0].channelBuffers32[0];
      Vst::Sample32 *outR = data.outputs[0].channelBuffers32[1];

      for (int32 i = 0; i < data.numSamples; i++) {
        // event processing
        auto range = _scheduledEvents.equal_range(i);
        for (auto it = range.first; it != range.second; it++) {
          auto &event = it->second;
          switch (event.type) {
          case Vst::Event::kNoteOnEvent:
            if (event.noteOn.velocity != 0)
              _synth.noteOn(event.noteOn.channel, event.noteOn.pitch,
                            event.noteOn.velocity);
            else
              _synth.noteOff(event.noteOn.channel, event.noteOn.pitch,
                             event.noteOn.velocity);
            break;
          case Vst::Event::kNoteOffEvent:
            _synth.noteOff(event.noteOff.channel, event.noteOff.pitch,
                           event.noteOff.velocity);
            break;
          }
        }

        // process sample-by-sample
        _synth.process32(outL[i], outR[i]);
        _biquadEQ32.process(outL[i], outR[i]);
        _chorus32.process(outL[i], outR[i]);
        _divider32.process(outL[i], outR[i]);
        _reverb32.process(outL[i], outR[i]);
      }
    }
    if (data.symbolicSampleSize == Vst::kSample64) {
      bufsize *= sizeof(Vst::Sample64);
      Vst::Sample64 *outL = data.outputs[0].channelBuffers64[0];
      Vst::Sample64 *outR = data.outputs[0].channelBuffers64[1];

      for (int32 i = 0; i < data.numSamples; i++) {
        // event processing
        auto range = _scheduledEvents.equal_range(i);
        for (auto it = range.first; it != range.second; it++) {
          auto &event = it->second;
          switch (event.type) {
          case Vst::Event::kNoteOnEvent:
            if (event.noteOn.velocity != 0)
              _synth.noteOn(event.noteOn.channel, event.noteOn.pitch,
                            event.noteOn.velocity);
            else
              _synth.noteOff(event.noteOn.channel, event.noteOn.pitch,
                             event.noteOn.velocity);
            break;
          case Vst::Event::kNoteOffEvent:
            _synth.noteOff(event.noteOff.channel, event.noteOff.pitch,
                           event.noteOff.velocity);
            break;
          }
        }

        // process sample-by-sample
        _synth.process64(outL[i], outR[i]);
        _biquadEQ64.process(outL[i], outR[i]);
        _chorus64.process(outL[i], outR[i]);
        _divider64.process(outL[i], outR[i]);
        _reverb64.process(outL[i], outR[i]);
      }
    }

    // clear the remaining output buffers
    for (int32 bus = 1; bus < data.numOutputs; bus++) {
      int32 numChannels = data.outputs[bus].numChannels;
      // clear output buffers
      for (int32 c = 0; c < numChannels; c++) {
        memset(data.outputs[bus].channelBuffers32[c], 0, bufsize);
      }
      // inform the host that this bus is silent
      data.outputs[bus].silenceFlags = ((uint64)1 << numChannels) - 1;
    }
  }

  return kResultOk;
}

tresult PLUGIN_API
InharmonicProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
  // called before any processing
  double newFs = newSetup.sampleRate;
  _synth.setSampleRate(newFs);
  _synth.allNoteOff();
  _biquadEQ32.setSampleRate(static_cast<float>(newFs));
  _biquadEQ64.setSampleRate(newFs);
  _chorus32.setSampleRate(newFs);
  _chorus64.setSampleRate(newFs);
  _reverb32.setSampleRate(newFs);
  _reverb64.setSampleRate(newFs);

  return AudioEffect::setupProcessing(newSetup);
}

tresult PLUGIN_API
InharmonicProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;
  if (symbolicSampleSize == Vst::kSample64)
    return kResultTrue;
  return kResultFalse;
}

tresult PLUGIN_API InharmonicProcessor::setState(IBStream *state) {
  // called when we load a preset, the model has to be reloaded
  IBStreamer streamer(state, kLittleEndian);

  for (size_t i = 0; i < kNumAllParameters; i++) {
    double value = 0.5;
    streamer.readDouble(value);
    applyParameter(kAllParameters[i].tag, value);
  }

  return kResultOk;
}

tresult PLUGIN_API InharmonicProcessor::getState(IBStream *state) {
  // here we need to save the model
  IBStreamer streamer(state, kLittleEndian);

  for (size_t i = 0; i < kNumAllParameters; i++) {
    streamer.writeDouble(_param[kAllParameters[i].tag]);
  }

  return kResultOk;
}

} // namespace AudioPlugin
