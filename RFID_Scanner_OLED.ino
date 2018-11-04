// RFID/NFC Scanner
// (C)2018 Pawel A. Hernik
// YouTube video: https://youtu.be/RnljdPqzepk
/*
CONNECTIONS
-----------
SSD1306 OLED:
SDA -> A4
SCL -> A5

RC522 RFID module:
SDA  -> D10 (SS)
SCK  -> D13
MOSI -> D11
MISO -> D12
IRQ  -> NC
GND  -> GND
RST  -> D9
3V3  -> 3V3 (5V seems to work fine, too)

RC522 library:
https://github.com/mdxs/MFRC522
*/


#include <OLEDSoftI2C_SSD1306.h>
//#include <OLEDSoftI2C_SH1106.h>
// define USEHW in above header for hw I2C version

OLEDSoftI2C_SSD1306 oled(0x3c);
//OLEDSoftI2C_SH1106 oled(0x3c);

#if USEHW==1
#include <Wire.h>
#endif

#include "small5x7bold_font.h"
#include "term7x14_font.h"
//#include "fonts_all.h"

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);

char *getPICCname(MFRC522::PICC_Type piccType)
{
  switch (piccType) {
    case MFRC522::PICC_TYPE_ISO_14443_4: return "ISO/IEC 14443-4";
    case MFRC522::PICC_TYPE_ISO_18092:   return "ISO/IEC 18092 (NFC)";
    case MFRC522::PICC_TYPE_MIFARE_MINI: return "MIFARE Mini";
    case MFRC522::PICC_TYPE_MIFARE_1K:   return "MIFARE 1KB";
    case MFRC522::PICC_TYPE_MIFARE_4K:   return "MIFARE 4KB";
    case MFRC522::PICC_TYPE_MIFARE_UL:   return "MIFARE Ultralight";
    case MFRC522::PICC_TYPE_MIFARE_PLUS: return "MIFARE Plus";
    case MFRC522::PICC_TYPE_TNP3XXX:     return "MIFARE TNP3XXX";
    case MFRC522::PICC_TYPE_NOT_COMPLETE: return "not complete";
    case MFRC522::PICC_TYPE_UNKNOWN:
    default:  return "Unknown";
  }
}

char buf[30];

void RFID_PrintInfo( MFRC522::Uid *uid ) 
{
  snprintf(buf,30,"UID: %d bytes",uid->size);
  oled.setFont(Small5x7PLBold);
  oled.printStr(ALIGN_CENTER, 3, buf);
  oled.setFont(uid->size>7 ?  Small5x7PLBold : Term7x14PL);
  oled.setCR(1);
  Serial.print(F("Card UID:"));
  int wd = 18; // for size=7
  if(uid->size>7) wd=15;
  if(uid->size<7) wd=19;
  int xs = (SCR_WD-uid->size*wd)/2;
  for(byte i = 0; i < uid->size; i++) {
    snprintf(buf,30,"%02X ",uid->uidByte[i]);
    oled.printStr(xs+i*wd, 0, buf);
    if(uid->uidByte[i] < 0x10)  Serial.print(F(" 0"));  else Serial.print(F(" "));
    Serial.print(uid->uidByte[i], HEX);
  } 
  Serial.println();
  
  oled.setCR(0);
  oled.setFont(Small5x7PLBold);
  Serial.print(F("Card SAK: "));
  if(uid->sak < 0x10) Serial.print(F("0"));
  Serial.println(uid->sak, HEX);
  snprintf(buf,30,"Card SAK: 0x%02X",uid->sak);
  oled.printStr(ALIGN_CENTER, 4, buf);

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(uid->sak);
  Serial.print(F("PICC type: "));
  Serial.println(getPICCname(piccType));
  oled.setFont(Small5x7PLBold);
  oled.printStr(ALIGN_CENTER, 2, getPICCname(piccType));

  char *txt="";
  switch (piccType) {
    case MFRC522::PICC_TYPE_MIFARE_MINI:
      txt="5 sectors = 320 bytes"; break;
    case MFRC522::PICC_TYPE_MIFARE_1K:
      txt="16 sectors = 1024 bytes"; break;
    case MFRC522::PICC_TYPE_MIFARE_4K:
      txt="40 sectors = 4096 bytes"; break;
    case MFRC522::PICC_TYPE_MIFARE_UL:
      txt="16 pages = 64 bytes"; break;
  }
  oled.printStr(ALIGN_CENTER, 5, txt);
  Serial.println(txt);
  Serial.println();
  mfrc522.PICC_HaltA();
}

void RFID_FirmwareVersion() 
{
  byte v = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print(F("Firmware Version: 0x")); Serial.print(v, HEX); Serial.print(F(" "));
  char *ver;
  switch(v) {
    case 0x88: ver="clone?";  break;
    case 0x90: ver="v0.0";  break;
    case 0x91: ver="v1.0";  break;
    case 0x92: ver="v2.0";  break;
    default:   ver="v???";
  }
  Serial.println(ver); Serial.println(); 
  oled.printStr(ALIGN_CENTER, 0, "MRC522 Firmware Version");
  snprintf(buf,30,"%s (0x%02x)",ver,v);
  oled.printStr(ALIGN_CENTER, 2, buf);
  if((v == 0x00) || (v == 0xFF)) {
    oled.printStr(ALIGN_CENTER, 4, "Communication failure!");
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}

void setup() 
{
  Serial.begin(115200);
  oled.init(0);  // 0-128x64, 1-128x32
  oled.clrScr();
  oled.setFont(Small5x7PLBold);
  SPI.begin();
  mfrc522.PCD_Init();
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  RFID_FirmwareVersion();
}

int cardNo = 1;
unsigned long t = 0;

void loop() 
{
  if( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ) {
    oled.clrScr();
    RFID_PrintInfo(&(mfrc522.uid));
    cardNo++;
  } else {
    if(millis()-t<400) {
      snprintf(buf,30,"Scan card #%d",cardNo);
      oled.printStr(ALIGN_CENTER, 7, buf);
    } else {
      oled.fillWin(0,7,SCR_WD,1,0);
    }
    if(millis()-t>800) t = millis();
  }
}

