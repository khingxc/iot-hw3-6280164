#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <DS1302.h>
#include <IRremote.hpp>

#define DHTPIN 8
#define DHTTYPE DHT11
#define DECODE_NEC          

LiquidCrystal_I2C lcd(0x27,20,4);
DHT dht(DHTPIN, DHTTYPE);
DS1302 rtc(2, 3, 4);

Time t;
const int signalPin = 11;

void setup() {
  lcd.init();                      
  lcd.backlight();
  dht.begin();
  rtc.halt(false);
  rtc.writeProtect(false);
  IrReceiver.begin(signalPin, ENABLE_LED_FEEDBACK);
  printActiveIRProtocols(&Serial);
  Serial.begin(9600); 
}

void loop() {
  float h = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(h) || isnan(temp)) {
    return;
  }

  t = rtc.getTime();
  int tHour = t.hour;
  int tMin = t.min;

  if (IrReceiver.decode()) {
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
      IrReceiver.printIRResultRawFormatted(&Serial, true);
    }

    IrReceiver.resume(); 
    
    if (IrReceiver.decodedIRData.command == 0x40) {
      switch (tHour) {
        case 23:
          tHour = 0;
          break;
        default:
          tHour++;
      }
     } else if (IrReceiver.decodedIRData.command == 0x44) {
        switch (tHour) {
          case 0:
            tHour = 23;
            break;
          default:
            tHour--;
        }
     } else if (IrReceiver.decodedIRData.command == 0x15) {
        switch (tMin) {
          case 59:
            tMin = 0;
            switch (tHour) {
              case 23:
                tHour = 0;
                break;
              default:
                tHour++;
            }
            break;
          default:
            tMin++;
        }
     } else if (IrReceiver.decodedIRData.command == 0x7) {
        switch (tMin) {
          case 0:
            tMin = 59;
            switch (tHour) {
              case 0:
                tHour = 23;
                break;
              default:
                tHour--;
            }
            break;
          default:
            tMin--;
        }
      }
    }

  rtc.setTime(tHour, tMin, t.sec);
  lcd.setCursor(0,0);
  lcd.print("H:");
  lcd.print(h);
  lcd.print("|");
  lcd.print("T:");
  lcd.print(temp);
  lcd.print("C");
  lcd.setCursor(4,1);
  lcd.print(rtc.getTimeStr());
  delay(1000);
}
