#include "buttons.h"
#include "avr/pgmspace.h"
#include "core_pins.h"
#include <Arduino.h>
#include <cstdint>

static constexpr uint32_t DEBOUNCE_US = 60000;           // 60 ms
static constexpr uint32_t DOUBLE_EDGE_TIMEOUT = 1000000; // 2s

static volatile uint32_t lastB1Time = 0;
static volatile uint32_t lastB2Time = 0;
static volatile uint32_t lastSWTime = 0;
static volatile uint32_t lastP1Time = 0;
static volatile uint32_t lastP2Time = 0;
static volatile uint32_t lastP3Time = 0;
static volatile uint32_t lastP4Time = 0;
static volatile uint32_t lastTSwitchTime = 0;

static volatile bool lastB1State = false;
static volatile bool lastB2State = false;
static volatile bool lastSWState = false;
static volatile bool lastP1State = false;
static volatile bool lastP2State = false;
static volatile bool lastP3State = false;
static volatile bool lastP4State = false;

static volatile bool lastTSwitchState = false;

static constexpr uint8_t QUEUE_SIZE = 8;
static volatile ButtonEvent eventQueue[QUEUE_SIZE];
static volatile uint8_t qHead = 0;
static volatile uint8_t qTail = 0;

static inline bool FASTRUN debounce(volatile uint32_t &lastTime,
                                    volatile bool &lastState) {
    uint32_t now = micros();
    uint32_t dt = (uint32_t)(now - lastTime);
    if (dt < DEBOUNCE_US) {
        return false;
    }

    if (dt > DOUBLE_EDGE_TIMEOUT) {
        lastState = false;
    }

    lastTime = now;
    lastState = !lastState;
    return lastState;
}

static void FASTRUN enqueue(ButtonEvent ev) {
    uint8_t next = (qTail + 1) & (QUEUE_SIZE - 1);

    if (next != qHead) {
        eventQueue[qTail] = ev;
        qTail = next;
    }
}

static bool dequeue(ButtonEvent &ev) {
    if (qHead == qTail)
        return false;
    ev = eventQueue[qHead];
    qHead = (qHead + 1) & (QUEUE_SIZE - 1);
    return true;
}

// FASTRUN puts this in better memory places so it can be run faster
void FASTRUN b1_ISR() {
    if (debounce(lastB1Time, lastB1State))
        enqueue(ButtonEvent::Navigate);
}

void FASTRUN b2_ISR() {
    if (debounce(lastB2Time, lastB2State))
        enqueue(ButtonEvent::Select);
}

void FASTRUN sw_ISR() {
    if (debounce(lastSWTime, lastSWState))
        enqueue(ButtonEvent::Select);
}

void FASTRUN p1_ISR() {
    if (debounce(lastP1Time, lastP1State))
        enqueue(ButtonEvent::Preset1);
}

void FASTRUN p2_ISR() {
    if (debounce(lastP2Time, lastP2State))
        enqueue(ButtonEvent::Preset2);
}

void FASTRUN p3_ISR() {
    if (debounce(lastP3Time, lastP3State))
        enqueue(ButtonEvent::Preset3);
}

void FASTRUN p4_ISR() {
    if (debounce(lastP4Time, lastP4State))
        enqueue(ButtonEvent::Preset4);
}

void FASTRUN tSwitch_ISR() {
    if (debounce(lastTSwitchTime, lastTSwitchState))
        enqueue(ButtonEvent::ToggleSwitch);
}

volatile long encoderPosition = 0;
static int _encAPin;
static int _encBPin;

void FASTRUN encoder_ISR() {
    if (digitalReadFast(_encAPin) == digitalReadFast(_encBPin)) {
        encoderPosition--;
    } else {
        encoderPosition++;
    }
}

void setup_buttons(int b1Pin, int b2Pin, int encAPin, int encBPin, int encSWPin,
                   int preset1Pin, int preset2Pin, int preset3Pin,
                   int preset4Pin, int tSwitchPin) {
    _encAPin = encAPin;
    _encBPin = encBPin;

    pinMode(b1Pin, INPUT_PULLUP);
    pinMode(b2Pin, INPUT_PULLUP);
    pinMode(encSWPin, INPUT_PULLUP);
    pinMode(encAPin, INPUT_PULLUP);
    pinMode(encBPin, INPUT_PULLUP);

    pinMode(preset1Pin, INPUT_PULLUP);
    pinMode(preset2Pin, INPUT_PULLUP);
    pinMode(preset3Pin, INPUT_PULLUP);
    pinMode(preset4Pin, INPUT_PULLUP);

    pinMode(tSwitchPin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(b1Pin), b1_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(b2Pin), b2_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(encSWPin), sw_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(encAPin), encoder_ISR, CHANGE);

    attachInterrupt(digitalPinToInterrupt(preset1Pin), p1_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(preset2Pin), p2_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(preset3Pin), p3_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(preset4Pin), p4_ISR, FALLING);

    attachInterrupt(digitalPinToInterrupt(tSwitchPin), tSwitch_ISR, CHANGE);
}

bool poll_button_event(ButtonEvent &ev) {
    // Non blocking poll for encoder loop
    __disable_irq();
    bool available = (qHead != qTail);
    __enable_irq();

    if (available && dequeue(ev))
        return true;
    return false;
}

ButtonEvent wait_for_button_event() {
    ButtonEvent ev;
    while (true) {
        __disable_irq();
        bool available = (qHead != qTail);
        __enable_irq();

        if (available && dequeue(ev))
            return ev;

        asm("wfi"); // Wait for interrupt
    }
}
