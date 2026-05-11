#pragma once
#include <Arduino.h>

enum class ButtonEvent : uint8_t {
    Navigate,
    Select,
    Preset1,
    Preset2,
    Preset3,
    Preset4,
    ToggleSwitch
};

void setup_buttons(int b1Pin, int b2Pin, int encAPin, int encBPin, int encSWPin,
                   int Preset1, int Preset2, int Preset3, int Preset4,
                   int tSwitchPin);

ButtonEvent wait_for_button_event();
bool poll_button_event(ButtonEvent &ev);

extern volatile long encoderPosition;
