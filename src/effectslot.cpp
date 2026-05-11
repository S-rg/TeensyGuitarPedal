#include "EffectSlot.h"
#include "AudioStream.h"
#include <Audio.h>
#include <cmath>
#include <cstdint>

EffectSlot::EffectSlot(AudioStream *in, AudioStream *out)
    : input(in), outputNode(out) {}

void EffectSlot::initialize(AudioStream *in, AudioStream *out) {
    input = in;
    outputNode = out;

    through.gain(1.0f);
    dryGain.gain(1.0f);
    wetGain.gain(1.0f);

    setBypass();
}

void EffectSlot::clearConnections() {
    if (patchIn) {
        delete patchIn;
        patchIn = nullptr;
    }

    if (patchOut) {
        delete patchOut;
        patchOut = nullptr;
    }

    if (patchDry) {
        delete patchDry;
        patchDry = nullptr;
    }

    if (patchWet) {
        delete patchWet;
        patchWet = nullptr;
    }

    if (patchMixOut) {
        delete patchMixOut;
        patchMixOut = nullptr;
    }
}

void EffectSlot::setDistortion(float value) {
    clearConnections();

    patchIn = new AudioConnection(*input, 0, distortion, 0);
    patchOut = new AudioConnection(distortion, 0, through, 0);

    static float curve[257];
    float drive = 1.0f + value * 20.0f;

    for (int i = 0; i < 257; i++) {
        float x = (i - 128) / 128.0f;
        float y = x / (1.0f + fabsf(x * drive));
        curve[i] = y;
    }

    distortion.shape(curve, 257);
}

void EffectSlot::setDelay(float value) {
    clearConnections();

    patchIn = new AudioConnection(*input, 0, delay, 0);
    patchWet = new AudioConnection(delay, 0, wetDryMixer, 0);

    patchDry = new AudioConnection(*input, 0, dryGain, 0);
    patchOut = new AudioConnection(dryGain, 0, wetDryMixer, 1);
    patchMixOut = new AudioConnection(wetDryMixer, 0, through, 0);

    delay.delay(0, value * 500.0f);

    float wet = value;
    float dry = 1.0f - (wet * 0.5f);

    wetDryMixer.gain(0, wet);
    wetDryMixer.gain(1, dry);
}

void EffectSlot::setReverb(float value) {
    clearConnections();

    patchIn = new AudioConnection(*input, 0, reverb, 0);
    patchWet = new AudioConnection(reverb, 0, wetDryMixer, 0);

    patchDry = new AudioConnection(*input, 0, dryGain, 0);
    patchOut = new AudioConnection(dryGain, 0, wetDryMixer, 1);
    patchMixOut = new AudioConnection(wetDryMixer, 0, through, 0);

    reverb.roomsize(value);
    reverb.damping(1.0f - value);

    float wet = value;
    float dry = 1.0f - (wet * 0.5f);

    wetDryMixer.gain(0, wet);
    wetDryMixer.gain(1, dry);
}

void EffectSlot::setBypass() {
    clearConnections();

    patchIn = new AudioConnection(*input, 0, bypass, 0);
    patchOut = new AudioConnection(bypass, 0, through, 0);

    bypass.gain(1.0f);
}

void EffectSlot::setChorus(int voices) {
    clearConnections();

    patchIn = new AudioConnection(*input, 0, chorus, 0);
    patchOut = new AudioConnection(chorus, 0, through, 0);

    chorus.begin(chorusDelayLine, CHORUS_DELAY_LENGTH, 2);
    chorus.voices(voices);
}

void EffectSlot::setFlange(float offset, float depth, float rate) {
    clearConnections();

    patchIn = new AudioConnection(*input, 0, flange, 0);
    patchOut = new AudioConnection(flange, 0, through, 0);

    flange.begin(flangeDelayLine, FLANGE_DELAY_LENGTH, offset, depth, rate);
}

void EffectSlot::applyEffect(EffectType typeIndex, int value) {
    currentEffect = typeIndex;
    currentValue = value;
    float fVal = value / 100.0f;

    switch (typeIndex) {
    case FX_DISTORTION:
        setDistortion(fVal);
        break;
    case FX_REVERB:
        setReverb(fVal);
        break;
    case FX_DELAY:
        setDelay(fVal);
        break;
    case FX_CHORUS:
        setChorus(1 + (value / 25));
        break;
    case FX_FLANGE:
        setFlange(fVal * 2.0f, fVal, 0.5f);
        break;
    case FX_BYPASS:
        setBypass();
        break;
    }
}

AudioStream &EffectSlot::output() { return *outputNode; }
