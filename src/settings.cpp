#include "settings.h"
#include "PedalBoard.h"
#include "buttons.h"
#include "preset.h"
#include "screen.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>

static int get_encoder_value(int minVal, int maxVal, int currentVal,
                             const char *title) {
    __disable_irq();
    encoderPosition = currentVal;
    __enable_irq();

    long lastPos = currentVal;
    draw_value_screen(title, currentVal);

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
            draw_value_screen(title, pos);
        }

        ButtonEvent ev;
        if (poll_button_event(ev)) {
            if (ev == ButtonEvent::Select) {
                return pos;
            }
        }
    }
}

static void run_effects_menu(Pedalboard &pedal) {
    const char *slotItems[] = {"Slot 1", "Slot 2", "Slot 3", "Slot 4", "Back"};
    int slotSel = 0;
    draw_menu(slotItems, 5, slotSel);

    while (true) {
        ButtonEvent ev;
        if (poll_button_event(ev)) {
            if (ev == ButtonEvent::Navigate) {
                slotSel = (slotSel + 1) % 5;
                draw_menu(slotItems, 5, slotSel);
            } else if (ev == ButtonEvent::Select) {
                if (slotSel == 4)
                    return;

                int effectSel = 0;
                draw_menu(Pedalboard::effectTypeNames, 6, effectSel);
                bool choosingEffect = true;

                while (choosingEffect) {
                    ButtonEvent ev2;
                    if (poll_button_event(ev2)) {
                        if (ev2 == ButtonEvent::Navigate) {
                            effectSel = (effectSel + 1) % 6;
                            draw_menu(Pedalboard::effectTypeNames, 6,
                                      effectSel);
                        } else if (ev2 == ButtonEvent::Select) {
                            int val = 0;
                            if (effectSel != 5) {
                                val = get_encoder_value(
                                    0, 100, 50,
                                    Pedalboard::effectTypeNames[effectSel]);
                            }
                            pedal.getSlot(slotSel)->applyEffect(effectSel, val);
                            choosingEffect = false;
                        }
                    }
                }
                draw_menu(slotItems, 5, slotSel); // Redraw slot menu when done
            }
        }
    }
}

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
    draw_menu(window, visibleCount, selection - scrollOffset);
}

static void run_tone_control_menu(Pedalboard &pedal) {
    int selection = 0;
    int scrollOffset = 0;
    draw_tone_menu(selection, scrollOffset);

    while (true) {
        ButtonEvent ev;
        if (poll_button_event(ev)) {
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
                    return;

                char title[32];
                snprintf(title, sizeof(title), "Band %d Gain", selection + 1);
                int currentGain = (int)pedal.getToneGain(selection);
                int newGain = get_encoder_value(-10, 10, currentGain, title);

                pedal.setToneGain(selection, newGain);
                draw_tone_menu(selection, scrollOffset);
            }
        }
    }
}

static void save_to_preset(Pedalboard &pedal) {
    const char *presetItems[] = {"Preset 1", "Preset 2", "Preset 3", "Preset 4",
                                 "Back"};
    int selection = 0;
    draw_menu(presetItems, 5, selection);

    while (true) {
        ButtonEvent ev;
        if (poll_button_event(ev)) {
            if (ev == ButtonEvent::Navigate) {
                selection = (selection + 1) % 5;
                draw_menu(presetItems, 5, selection);
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
}

static const char *settingsItems[] = {
    "Effects",
    "Tone Control",
    "Save To Preset",
    "Back",
};
static constexpr int NUM_SETTINGS = 4;

void run_settings_menu(Pedalboard &pedal) {
    int selection = 0;
    draw_menu(settingsItems, NUM_SETTINGS, selection);

    while (true) {
        ButtonEvent ev;
        if (poll_button_event(ev)) {
            if (ev == ButtonEvent::Navigate) {
                selection = (selection + 1) % NUM_SETTINGS;
                draw_menu(settingsItems, NUM_SETTINGS, selection);
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
                    return;
                }
                draw_menu(settingsItems, NUM_SETTINGS, selection);
            }
        }
    }
}
