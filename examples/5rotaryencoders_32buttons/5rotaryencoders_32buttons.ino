#include <ESP32Encoder.h>
#include <Keypad.h>
#include <BleGamepad.h> 



BleGamepad bleGamepad("RaceKeys", "Arduino", 100);



////////////////////// BUTTON MATRIX //////////////////////

#define ROWS 4
#define COLS 4

// position on panel  UL, UR, LR, LL         UL = upper left   UR = upper right   LR = lower right   LL = lower left
byte rowPins[ROWS] = {27, 25, 12, 32};
byte colPins[COLS] = {17, 16, 02, 00};

uint32_t keymap[ROWS][COLS] = {
  {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4 },
  {BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8 },
  {BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12 },
  {BUTTON_13, BUTTON_14, BUTTON_15, BUTTON_16 }
};

Keypad customKeypad = Keypad( makeKeymap(keymap), rowPins, colPins, ROWS, COLS); 




//////////// ROTARY ENCODERS (WITH PUSH SWITCHES) ////////////

#define MAXENC 5

// position on panel   UL, UM, UR, LR, LL      UL = upper left   UM = upper middle UR = upper right   LR = lower right   LL = lower left
byte uppPin[MAXENC] = {36, 22, 39, 34, 19};
byte dwnPin[MAXENC] = {26, 21, 35, 14, 23};
byte prsPin[MAXENC] = {18, 15, 33, 13, 05};

uint32_t encoderUpp[MAXENC] = {BUTTON_17, BUTTON_20, BUTTON_23, BUTTON_26, BUTTON_29};
uint32_t encoderDwn[MAXENC] = {BUTTON_18, BUTTON_21, BUTTON_24, BUTTON_27, BUTTON_30};
uint32_t encoderPrs[MAXENC] = {BUTTON_19, BUTTON_22, BUTTON_25, BUTTON_28, BUTTON_31};
ESP32Encoder encoder[MAXENC];
unsigned long holdoff[MAXENC] = {0,0,0,0,0};
int32_t prevenccntr[MAXENC] = {0,0,0,0,0};
bool prevprs[MAXENC] = {0,0,0,0,0};
#define HOLDOFFTIME 150   // TO PREVENT MULTIPLE ROTATE "CLICKS" WITH CHEAP ENCODERS WHEN ONLY ONE CLICK IS INTENDED








////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  for (int8_t i=0; i<MAXENC; i++) {
    encoder[i].clearCount();
    encoder[i].attachSingleEdge(dwnPin[i], uppPin[i]);
    pinMode(prsPin[i], INPUT_PULLUP);
  }
  bleGamepad.begin();
  Serial.println("Booted!");
}



////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  unsigned long now = millis();


  // -- ROTARY ENCODERS : ROTATION -- //

  for (int8_t i=0; i<MAXENC; i++) {
    int32_t cntr = encoder[i].getCount();
    if (cntr!=prevenccntr[i]) {
      if (!holdoff[i]) {
        if (cntr>prevenccntr[i]) { sendKey(encoderUpp[i]); }
        if (cntr<prevenccntr[i]) { sendKey(encoderDwn[i]); }
        holdoff[i] = now;
        if (holdoff[i]==0) holdoff[i] = 1;  // SAFEGUARD WRAP AROUND OF millis() (WHICH IS TO 0) SINCE holdoff[i]==0 HAS A SPECIAL MEANING ABOVE
      }
      else if (now-holdoff[i] > HOLDOFFTIME) {
        prevenccntr[i] = encoder[i].getCount();
        holdoff[i] = 0;
      }
    }
    
  // -- ROTARY ENCODERS : PUSH SWITCH -- //

    bool pressed = !digitalRead(prsPin[i]);
    if (pressed != prevprs[i]) {
      if (pressed) {  // PRESSED
        pressKey(encoderPrs[i]);
      }
      else {          // RELEASED
        releaseKey(encoderPrs[i]);
      }
      prevprs[i] = !prevprs[i];
    }
  }



  // - BUTTON MATRIX - //

  char key = customKeypad.getKey();
  if (key) { sendKey(key); }


  delay(10);
  
}






////////////////////////////////////////////////////////////////////////////////////////

void sendKey(uint32_t key) {
    Serial.print("pulse   0x");
    Serial.println(key, HEX);
    if(bleGamepad.isConnected()) {
      bleGamepad.press(key);
      delay(150);
      bleGamepad.release(key);
    }
}

void pressKey(uint32_t key) {
    Serial.print("hold    0x");
    Serial.println(key, HEX);
    if(bleGamepad.isConnected()) {
      bleGamepad.press(key);
    }
}

void releaseKey(uint32_t key) {
    Serial.print("release 0x");
    Serial.println(key, HEX);
    if(bleGamepad.isConnected()) {
      bleGamepad.release(key);
    }
}



////////////////////////////////////////////////////////////////////////////////////////
