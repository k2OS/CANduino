//#include <LiquidCrystal.h>
//  LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
//  LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
 
RTC_DS1307 RTC;

LiquidCrystal_I2C lcd(0x20,6,5,4,3,2,1,0);

int debug = 1;

float v = 1; // init
float scalea = 0.999;
float scaleb = 0.001;
int state = 0; // on or off (0 == free, 1 == busy)

int IN = 5;
int LED = 6; // change to 7
int REL = 12;


long int prevm = 0; // counter for seconds
long int interval = 1000;
// for message toggle
long int displayinterval = 2000;
long int prevdism = 0;
// skal skifte til state = 0,1,2..
int displaystate = 0; // 0 = visits, 1 = average, 2 = last visit

long int unsigned busytemp = 0; // temp timer
//long int unsigned busyshort = 0; // short visits - phased out
//long int unsigned busylong = 0; // long visits - phased out
long int unsigned busytotal = 0; // total use time
long int unsigned visits = 0;
long int busylast = 0;

long avgmins; 
long avgsecs;

void setup(){
//  lcd.setBacklightPin(13, POSITIVE);
  lcd.begin(16, 2);
  lcd.setCursor(0,0);lcd.print("Toilet is VACANT");

  //start serial connection
  if (debug) { Serial.begin(9600); }// works best with Serial enabled
  if (debug) { Serial.println("CANduino v3 - k2OS"); }
  
  //configure pin2 as an input and enable the internal pull-up resistor
  pinMode(IN, INPUT_PULLUP);

  pinMode(REL,OUTPUT); // one is for the LED on the door and the other is for the relay
  pinMode(LED,OUTPUT);

  digitalWrite(IN,HIGH);
  digitalWrite(REL,LOW);
  digitalWrite(LED,LOW);
  
  pinMode(13, OUTPUT); // onboard LED

}

void loop(){
  //read the pushbutton value into a variable
  int sensorVal = digitalRead(IN);
  // set a flag and start timer when door is locked. door is not declared 'locked' until after a set interval

  v = scalea*v + scaleb*sensorVal;
  // is the visit over?
  if (v >= 0.999 && state == 1) { 
    // we ignore all visits under 15 seconds
    if (busytemp > 15) {
      visits++;
      busylast = busytemp;
// we no longer handle long vs short visits on the 'duino as we save all data to SD (later)
      busytotal = busytotal + busytemp;
/*      if (busytemp >= 90) {
        // tissetår tager fra ca 50 sekunder og op 
        busylong = busylong + busytemp; 
        busytemp = 0; 
      } else {
        busyshort = busyshort + busytemp;
        busytemp = 0;
      } */
    }
    if (debug) { Serial.println(v); }
    digitalWrite(13,LOW); 
    digitalWrite(REL,LOW); 
    digitalWrite(LED,LOW); 
    state = 0; 
    lcd.setCursor(10,0);
    lcd.print("VACANT");
    lcd.setCursor(11,1);
    lcd.print("     ");
    busytemp = 0;
  } else if (v <= 0.0001 && state == 0) { 
    if (debug) { Serial.println(v); }
    digitalWrite(13,HIGH); 
    digitalWrite(REL,HIGH); 
    digitalWrite(LED,HIGH); 
    state=1;
    lcd.setCursor(10,0);
    lcd.print("IN USE");
  }

  if (millis()-prevm > interval) {
    // show busy message
    if (state == 1) {
      busytemp++; // increase the busy temp counter
      long mins = busytemp/60; //convert seconds to minutes
      long secs = busytemp-(mins*60); //subtract the coverted seconds to minutes in order to display 59 secs max 
 
      lcd.setCursor(11,1);
      lcd.print("        ");
      lcd.setCursor(11,1);
      if (mins < 10) { lcd.print("0"); }
      lcd.print(mins);
      lcd.print(":");
      if (secs < 10) { lcd.print("0"); }
      lcd.print(secs);
    }  // eo 'if state ..'
      else {
      lcd.setCursor(11,1);
      lcd.print("        ");
    }

    //show one message or the other, based on prevdism
    if ((millis()-prevdism) > displayinterval) {
      prevdism = millis();
      displaystate++; 
      if (displaystate == 3) { displaystate = 0; }
    }
      
    switch(displaystate) {
     case 0: 
        lcd.setCursor(0,1);
        lcd.print("           ");
        lcd.setCursor(0,1);
        lcd.print("Visits:");
        if (visits < 10) { lcd.print(" "); }
        lcd.print(visits); 
     break;
     case 1:
       lcd.setCursor(0,1);     
       lcd.print("           ");
       lcd.setCursor(0,1);
       lcd.print("Avg ");
//       avgmins = ((busylong+busyshort)/visits+1)/60; //convert seconds to minutes
//       avgsecs = ((busylong+busyshort)/visits+1)-(avgmins*60); //subtract the coverted seconds to minutes in order to display 59 secs max 
       avgmins = (busytotal/visits+1)/60; //convert seconds to minutes
       avgsecs = (busytotal/visits+1)-(avgmins*60); //subtract the coverted seconds to minutes in order to display 59 secs max 

       if (avgmins < 10) { lcd.print("0"); }
       lcd.print(avgmins);
       lcd.print(":");
       if (avgsecs < 10) { lcd.print("0"); }
       lcd.print(avgsecs); 
     break;
     case 2:
       lcd.setCursor(0,1);     
       lcd.print("           ");
       lcd.setCursor(0,1);
       lcd.print("Last ");
       avgmins = busylast/60; //convert seconds to minutes
       avgsecs = busylast-(avgmins*60); //subtract the coverted seconds to minutes in order to display 59 secs max 
       if (avgmins < 10) { lcd.print("0"); }
       lcd.print(avgmins);
       lcd.print(":");
       if (avgsecs < 10) { lcd.print("0"); }
       lcd.print(avgsecs); 
       break;
    }
    prevm = millis();
  } // eo 'is ... prevm'
 
} // eo loop

