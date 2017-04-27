#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* conc(const char* str1, const char* str2)
{
    char* str = malloc(strlen(str1) + strlen(str2) + 1);
    
    int i = 0;
    for ( ; i < strlen(str1); i++)
    {
        str[i] = str1[i];
    }
    for ( ; i - strlen(str1) < strlen(str2); i++)
    {
        str[i] = str2[i];
    }
    str[i] = 0;
    
    return str;
}

int main(int argc, const char * argv[])
{
    if (argc <= 1)
    {
        printf("Too few arguments\n");
        return 0;
    }
    
    if (argc == 2)
    {
        system(conc(conc("avr-gcc -g -Os -mmcu=atmega8535 -c ", argv[1]),".c"));
        system(conc(conc(conc(conc("avr-gcc -g -mmcu=atmega8535 -o ", argv[1]), ".elf "), argv[1]), ".o"));
        system(conc(conc(conc(conc("avr-objcopy -j .text -j .data -O ihex ", argv[1]), ".elf "), argv[1]), ".hex"));
        system(conc(conc("sudo avrdude -p atmega8535 -c usbasp -U lfuse:w:0xc1:m -U hfuse:w:0xD9:m -U flash:w:", argv[1]), ".hex"));
        
    }
    
    if (argc >= 3)
    {
        printf("Too many arguments\n");
        return 0;
    }
    
    return 0;
}
