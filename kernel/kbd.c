#include "kbd.h"
#include "debug.h"
#include "primitive.h"
#include "interrupts.h"
#include "isr.h"
#include "pic.h"
#include "cpu.h"
#include "pic.h"

#define SCANCODE_PORT 0x60
#define IRQ_INDEX 33
#define BUFFER_SIZE 100

int echo=1;

int index_write = 0;
int index_read = 0;

char keyboard_buffer[BUFFER_SIZE];

void init_keyboard_handler(){
    register_interrupt_handler(33, keyboard_isr);
    unmask_IRQ(1);
}

void keyboard_handler() {
    int scancode = inb(SCANCODE_PORT);
    do_scancode(scancode);
    EOI(IRQ_INDEX);
}

//unsigned long cons_read(char *string, unsigned long length) {
//    (void) string;
//    (void) length;
//    return 1;
//}
unsigned long cons_read(char *string, unsigned long length) {
    (void) string;
    if(length == 0) {
        return 0;
    }
    unsigned long char_length = 0;
    // TODO do a memset on the part of the buffer we read
    while(char_length != length) {

    }

    return 0;
}

void cons_echo(int on){
    echo = on;
}

void keyboard_data(char *str){
    int ind = 0;

    while((str[ind] != '\0')){
        if((int) str[ind] == 127) {
            // we delete the previous char
            int prev = index_write == 0 ? 99 : index_write - 1;
            if((int) keyboard_buffer[prev] != 0 && keyboard_buffer[prev] != '\n') {
                // only delete a char if we can
                index_write = prev;
            }
        } else {
            keyboard_buffer[index_write] = str[ind];
            index_write++;
        }
        ind++;

        // we can overwrite data that is not read in our convention
        if(index_write == BUFFER_SIZE) {
            index_write = 0;
        }
    }

    if(echo == 1){
        cons_write(str, ind);
    }
}

/* You may implement this function to keep keyboard LEDs in sync with the
state of the keyboard driver (do_scancode). */
void kbd_leds(unsigned char leds){
    (void)leds;
}
