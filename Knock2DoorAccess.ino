/* Arduino 'Knock Knock' Door Access System

   Components:
                - Arduino Uno
                - Digital Sound sensor (KY-037)
                - Green LED (x3)
                - 220Ohm resistor (x3)
                - Push button tactile switch
                - LCD I2C (20x4)
                - Breakboard
                - Some jumper wires

   Libraries:
                - LiquidCrystal_I2C library

   Created on 17 July 2022 by c010blind3ngineer
*/

#include <LiquidCrystal_I2C.h>

int soundDetectedVal;               // This is where we record our sound measurement
unsigned long lastSoundDetectTime;  // Record the time that we measured a sound
int soundAlarmTime = 200;           // Number of milli seconds to keep the sound alarm high
boolean noiseAlarm = false;

const int LED1 = 5;
const int LED2 = 6;
const int LED3 = 7;
const int rstBtn = 4;

// Knocking sequence for the 3 LEDs
int LED1_seq = 2;   // Knock 2 times
int LED2_seq = 1;   // Knock 1 time
int LED3_seq = 3;   // Knock 3 times

boolean f_knocks = false;   // first sequence of knocks
boolean s_knocks = false;   // second sequence of knocks
boolean t_knocks = false;   // third sequence of knocks

int t = 0;
int load = 0;
int knocks = 0;
int count = 0;

unsigned long f_knocksTime = 0;
unsigned long t_knocksTime = 0;
int durationForKnocks = 4000;   // normally the knocking sequence I do lasts about 4 seconds

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(rstBtn, INPUT);
  lcd.init();
  lcd.backlight();
}

void loop() {
  while (knocks < 6) {    // Once system got all 6 knocks, it will exit the 'while' loop
    soundDetectedVal = digitalRead(10);   // read the sound value from Digital Pin 10
    if (soundDetectedVal == HIGH) {
      
      // This will output the "Knock" message only once even if the signal remains at HIGH.
      if (!noiseAlarm) {                
        lastSoundDetectTime = millis();
        Serial.println("Knock");
        knocks++;
        count++;
        noiseAlarm = true;
      }
    }
    if ((millis() - lastSoundDetectTime) > soundAlarmTime && noiseAlarm) {
      noiseAlarm =  false;
    }
    while (load < 1) {    // Load the initialization message once
      doorKnockInit();
      load = 1;
    }

    // Start to check the knocks that correspond to the LEDs
    turnLED1_ON();
    if (f_knocks) {    // If first set of knocks is TRUE, go to LED2 function
      turnLED2_ON();
      if (s_knocks) {   // If second set of knocks is TRUE, go to LED3 function
        turnLED3_ON();
      }
    }
  }
  delay(1000);    // Give system some time to register all 6 knocks
  
  while (f_knocks && s_knocks && t_knocks) {    // When all three sets of knocks TRUE
    
    // Check to see if User didn't follow the correct knocking 'rhythm'
    if (t_knocksTime - f_knocksTime < 2000) {
      while ( t < 1) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("   Access Denied    ");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Press reset button..");
        t = 1;
      }
      // RESET and try again
      while (digitalRead(rstBtn) != HIGH) {};
      if (digitalRead(rstBtn) == HIGH) {
        resetting();
      }
    }
    
     // User follows the correct knocking 'rhythm'...
     // ...to attain the correct duration while knocking
    if ((t_knocksTime - f_knocksTime > 2000) && (t_knocksTime - f_knocksTime < durationForKnocks)) {
      while (t < 1) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("  Knocks accepted!  ");
        lcd.setCursor(0, 2);
        lcd.print("   Access Granted   ");
        t = 1;
      }
      delay(5000);
      // End of Access, exit the system
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("      Goodbye!      ");
      delay(1000);
      lcd.clear();
      lcd.noBacklight();
      f_knocks = false;
      s_knocks = false;
      t_knocks = false;
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
    }
    
    // User took too long to enter the knocking 'rhythm'
    if (t_knocksTime - f_knocksTime > durationForKnocks) {
      while (t < 1) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("  Ran out of time!  ");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Press reset button..");
        t = 1;
      }
      // RESET and try again
      while (digitalRead(rstBtn) != HIGH) {};
      if (digitalRead(rstBtn) == HIGH) {
        resetting();
      }
    }
  }
  delay(1000);  // let the system register the reset before detecting sound
}

// First Knocking Sequence 
void turnLED1_ON() {
  if (count == LED1_seq && f_knocks == false) {
    Serial.println("LED 1 - ACCEPTED");
    digitalWrite(LED1, HIGH);
    count = 0;
    f_knocks = true;
    f_knocksTime = millis(); // capture the time AFTER the 'first knocks' completed
  }
}

// Second Knocking Sequence
void turnLED2_ON() {
  if (count == LED2_seq && s_knocks == false) {
    Serial.println("LED 2 - ACCEPTED");
    digitalWrite(LED2, HIGH);
    count = 0;
    s_knocks = true;
  }
}

// Third Knocking Sequence
void turnLED3_ON() {
  if (count == LED3_seq && t_knocks == false) {
    Serial.println("LED 3 - ACCEPTED");
    digitalWrite(LED3, HIGH);
    count = 0;
    t_knocks = true;
    t_knocksTime = millis();  // capture the time AFTER the 'third knocks' completed
  }
}

void doorKnockInit() {
  lcd.setCursor(0, 0);
  lcd.print("      Arduino       ");
  lcd.setCursor(0, 1);
  lcd.print("'Knock Knock' System");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Please enter knock..");
}

void resetting() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Please enter knock..");
  knocks = 0;
  t = 0;
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  f_knocks = false;
  s_knocks = false;
  t_knocks = false;
}
