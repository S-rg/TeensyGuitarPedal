#pragma once

#include "EffectSlot.h"
#include "PeakingBiquad.h"
#include "preset.h"
#include <Audio.h>

class Pedalboard {
  public:
    Pedalboard();
    ~Pedalboard();

    void begin();

    void setupTone();
    void setToneGain(int band, float gainDB);
    float getToneGain(int band);
    void print_menu(int selected);

    void loadPreset(const PresetData &p);

    AudioInputI2S input;
    AudioOutputI2S output;

    static const char *effectTypeNames[];

    EffectSlot *getSlot(int index);

  private:
    AudioAmplifier finalOut;
    AudioConnection *cFinalOut;

    static constexpr int NUM_TONE_BANDS = 10;
    static constexpr float SAMPLE_RATE = 44100.0f;

    EffectSlot *slot1;
    EffectSlot *slot2;
    EffectSlot *slot3;
    EffectSlot *slot4;

    float toneGains[10];

    AudioFilterPeaking tone[NUM_TONE_BANDS];

    AudioConnection *cTone[NUM_TONE_BANDS];
    AudioConnection *cOut;

    static const float freqs[NUM_TONE_BANDS];
    static const int NUM_EFFECT_TYPES = 6;

    static const char *effectTypeToString(EffectType type);
};
