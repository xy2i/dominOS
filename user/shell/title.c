#include <stdio.h>
#include "primitive.h"

void display_title()
{
    printf("\f");
    change_color(BLUE_FG);
    printf(
        "$$$$$$$\\                          $$\\            $$$$$$\\   $$$$$$\\  \n"
        "$$  __$$\\                         \\__|          $$  __$$\\ $$  __$$\\ \n"
        "$$ |  $$ | $$$$$$\\  $$$$$$\\$$$$\\  $$\\ $$$$$$$\\  $$ /  $$ |$$ /  \\__|\n"
        "$$ |  $$ |$$  __$$\\ $$  _$$  _$$\\ $$ |$$  __$$\\ $$ |  $$ |\\$$$$$$\\  \n"
        "$$ |  $$ |$$ /  $$ |$$ / $$ / $$ |$$ |$$ |  $$ |$$ |  $$ | \\____$$\\ \n"
        "$$ |  $$ |$$ |  $$ |$$ | $$ | $$ |$$ |$$ |  $$ |$$ |  $$ |$$\\   $$ |\n"
        "$$$$$$$  |\\$$$$$$  |$$ | $$ | $$ |$$ |$$ |  $$ | $$$$$$  |\\$$$$$$  |\n"
        "\\_______/  \\______/ \\__| \\__| \\__|\\__|\\__|  \\__| \\______/  \\______/ \n"
        "                                                                    \n");
    change_color(DEFAULT);
    change_color(RED_FG);
    printf("                               _______\n"
           "                              /______/|\n"
           "                             |     o ||\n"
           "                             |   o   ||\n"
           "                             | o     ||\n"
           "                             |-------||\n"
           );
    change_color(BLUE_FG);
    printf("                             | o   o ||\n"
           "                             |   o   ||\n"
           "                             | o   o ||\n"
           "                             |_______|/\n"
           );
    change_color(DEFAULT);
    printf("\n\nWelcome to our OS ! Tap help to see the command you can make\n");
}
