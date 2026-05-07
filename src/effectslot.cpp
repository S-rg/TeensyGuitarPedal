#include "EffectSlot.h"
#include "AudioStream.h"
#include <cmath>
#include <cstdint>

EffectSlot::EffectSlot(AudioStream *in, AudioStream *out)
    : input(in), outputNode(out) {
    setBypass();
}

void EffectSlot::initialize(AudioStream *in, AudioStream *out) {
    input = in;
    outputNode = out;
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
}

void EffectSlot::setDistortion(float value) {
    clearConnections();

    patchIn = new AudioConnection(*input, distortion);
    patchOut = new AudioConnection(distortion, *outputNode);

    static float curve[256];
    float drive = 1.0f + value * 20.0f;

    for (int i = 0; i < 256; i++) {
        float x = (i - 128) / 128.0f;
        float y = x / (1.0f + fabsf(x * drive));
        curve[i] = y;
    }

    distortion.shape(curve, 256);
}

void EffectSlot::setDelay(float value) {
    clearConnections();

    patchIn = new AudioConnection(*input, delay);
    patchOut = new AudioConnection(delay, *outputNode);

    delay.delay(0, value * 500.0f);
}

void EffectSlot::setReverb(float value) {
    clearConnections();
    patchIn = new AudioConnection(*input, reverb);
    patchOut = new AudioConnection(reverb, *outputNode);
    reverb.roomsize(value);
    reverb.damping(1.0f - value);
}

void EffectSlot::setBypass() {
    clearConnections();

    patchIn = new AudioConnection(*input, bypass);
    patchOut = new AudioConnection(bypass, *outputNode);

    bypass.gain(1.0f);
}

void EffectSlot::setChorus(int voices) {
    clearConnections();

    patchIn = new AudioConnection(*input, chorus);
    patchOut = new AudioConnection(chorus, *outputNode);

    chorus.begin(chorusDelayLine, CHORUS_DELAY_LENGTH, 2);
    chorus.voices(voices);
}

void EffectSlot::setFlange(float offset, float depth, float rate) {
    clearConnections();

    patchIn = new AudioConnection(*input, flange);
    patchOut = new AudioConnection(flange, *outputNode);

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
