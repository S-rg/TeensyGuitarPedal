#include "screen.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup_screen() {
    while (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    }

    display.clearDisplay();
    display.display();
}

void write_to_screen(const char *text) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

void clear_screen() {
    display.clearDisplay();
    display.display();
}

void draw_value_screen(const char *title, int value) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(title);

    display.setTextSize(3);
    display.setCursor(0, 20);
    display.println(value);
    display.display();
}

void draw_menu(const char *items[], int itemCount, int selectedIndex) {
    display.clearDisplay();
    display.setTextSize(1);

    int lineHeight = 10;

    for (int i = 0; i < itemCount; i++) {
        int y = i * lineHeight;

        if (i == selectedIndex) {
            display.fillRect(0, y, SCREEN_WIDTH, lineHeight, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(0, y);
        display.println(items[i]);
    }

    display.display();
}
