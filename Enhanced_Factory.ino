/* 
 
 Enhanced_Factory firmware for HexBright FLEX 
 v1.1 - Mar 18, 2013
 Based on Factory Firmware v2.4 - Dec 6, 2012
 
 btnReleaseTime timout added by David Blume, Jan 31, 2013
 Modifications by Jered Brown
 - Blinkng Mode changes for longer run time as Emergency Beacon - Mar 17, 2013
   - Brightness reduced to MED values
   - Blink rate reduced
 - Dazzle Mode Added - Mar 18, 2013
   - Activated as Long Press while any regular mode is active (Low, Med, High)
 - Morse Mode Added - Mar 18, 2013 **Disabled by Default**
   - SOS Beacon activated with Long Press while Blinking Mode is active
   - Currently Disabled. The exit code is unreliable to turn this mode off. 
     Best way right now is to rapidly press button until light starts cycling through regular modes.
 
 
 */

#include <math.h>
#include <Wire.h>

// Settings
#define OVERTEMP                340
// Pin assignments
#define DPIN_RLED_SW            2
#define DPIN_GLED               5
#define DPIN_PWR                8
#define DPIN_DRV_MODE           9
#define DPIN_DRV_EN             10
#define APIN_TEMP               0
#define APIN_CHARGE             3
// Modes
#define MODE_OFF                0
#define MODE_LOW                1
#define MODE_MED                2
#define MODE_HIGH               3
#define MODE_BLINKING           4
#define MODE_BLINKING_PREVIEW   5
#define MODE_DAZZLE             6
//#define MODE_MORSE              7

// State
byte mode = 0;
unsigned long btnReleaseTime = 0;
unsigned long btnTime = 0;
boolean btnDown = false;


