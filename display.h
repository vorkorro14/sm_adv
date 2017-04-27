#define F_CPU 8000000L

#include <avr/io.h> //библиотека ввода/вывода
#include <util/delay.h>
#include <string.h>


 
#define RS 2        //RS=PD2 - сигнал управления ЖКИ
#define E  3        //E=PD3  - сигнал управления ЖКИ
#define RUN 0b00011000
#define CLEAN 0b00000001
#define OSHIBKA_STOP 0xDED

#define TIME 10.0    //Константа временной задержки для ЖКИ
		    //Частота тактирование МК - 4Мгц
 
//Программа формирвоания задержки
void pause (unsigned int a)				
{ 
    unsigned int i;
    for (i=a*16000;i>0;i--) {
    asm("nop");
    };
}
void port_init()
{
    DDRD = 0xfc;	//Инициализация portD
    PORTD = 0x00;
}
//Программа передачи команд в ЖКИ
void lcd_com (unsigned char lcd)
{   
    unsigned char temp;
    temp=(lcd&~(1<<RS))|(1<<E);		//RS=0 – это команда
    PORTD=temp;	                        //Выводим на portD старшую тетраду команды, сигналы RS, E 
    asm("nop");	                        //Небольшая задержка в 1 такт МК, для стабилизации
    PORTD=temp&~(1<<E);	                //Сигнал записи команды
    temp=((lcd*16)&~(1<<RS))|(1<<E);	//RS=0 – это команда 
    PORTD=temp;	                        //Выводим на portD младшую тетраду команды, сигналы RS, E 
    asm("nop");	                        //Небольшая задержка в 1 такт МК, для стабилизации 
    PORTD=temp&~(1<<E);	                //Сигнал записи команды 
    pause (10*TIME);	                //Пауза для выполнения команды
}
 void lcd_com_inf (unsigned char lcd)
 {
     while(1)
     {
         lcd_com(lcd);
     }
 }

void lcd_dat (unsigned char lcd)
{ 
    unsigned char temp;
 
    temp=(lcd|(1<<RS))|(1<<E);		//RS=1 – это данные 
    PORTD=temp;	                        //Выводим на portD старшую тетраду данных, сигналы RS, E   
    asm("nop");		                //Небольшая задержка в 1 такт МК, для стабилизации 
    PORTD=temp&~(1<<E);	                //Сигнал записи данных 
 
    temp=((lcd*16)|(1<<RS))|(1<<E);	//RS=1 – это данные 
    PORTD=temp;	                        //Выводим на portD младшую тетраду данных, сигналы RS, E   
    asm("nop");	                        //Небольшая задержка в 1 такт МК, для стабилизации 
    PORTD=temp&~(1<<E);	                //Сигнал записи данных 
    pause(0.1*TIME);	                        //Пауза для вывода данных
}
 
//Программа иниализации ЖКИ
void lcd_init (void)
{
    port_init();
    pause(1000);	
    lcd_com(0x2c);		//4-проводный интерфейс, 5x8 размер символа
    pause(100*TIME);
    lcd_com(0x0c);		//Показать изображение, курсор не показывать
    pause(100*TIME);
    lcd_com(0x01);		//Очистить DDRAM и установить курсор на 0x00
    pause (100*TIME);
}

void lcd_data16(char* string)
{
    int i = 0;
    for (i ; i < 16; i++)
    {
        lcd_dat(string[i]);
    }  
}
void lcd_data16r(char* string)
{
    int i = 0;
    for (i ; i < 16; i++)
    {
        lcd_dat(string[i]);
        lcd_com(RUN);

    }  
}
 void lcd_print(char* string) 
{
    
    int counter = 0;
    size_t N = strlen(string); //количество символов
    lcd_data16(string);
    lcd_data16r(string + 16);
    lcd_com_inf(RUN);
}

void next_string()
{
    int i = 0;
    for (i; i<40  ;i++)
    {
        lcd_com(0b00010000);
    }
}
void lcd_clean()
{
    lcd_com(CLEAN);
}
