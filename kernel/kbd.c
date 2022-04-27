#include "kbd.h"
#include "debug.h"
#include "primitive.h"
#include "interrupts.h"
#include "isr.h"
#include "pic.h"

int echo=1;
int ind_kb=0;

void init_keyboard_handler(){
    register_interrupt_handler(33, keyboard_isr);
    unmask_IRQ(1);
}

unsigned long cons_read(char *string, unsigned long length){
    printf("read_line");
    unsigned long i = 0;
    while((ind_kb<100) && (i>length)){
        keyboard_buffer[ind_kb++] = string[length++];
    }
    return 0;
}
/* DEBUG
#if defined CONS_READ_LINE
unsigned long cons_read(char *string, unsigned long length){
    printf("read_line");
    int i = 0;
    while(ind_kb<100 && i>length){
        keyboard_buffer[ind_kb++] = string[length++];
    }
    return 0;
}
#elif defined CONS_READ_CHAR
int cons_read(void){
    printf("read_char");
    keyboard_buffer[ind_kb++] = ?
    return 0;
}
#endif
*/
void cons_echo(int on){
    printf("echo");
    echo = on;
}

/* You have to implement this function. It is called by do_scancode with
a string that is the translation of the scancodes into characters. */
void keyboard_data(char *str){
    //(void)str;
    
    //cons_read doit être appelée quelque part au préalable

    int i = 0;
    while((str[i] != '\0') && (i<100)){
        keyboard_buffer[i] = str[i];
        i++;
    }

    if(echo==1){
        //printf("echo is on");
        cons_write(keyboard_buffer, i);
    }
}

/* You may implement this function to keep keyboard LEDs in sync with the
state of the keyboard driver (do_scancode). */
void kbd_leds(unsigned char leds){
    (void)leds;
}