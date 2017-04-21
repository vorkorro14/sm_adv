#include "cam.h"

void config(SoftwareSerial *ser) 
{
	swSerial = ser;
	frameptr  = 0;
	bufferLen = 0;
	serialNum = 0;
}
#endif

boolean camBegin(uint16_t baud) 
{
    swSerial->begin(baud);
  return reset();
}

boolean reset() {
  uint8_t args[] = {0x0};

  return runCommand(VC0706_RESET, args, 1, 5);
}

boolean setImageSize(uint8_t x) {
  uint8_t args[] = {0x05, 0x04, 0x01, 0x00, 0x19, x};

  return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
}


char * getVersion(void) {
  uint8_t args[] = {0x01};
  
  sendCommand(VC0706_GET_VERSION, args, 1);
  // get reply
  if (!readResponse(CAMERABUFFSIZ, 200)) 
    return 0;
  camerabuff[bufferLen] = 0;  // end it!
  return (char *)camerabuff;  // return it!
}


boolean takePicture() {
  frameptr = 0;
  return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME); 
}


uint32_t frameLength(void) {
  uint8_t args[] = {0x01, 0x00};
  if (!runCommand(VC0706_GET_FBUF_LEN, args, sizeof(args), 9))
    return 0;

  uint32_t len;
  len = camerabuff[5];
  len <<= 8;
  len |= camerabuff[6];
  len <<= 8;
  len |= camerabuff[7];
  len <<= 8;
  len |= camerabuff[8];

  return len;
}


uint8_t available(void) {
	return bufferLen;
}


uint8_t * readPicture(uint8_t n) {
  uint8_t args[] = {0x0C, 0x0, 0x0A, 
                    0, 0, frameptr >> 8, frameptr & 0xFF, 
                    0, 0, 0, n, 
                    CAMERADELAY >> 8, CAMERADELAY & 0xFF};

  if (! runCommand(VC0706_READ_FBUF, args, sizeof(args), 5, false))
    return 0;


  // read into the buffer PACKETLEN!
  if (readResponse(n+5, CAMERADELAY) == 0) 
      return 0;


  frameptr += n;

  return camerabuff;
}


boolean runCommand(uint8_t cmd, uint8_t *args, uint8_t argn, 
			   uint8_t resplen, boolean flushflag) {
  // flush out anything in the buffer?
  if (flushflag) {
    readResponse(100, 10); 
  }

  sendCommand(cmd, args, argn);
  if (readResponse(resplen, 200) != resplen) 
    return false;
  if (! verifyResponse(cmd))
    return false;
  return true;
}

void sendCommand(uint8_t cmd, uint8_t args[] = 0, uint8_t argn = 0) 
{
    if(swSerial) 
	{
    swSerial->write((byte)0x56);
    swSerial->write((byte)serialNum);
    swSerial->write((byte)cmd);

    for (uint8_t i=0; i<argn; i++) 
	{
      swSerial->write((byte)args[i]);
    }
	}
}

uint8_t readResponse(uint8_t numbytes, uint8_t timeout) {
  uint8_t counter = 0;
  bufferLen = 0;
  boolean avail;
 
  while ((timeout != counter) && (bufferLen != numbytes)){
    avail = swSerial->available();
    if (avail <= 0) {
      delay(1);
      counter++;
      continue;
    }
    counter = 0;
    // there's a byte!
    camerabuff[bufferLen++] = swSerial->read();
  }

  return bufferLen;
}

boolean verifyResponse(uint8_t command) {
  if ((camerabuff[0] != 0x76) ||
      (camerabuff[1] != serialNum) ||
      (camerabuff[2] != command) ||
      (camerabuff[3] != 0x0))
      return false;
  return true;
  
}