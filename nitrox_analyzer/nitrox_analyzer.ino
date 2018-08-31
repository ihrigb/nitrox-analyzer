#define _
#ifdef DEBUG
#define DEBUG(x) Serial.print(x)
#define DEBUGLN(x) Serial.println(x)
#else
#define DEBUG(x)
#define DEBUGLN(x)
#endif

#include <Wire.h>
#include <RunningAverage.h>
#include <Adafruit_ADS1015.h>
#include "U8glib.h"

// see http://code.google.com/p/u8glib/wiki/device
U8GLIB_SSD1306_128X64 u8g(12, 7, 8, 9, 10);  // SW SPI Com: SCK = 4, MOSI = 5, CS = 6, A0 = 7 (new blue HalTec OLED)

char tmpstr[4];
char percentString[6];

//******** Create Objects
Adafruit_ADS1115 ads(0x48); /* Use this for the 16-bit version */

char disclaimer1[] = "NX Analyzer\0";
char disclaimer2[] = "    No Warranty!\0";
byte disclaimerid = 1;
char *warrantyptr = disclaimer1;
#define disclaimerdelay 2000
long nextdisclaimerchange = millis() + disclaimerdelay;
float o2percent = 0;
int16_t ADSresults;
float multiplier;
long millivolts = 0;
float slope = 0;
float pc = 0;
#define RA_SIZE 80
#define xdelay 20
RunningAverage RA(RA_SIZE);
int lastRAvalue = 0;

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  if (millis() > nextdisclaimerchange) {
    nextdisclaimerchange = millis() + disclaimerdelay;
    if (disclaimerid == 1) {
      disclaimerid = 2;
      warrantyptr = disclaimer2;
    } else {
      disclaimerid = 1;
      warrantyptr = disclaimer1;
    }
  }

  u8g.drawStr(0, 12, warrantyptr);
  u8g.setFont(u8g_font_helvR24);
  dtostrf(o2percent, 2, 1, percentString);

  DEBUG("D: O=");
  DEBUG(o2percent);
  DEBUG(" D=");
  DEBUG(RA.getDelta());
  DEBUG(" Av=");
  DEBUGLN(RA.getAverage());

  if (slope != 0 and o2percent > 0 and o2percent <= 100) {
    u8g.drawStr(0, 40, percentString);
  } else {
    u8g.drawStr(0, 40, "- - -");
  }
  u8g.drawStr(70, 40, "%");
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(103, 40, "O2");

  u8g.setFont(u8g_font_unifont);
  dtostrf(RA.getAverage() * multiplier, 3, 3, tmpstr);
  u8g.drawStr(0, 63, tmpstr);
  u8g.drawStr(40, 63, " mV s");
  dtostrf(slope, 2, 2, tmpstr);
  u8g.drawStr(85, 63, tmpstr);
}

void getADS(void) {
  millivolts = abs(ads.readADC_Differential_0_1());
  RA.addValue(millivolts);
}

void setup(void) {
  Serial.begin(9600);
  DEBUGLN("Start");
  pinMode(5, INPUT);

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV    n  0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV    n  0.0625mV
  multiplier = 0.0625F;

  // flip screen if required
  // u8g.setRot180();

  // set SPI backup if required
  // u8g.setHardwareBackup(u8g_backup_avr_spi);

  // assign default color value
  if (u8g.getMode() == U8G_MODE_R3G3B2) {
    u8g.setColorIndex(255); // white
  } else if (u8g.getMode() == U8G_MODE_GRAY2BIT) {
    u8g.setColorIndex(3); // max intensity
  } else if (u8g.getMode() == U8G_MODE_BW) {
    u8g.setColorIndex(1); // pixel on
  } else if (u8g.getMode() == U8G_MODE_HICOLOR) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  ads.begin();
  for (int cx = 0; cx <= RA_SIZE; cx++) {
    getADS();
  }

  delay(1000);
}

void calib() {
  slope = (float) RA.getAverage() / 20.95;
  delay(1200);
}

void loop(void) {
  // picture loop
  getADS();
  DEBUG("LOOP : READ=");
  DEBUG(lastRAvalue);
  DEBUG(" AV=");
  DEBUG(RA.getAverage());
  DEBUG(" PC=");
  DEBUGLN(RA.getDeltaPc());

  if (slope == 0 and lastRAvalue == (int) RA.getAverage()) {
    calib();
  }

  lastRAvalue = RA.getAverage();
  if (digitalRead(5)) {
    calib();
  }

  if (slope != 0) {
    o2percent = (float) millivolts / slope;
    delay(xdelay);
  }

  u8g.firstPage();
  do {
    draw();
  } while (u8g.nextPage());
}
