#include "settings.h"
#include "PedalBoard.h"
#include "buttons.h"
#include "preset.h"
#include "screen.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <SD.h>
#include <stdint.h>
#include <string.h>

// ─── Encoder value picker ────────────────────────────────────────────────────

static int get_encoder_value(int minVal, int maxVal, int currentVal,
                             const char *title, float div = 1.0f) {
    __disable_irq();
    encoderPosition = currentVal;
    __enable_irq();

    long lastPos = currentVal;
    draw_value_screen(title, (float)currentVal / div);

    while (true) {
        __disable_irq();
        long pos = encoderPosition;
        __enable_irq();

        if (pos != lastPos) {
            if (pos < minVal)
                pos = minVal;
            if (pos > maxVal)
                pos = maxVal;

            __disable_irq();
            encoderPosition = pos;
            __enable_irq();

            lastPos = pos;
            draw_value_screen(title, pos / div);
        }

        ButtonEvent ev;
        if (poll_button_event(ev))
            if (ev == ButtonEvent::Select)
                return (int)pos;
    }
}

// ─── Effects menu ────────────────────────────────────────────────────────────

// Rebuild slot labels (e.g. "1:Overdrive 75") and redraw the slot chooser
static void refresh_slot_menu(Pedalboard &pedal, char slotLines[][20],
                              const char **slotItems, int sel) {
    for (int i = 0; i < 4; i++) {
        EffectSlot *slot = pedal.getSlot(i);
        snprintf(slotLines[i], 20, "%d:%.8s %d", i + 1,
                 Pedalboard::effectTypeNames[slot->currentEffect],
                 slot->currentValue);
        slotItems[i] = slotLines[i];
    }
    slotItems[4] = "Back";
    draw_menu("Slots", slotItems, 5, sel);
}

static void run_effects_menu(Pedalboard &pedal) {
    static char slotLines[4][20];
    const char *slotItems[5];

    int slotSel = 0;
    refresh_slot_menu(pedal, slotLines, slotItems, slotSel);

    while (true) {
        ButtonEvent ev;
        if (!poll_button_event(ev))
            continue;

        if (ev == ButtonEvent::Navigate) {
            slotSel = (slotSel + 1) % 5;
            refresh_slot_menu(pedal, slotLines, slotItems, slotSel);
        } else if (ev == ButtonEvent::Select) {
            if (slotSel == 4)
                return; // Back

            static constexpr int NUM_FX = 7; // Overdrive…Fuzz
            int effectSel = 0;
            bool choosingEffect = true;
            draw_menu("Effects", Pedalboard::effectTypeNames, NUM_FX,
                      effectSel);

            while (choosingEffect) {
                ButtonEvent ev2;
                if (!poll_button_event(ev2))
                    continue;

                if (ev2 == ButtonEvent::Navigate) {
                    effectSel = (effectSel + 1) % NUM_FX;
                    draw_menu("Effects", Pedalboard::effectTypeNames, NUM_FX,
                              effectSel);
                } else if (ev2 == ButtonEvent::Select) {
                    int val = 0;
                    if (effectSel != FX_BYPASS) { // bypass needs no value
                        val = get_encoder_value(
                            0, 100, 50, Pedalboard::effectTypeNames[effectSel]);
                    }
                    pedal.getSlot(slotSel)->applyEffect(
                        static_cast<EffectType>(effectSel), val);
                    choosingEffect = false;
                }
            }
            // Refresh slot list so the updated label is visible
            refresh_slot_menu(pedal, slotLines, slotItems, slotSel);
        }
    }
}

// ─── Tone control menu ───────────────────────────────────────────────────────

static const char *bandLabels[] = {
    "Band 1 :   80 Hz",
    "Band 2 :  120 Hz",
    "Band 3 :  250 Hz",
    "Band 4 :  500 Hz",
    "Band 5 : 1000 Hz",
    "Band 6 : 2000 Hz",
    "Band 7 : 4000 Hz",
    "Band 8 : 6000 Hz",
    "Band 9 : 8000 Hz",
    "Band 10: 12000 Hz",
    "Back",
};
static constexpr int NUM_BAND_ITEMS = 11;
static constexpr int VISIBLE_ROWS = 6;

static void draw_tone_menu(int selection, int scrollOffset) {
    int visibleCount = min(NUM_BAND_ITEMS - scrollOffset, VISIBLE_ROWS);
    const char *window[VISIBLE_ROWS];
    for (int i = 0; i < visibleCount; i++)
        window[i] = bandLabels[scrollOffset + i];
    draw_menu("Bands", window, visibleCount, selection - scrollOffset);
}

