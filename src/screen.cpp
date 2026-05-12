#include "screen.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

void setup_screen() {
    Wire1.begin();

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

void draw_value_screen(const char *title, float value) {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(title);

    display.setTextSize(3);
    display.setCursor(0, 20);
    display.println(value, 1);

    display.display();
}

void draw_menu(const char *title, const char *items[], int itemCount,
               int selectedIndex) {

    display.clearDisplay();

    const int titleHeight = 16;
    const int lineHeight = 10;
    const int visibleRows = 5;

    int scrollOffset = 0;

    if (selectedIndex >= visibleRows) {
        scrollOffset = selectedIndex - visibleRows + 1;
    }

    int maxScroll = max(0, itemCount - visibleRows);
    scrollOffset = min(scrollOffset, maxScroll);

    display.fillRect(0, 0, SCREEN_WIDTH, titleHeight, SSD1306_WHITE);

    display.setTextSize(2);
    display.setTextColor(SSD1306_BLACK);

    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);

    int titleX = (SCREEN_WIDTH - w) / 2;
    int titleY = (titleHeight - h) / 2;

    display.setCursor(titleX, titleY);
    display.println(title);

    display.setTextSize(1);

    int visibleCount = min(itemCount - scrollOffset, visibleRows);

    for (int i = 0; i < visibleCount; i++) {
        int actualIndex = scrollOffset + i;
        int y = titleHeight + (i * lineHeight);

        if (actualIndex == selectedIndex) {
            display.fillRect(0, y, SCREEN_WIDTH, lineHeight, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(0, y + 1);
        display.println(items[actualIndex]);
    }

    display.display();
}

void draw_wahhh_pedals() {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(9, 17);
    display.println("WAHHHH");

    display.setCursor(15, 41);
    display.println("PEDALS");

    display.setTextColor(SSD1306_BLACK);
    display.fillRect(4, 12, 120, 18, SSD1306_WHITE);
    display.fillRect(10, 36, 108, 18, SSD1306_WHITE);

    display.setCursor(6, 14);
    display.println("WAHHHH");

    display.setCursor(12, 38);
    display.println("PEDALS");

    display.display();
}
