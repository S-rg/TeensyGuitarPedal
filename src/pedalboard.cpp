#include "PedalBoard.h"
#include "mixer.h"
#include "preset.h"
#include "screen.h"

const float Pedalboard::freqs[NUM_TONE_BANDS] = {80,   120,  250,  500,  1000,
                                                 2000, 4000, 6000, 8000, 12000};

const char *Pedalboard::effectTypeNames[] = {"Distortion", "Reverb", "Delay",
                                             "Chorus",     "Flange", "None"};

Pedalboard::Pedalboard() {}

void Pedalboard::begin() {
    // AudioAmplifier dummyNode;

    cTone[0] = new AudioConnection(input, 1, tone[0], 0);
    for (int i = 0; i < NUM_TONE_BANDS - 1; i++) {
        cTone[i + 1] = new AudioConnection(tone[i], 0, tone[i + 1], 0);
    }

    slot1 = new EffectSlot(nullptr, nullptr);
    slot2 = new EffectSlot(nullptr, nullptr);
    slot3 = new EffectSlot(nullptr, nullptr);
    slot4 = new EffectSlot(nullptr, nullptr);

    slot1->initialize(&tone[NUM_TONE_BANDS - 1], &slot2->bypass);
    slot2->initialize(&slot1->bypass, &slot3->bypass);
    slot3->initialize(&slot2->bypass, &slot4->bypass);
    slot4->initialize(&slot3->bypass, &finalOut);

    cFinalOut = new AudioConnection(finalOut, 0, output, 1);
    finalOut.gain(1.0f);

    for (int i = 0; i < NUM_TONE_BANDS; i++) {
        toneGains[i] = 0.0f;
    }

    setupTone();
}

void Pedalboard::loadPreset(const PresetData &p) {
    for (int i = 0; i < 4; i++) {
        EffectSlot *slot = getSlot(i);
        EffectType type = static_cast<EffectType>(p.effectTypes[i]);
        int value = p.effectValues[i];
        slot->applyEffect(type, value);
    }

    for (int i = 0; i < NUM_TONE_BANDS; i++) {
        setToneGain(i, p.toneGains[i]);
    }
}

Pedalboard::~Pedalboard() {
    delete slot1;
    delete slot2;
    delete slot3;
    delete slot4;

    for (int i = 0; i < NUM_TONE_BANDS; i++) {
        delete cTone[i];
    }
}

void Pedalboard::setupTone() {
    for (int i = 0; i < NUM_TONE_BANDS; i++) {
        tone[i].setPeaking(0, freqs[i], SAMPLE_RATE, 1.2f, 0.0f);
    }
}

void Pedalboard::setToneGain(int band, float gainDB) {
    if (band < 0 || band >= NUM_TONE_BANDS)
        return;

    tone[band].setPeaking(0, freqs[band], SAMPLE_RATE, 1.2f, gainDB);
    toneGains[band] = gainDB;
}

const char *Pedalboard::effectTypeToString(EffectType type) {
    if (type < 0 || type >= NUM_EFFECT_TYPES) {
        return "Unknown";
    }
    return effectTypeNames[type];
}

EffectSlot *Pedalboard::getSlot(int index) {
    switch (index) {
    case 0:
        return slot1;
    case 1:
        return slot2;
    case 2:
        return slot3;
    case 3:
        return slot4;
    default:
        return slot1;
    }
}

float Pedalboard::getToneGain(int band) {
    if (band < 0 || band >= NUM_TONE_BANDS)
        return 0.0f;
    return toneGains[band];
}

void Pedalboard::print_menu(int selected) {
    constexpr int NUM_SLOTS = 4;

    EffectSlot *slots[NUM_SLOTS] = {slot1, slot2, slot3, slot4};

    static char lines[NUM_SLOTS][32];
    const char *items[NUM_SLOTS];

    for (int i = 0; i < NUM_SLOTS; i++) {
        snprintf(lines[i], sizeof(lines[i]), "Effect %d: %s", i + 1,
                 effectTypeToString(slots[i]->currentEffect));

        items[i] = lines[i];
    }

    draw_menu(items, NUM_SLOTS, selected);
}