static void run_tone_control_menu(Pedalboard &pedal) {
    int selection = 0;
    int scrollOffset = 0;
    draw_tone_menu(selection, scrollOffset);

    while (true) {
        ButtonEvent ev;
        if (!poll_button_event(ev))
            continue;

        if (ev == ButtonEvent::Navigate) {
            selection = (selection + 1) % NUM_BAND_ITEMS;
            if (selection == 0) {
                scrollOffset = 0;
            } else if (selection >= scrollOffset + VISIBLE_ROWS) {
                scrollOffset = selection - VISIBLE_ROWS + 1;
            }
            draw_tone_menu(selection, scrollOffset);
        } else if (ev == ButtonEvent::Select) {
            if (selection == NUM_BAND_ITEMS - 1)
                return; // Back

            char title[32];
            snprintf(title, sizeof(title), "Band %d Gain", selection + 1);
            int currentGain = (int)pedal.getToneGain(selection);
            int newGain =
                get_encoder_value(-100, 100, currentGain, title, 10.0f);
            pedal.setToneGain(selection, newGain / 10.f);
            draw_tone_menu(selection, scrollOffset);
        }
    }
}

// ─── Save preset menu ────────────────────────────────────────────────────────

static void save_to_preset(Pedalboard &pedal) {
    const char *presetItems[] = {"Preset 1", "Preset 2", "Preset 3", "Preset 4",
                                 "Back"};
    int selection = 0;
    draw_menu("Presets", presetItems, 5, selection);

    while (true) {
        ButtonEvent ev;
        if (!poll_button_event(ev))
            continue;

        if (ev == ButtonEvent::Navigate) {
            selection = (selection + 1) % 5;
            draw_menu("Presets", presetItems, 5, selection);
        } else if (ev == ButtonEvent::Select) {
            if (selection == 4)
                return; // Back

            PresetData p;
            for (int i = 0; i < 4; i++) {
                p.effectTypes[i] = pedal.getSlot(i)->currentEffect;
                p.effectValues[i] = pedal.getSlot(i)->currentValue;
            }
            for (int i = 0; i < 10; i++) {
                p.toneGains[i] = (int)pedal.getToneGain(i);
            }
            p.savePreset(selection);

            write_to_screen("Saved!");
            delay(1000);
            return;
        }
    }
}

// ─── Backing track menu ──────────────────────────────────────────────────────

static constexpr int MAX_WAV_FILES = 16;

static void run_backing_track_menu(Pedalboard &pedal) {
    static char filenames[MAX_WAV_FILES][13];    // 8.3 name + null
    static const char *items[MAX_WAV_FILES + 2]; // files + Stop + Back
    int count = 0;

    // Scan SD card root for .WAV files
    File root = SD.open("/");
    if (root) {
        while (count < MAX_WAV_FILES) {
            File entry = root.openNextFile();
            if (!entry)
                break;
            if (!entry.isDirectory()) {
                const char *name = entry.name();
                int len = strlen(name);
                if (len >= 4) {
                    const char *ext = name + len - 4;
                    // FAT returns uppercase; tolerate lowercase just in case
                    if (strcmp(ext, ".WAV") == 0 || strcmp(ext, ".wav") == 0) {
                        strncpy(filenames[count], name, 12);
                        filenames[count][12] = '\0';
                        items[count] = filenames[count];
                        count++;
                    }
                }
            }
            entry.close();
        }
        root.close();
    }

    if (count == 0) {
        write_to_screen("No WAV files!");
        delay(1500);
        return;
    }

    int stopIdx = count;
    int backIdx = count + 1;
    items[stopIdx] = "Stop";
    items[backIdx] = "Back";
    int total = count + 2;

    int selection = 0;
    draw_menu("Tracks", items, total, selection);

    while (true) {
        ButtonEvent ev;
        if (!poll_button_event(ev))
            continue;

        if (ev == ButtonEvent::Navigate) {
            selection = (selection + 1) % total;
            draw_menu("Tracks", items, total, selection);
        } else if (ev == ButtonEvent::Select) {
            if (selection == backIdx) {
                return;
            } else if (selection == stopIdx) {
                pedal.stopLoop();
                write_to_screen("Stopped.");
                delay(800);
                return;
            } else {
                // Start looping selected track; brief confirmation then exit
                pedal.startLoop(filenames[selection]);
                write_to_screen(filenames[selection]);
                delay(800);
                return;
            }
        }
    }
}

// ─── Top-level settings menu ─────────────────────────────────────────────────

static const char *settingsItems[] = {
    "Effects", "Tone Control", "Save To Preset", "Backing Track", "Back",
};
static constexpr int NUM_SETTINGS = 5;

void run_settings_menu(Pedalboard &pedal) {
    int selection = 0;
    draw_menu("Settings", settingsItems, NUM_SETTINGS, selection);

    while (true) {
        ButtonEvent ev;
        if (!poll_button_event(ev))
            continue;

        if (ev == ButtonEvent::Navigate) {
            selection = (selection + 1) % NUM_SETTINGS;
            draw_menu("Settings", settingsItems, NUM_SETTINGS, selection);
        } else if (ev == ButtonEvent::Select) {
            switch (selection) {
            case 0:
                run_effects_menu(pedal);
                break;
            case 1:
                run_tone_control_menu(pedal);
                break;
            case 2:
                save_to_preset(pedal);
                break;
            case 3:
                run_backing_track_menu(pedal);
                break;
            case 4:
                return;
            }
            draw_menu("Settings", settingsItems, NUM_SETTINGS, selection);
        }
    }
}
