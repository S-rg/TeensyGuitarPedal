#include "buttons.h"
#include "avr/pgmspace.h"
#include "core_pins.h"
#include <Arduino.h>

static constexpr uint8_t QUEUE_SIZE = 8;
static volatile ButtonEvent eventQueue[QUEUE_SIZE];
static volatile uint8_t qHead = 0;
static volatile uint8_t qTail = 0;

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
void FASTRUN b1_ISR() { enqueue(ButtonEvent::Navigate); }
void FASTRUN b2_ISR() { enqueue(ButtonEvent::Select); }
void FASTRUN sw_ISR() { enqueue(ButtonEvent::Select); }
void FASTRUN p1_ISR() { enqueue(ButtonEvent::Preset1); }
void FASTRUN p2_ISR() { enqueue(ButtonEvent::Preset2); }
void FASTRUN p3_ISR() { enqueue(ButtonEvent::Preset3); }
void FASTRUN p4_ISR() { enqueue(ButtonEvent::Preset4); }

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
                   int preset4Pin) {
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

    attachInterrupt(digitalPinToInterrupt(b1Pin), b1_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(b2Pin), b2_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(encSWPin), sw_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(encAPin), encoder_ISR, CHANGE);

    attachInterrupt(digitalPinToInterrupt(preset1Pin), p1_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(preset2Pin), p2_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(preset3Pin), p3_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(preset4Pin), p4_ISR, FALLING);
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
