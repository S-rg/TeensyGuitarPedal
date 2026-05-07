#pragma once
#include <Audio.h>

#define CHORUS_DELAY_LENGTH 1024
#define FLANGE_DELAY_LENGTH 1024

enum EffectType {
    FX_DISTORTION = 0,
    FX_REVERB,
    FX_DELAY,
    FX_CHORUS,
    FX_FLANGE,
    FX_BYPASS
};

class EffectSlot {
  public:
    int16_t chorusDelayLine[CHORUS_DELAY_LENGTH];
    int16_t flangeDelayLine[FLANGE_DELAY_LENGTH];

    AudioEffectWaveshaper distortion;
    AudioEffectFreeverb reverb;
    AudioEffectDelay delay;
    AudioEffectChorus chorus;
    AudioEffectFlange flange;
    AudioAmplifier bypass;

    AudioConnection *patchIn = nullptr;
    AudioConnection *patchOut = nullptr;

    AudioStream *input;
    AudioStream *outputNode;

    EffectType currentEffect;
    int currentValue;

    EffectSlot(AudioStream *in, AudioStream *out);

    void setIn(AudioStream *in);
    void setOut(AudioStream *out);
    void initialize(AudioStream *in, AudioStream *out);

    void selectEffect(EffectType fx, float value);
    void applyEffect(EffectType typeIndex, int value);

    AudioStream &output();

  private:
    void clearConnections();

    void setDistortion(float value);
    void setReverb(float value);
    void setDelay(float value);
    void setChorus(int voices);
    void setFlange(float offset, float depth, float rate);
    void setBypass();
};
