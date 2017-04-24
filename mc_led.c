/*****************************************************
 Chip type           : ATmega8535
 Program type        : Application
 Clock frequency     : 11,059200 MHz
 Memory model        : Small
 External SRAM size  : 0
 Data Stack size     : 128
 *****************************************************/

#include <mega8535.h>
#include <lcd_wg.h>
#ifndef __SLEEP_DEFINED__
#define __SLEEP_DEFINED__
.EQU __se_bit=0x40
.EQU __sm_mask=0xB0
.EQU __sm_powerdown=0x20
.EQU __sm_powersave=0x30
.EQU __sm_standby=0xA0
.EQU __sm_ext_standby=0xB0
.EQU __sm_adc_noise_red=0x10
.SET power_ctrl_reg=mcucr
#endif

#include <delay.h>

char* string; //входная строка
size_t N; //количество символов

#pragma used+

void display_reset(void) {
    rst_on;
    delay_us(10);
    rst_off;
}

unsigned char read_flags_busy(unsigned char crystal) {
    unsigned char status;
    unsigned char temp;
    
    if(!crystal) {
        cs1_on;
        cs2_off;
    } else {
        cs1_off;
        cs2_on;
    }
    
    __direction_in;
    cd_command;
    rw_read;
    delay_us(2);
    e_on;
    nop;
    nop;
    e_off;
    temp=pin_data;
    
    if(temp==flags_busy) {
        status=1;
    } else {
        status=0;
    }
    
    __direction_out;
    return (status);
}

void write_data_display(unsigned char mode, unsigned char data, unsigned char cs) {
    switch (cs) {
        case cs1:
            cs1_on;
            cs2_off;
            break;
        case cs2:
            cs1_off;
            cs2_on;
            break;
        case cstwo:
            cs1_on;
            cs2_on;
            break;
    }
    
    if (mode) {
        cd_data;
    } else {
        cd_command;
    }
    
    rw_write;
    nop;
    nop;
    port_data=data;
    e_on;
    nop;
    nop;
    e_off;
    cs1_off;
    cs2_off;
    while (read_flags_busy(cs1) !=0);
    while (read_flags_busy(cs2) !=0);
}

void display_xy(unsigned char x,unsigned char y,unsigned char cs) {
    write_data_display(command,0xb8+y,cs);
    write_data_display(command,0x40+x,cs);
}

unsigned char goto_xy(unsigned char x,unsigned char y) {
    unsigned char cs,textcs;
    
    if (x<10) {
        cs=cs1;
        textcs=0;
    } else {
        cs=cs2;
        textcs=64;
    }
    display_xy(x*6-textcs+4,y,cs);
    
    return (cs);
}

void display_putchar(unsigned char character,unsigned char x,unsigned char y,unsigned char inv) {
    unsigned char k=0;
    unsigned char strLen;
    unsigned char cs;
    
    cs=goto_xy(x,y);
    if (character < 0x7F) {
        strLen=0X20;
    }
    if (character > 0xBF) {
        strLen=0X60;
    }
    
    do {
        write_data_display(data,charsets[character-strLen][k]^inv,cs);
        if (++k==array) {
            break;
        }
    } while (1);
    if (textx < 19) {
        textx++;
    } else {
        textx=0;
        if (texty < 8) {
            texty++;
        }
    }
}

void display_charcode(unsigned char data,unsigned char x,unsigned char y,unsigned char cs) {
    unsigned char k=0;
    
    display_xy(x,y,cs);
    do {
        write_data_display(data,charcode[data][k],cs);
        if (++k==8) {
            break;
        }
    } while (1);
}

void display_putsf(flash char str[],unsigned char x,unsigned char y,unsigned char inv) {
    char k;
    unsigned char j=0;
    
    textx=x;
    texty=y;
    do {
        if ((k=str[j])==0) {
            break;
        }
        display_putchar(k,textx,texty,inv);
    } while (++j<20);
}

void display_puts(char *str,unsigned char x,unsigned char y,unsigned char inv) {
    char k;
    
    textx=x;
    texty=y;
    do {
        k=*str++;
        if( k==0 ){break;}
        display_putchar( k,textx,texty,inv );
    } while (1);
}

void display_init( void ) {
    __port_init;
    display_reset();
    delay_ms(1);
    write_data_display(command,0xc0,cstwo);
    write_data_display(command,0x3f,cstwo);
}


