#define F_CPU 8000000L

#include <avr/io.h> //библиотека ввода/вывода
#include <util/delay.h>

 
#define RS 2        //RS=PD2 - сигнал управления ЖКИ
#define E  3        //E=PD3  - сигнал управления ЖКИ
 
#define TIME 10	    //Константа временной задержки для ЖКИ
		    //Частота тактирование МК - 4Мгц
 
//Программа формирвоания задержки
void pause (unsigned int a)				
{ unsigned int i;
 
  for (i=a*16000;i>0;i--) {
      asm("nop");
  };
}
 
//Программа передачи команд в ЖКИ
void lcd_com (unsigned char lcd)
{ unsigned char temp;
 
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
 
//Программа записи данных в ЖКИ
void lcd_dat (unsigned char lcd)
{ unsigned char temp;
 
  temp=(lcd|(1<<RS))|(1<<E);		//RS=1 – это данные 
  PORTD=temp;	                        //Выводим на portD старшую тетраду данных, сигналы RS, E   
   asm("nop");		                //Небольшая задержка в 1 такт МК, для стабилизации 
  PORTD=temp&~(1<<E);	                //Сигнал записи данных 
 
  temp=((lcd*16)|(1<<RS))|(1<<E);	//RS=1 – это данные 
  PORTD=temp;	                        //Выводим на portD младшую тетраду данных, сигналы RS, E   
   asm("nop");	                        //Небольшая задержка в 1 такт МК, для стабилизации 
  PORTD=temp&~(1<<E);	                //Сигнал записи данных 
 
  pause(TIME);	                        //Пауза для вывода данных
}
 
//Программа иниализации ЖКИ
void lcd_init (void)
{
 lcd_com(0x2c);		//4-проводный интерфейс, 5x8 размер символа
 pause(100*TIME);
 lcd_com(0x0c);		//Показать изображение, курсор не показывать
 pause(100*TIME);
 lcd_com(0x01);		//Очистить DDRAM и установить курсор на 0x00
 pause (100*TIME);
}
 
//Основная программа
int main(void)
{    
DDRD=0xfc;	//Инициализация portD
PORTD=0x00;
 
pause(1000);	//Задержка, чтобы ЖКИ успел включиться
lcd_init();	//Инициализация ЖКИ
 
while(1) { 
lcd_dat('w');   //Вывод "www.avrlab.com"
pause(1000);
lcd_dat('a');
}
 
return 1;
}
