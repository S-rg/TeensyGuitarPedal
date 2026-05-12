#pragma once

#include "EffectSlot.h"
#include "PeakingBiquad.h"
#include "preset.h"
#include <Audio.h>
#include <SD.h>
#include <SerialFlash.h>

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
    PresetData to_preset();
    PresetData bypass_save;
    static PresetData bypass_clean;

    AudioInputI2S input;
    AudioOutputI2S output;

    static const char *effectTypeNames[];
    EffectSlot *getSlot(int index);
    void togglePedal(bool state);

    AudioPlaySdWav wavPlayer;
    AudioMixer4 outputMixer;

    void startLoop(const char *filename);
    void stopLoop();
    bool isLoopPlaying();

    void playWav(char name[]);

  private:
    AudioConnection *cFinalOut;
    AudioConnection *cWav;

    static constexpr int NUM_TONE_BANDS = 10;
    static constexpr float SAMPLE_RATE = 44100.0f;
    static constexpr int NUM_EFFECT_TYPES = 7;

    EffectSlot *slot1;
    EffectSlot *slot2;
    EffectSlot *slot3;
    EffectSlot *slot4;

    float toneGains[10];

    AudioFilterPeaking tone[NUM_TONE_BANDS];

    AudioConnection *cTone[NUM_TONE_BANDS];
    AudioConnection *cOut;

    static const float freqs[NUM_TONE_BANDS];

    static const char *effectTypeToString(EffectType type);

    bool loopActive = false;
    char loopFile[13];
};
