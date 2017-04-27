#include <stdlib.h>

int main(int argc, const char * argv[]) {
    
    system("avr-gcc -g -Os -mmcu=atmega32 -c not.c");
    system("avr-gcc -g -mmcu=atmega32 -o not.elf not.o");
    system("avr-objcopy -j .text -j .data -O ihex not.elf not.hex");
    system("sudo avrdude -p atmega32 -c usbasp -U lfuse:w:0xc1:m -U hfuse:w:0xD9:m -U flash:w:not.hex");
    
    return 0;
}
