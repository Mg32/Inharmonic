// SPDX-License-Identifier: MIT
#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace Effect {

namespace {
static constexpr double kPi = 3.141592653589793238;
template <typename T> static T fixDenormal(T x) {
  if (!std::isnormal(x))
    return 0;
  return x;
}
} // namespace

template <typename T> class SampleDivider {
public:
  void process(T &inoutL, T &inoutR) {
    if (_n_divs <= 1) {
      return;
    }
    if (_phase == 0) {
      _prevL = inoutL;
      _prevR = inoutR;
    }
    _phase += 1;
    if (_phase >= _n_divs) {
      _phase = 0;
    }
    inoutL = _prevL;
    inoutR = _prevR;
  }

  void setDivision(size_t div) { _n_divs = div; }

private:
  T _prevL = 0;
  T _prevR = 0;
  size_t _phase = 0;
  size_t _n_divs = 1;
};

template <typename T> class BiquadEQ {
public:
  BiquadEQ() { setParameters(48000, 1000, 0, 1); }

  void process(T &inoutL, T &inoutR) {
    // stock the input signal
    T inL = inoutL;
    T inR = inoutR;

    // biquad filtering
    inoutL = _coeff[0] * inL + _coeff[1] * _delayL[0] + _coeff[2] * _delayL[1] +
             _coeff[3] * _delayL[2] + _coeff[4] * _delayL[3];
    inoutR = _coeff[0] * inR + _coeff[1] * _delayR[0] + _coeff[2] * _delayR[1] +
             _coeff[3] * _delayR[2] + _coeff[4] * _delayR[3];

    // bucket-brigading (L)
    _delayL[1] = _delayL[0];
    _delayL[0] = inL;
    _delayL[3] = _delayL[2];
    _delayL[2] = inoutL;

    // bucket-brigading (R)
    _delayR[1] = _delayR[0];
    _delayR[0] = inR;
    _delayR[3] = _delayR[2];
    _delayR[2] = inoutR;
  }

  void setParameters(T fs, T freq, T gain_dB, T q) {
    _fs = fs;
    _freq = freq;
    _gain_dB = gain_dB;
    _q = q;
    T a = std::pow(static_cast<T>(10), gain_dB / 40);
    T w = 2 * static_cast<T>(kPi) * freq / fs;
    T cosW = std::cos(w);
    T sinW = std::sin(w);
    T alpha = sinW / (2 * std::max(q, static_cast<T>(1e-3)));
    T b0 = 1 + alpha * a;
    T b1 = -2 * cosW;
    T b2 = 1 - alpha * a;
    T a0 = 1 + alpha / a;
    T a1 = -2 * cosW;
    T a2 = 1 - alpha / a;
    _coeff[0] = b0 / a0;
    _coeff[1] = b1 / a0;
    _coeff[2] = b2 / a0;
    _coeff[3] = -a1 / a0;
    _coeff[4] = -a2 / a0;
  }
  void setSampleRate(T fs) { setParameters(fs, _freq, _gain_dB, _q); }
  void setFrequency(T freq) { setParameters(_fs, freq, _gain_dB, _q); }
  void setGain(T gain_dB) { setParameters(_fs, _freq, gain_dB, _q); }
  void setQ(T q) { setParameters(_fs, _freq, _gain_dB, q); }

private:
  T _fs = 48000;
  T _freq = 1000;
  T _gain_dB = 0;
  T _q = 1;
  T _coeff[5] = {};
  T _delayL[4] = {};
  T _delayR[4] = {};
};

