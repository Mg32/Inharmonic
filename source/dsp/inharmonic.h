// SPDX-License-Identifier: MIT
#pragma once

#include <algorithm>
#include <cmath>
#include <random>

namespace Inharmonic {

namespace {
static constexpr double kPi = 3.141592653589793238;
static constexpr size_t kTableSize = 8192;
static double cosTable[kTableSize] = {};

static inline void initializeCosTable() {
  if (cosTable[0] != 0)
    return;
  for (size_t i = 0; i < kTableSize; i++) {
    cosTable[i] = cos(2.0 * kPi * i / kTableSize);
  }
}

static inline double unsafeFastCos2pi(const double &x) {
  return cosTable[static_cast<int>(x * kTableSize)];
}

} // namespace

class PseudoRandom {
public:
  double next() {
    _seed = (kMultiplier * _seed + kIncrement) & kMask;
    uint32_t result = (_seed >> kResultShift) & kResultMask;
    return result * (1.0 / (kResultMask + 1));
  }
  void seed(uint32_t value) { _seed = value & kMask; }

private:
  static constexpr uint32_t kResultShift = 16;
  static constexpr uint32_t kResultMask = (1U << (kResultShift - 1)) - 1;
  static constexpr uint32_t kMask = (1U << 31) - 1;
  static constexpr uint32_t kMultiplier = 214013U;
  static constexpr uint32_t kIncrement = 2531011U;
  uint32_t _seed = 0;
};

class InharmonicOscillator {
public:
  InharmonicOscillator() {
    std::random_device seedGen;
    std::mt19937 metaRng(seedGen());
    for (size_t i = 0; i < kMaxSines; i++) {
      uint32_t seed = metaRng();
      _rng[i].seed(seed);
    }
    initializeCosTable();
  }

  void resetStateRandom() {
    for (size_t i = 0; i < kMaxSines; i++) {
      _phase[i] = _rng[i].next();
    }
  }

  void resetStateZero() {
    for (size_t i = 0; i < kMaxSines; i++) {
      _phase[i] = 0;
    }
  }

  void setFreq(double f, double inharmonicB) {
    const double thresh = 0.5 / f;
    size_t i = 0;
    for (i = 1; i < kMaxSines; i++) {
      // const double scale = i * (1.0 + 0.5 * inharmonicB * i * i);
      const double scale = i * sqrt(1.0 + inharmonicB * i * i);
      if (scale >= thresh)
        break;
      _amp[i] = 1.0 / scale;
      _steps[i] = scale * f;
    }
    _numSines = i;
  }

  double process(double oscMod) {
    double out = 0;
    for (size_t i = 1; i < _numSines; i++) {
      out += _amp[i] * unsafeFastCos2pi(_phase[i]);
      _phase[i] += _steps[i] * oscMod;
      _phase[i] -= static_cast<int>(_phase[i]);
    }
    return out;
  }

private:
  static constexpr size_t kMaxSines = 128;
  PseudoRandom _rng[kMaxSines];
  size_t _numSines = 1;
  double _amp[kMaxSines] = {};
  double _steps[kMaxSines] = {};
  double _phase[kMaxSines] = {};
};

class StateVariableFilter {
public:
  void resetState() { _p1 = _p2 = _p3 = _p4 = 0; }

  void setFreq(double f, double fs, double q) {
    f = std::max(20.0, std::min(0.9 * fs / 2.0, f));
    _k = 2 * sin(kPi * f / fs);
    _oqk = 1.0 / q + _k;
    _denom = 1.0 + _k * _oqk;
  }

  double process(double x, short type, short iter) {
    // V. Lazzarini and J. Timoney: "Improving the Chamberlin Digital State
    // Variable Filter" (2021) <https://arxiv.org/abs/2111.05592>
    double u, res[3];

    // HPF1
    res[1] = (x - _oqk * _p1 - _p2) / _denom;

    // BPF1
    u = res[1] * _k;
    res[2] = u + _p1;
    _p1 = u + res[2];

    // LPF1
    u = res[2] * _k;
    res[0] = u + _p2;
    _p2 = u + res[0];

    if (iter == 0)
      return res[type];

    // HPF2
    res[1] = (res[type] - _oqk * _p3 - _p4) / _denom;

    // BPF2
    u = res[1] * _k;
    res[2] = u + _p3;
    _p3 = u + res[2];

    // LPF2
    u = res[2] * _k;
    res[0] = u + _p4;
    _p4 = u + res[0];

    return res[type];
  }

private:
  double _k = 0;
  double _oqk = 0;
  double _denom = 1;
  double _p1 = 0;
  double _p2 = 0;
  double _p3 = 0;
  double _p4 = 0;
};

enum class EnvState {
  kAttack,
  kDecay,
  kSustain,
  kRelease,
  kStop,
};

class InharmonicEnvGen {
public:
  void setA(double a, double fs) { _envA = 1.0 / std::max(1.0, 1e-3 * a * fs); }
  void setD(double d, double fs) { _envD = 1.0 / std::max(1.0, 1e-3 * d * fs); }
  void setS(double s) { _envS = std::max(0.0, s); }
  void setR(double r, double fs) { _envR = 1.0 / std::max(1.0, 1e-3 * r * fs); }
  const EnvState &getState() const { return _state; }

