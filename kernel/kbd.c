#include "kbd.h"
#include "debug.h"


/* You have to implement this function. It is called by do_scancode with
a string that is the translation of the scancodes into characters. */
void keyboard_data(char *str){
    //(void)str;
    
    //cons_read doit être appelée quelque part au préalable

    int i = 0;
    while((str[i] != '\0') && (i<100)){
        if(str[i]== 13){  // Entrée
            break;
        }else if(str[i]== 127){
            i--;
        }else{
            keyboard_buffer[i] = str[i];
            i++;
        }
    }

    // if echo
    // cons_write(keyboard_buffer, i)
}

unsigned long cons_read(char *string, unsigned long length){
    printf("read_line");
    (void)string;
    (void)length;
    return 0;
}
/* DEBUG
#if defined CONS_READ_LINE
unsigned long cons_read(char *string, unsigned long length){
    printf("read_line");
    (void)string;
    (void)length;
    return 0;
}
#elif defined CONS_READ_CHAR
int cons_read(void){
    printf("read_char");
    return 0;
}
#endif
*/
void cons_echo(int on){
    printf("ok");
    (void)on;
}

/* You may implement this function to keep keyboard LEDs in sync with the
state of the keyboard driver (do_scancode). */
void kbd_leds(unsigned char leds){
    (void)leds;
}