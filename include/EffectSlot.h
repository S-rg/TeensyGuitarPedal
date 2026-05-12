#pragma once
#include <Audio.h>

#define CHORUS_DELAY_LENGTH 1024
#define FLANGE_DELAY_LENGTH 1024

enum EffectType {
    FX_OVERDRIVE = 0,
    FX_REVERB,
    FX_DELAY,
    FX_CHORUS,
    FX_FLANGE,
    FX_BYPASS,
    FX_FUZZ
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

    AudioFilterBiquad lowpass;

    AudioMixer4 wetDryMixer;
    AudioAmplifier dryGain;
    AudioAmplifier wetGain;

    AudioConnection *patchDry = nullptr;
    AudioConnection *patchWet = nullptr;
    AudioConnection *patchMixOut = nullptr;
    AudioConnection *patchIn = nullptr;
    AudioConnection *patchOut = nullptr;

    AudioStream *input;
    AudioStream *outputNode;
    AudioAmplifier through;

    EffectType currentEffect;
    int currentValue;

    EffectSlot(AudioStream *in, AudioStream *out);
    void initialize(AudioStream *in, AudioStream *out);
    void applyEffect(EffectType typeIndex, int value);
    AudioStream &output();

  private:
    void clearConnections();

    void setOverdrive(float value);
    void setFuzz(float value);
    void setReverb(float value);
    void setDelay(float value);
    void setChorus(int voices);
    void setFlange(float offset, float depth, float rate);
    void setBypass();
};
