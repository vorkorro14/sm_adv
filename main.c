#include "cam.h"
#include <SPI.h>

#include <SoftwareSerial.h>

#define chipSelect 10

SoftwareSerial cameraconnection = SoftwareSerial(2, 3);

config(&cameraconnection);


byte main(void) {

	/*

	// When using hardware SPI, the SS pin MUST be set to an
	// output (even if not connected or used).  If left as a
	// floating input w/SPI on, this can cause lockuppage.
	#if !defined(SOFTWARE_SPI)
	if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
	#endif

	Serial.begin(9600);
	Serial.println("VC0706 Camera snapshot test");
	
	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}
	*/
	
	// Try to locate the camera
	if (begin()) 
	{
		Serial.println("Camera Found:");
	} 
	else 
	{
		Serial.println("No camera found");
		return 1;
	}
	// Print out the camera version information (optional)
	char *reply = getVersion();
	if (reply == 0) 
	{
		Serial.print("Failed to get version");
	} 
	else 
	{
		Serial.println("-----------------");
		Serial.print(reply);
		Serial.println("-----------------");
	}

	setImageSize(VC0706_160x120);

	while (True)
	{
		if (button_clicked())
		{
			Serial.println("Snap in 3 secs...");
			delay(3000);

			if (! takePicture())
			Serial.println("Failed to snap!");
			else
			Serial.println("Picture taken!");
		

			// Get the size of the image (frame) taken
			uint16_t jpglen = frameLength();
			Serial.print("Storing ");
			Serial.print(jpglen, DEC);
			Serial.print(" byte image.");

			pinMode(8, OUTPUT);
			// Read all the data up to # bytes!
			byte wCount = 0; // For counting # of writes
			while (jpglen > 0)
			{
				// read 32 bytes at a time;
				uint8_t *buffer;
				uint8_t bytesToRead = min(32, jpglen);
				buffer = readPicture(bytesToRead);
				//writing to EEPROM
				imgFile.write(buffer, bytesToRead);
				if(++wCount >= 64)
				{
					// Every 2K, give a little feedback so it doesn't appear locked up
					Serial.print('.');
					wCount = 0;
				}
				jpglen -= bytesToRead;
			}

			Serial.println("done!");
		}
	}

	return 0;
}
