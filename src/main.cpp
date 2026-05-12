#include "EffectSlot.h"
#include "PedalBoard.h"
#include "Presets.h"
#include "buttons.h"
#include "pins.h"
#include "screen.h"
#include "settings.h"
#include <Audio.h>
#include <SPI.h>
#include <Wire.h>

Pedalboard pedal;
static bool bypass = false;

void setup() {
    AudioMemory(220);
    pedal.begin();
    setup_screen();
    setup_buttons(B1PIN, B2PIN, ENCAPIN, ENCBPIN, ENCSWPIN, P1PIN, P2PIN, P3PIN,
                  P4PIN, TSWITCHPIN);

    clear_screen();
    draw_wahhh_pedals();
}

void loop() {
    // Tick the loop engine – transparently restarts backing track when it ends
    pedal.isLoopPlaying();

    ButtonEvent ev;
    if (poll_button_event(ev)) {
        if (ev == ButtonEvent::Select || ev == ButtonEvent::Navigate) {
            run_settings_menu(pedal);
            clear_screen();
        } else if (ev == ButtonEvent::Preset1) {
            pedal.loadPreset(the_blues_squared);
        } else if (ev == ButtonEvent::Preset2) {
            pedal.loadPreset(bi_hamba);
        } else if (ev == ButtonEvent::Preset3) {
            pedal.loadPreset(THEBIGBAD);
        } else if (ev == ButtonEvent::Preset4) {
            pedal.loadPreset(the_dark_blues);
        } else if (ev == ButtonEvent::ToggleSwitch) {
            bypass = !bypass;
            pedal.togglePedal(bypass);
        }
    }
}
