#include "PedalBoard.h"
#include "Presets.h"
#include "SD.h"
#include "mixer.h"
#include "preset.h"
#include "screen.h"

PresetData Pedalboard::bypass_clean = the_blues;

const float Pedalboard::freqs[NUM_TONE_BANDS] = {80,   120,  250,  500,  1000,
                                                 2000, 4000, 6000, 8000, 12000};

const char *Pedalboard::effectTypeNames[] = {
    "Overdrive", "Reverb", "Delay", "Chorus", "Flange", "None", "Fuzz"};

Pedalboard::Pedalboard() {}

void Pedalboard::begin() {
    SPI.begin();
    SD.begin(BUILTIN_SDCARD);

    cTone[0] = new AudioConnection(input, 1, tone[0], 0);
    for (int i = 0; i < NUM_TONE_BANDS - 1; i++) {
        cTone[i + 1] = new AudioConnection(tone[i], 0, tone[i + 1], 0);
    }

    slot1 = new EffectSlot(nullptr, nullptr);
    slot2 = new EffectSlot(nullptr, nullptr);
    slot3 = new EffectSlot(nullptr, nullptr);
    slot4 = new EffectSlot(nullptr, nullptr);

    slot1->initialize(&tone[NUM_TONE_BANDS - 1], &slot1->through);
    slot2->initialize(&slot1->through, &slot2->through);
    slot3->initialize(&slot2->through, &slot3->through);
    slot4->initialize(&slot3->through, &slot4->through);

    cOut = new AudioConnection(slot4->through, 0, outputMixer, 0);
    cWav = new AudioConnection(wavPlayer, 0, outputMixer, 1);
    cFinalOut = new AudioConnection(outputMixer, 0, output, 1);

    for (int i = 0; i < NUM_TONE_BANDS; i++)
        toneGains[i] = 0.0f;

    outputMixer.gain(0, 1.0f);
    outputMixer.gain(1, 0.2f);

    loopActive = false;
    loopFile[0] = '\0';

    setupTone();
}

void Pedalboard::loadPreset(const PresetData &p) {
    for (int i = 0; i < 4; i++) {
        getSlot(i)->applyEffect(static_cast<EffectType>(p.effectTypes[i]),
                                p.effectValues[i]);
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
    for (int i = 0; i < NUM_TONE_BANDS; i++)
        delete cTone[i];
}

void Pedalboard::setupTone() {
    for (int i = 0; i < NUM_TONE_BANDS; i++)
        tone[i].setPeaking(0, freqs[i], SAMPLE_RATE, 1.2f, 0.0f);
}

void Pedalboard::setToneGain(int band, float gainDB) {
    if (band < 0 || band >= NUM_TONE_BANDS)
        return;
    tone[band].setPeaking(0, freqs[band], SAMPLE_RATE, 1.2f, gainDB);
    toneGains[band] = gainDB;
}

float Pedalboard::getToneGain(int band) {
    if (band < 0 || band >= NUM_TONE_BANDS)
        return 0.0f;
    return toneGains[band];
}

const char *Pedalboard::effectTypeToString(EffectType type) {
    if (type < 0 || type >= NUM_EFFECT_TYPES)
        return "Unknown";
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

void Pedalboard::print_menu(int selected) {
    constexpr int NUM_SLOTS = 4;
    EffectSlot *slots[NUM_SLOTS] = {slot1, slot2, slot3, slot4};

    static char lines[NUM_SLOTS][32];
    const char *items[NUM_SLOTS];

    for (int i = 0; i < NUM_SLOTS; i++) {
        snprintf(lines[i], sizeof(lines[i]), "%d:%.9s %d", i + 1,
                 effectTypeToString(slots[i]->currentEffect),
                 slots[i]->currentValue);
        items[i] = lines[i];
    }
    draw_menu("Slots", items, NUM_SLOTS, selected);
}

void Pedalboard::playWav(char name[]) {
    Serial.print("Playing: ");
    Serial.println(name);
    wavPlayer.play(name);
    delay(10);
    Serial.println(wavPlayer.isPlaying() ? "WAV playing" : "WAV failed");
}

void Pedalboard::startLoop(const char *filename) {
    strncpy(loopFile, filename, sizeof(loopFile) - 1);
    loopFile[sizeof(loopFile) - 1] = '\0';
    loopActive = true;
    wavPlayer.play(loopFile);
}

void Pedalboard::stopLoop() {
    loopActive = false;
    wavPlayer.stop();
}

bool Pedalboard::isLoopPlaying() {
    if (loopActive && !wavPlayer.isPlaying()) {
        wavPlayer.play(loopFile);
    }
    return loopActive;
}

void Pedalboard::togglePedal(bool state) {
    if (state) {
        bypass_save = this->to_preset();
        this->loadPreset(bypass_clean);
    } else {
        this->loadPreset(bypass_save);
    }
}

PresetData Pedalboard::to_preset() {
    PresetData p{};
    for (int i = 0; i < 4; i++) {
        EffectSlot *slot = getSlot(i);
        if (slot) {
            p.effectTypes[i] = static_cast<int8_t>(slot->currentEffect);
            p.effectValues[i] = static_cast<int8_t>(slot->currentValue);
        }
    }
    for (int i = 0; i < NUM_TONE_BANDS; i++) {
        p.toneGains[i] = static_cast<int8_t>(toneGains[i]);
    }
    return p;
}
