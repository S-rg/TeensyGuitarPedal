#include "PedalBoard.h"
#include "buttons.h"
#include "screen.h"
#include "settings.h"
#include <Audio.h>
#include <SPI.h>
#include <Wire.h>

Pedalboard pedal;

void setup() {
    AudioMemory(20);
    pedal.begin();
    setup_screen();
    setup_buttons(2, 3, 4, 5, 6, 7, 8, 9, 10);

    clear_screen();
}

void loop() {
    ButtonEvent ev;
    if (poll_button_event(ev)) {
        if (ev == ButtonEvent::Select || ev == ButtonEvent::Navigate) {
            run_settings_menu(pedal);

            clear_screen(); // Run on completion;
        }

        else if (ev == ButtonEvent::Preset1) {
            PresetData p;
            p.loadPreset(0);

            pedal.loadPreset(p);
        } else if (ev == ButtonEvent::Preset2) {
            PresetData p;
            p.loadPreset(1);

            pedal.loadPreset(p);
        } else if (ev == ButtonEvent::Preset3) {
            PresetData p;
            p.loadPreset(2);

            pedal.loadPreset(p);
        } else if (ev == ButtonEvent::Preset4) {
            PresetData p;
            p.loadPreset(3);

            pedal.loadPreset(p);
        }
    }
}
