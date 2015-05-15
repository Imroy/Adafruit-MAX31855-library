/*************************************************** 
  This is a library for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_MAX31855.h"
#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif
#include <stdlib.h>
#include <SPI.h>


Adafruit_MAX31855::Adafruit_MAX31855(int8_t SCLK, int8_t CS, int8_t MISO) {
  sclk = SCLK;
  cs = CS;
  miso = MISO;
  hSPI = 0;

  //define pin modes
  pinMode(cs, OUTPUT);
  pinMode(sclk, OUTPUT); 
  pinMode(miso, INPUT);

  digitalWrite(cs, HIGH);
}

Adafruit_MAX31855::Adafruit_MAX31855(int8_t CS) {
  cs = CS;
  hSPI = 1;

  //define pin modes
  pinMode(cs, OUTPUT);
  
  //start and configure hardware SPI
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  
  digitalWrite(cs, HIGH);
}

double Adafruit_MAX31855::internal(void) {
  uint16_t internal = (data >> 4) & 0x0fff;
  if (internal & 0x800)	// sign extension
    internal |= 0xf000;

  return internal * 0.0625;
}

double Adafruit_MAX31855::celsius(void) {
  if (data & 0x7)
    return NAN;		// uh oh, a serious problem!

  uint16_t tc = data >> 18;
  if (tc & 0x2000)	// sign extension
    tc |= 0xc000;

  return (int16_t)tc * 0.25;
}

double Adafruit_MAX31855::farenheit(void) {
  float f = celsius();
  f *= 9.0;
  f /= 5.0;
  f += 32;
  return f;
}

void Adafruit_MAX31855::readData(void) {
  if (hSPI) {
    hspiread32();
    return;
  }

  digitalWrite(sclk, LOW);
  delayMicroseconds(1);
  digitalWrite(cs, LOW);
  delayMicroseconds(1);

  data = 0;
  for (int i = 32; i; i--) {
    digitalWrite(sclk, LOW);
    delayMicroseconds(1);
    data <<= 1;
    if (digitalRead(miso))
      data |= 1;

    digitalWrite(sclk, HIGH);
    delayMicroseconds(1);
  }

  digitalWrite(cs, HIGH);
}

void Adafruit_MAX31855::hspiread32(void) {
  digitalWrite(cs, LOW);
  delayMicroseconds(1);
  
  data = 0;
  for (uint8_t i = 4; i; i--) {
    data <<= 8;
    data |= SPI.transfer(0x00);
  }
  
  digitalWrite(cs, HIGH);
}