  void noteOn() {
    _state = EnvState::kAttack;
    _remain = 1.0;
    _attackBegin = _last;
  }

  void noteOff() {
    if (_state >= EnvState::kRelease)
      return;
    _state = EnvState::kRelease;
    _remain = 1.0;
    _releaseBegin = _last;
  }

  bool process(double &env) {
    switch (_state) {
    case EnvState::kAttack:
      _remain -= _envA;
      if (_remain <= 0.0) {
        _state = EnvState::kDecay;
        _remain = 1.0;
        _last = 1.0;
      } else {
        const double t = 1.0 - _remain;
        _last = _attackBegin + (1.0 - _attackBegin) * t;
      }
      env = _last;
      return false;

    case EnvState::kDecay:
      _remain -= _envD;
      if (_remain <= 0.0) {
        _state = EnvState::kSustain;
        _remain = 1.0;
        _last = _envS;
        _releaseBegin = _envS;
      } else {
        _last = _envS + (1.0 - _envS) * _remain;
      }
      env = _last;
      return false;

    case EnvState::kSustain:
      env = _last;
      return false;

    case EnvState::kRelease:
      _remain -= _envR;
      if (_remain <= 0.0) {
        _state = EnvState::kStop;
        _remain = 1.0;
        env = _last = 0.0;
        return true;
      } else {
        env = _last = _releaseBegin * _remain;
      }
      return false;

    default:
      break;
    }
    env = 0.0;
    return true;
  }

private:
  double _envA = 1.0 / (20e-3 * 48000);
  double _envD = 1.0 / (500e-3 * 48000);
  double _envS = 0.8;
  double _envR = 1.0 / (1500e-3 * 48000);
  EnvState _state = EnvState::kStop;
  double _remain = 1;
  double _attackBegin = 0;
  double _releaseBegin = 0;
  double _last = 0;
};

class InharmonicLFO {
public:
  void noteOn() {
    _remain = 1.0;
    _phase = 0.25;
  }

  double process(double delay, double step) {
    double out = 0.0;
    if (_remain > 0.0) {
      _remain -= delay;
    } else {
      out = unsafeFastCos2pi(_phase);
      _phase += step;
      _phase -= static_cast<int>(_phase);
    }
    return out;
  }

private:
  double _remain = 1.0;
  double _phase = 0.0;
};

class InharmonicVoice {
public:
  void setSampleRate(double fs) { _fs = std::max(8000.0, fs); }
  void setOscMix(double mix) {
    mix = std::max(0.0, std::min(1.0, mix));
    _mixOsc1 = 1.0 - mix;
    _mixOsc2 = mix;
  }
  void setInharmonicB(double b) {
    _inharmonicB1 = std::max(0.0, b);
    _inharmonicB2 = _inharmonicB1 * _inharmonicSubscale;
    updateOscFreq();
  }
  void setInharmonicSubscale(double s) {
    _inharmonicSubscale = std::max(0.0, s);
    _inharmonicB2 = _inharmonicB1 * _inharmonicSubscale;
    updateOscFreq();
  }
  void setInharmKeyFollow(double x) { _inharmKeyFollow = x; }
  void setAmpVeloSens(double x) { _ampVeloSens = x; }
  void setVibDelay(double x) { _vibDelay = x; }
  void setVibDepth(double x) { _vibDepth = x; }
  void setVibSpeed(double x) { _vibSpeed = x; }
  void setFilterType(short type) {
    _filtType = type % 3;
    _filtIter = type / 3;
  }
  void setFilterFreq(double freq) {
    _filtFreq = freq;
    _svf.setFreq(_filtFreq, _fs, _filtQ);
  }
  void setFilterQ(double q) {
    _filtQ = q;
    _svf.setFreq(_filtFreq, _fs, _filtQ);
  }
  void setFilterEnvAmount(double amount) { _filtEnvAmount = amount; }
  void setFiltKeyFollow(double x) { _filtKeyFollow = x; }
  short getPitch() const { return _pitch; }
  InharmonicEnvGen &getEnvAmp() { return _envAmp; }
  InharmonicEnvGen &getEnvFilt() { return _envFilt; }

