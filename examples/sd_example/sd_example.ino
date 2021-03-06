/*
	Grab bmp image from an sd card.
 	quick & slow dirty method!
	Look inside the folder RA8875/examples/sd_example
	there's a folder, copy the content in a formatted FAT32 SD card
WARNING:
This example needs a working SD library for Energia IDE and actually it uses
one for Teensy3.x and arduino so at the moment you cannot use this example.
 */


#include <SPI.h>
#include <RA8875.h>

#if !defined(ENERGIA)
#include <SD.h>
#else
#error this example need a working SD library not compatible with Energia
#endif

/*
Teensy3.x and Arduino's
You are using 4 wire SPI here, so:
 MOSI:  11//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 MISO:  12//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 SCK:   13//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 the rest of pin below:
 */
#define RA8875_INT 2 //any pin
#define RA8875_CS 10 //see below...
/*
Teensy 3.x can use: 2,6,9,10,15,20,21,22,23
Arduino's 8 bit: any
DUE: should be any but not sure
*/
#define RA8875_RESET 9//any pin or nothing!

#if defined(NEEDS_SET_MODULE)//Energia, this case is for stellaris/tiva

RA8875 tft = RA8875(3);//select SPI module 3
/*
for module 3 (stellaris)
SCLK:  PD_0
MOSI:  PD_3
MISO:  PD_2
SS:    PD_1
*/
#else

RA8875 tft = RA8875(RA8875_CS,RA8875_RESET);//Teensy3/arduino's

#endif


#define SDCSPIN      6//for SD


void setup() 
{
  Serial.begin(9600);
  //while (!Serial) {;}
  Serial.println("RA8875 start");

  tft.begin(RA8875_480x272);
#if !defined(ENERGIA)
  if (!SD.begin(SDCSPIN)) {
    Serial.println("SD failed!");
    return;
  }
  Serial.println("OK!");

  bmpDraw("alert.bmp", 0, 0);//copy the enclosed image in a SD card (check the folder!!!)
#endif
}

void loop() 
{

}


#define BUFFPIXEL 30

#if !defined(ENERGIA)
void bmpDraw(const char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int16_t  bmpWidth = 0, bmpHeight = 0;   // W+H in pixels
  uint8_t  bmpDepth = 0;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset = 0;        // Start of image data in file
  uint32_t rowSize = 0;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int16_t  w=0, h=0, row=0, col=0;
  uint32_t pos = 0;
  uint16_t lcdidx = 0;

  if((x >= tft.width()) || (y >= tft.height())) return;
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == 0) return;
  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // check BMP signature
    read32(bmpFile);
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // determine color depth
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;
        // If bmpHeight is negative, image is in top-down order.This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }
        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        for (row=0; row<h; row++) { // For each scanline...
          tft.setY(row+y);
          if (flip) { // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          } 
          else {     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          }
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }
          for (col=0; col<w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if (lcdidx > 0) {
                writePixb(col+x,lcdbuffer[lcdidx]);
                lcdidx = 0;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }
            lcdbuffer[lcdidx] = tft.Color565(sdbuffer[buffidx++],sdbuffer[buffidx++],sdbuffer[buffidx++]);
            writePixb(col+x,lcdbuffer[lcdidx]);
          } // end pixel

        } // end scanline
        // Write any remaining data to LCD
        if(lcdidx > 0) writePixb(col+x,lcdbuffer[lcdidx]);
      } // end goodBmp
    }//correct plane
  }//end BMP signature
  bmpFile.close();
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

void writePixb(int16_t x,uint16_t color){
  tft.setX(x);
  tft.writeCommand(RA8875_MRWC);
  tft.writeData16(color); 
}

uint16_t read16(File f) {
  uint16_t result = 0;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result = 0;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
#endif

