#include "kbd.h"
#include "primitive.h"
#include "interrupts.h"
#include "isr.h"
#include "pic.h"
#include "cpu.h"
#include "pic.h"
#include "debug.h"

#define SCANCODE_PORT 0x60
#define IRQ_INDEX 33
#define BUFFER_SIZE 1000

int echo = 1;

int index_write = 0;
int index_read  = 0;
int nb_write    = 0;

char keyboard_buffer[BUFFER_SIZE];

void init_keyboard_handler()
{
    register_interrupt_handler(33, keyboard_isr);
    unmask_IRQ(1);
}

void keyboard_handler()
{
    int scancode = inb(SCANCODE_PORT);
    do_scancode(scancode);
    EOI(IRQ_INDEX);
}

unsigned long cons_read(char *string, unsigned long length) {
    (void) string;
    if(length == 0) {
        return 0;
    }
    unsigned long char_length = 0;
    while(char_length != length) {
        //wait until a character is tap
        while (keyboard_buffer[index_read] == 0) {
            sti();
            hlt();
            cli();
        }
        if (keyboard_buffer[index_read] == '\r') {
            keyboard_buffer[index_read] = 0;
            index_read = index_read == BUFFER_SIZE - 1 ? 0 : index_read + 1;
            break;
        } else if ((int)keyboard_buffer[index_read] == 127) {
            string[char_length] = 0;
            if (char_length > 0) {
                char_length--;
            }
        } else if ((int)keyboard_buffer[index_read] != 127) {
            string[char_length]         = keyboard_buffer[index_read];
            keyboard_buffer[index_read] = (char)0;
            char_length++;
        }
        index_read = index_read == BUFFER_SIZE - 1 ? 0 : index_read + 1;
    }
    return char_length;
}

void cons_echo(int on){
    echo = on;
}

void keyboard_data(char *str)
{
    int ind = 0;

    while ((str[ind] != '\0')) {
        keyboard_buffer[index_write] = str[ind];
        index_write++;

        // we can overwrite data that is not read in our convention
        if (index_write == BUFFER_SIZE) {
            index_write = 0;
        }

        if (str[ind] == '\r') {
            nb_write = 0;
            if (echo == 1) {
                cons_write("\r\n", 2);
            }
        } else if ((int)str[ind] == 127) {
            if (nb_write != 0) {
                nb_write--;
                if (echo == 1) {
                    cons_write(&str[ind], 1);
                }
            }
        } else {
            if (echo == 1) {
                cons_write(&str[ind], 1);
            }
            nb_write++;
        }

        ind++;
    }
}

/* You may implement this function to keep keyboard LEDs in sync with the
state of the keyboard driver (do_scancode). */
void kbd_leds(unsigned char leds){
    (void)leds;
}
