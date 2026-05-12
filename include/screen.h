#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>

void setup_screen();

void write_to_screen(const char *text);

void draw_menu(const char *title, const char *items[], int itemCount,
               int selectedIndex);

void draw_value_screen(const char *title, float value);

void clear_screen();

void draw_wahhh_pedals();
#endif