void setup()
{
  // We just powered on!  That means either we got plugged 
  // into USB, or the user is pressing the power button.
  pinMode(DPIN_PWR,      INPUT);
  digitalWrite(DPIN_PWR, LOW);

  // Initialize GPIO
  pinMode(DPIN_RLED_SW,  INPUT);
  pinMode(DPIN_GLED,     OUTPUT);
  pinMode(DPIN_DRV_MODE, OUTPUT);
  pinMode(DPIN_DRV_EN,   OUTPUT);
  digitalWrite(DPIN_DRV_MODE, LOW);
  digitalWrite(DPIN_DRV_EN,   LOW);

  // Initialize serial busses
  Serial.begin(9600);
  Wire.begin();

  btnTime = millis();
  btnDown = digitalRead(DPIN_RLED_SW);
  mode = MODE_OFF;

  Serial.println("Powered up!");

  // Check the state of the charge controller
  int chargeState = analogRead(APIN_CHARGE);

  if (chargeState < 128)  // Low - charging
  {
    Serial.println("Charge State = Charging");
  }
  else if (chargeState > 768) // High - charged
  {
    Serial.println("Charge State = Charged");
  } //End Charge State Function


} //End void setup()
void loop()
{
  static unsigned long lastTempTime;
  static unsigned long lastBattCheck;
  unsigned long time = millis();

  // Check the state of the charge controller
  int chargeState = analogRead(APIN_CHARGE);

  if (chargeState < 128)  // Low - charging
  {
    digitalWrite(DPIN_GLED, (time&0x0100)?LOW:HIGH);
  }
  else if (chargeState > 768) // High - charged
  {
    digitalWrite(DPIN_GLED, HIGH);
  }
  else // Hi-Z - shutdown
  {
    digitalWrite(DPIN_GLED, LOW);    
  }

  // Check the temperature sensor
  if (time-lastTempTime > 1000)
  {
    lastTempTime = time;
    int temperature = analogRead(APIN_TEMP);
    Serial.print("Temp: ");
    Serial.println(temperature);
    if (temperature > OVERTEMP && mode != MODE_OFF)
    {
      Serial.println("Overheating!");

      for (int i = 0; i < 6; i++)
      {
        digitalWrite(DPIN_DRV_MODE, LOW);
        delay(100);
        digitalWrite(DPIN_DRV_MODE, HIGH);
        delay(100);
      }
      digitalWrite(DPIN_DRV_MODE, LOW);

      mode = MODE_LOW;
    }
  } //end temperature check function

  /* - Removed because Charge state has only 3 values. can't find charge percentage
   //Check the charge level
   if (time-lastBattCheck > 5000)
   {
   lastBattCheck = time;
   int chargeState = analogRead(APIN_CHARGE);
   Serial.print("Charge State: ");
   Serial.println(chargeState);
   Serial.print("Charge Level: ");
   if (chargeState < 128)  // Low - charging
   {
   Serial.println("Low(0-128) - Charging");
   }
   else if (chargeState < 768) // Med - Charging
   {
   Serial.println("Medium(129-768) - Charging");
   }
   else if (chargeState > 768) // High - Charged
   {
   Serial.println("High(>768) - Charged");
   }
   
   }
   */

  /* - Disabled until I find a reliable way to exit Morse Mode
   //Message for Morse Mode  
   char message[] = "SOS";
   int millisPerBeat = 150;
   
   word morse[] = {
   0x0202, // A .-
   0x0401, // B -...
   0x0405, // C -.-.
   0x0301, // D -..
   0x0100, // E .
   0x0404, // F ..-.
   0x0303, // G --.
   0x0400, // H ....
   0x0200, // I ..
   0x040E, // J .---
   0x0305, // K -.-
   0x0402, // L .-..
   0x0203, // M --
   0x0201, // N -.
   0x0307, // O ---
   0x0406, // P .--.
   0x040B, // Q --.-
   0x0302, // R .-.
   0x0300, // S ...
   0x0101, // T -
   0x0304, // U ..-
   0x0408, // V ...-
   0x0306, // W .--
   0x0409, // X -..-
   0x040D, // Y -.--
   0x0403, // Z --..
   0x051F, // 0 -----
   0x051E, // 1 .----
   0x051C, // 2 ..---
   0x0518, // 3 ...--
   0x0510, // 4 ....-
   0x0500, // 5 .....
   0x0501, // 6 -....
   0x0503, // 7 --...
   0x0507, // 8 ---..
   0x050F, // 9 ----.
   };
   */
  // Do whatever this mode does
  switch (mode)
  {
  case MODE_BLINKING:
  case MODE_BLINKING_PREVIEW:
    digitalWrite(DPIN_DRV_EN, (time%750)<75);
    break;
  case MODE_DAZZLE:
    static unsigned long lastTime;
    if (millis() - lastTime > 10)
    {
      digitalWrite(DPIN_DRV_EN, random(4)<1);
      lastTime = millis();
    }    
    break;

    /* - Disabled until I find a reliable way to exit Morse Mode
     // Code for Morse Mode 
     case MODE_MORSE:
     
     for (int i = 0; i < sizeof(message); i++)
     {
     char ch = message[i];
     if (ch == ' ')
     {
     // 7 beats between words, but 3 have already passed
     // at the end of the last character
     delay(millisPerBeat * 5);
     }
     else
     {
     // Remap ASCII to the morse table
     if      (ch >= 'A' && ch <= 'Z') ch -= 'A';
     else if (ch >= 'a' && ch <= 'z') ch -= 'a';
     else if (ch >= '0' && ch <= '9') ch -= '0' - 26;
     else continue;
     
     // Extract the symbols and length
     byte curChar  = morse[ch] & 0x00FF;
     byte nSymbols = morse[ch] >> 8;
     
     // Play each symbol
     for (int j = 0; j < nSymbols; j++)
     {
     digitalWrite(DPIN_DRV_EN, HIGH);
     if (curChar & 1)  // Dash - 3 beats
     delay(millisPerBeat * 3);
     else              // Dot - 1 beat
     delay(millisPerBeat);
     digitalWrite(DPIN_DRV_EN, LOW);
     // One beat between symbols
     delay(millisPerBeat);
     curChar >>= 1;
     
     }
     // 3 beats between characters, but one already
     // passed after the last symbol.
     delay(millisPerBeat * 3);
     } 
     }
     delay(millisPerBeat * 15);
     } // end Morse Mode function
     */
  } //End void loop()


  // Periodically pull down the button's pin, since
  // in certain hardware revisions it can float.
  pinMode(DPIN_RLED_SW, OUTPUT);
  pinMode(DPIN_RLED_SW, INPUT);

  // Check for mode changes
  byte newMode = mode;
  byte newBtnDown = digitalRead(DPIN_RLED_SW);
  switch (mode)
  {
  case MODE_OFF:
    if (btnDown && !newBtnDown && (time-btnTime)>20)
      newMode = MODE_LOW;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_BLINKING_PREVIEW;
    break;
  case MODE_LOW:
    if (btnDown && !newBtnDown && (time-btnTime)>50)
      if (time - btnReleaseTime > 2000)
        newMode = MODE_OFF;
      else
        newMode = MODE_MED;
    if (btnDown && !newBtnDown && (time-btnTime)>500)
      newMode = MODE_DAZZLE;
    break;
  case MODE_MED:
    if (btnDown && !newBtnDown && (time-btnTime)>50)
      if (time - btnReleaseTime > 2000)
        newMode = MODE_OFF;
      else
        newMode = MODE_HIGH;
    if (btnDown && !newBtnDown && (time-btnTime)>500)
      newMode = MODE_DAZZLE;
    break;
  case MODE_HIGH:
    if (btnDown && !newBtnDown && (time-btnTime)>50)
      newMode = MODE_OFF;
    if (btnDown && !newBtnDown && (time-btnTime)>500)
      newMode = MODE_DAZZLE;
    break;
  case MODE_BLINKING_PREVIEW:
    // This mode exists just to ignore this button release.
    if (btnDown && !newBtnDown)
      newMode = MODE_BLINKING;
    break;
  case MODE_BLINKING:
    if (btnDown && !newBtnDown && (time-btnTime)>50)
      newMode = MODE_OFF;
    // - Disabled until I find a reliable way to exit Morse Mode
    //if (btnDown && !newBtnDown && (time-btnTime)>500)
    //newMode = MODE_MORSE;
    break;
  case MODE_DAZZLE:
    if (btnDown && !newBtnDown && (time-btnTime)>50)
      newMode = MODE_OFF;
    break;   
    /* - Disabled until I find a reliable way to exit Morse Mode
     case MODE_MORSE:
     if (btnDown && !newBtnDown && (time-btnTime)>50)
     newMode = MODE_OFF;
     break;
     */
  }



  // Do the mode transitions
  if (newMode != mode)
  {
    switch (newMode)
    {
    case MODE_OFF:
      Serial.println("Mode = off");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, LOW);
      digitalWrite(DPIN_DRV_MODE, LOW);
      digitalWrite(DPIN_DRV_EN, LOW);
      break;
    case MODE_LOW:
      Serial.println("Mode = low");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 64);
      break;
    case MODE_MED:
      Serial.println("Mode = medium");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 255);
      break;
    case MODE_HIGH:
      Serial.println("Mode = high");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      analogWrite(DPIN_DRV_EN, 255);
      break;
    case MODE_BLINKING:
    case MODE_BLINKING_PREVIEW:
      Serial.println("Mode = blinking");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      break;
    case MODE_DAZZLE:
      Serial.println("Mode = dazzle");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;    
      /* - Disabled until I find a reliable way to exit Morse Mode
       case MODE_MORSE:
       Serial.println("Mode = Morse SOS");
       pinMode(DPIN_PWR, OUTPUT);
       digitalWrite(DPIN_PWR, HIGH);
       digitalWrite(DPIN_DRV_MODE, LOW);
       analogWrite(DPIN_DRV_EN, 255);
       break;
       */
    } // end Mode switch

    mode = newMode;
  } // end Mode if Function

  // Remember button state so we can detect transitions
  if (newBtnDown != btnDown)
  {
    if (!newBtnDown) {
      btnReleaseTime = time;
    }
    btnTime = time;
    btnDown = newBtnDown;
    delay(50);
  }
}