  void noteOn(short pitch, double velocity, bool isRandomPhase) {
    _pitch = std::max((short)0, std::min((short)127, pitch));
    _freq = 440.0 * exp2((_pitch - 69.0) / 12.0) / _fs;
    _velocity = velocity;
    if (isRandomPhase) {
      _osc1.resetStateRandom();
      _osc2.resetStateRandom();
    } else {
      _osc1.resetStateZero();
      _osc2.resetStateZero();
    }
    updateOscFreq();
    _svf.setFreq(_filtFreq, _fs, _filtQ);
    _envAmp.noteOn();
    _envFilt.noteOn();
    _lfoVib.noteOn();

    _ampVelMod = (_velocity - 1.0) * _ampVeloSens + 1.0;
    _filtKeyMod = exp2(((_pitch - 60.0) / 12.0) * _filtKeyFollow);
  }

  void noteOff() {
    _envAmp.noteOff();
    _envFilt.noteOff();
  }

  void setFreqBend(double x) {
    _freqBend = x;
    _osc1.setFreq(_freq * _freqBend, _inharmonicB1);
    _osc2.setFreq(_freq * _freqBend, _inharmonicB2);
  }

  double process() {
    // amp
    double a = 0.0;
    if (_envAmp.process(a))
      return 0.0;
    a *= _ampVelMod;

    // freq
    double f = 0.0;
    _envFilt.process(f);

    // vco
    double oscMod = 1.0;
    if (_vibDepth != 0.0) {
      // vibrato
      const double del = 1.0 / (1e-3 * _vibDelay * _fs);
      const double step = _vibSpeed / _fs;
      oscMod *= exp2(_vibDepth * _lfoVib.process(del, step) / 1200.0);
    }
    const double out1 = _osc1.process(oscMod);
    const double out2 = _osc2.process(oscMod);
    const double vco = out1 * _mixOsc1 + out2 * _mixOsc2;

    // vcf
    bool isFiltModified = false;
    double filtMod = 1.0;
    if (_filtEnvAmount != 0.0) {
      // filter envelope
      filtMod *= exp2(f * _filtEnvAmount);
      isFiltModified = true;
    }
    if (_filtKeyFollow != 0.0) {
      // filter velocity
      filtMod *= _filtKeyMod;
      isFiltModified = true;
    }
    if (isFiltModified) {
      _svf.setFreq(_filtFreq * filtMod, _fs, _filtQ);
    }
    double vcf = _svf.process(vco, _filtType, _filtIter);

    return a * a * vcf;
  }

private:
  void updateOscFreq() {
    double inharmKeyMod = exp2((_pitch - 60.0) / 12.0 * 4.0 * _inharmKeyFollow);
    _osc1.setFreq(_freq * _freqBend, _inharmonicB1 * inharmKeyMod);
    _osc2.setFreq(_freq * _freqBend, _inharmonicB2 * inharmKeyMod);
  }

  double _fs = 48000;
  short _pitch = 69;
  double _freq = 440.0 / 48000.0;
  double _freqBend = 1.0;
  double _velocity = 1.0;
  double _ampVelMod = 1.0;
  double _filtKeyMod = 1.0;

  double _mixOsc1 = 0.7;
  double _mixOsc2 = 0.3;
  double _inharmonicB1 = 0.1;
  double _inharmonicB2 = 0.025;
  double _inharmonicSubscale = 0.25;
  double _inharmKeyFollow = 0;
  double _ampVeloSens = 1.0;
  double _vibDelay = 0;
  double _vibDepth = 0;
  double _vibSpeed = 2.0;
  short _filtType = 0;
  short _filtIter = 0;
  double _filtFreq = 4000;
  double _filtQ = 0.5;
  double _filtEnvAmount = 0.0;
  double _filtKeyFollow = 0.0;

  InharmonicOscillator _osc1;
  InharmonicOscillator _osc2;
  InharmonicEnvGen _envAmp;
  InharmonicEnvGen _envFilt;
  InharmonicLFO _lfoVib;
  StateVariableFilter _svf;
};

class InharmonicSynth {
public:
  void noteOn(short channel, short pitch, double velocity) {
    // find stopped notes
    for (size_t i = 0; i < kMaxVoices; i++) {
      if (_voices[i].getEnvAmp().getState() == EnvState::kStop) {
        _voices[i].noteOn(pitch, velocity, _isRandomPhase);
        return;
      }
    }

    // find released notes
    for (size_t i = 0; i < kMaxVoices; i++) {
      if (_voices[i].getEnvAmp().getState() == EnvState::kRelease) {
        _voices[i].noteOn(pitch, velocity, _isRandomPhase);
        return;
      }
    }
  }