template <typename T> class DelayLine {
public:
  explicit DelayLine(size_t size) : _state_head(0) {
    if (size == 0) {
      size = 1;
    }
    this->_state_buffer = std::move(std::vector<T>(size, 0.0));
    this->reset();
  }

  void reset() {
    // reset head
    this->_state_head = 0;

    // reset buffer
    for (size_t i = 0, size = this->_state_buffer.size(); i < size; i++) {
      this->_state_buffer[i] = 0.0;
    }
  }

  void push(T signal_in) {
    // move to current tail
    size_t index = this->_state_head + 1;
    size_t size = this->_state_buffer.size();
    index -= (index / size) * size;

    // push the signal value
    this->_state_buffer[index] = signal_in;

    // set the current head
    this->_state_head = index;
  }

  T tail(size_t offset = 0) const {
    // move to current tail
    size_t index = this->_state_head + 1 + offset;
    size_t size = this->_state_buffer.size();
    index -= (index / size) * size;

    return this->_state_buffer[index];
  }

  T read(size_t delay) const {
    size_t size = this->_state_buffer.size();
    if (delay >= size) {
      return 0.0;
    }

    // move to the position
    size_t index = this->_state_head + size - delay;
    index -= (index / size) * size;

    return this->_state_buffer[index];
  }

  T readInterp(T delay) const {
    size_t size = this->_state_buffer.size();
    if (delay < 0 || delay >= size) {
      return 0.0;
    }

    size_t delayInt1 = static_cast<size_t>(delay);
    size_t delayInt2 = std::min(delayInt1 + 1, size - 1);
    T fract = delay - delayInt1;
    size_t index1 = this->_state_head + size - delayInt1;
    size_t index2 = this->_state_head + size - delayInt2;
    index1 -= (index1 / size) * size;
    index2 -= (index2 / size) * size;
    T x1 = this->_state_buffer[index1];
    T x2 = this->_state_buffer[index2];
    return x1 + fract * (x2 - x1);
  }

  size_t size() const noexcept { return this->_state_buffer.size(); };

private:
  size_t _state_head;
  std::vector<T> _state_buffer;
};

template <typename T> class DelayLineAllpass {
public:
  DelayLineAllpass(size_t size) : _line(size) {}

  T process(T input) {
    const T a = _line.tail();
    const T b = input - 0.5 * a;
    _line.push(b);
    return a + 0.5 * b;
  }

  size_t size() const noexcept { return this->_line.size(); };

private:
  DelayLine<T> _line;
};

template <typename T> class Reverb {
public:
  // Reverb
  // https://ryukau.github.io/filter_notes/feedback_delay_network/feedback_delay_network.html
  // https://valhalladsp.com/2010/08/25/rip-keith-barr/
  // https://www.spinsemi.com/knowledge_base/effects.html#Reverberation

  Reverb()
      : _line1(1637), _line2(2693), _line3(5813), _line4(6871), _ap1a(523),
        _ap1b(1259), _ap2a(233), _ap2b(1459), _ap3a(631), _ap3b(1103),
        _ap4a(131), _ap4b(797) {
    setParameters(_fs, _t60);
  }

  void process(T &inoutL, T &inoutR) {
    const size_t delay = 34;
    const size_t delayHalf = delay / 2;
    const T input = 0.5 * (inoutL + inoutR);
    const T dl1 = _line1.tail() * _attenuation;
    const T dl2 = _line2.tail() * _attenuation;
    const T dl3 = _line3.tail() * _attenuation;
    const T dl4 = _line4.tail() * _attenuation;
    const T mix1 = dl4 + input;
    const T mix2 = dl1;
    const T mix3 = dl2;
    const T mix4 = dl3;
    const T ap1 = _ap1b.process(_ap1a.process(mix1));
    const T ap2 = _ap2b.process(_ap2a.process(mix2));
    const T ap3 = _ap3b.process(_ap3a.process(mix3));
    const T ap4 = _ap4b.process(_ap4a.process(mix4));
    const T sig1N = _line1.read(0);
    const T sig1D = _line1.read(delayHalf);
    const T sig2N = _line2.read(0);
    const T sig2D = _line2.read(delay);
    const T sig3N = _line3.read(0);
    const T sig3D = _line3.read(delay);
    const T sig4N = _line4.read(0);
    const T sig4D = _line4.read(delay);
    T o1 =
        0.7 * sig1N + 0.3 * sig1D + 0.8 * sig2N + 0.2 * sig2D + sig3N + sig4D;
    T o2 =
        0.3 * sig1N + 0.7 * sig1D + 0.2 * sig2N + 0.8 * sig2D + sig3D + sig4N;
    inoutL += _mix * (o1 - inoutL);
    inoutR += _mix * (o2 - inoutR);
    _line1.push(fixDenormal(ap1));
    _line2.push(fixDenormal(ap2));
    _line3.push(fixDenormal(ap3));
    _line4.push(fixDenormal(ap4));
  }

  void setParameters(T fs, T t60) {
    _fs = fs;
    _t60 = t60;
    const size_t sumAllpassLength = _ap1a.size() + _ap1b.size() + _ap2a.size() +
                                    _ap2b.size() + _ap3a.size() + _ap3b.size() +
                                    _ap4a.size() + _ap4b.size();
    const size_t delayLength =
        _line1.size() + _line2.size() + _line3.size() + _line4.size();
    T totalDelayLength = sumAllpassLength / 8.0 + delayLength;
    _attenuation = std::pow(10.0, -3.0 * totalDelayLength / (t60 * fs));
  }
  void setSampleRate(T fs) { setParameters(fs, _t60); }
  void setTime(T t60) { setParameters(_fs, t60); }
  void setMix(T mix) { _mix = mix; }

private:
  T _fs = 48000;
  T _t60 = 1;
  T _mix = 0;

  T _attenuation = 0;
  DelayLine<T> _line1, _line2, _line3, _line4;
  DelayLineAllpass<T> _ap1a, _ap1b, _ap2a, _ap2b, _ap3a, _ap3b, _ap4a, _ap4b;
};

