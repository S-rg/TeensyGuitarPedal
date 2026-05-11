#include "EffectSlot.h"
#include "PedalBoard.h"
#include "buttons.h"
#include "pins.h"
#include "screen.h"
#include "settings.h"
#include <Audio.h>
#include <SPI.h>
#include <Wire.h>

Pedalboard pedal;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
    }

    AudioMemory(220);
    pedal.begin();
    setup_screen();
    setup_buttons(B1PIN, B2PIN, ENCAPIN, ENCBPIN, ENCSWPIN, P1PIN, P2PIN, P3PIN,
                  P4PIN, TSWITCHPIN);

    clear_screen();
    // pedal.playWav((char *)"B.WAV");
}

void loop() {
    // pedal.printAudioDebug();
    // Serial.printf("Audio mem used: %u | max: %u\n", AudioMemoryUsage(),
    // AudioMemoryUsageMax());

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
        } else if (ev == ButtonEvent::ToggleSwitch) {
            pedal.togglePedal(false);
        }
    }
}