  void noteOff(short channel, short pitch, double velocity) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      if (_voices[i].getPitch() == pitch) {
        _voices[i].noteOff();
      }
    }
  }

  void allNoteOff() {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].noteOff();
    }
  }

  void process64(double &outL, double &outR) {
    double outVoice = 0;
    for (size_t i = 0; i < kMaxVoices; i++) {
      outVoice += _voices[i].process();
    }
    outVoice *= _outVolume;
    outL = outVoice;
    outR = outVoice;
  }

  void process32(float &outL, float &outR) {
    double outL64, outR64;
    process64(outL64, outR64);
    outL = static_cast<float>(outL64);
    outR = static_cast<float>(outR64);
  }

  void setSampleRate(double fs) {
    _fs = fs;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setSampleRate(fs);
      _voices[i].getEnvAmp().setA(_ampEnvA, _fs);
      _voices[i].getEnvAmp().setD(_ampEnvD, _fs);
      _voices[i].getEnvAmp().setR(_ampEnvR, _fs);
      _voices[i].getEnvFilt().setA(_filtEnvA, _fs);
      _voices[i].getEnvFilt().setD(_filtEnvD, _fs);
      _voices[i].getEnvFilt().setR(_filtEnvR, _fs);
    }
  }

  void setVolume(double value) { _volume = value; }
  void setExpression(double value) { _expression = value; }
  void setPitchBend(double value) {
    const double freqBend = exp2(_bendRange * value / 12.0);
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setFreqBend(freqBend);
    }
  }
  void setModWheel(double value) { _modwheel = value; }
  void setSustainPedal(bool value) { _sustainPedal = value; }
  void setSostenutoPedal(bool value) { _sostenutoPedal = value; }
  void setSoftPedal(double value) { _softPedal = value; }

  void setOutVol(double value) { _outVolume = value; }
  void setOscMix(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setOscMix(x);
    }
  }
  void setIsRandomPhase(bool x) { _isRandomPhase = x; }
  void setInharmonic(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setInharmonicB(x);
    }
  }
  void setInharmonicSubscale(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setInharmonicSubscale(x);
    }
  }
  void setInharmKeyFollow(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setInharmKeyFollow(x);
    }
  }
  void setAmpEnvA(double x) {
    _ampEnvA = x;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvAmp().setA(_ampEnvA, _fs);
    }
  }
  void setAmpEnvD(double x) {
    _ampEnvD = x;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvAmp().setD(_ampEnvD, _fs);
    }
  }
  void setAmpEnvS(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvAmp().setS(x);
    }
  }
  void setAmpEnvR(double x) {
    _ampEnvR = x;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvAmp().setR(_ampEnvR, _fs);
    }
  }
  void setAmpVeloSens(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setAmpVeloSens(x);
    }
  }
  void setVibDelay(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setVibDelay(x);
    }
  }
  void setVibDepth(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setVibDepth(x);
    }
  }
  void setVibSpeed(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setVibSpeed(x);
    }
  }
  void setFiltType(short x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setFilterType(x);
    }
  }
  void setFiltCutoff(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setFilterFreq(x);
    }
  }
  void setFiltReso(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setFilterQ(x);
    }
  }
  void setFiltEnvAmount(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setFilterEnvAmount(x);
    }
  }
  void setFiltEnvA(double x) {
    _filtEnvA = x;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvFilt().setA(_filtEnvA, _fs);
    }
  }
  void setFiltEnvD(double x) {
    _filtEnvD = x;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvFilt().setD(_filtEnvD, _fs);
    }
  }
  void setFiltEnvS(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvFilt().setS(x);
    }
  }
  void setFiltEnvR(double x) {
    _filtEnvR = x;
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].getEnvFilt().setR(_filtEnvR, _fs);
    }
  }
  void setFiltKeyFollow(double x) {
    for (size_t i = 0; i < kMaxVoices; i++) {
      _voices[i].setFiltKeyFollow(x);
    }
  }

private:
  double _fs = 48000.0;

  // control state
  double _volume = 1;
  double _expression = 1;
  double _modwheel = 0;
  bool _sustainPedal = false;
  bool _sostenutoPedal = false;
  double _softPedal = 0;

  double _outVolume = 0.25;
  double _bendRange = 2.0;
  bool _isRandomPhase = false;
  double _ampEnvA = 0.0;
  double _ampEnvD = 0.0;
  double _ampEnvR = 0.0;
  double _filtEnvA = 0.0;
  double _filtEnvD = 0.0;
  double _filtEnvR = 0.0;

  static constexpr size_t kMaxVoices = 16;
  InharmonicVoice _voices[kMaxVoices];
};

} // namespace Inharmonic