template <typename T> class TriangleLFO {
public:
  T process() {
    _lfoPhase += _lfoDelta;
    if (_lfoPhase > 1.0)
      _lfoPhase = 0;

    T tr = _lfoPhase;
    if (tr > 0.5) {
      tr = 1 - tr;
    }
    tr *= 2;
    return 2 * (tr - 0.5);
  }

  void setParameters(T fs, T freq) {
    _fs = fs;
    _freq = freq;
    _lfoDelta = _freq / _fs;
  }
  void setSampleRate(T fs) { setParameters(fs, _freq); }
  void setFrequency(T freq) { setParameters(_fs, freq); }
  void reset(T phase = 0) { _lfoPhase = phase; }

private:
  T _fs = 48000;
  T _freq = 1;

  T _lfoDelta = 0;
  T _lfoPhase = 0;
};

template <typename T> class Chorus {
public:
  Chorus() : _lineL(48000), _lineR(48000) {}

  void process(T &inoutL, T &inoutR) {
    const T mod1 = _lfo1.process() * _depth;
    const T mod2 = _lfo2.process() * _depth;
    const T offset1L = _delaySamples * (1.1 + 0.9 * mod1);
    const T offset1R = _delaySamples * (1.1 - 0.9 * mod1);
    const T offset2L = _delaySamples * (1.1 + 0.9 * mod2);
    const T offset2R = _delaySamples * (1.1 - 0.9 * mod2);
    const T inL = inoutL;
    const T inR = inoutR;
    _lineL.push(inL);
    _lineR.push(inR);
    const T chorusL =
        0.7 * _lineL.readInterp(offset1L) + 0.3 * _lineL.readInterp(offset2L);
    const T chorusR =
        0.7 * _lineR.readInterp(offset1R) + 0.3 * _lineR.readInterp(offset2R);
    inoutL += _mix * (chorusL - inL);
    inoutR += _mix * (chorusR - inR);
  }

  void setParameters(T fs, T delay, T freq) {
    _fs = fs;
    _delay = delay;
    _freq = freq;
    _lfo1.setParameters(_fs, _freq);
    _lfo2.setParameters(_fs, _freq * 11 / 12);
    _lfo1.reset();
    _lfo2.reset();
    _delaySamples = _delay * _fs * 1e-3;
  }
  void setSampleRate(T fs) { setParameters(fs, _delay, _freq); }
  void setDelayTime(T delay) { setParameters(_fs, delay, _freq); }
  void setSpeed(T freq) { setParameters(_fs, _delay, freq); }
  void setDepth(T depth) { _depth = depth; }
  void setMix(T mix) { _mix = mix; }

private:
  T _fs = 48000;
  T _delay = 8;
  T _freq = 1;
  T _depth = 1;
  T _mix = 0;

  T _delaySamples = 384;

  TriangleLFO<T> _lfo1;
  TriangleLFO<T> _lfo2;
  DelayLine<T> _lineL, _lineR;
};

} // namespace Effect