void display_clear(unsigned char cs) {
    unsigned char x,y;
    
    for (x=0;x<64;x++) {
        for (y=0;y<8;y++) {
            display_xy(x,y,cs);
            write_data_display(data,0,cs);
        }
    }
}

void display_reset(void) {
    rst_on;
    delay_us(2000);
    rst_off;
}

unsigned char read_flags_busy(unsigned char crystal) {
    unsigned char status;
    unsigned char temp;
    
    if (!crystal) {
        cs1_on;
        cs2_off;
    } else {
        cs1_off;
        cs2_on;
    }
    
    __direction_in;
    cd_command;
    rw_read;
    delay_us(4);
    e_on;
    nop;
    nop;
    e_off;
    temp=pin_data;
    
    if (temp==flags_busy) {
        status=1;
    } else {
        status=0;
    }
    
    __direction_out;
    return (status);
}

void write_data_display(unsigned char mode, unsigned char data, unsigned char cs) {
    switch(cs) {
        case cs1:
            cs1_on;
            cs2_off;
            break;
        case cs2:
            cs1_off;
            cs2_on;
            break;
        case cstwo:
            cs1_on;
            cs2_on;
            break;
    }
    
    if (mode) {
        cd_data;
    } else {
        cd_command;
    }
    
    rw_write;
    nop;
    nop;
    port_data=data;
    e_on;
    nop;
    nop;
    e_off;
    cs1_off;
    cs2_off;
    while (read_flags_busy(cs1) !=0 );
    while (read_flags_busy(cs2) !=0 );
    e_on;
}

void display_xy(unsigned char x,unsigned char y,unsigned char cs) {
    write_data_display(0,0xb8+y,cs);
    write_data_display(0,0x40+x,cs);
}

unsigned char gotoxy(unsigned char x,unsigned char y) {
    unsigned char cs,textcs;
    
    if (x<10) {
        cs=cs1;
        textcs=0;
    } else {
        cs=cs2;
        textcs=64;
    }
    display_xy(x*6-textcs+4,y,cs);
    
    return cs;
}

void clear_xy(unsigned char x, unsigned char y, unsigned cs) {
    write_data_display(0,0xb8 + y,cs);
    write_data_display(0,0x40 + x,cs);
}

void display_putchar(unsigned char character,unsigned char x,unsigned char y,unsigned char inv) {
    unsigned char k=0;
    unsigned char strLen;
    unsigned char cs;
    
    textx=x;
    texty=y;
    cs=gotoxy(textx,texty);
    if (character < 0x90) {
        strLen=0x20;
    } else {
        strLen=0x60;
    }
    do {
        write_data_display(1,charsets[character-strLen][k]^inv,cs);
        if (++k==array) {
            break;
        }
    } while (1);
    if (textx < 19) {
        textx++;
    } else {
        textx=0;
        if (texty < 8) {
            texty++;
        }
    }
}

void display_charcode(unsigned char data,unsigned char x,unsigned char y,unsigned char cs) {
    unsigned char k=0;
    
    display_xy(x,y,cs);
    do {
        write_data_display(1,charcode[data][k],cs);
        if(++k==7) {
            break;
        }
    } while (1);
}

void display_putsf(flash char str[],unsigned char x,unsigned char y,unsigned char inv) {
    char k;
    unsigned char j=0;
    
    textx=x;
    texty=y;
    do {
        if((k=str[j]) !=0 ) {
        } else {
            break;
        }
        display_putchar(k,textx,texty,inv);
    } while (++j<50);
}

void display_puts(char *str,unsigned char x,unsigned char y,unsigned char inv) {
    char k;
    
    textx=x;
    texty=y;
    do {
        k=*str++;
        if (k==0) {
            break;
        }
        display_putchar(k,textx,texty,inv);
    } while (1);
}

void display_init(void) {
    __port_init;
    display_reset();
    delay_ms(1);
    write_data_display(0,0xc0,cstwo);
    write_data_display(0,0x3f,cstwo);
}

void display_clear(unsigned char cs) {
    unsigned char x,y;
    
    for (x=0;x<16;x++) {
        for (y=0;y<2;y++) {
            clear_xy(x,y,cs);
            write_data_display(1,0,cs);
        }
    }
}

#pragma used-




void main(void) {
    int j=1;
    for (int i=0; i<N; ++i) {
        for (int k=0; k<j; ++k) {
            int x=32-i+k;
            if (x>=0) {
                display_putchar(string[k], x%16, x/16, 0);
            }
        }
        delay_ms(500);
    }
    
    while(1);
}
