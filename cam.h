#include <SoftwareSerial.h>
#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>
#define delay	delay_ms

#define boolean unsigned char
#define byte    unsigned char
#define True	1
#define False	0

#define VC0706_RESET			0x26
#define VC0706_GET_VERSION		0x11
#define VC0706_SET_PORT			0x24
#define VC0706_READ_FBUF		0x32
#define VC0706_GET_FBUF_LEN		0x34
#define VC0706_FBUF_CTRL		0x36
#define VC0706_WRITE_DATA		0x31

#define VC0706_STOPCURRENTFRAME	0x0
#define VC0706_STOPNEXTFRAME	0x1

#define VC0706_160x120			0x22

#define CAMERABUFFSIZ			100
#define CAMERADELAY				10

Softwareserial *swSerial;
uint8_t  serialNum;
uint8_t  camerabuff[CAMERABUFFSIZ+1];
uint8_t  bufferLen;
uint16_t frameptr;

void common_init(void);
boolean runCommand(uint8_t cmd, uint8_t args[], uint8_t argn, uint8_t resp, boolean flushflag = True);
void sendCommand(uint8_t cmd, uint8_t args[], uint8_t argn);
uint8_t readResponse(uint8_t numbytes, uint8_t timeout);
boolean verifyResponse(uint8_t command);
void printBuff(void);

boolean config(SoftwareSerial *ser);
boolean begin(uint16_t baud = 38400);
boolean reset(void);
boolean takePicture(void);
uint8_t *readPicture(uint8_t n);
uint32_t frameLength(void);
char *getVersion(void);
boolean setImageSize(uint8_t);
boolean cameraFrameBuffCtrl(uint8_t command);