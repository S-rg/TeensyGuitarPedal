#pragma once
#include <EEPROM.h>
#include <stdint.h>

struct PresetData {
    int8_t effectTypes[4];
    int8_t effectValues[4];
    int8_t toneGains[10];

    void savePreset(int presetIndex) const {
        const int presetSize = sizeof(PresetData);
        int addr = presetIndex * presetSize;

        EEPROM.put(addr, *this);
    }

    void loadPreset(int presetIndex) {
        const int presetSize = sizeof(PresetData);
        int addr = presetIndex * presetSize;

        EEPROM.get(addr, *this);
    }
};
