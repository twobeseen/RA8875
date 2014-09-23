/******************************************************************
An example of how use an external ROM chip with RA8875, this time
the ER3303_1 (same as GT23L24T3Y or GT30H24T3Y), an unified chinese
font rom. 
In short:

1) use setExternalFontRom to set ROM and it's correct encoding
2) use setFont(EXT) to use the external font
3) to switch back to internal font ROM, use setFont(INT)

Library can handle the following Font ROM:
GT21L16T1W, GT21H16T1W, GT23L16U2W, GT30H24T3Y, GT23L24T3Y, GT23L24M1Z, 
GT23L32S4W, GT30H32S4W, ER3303_1
Each font ROM has it's encoding so follow font rom datasheet!


 ******************************************************************/
//#define USEEXTSDCARD
 
#include <SPI.h>
#include <RA8875.h>
#if defined(USEEXTSDCARD)
#include <SD.h>
#endif

#define RA8875_INT   2
#define RA8875_CS    10
#define RA8875_RESET 9
#define SDCSPIN      6

RA8875 tft = RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;

void setup() 
{
  //Serial.begin(9600);
  //while (!Serial) {;}
  //Serial.println("RA8875 start");

 
  if (!tft.begin(RA8875_480x272)) {
    //Serial.println("RA8875 Not Found!");
    while (1);
  }
#if defined(USEEXTSDCARD)
  //begin SD
  if (!SD.begin(SDCSPIN)) {
    //Serial.println("SD failed!");
    while(1);
  }
#endif
  //Serial.println("SD ready");
  tft.changeMode(TEXT);
  tft.setTextColor(RA8875_WHITE);
  //now set the external rom font ER3303_1 an unified chinese font chip,
  //it contains also some ASCII char and use GB12345 encoding. 
  //It's important to read your font chip datasheet or you will 
  //get from nothing ti garbage on screen!
  tft.setExternalFontRom(ER3303_1,GB12345);
  //switch to external rom
  tft.setFont(EXT);
  //now write some chinese....
  //note that when using ext font chip the size will be X24
  //since most of font maps are 24x24.
  tft.print("ÉîÛÚÐñÈÕ¶«·½¿Æ¼¼ÓÐÏÞ¹«Ë¾"); //
  //the X16 sixe of ROM ER3303_1 can also use ASCII...
  tft.setFontSize(X16);//switch to X16
  tft.setTextColor(RA8875_RED);
  tft.setCursor(0,50);
  tft.println("ABCD 1234567890"); //I will use println this time!
  //switch to internal rom 
  tft.setFont(INT);
  tft.setTextColor(RA8875_GREEN);
  tft.println("ABCD 1234567890"); 
  //not bad neh? you can use different encodings without use
  //any MCU memory, just add an hardware font chip and istruct library.
  tft.setFont(EXT);
  //you can switching back on the fly to EXT
  tft.print("ÉîÛÚÐñÈÕ¶«·½¿Æ¼¼ÓÐÏÞ¹«Ë¾");
  //voilà
}

void loop() 
{
}