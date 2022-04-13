

#include "kbd.h"

/* You have to implement this function. It is called by do_scancode with
a string that is the translation of the scancodes into characters. */
void keyboard_data(char *str){
    (void)str;

    // int len = 0;
    // (recherche de len)
    // cons_write(str, len(str)) s le caractère est writeable
}

/* You may implement this function to keep keyboard LEDs in sync with the
state of the keyboard driver (do_scancode). */
void kbd_leds(unsigned char leds){
    (void)leds;
}