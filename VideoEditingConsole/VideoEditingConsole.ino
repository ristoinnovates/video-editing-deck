// Risto Innovates
// Video Editing Deck v1
// Currently Works in Edit Page in Davinci Resolve (MacOS)
// Feel Free to program in the rest of the Pages to your liking


#include <Arduino.h>
#include "Keyboard.h"
#include <Encoder.h>

int currentTabIndex = 0;  // Variable to store the current tab index (0 to 7 for tabs 2 to 9)
bool isSelectionTool = false;
bool isSettingsOn = false;
bool isTimelineNotSource = true;

// MUX
// Outputs
int muxChannel1 = 4;
int muxChannel2 = 5;
int muxChannel3 = 6;
int muxChannel4 = 7;
int muxLEDOutput = 10; // Controls the selected LED on the multiplexer

// Inputs
int muxSwitchesInput1 = 8; // 16 channel digital
int muxSwitchesInput2 = 9; // 8 channel digital

// MULTIPLEXER 1 BUTTONS - 16 CH
const int NUMBER_MUX_1_BUTTONS = 16;
bool muxButtons1CurrentState[NUMBER_MUX_1_BUTTONS] = {0};
bool muxButtons1PreviousState[NUMBER_MUX_1_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX1[NUMBER_MUX_1_BUTTONS] = {0};
unsigned long debounceDelayMUX1 = 5;

// MULTIPLEXER 2 BUTTONS - 16 CH
const int NUMBER_MUX_2_BUTTONS = 16;
bool muxButtons2CurrentState[NUMBER_MUX_2_BUTTONS] = {0};
bool muxButtons2PreviousState[NUMBER_MUX_2_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX2[NUMBER_MUX_2_BUTTONS] = {0};
unsigned long debounceDelayMUX2 = 5;

// MULTIPLEXER 3 LEDS - 8 CH
const int NUMBER_MUX_LEDS = 8;
bool muxLEDsPreviousState[NUMBER_MUX_LEDS] = {0};
bool muxLEDsCurrentState[NUMBER_MUX_LEDS] = {0};

// Encoder
int encoderPinA = 2;
int encoderPinB = 3;

Encoder myEncoder(encoderPinA, encoderPinB); // Use the Encoder library for easy handling
int oldEncoderPos = 0; // To track changes in encoder position


int encoderControl = 0; // 0 - scrub timeline in Edit // 1 - zoom timeline on Edit

void setup() {
  // Initialize serial communication for debugging (optional)
  Serial.begin(9600);
 
  // Configure multiplexer output pins
  pinMode(muxChannel1, OUTPUT);
  pinMode(muxChannel2, OUTPUT);
  pinMode(muxChannel3, OUTPUT);
  pinMode(muxChannel4, OUTPUT);
  digitalWrite(muxChannel1, LOW);
  digitalWrite(muxChannel2, LOW);
  digitalWrite(muxChannel3, LOW);
  digitalWrite(muxChannel4, LOW);

  // Set up multiplexer input pins
  pinMode(muxLEDOutput, OUTPUT); // Control the selected LED
  pinMode(muxSwitchesInput1, INPUT_PULLUP); // Digital input for the first set of switches
  pinMode(muxSwitchesInput2, INPUT_PULLUP); // Digital input for the second set of switches

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
}

void loop() {
  updateEncoder();
  updateMUXButtons(muxSwitchesInput1, NUMBER_MUX_1_BUTTONS, muxButtons1CurrentState, muxButtons1PreviousState, lastDebounceTimeMUX1, debounceDelayMUX1, 1);
  updateMUXButtons(muxSwitchesInput2, NUMBER_MUX_2_BUTTONS, muxButtons2CurrentState, muxButtons2PreviousState, lastDebounceTimeMUX2, debounceDelayMUX2, 2);
  controlMuxLED();
}

void updateMUXButtons(int muxInputPin, int numberOfButtons, bool *currentState, bool *previousState, unsigned long *lastDebounceTime, unsigned long debounceDelay, int muxNumber) {
  for (int i = 0; i < numberOfButtons; i++) {
    int A = bitRead(i, 0);
    int B = bitRead(i, 1);
    int C = bitRead(i, 2);
    int D = bitRead(i, 3);
    digitalWrite(muxChannel1, A);
    digitalWrite(muxChannel2, B);
    digitalWrite(muxChannel3, C);
    digitalWrite(muxChannel4, D);
    delay(1);
    currentState[i] = digitalRead(muxInputPin);

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (currentState[i] != previousState[i]) {
        lastDebounceTime[i] = millis();

        if (currentState[i] == LOW) {
          if (muxNumber == 1 && i >= 7 && i <= 14) {
            selectTab(i - 7);
          } else {
            pressButton(muxNumber, i, 1);
          }
        } else {
          pressButton(muxNumber, i, 0);
        }

        previousState[i] = currentState[i];
      }
    }
  }
}

void selectTab(int tabIndex) {
  if (tabIndex < 0 || tabIndex > 7) return; // Ensure tabIndex is valid

  // Update the currentTabIndex
  currentTabIndex = tabIndex;

  if (currentTabIndex != 7 && isSettingsOn == false){
    // Send the corresponding keyboard shortcut (Shift + 2 through Shift + 9)
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press('2' + tabIndex); // '2' is the keycode for '2'; tabIndex 0 corresponds to '2', tabIndex 7 to '9'
    delay(2); // Small delay to ensure the keypress is registered
    Keyboard.releaseAll();  
  } else if (currentTabIndex != 7 && isSettingsOn == true) {
    Keyboard.press(KEY_ESC); // '2' is the keycode for '2'; tabIndex 0 corresponds to '2', tabIndex 7 to '9'
    delay(2); // Small delay to ensure the keypress is registered
    Keyboard.releaseAll();
    // Send the corresponding keyboard shortcut (Shift + 2 through Shift + 9)
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press('2' + tabIndex); // '2' is the keycode for '2'; tabIndex 0 corresponds to '2', tabIndex 7 to '9'
    delay(2); // Small delay to ensure the keypress is registered
    Keyboard.releaseAll();
  } else if (currentTabIndex == 7 && isSettingsOn == false) {
    // Send the corresponding keyboard shortcut (Shift + 2 through Shift + 9)
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press('2' + tabIndex); // '2' is the keycode for '2'; tabIndex 0 corresponds to '2', tabIndex 7 to '9'
    delay(2); // Small delay to ensure the keypress is registered
    Keyboard.releaseAll();
    isSettingsOn = !isSettingsOn;
  } else {
    Keyboard.press(KEY_ESC); // '2' is the keycode for '2'; tabIndex 0 corresponds to '2', tabIndex 7 to '9'
    delay(2); // Small delay to ensure the keypress is registered
    Keyboard.releaseAll();
    isSettingsOn = !isSettingsOn;
  }
  
}

void controlMuxLED() {
  // Turn off all LEDs first
  for (int i = 0; i < NUMBER_MUX_LEDS; i++) {
    int A = bitRead(i, 0);
    int B = bitRead(i, 1);
    int C = bitRead(i, 2);
    digitalWrite(muxChannel1, A);
    digitalWrite(muxChannel2, B);
    digitalWrite(muxChannel3, C);
    delay(1);
    if (currentTabIndex != i){
      digitalWrite(muxLEDOutput, LOW); // Turn off the LED
    } else {
      digitalWrite(muxLEDOutput, HIGH); // Turn on the LED
      delay(1);
      digitalWrite(muxLEDOutput, LOW);
    }
  }

}

void encoderKeyLeftArrow() {
  Keyboard.press(KEY_LEFT_ARROW);
  Keyboard.release(KEY_LEFT_ARROW); 
}

void encoderKeyRightArrow() {
  Keyboard.press(KEY_RIGHT_ARROW);
  Keyboard.release(KEY_RIGHT_ARROW); 
}

void encoderKeyDownArrow() {
  Keyboard.press(KEY_DOWN_ARROW);
  Keyboard.release(KEY_DOWN_ARROW); 
}

void encoderKeyUpArrow() {
  Keyboard.press(KEY_UP_ARROW);
  Keyboard.release(KEY_UP_ARROW); 
}

void encoderKeyZoomOut() {
  Keyboard.press(KEY_LEFT_GUI); // change with KEY_LEFT_CTRL for Windows, or add both
  Keyboard.press('-');
  Keyboard.release(KEY_LEFT_GUI); 
  Keyboard.release('-'); 
}

void encoderKeyZoomIn() {
  Keyboard.press(KEY_LEFT_GUI); // change with KEY_LEFT_CTRL for Windows, or add both
  Keyboard.press('+');
  Keyboard.release(KEY_LEFT_GUI); 
  Keyboard.release('+');
}


void updateEncoder() {
  int newEncoderPos = myEncoder.read() / 2; // Read the current encoder position
  if (newEncoderPos != oldEncoderPos) {

    // Calculate direction of rotation
    int direction = (newEncoderPos > oldEncoderPos) ? 1 : -1;

    
    switch (currentTabIndex) {
      case 0:
        break;
      case 1:
        break;
      case 2:
        if (direction == 1) {
          
          if (encoderControl == 0){
            encoderKeyLeftArrow();
          } else {
            if (isTimelineNotSource) {
              encoderKeyZoomOut();
            } else {
              encoderKeyUpArrow();
            }
          }  
          
        } else {
          if (encoderControl == 0){
            encoderKeyRightArrow();
          } else {
            if (isTimelineNotSource) {
              encoderKeyZoomIn();
            } else {
              encoderKeyDownArrow();
            }
          }
        }
        break;
      case 3:
        break;
      case 4:
        break;
      case 5:
        break;
      case 6:
        break;
      case 7:
        break;
    }
    
    oldEncoderPos = newEncoderPos; // Update the last known position
  }
}

void pressButton(int muxNumber, int buttonNumber, int state) {
  int midiNote = 70; // starting midi note
  if (state){
    Serial.print("Mux: ");
    Serial.print(muxNumber);
    Serial.print(" | Button: ");
    Serial.println(buttonNumber);
  }
  if (muxNumber == 1) {
   switch (currentTabIndex) {
    case 0:
    // MEDIA TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Media 0");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 1:
    // CUT TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Cut 1");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 2:
    // EDIT TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          if (state){
            Keyboard.press('n');
          } else {
            Keyboard.release('n');
          }
          break;
        case 1:
          if (state){
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('v');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('v');
          }
          break;
        case 2:
          if (state){
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('b');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('b');
          }
          break;
        case 3:
          if (state){
            if (isSelectionTool){
              Keyboard.press('a');
              Keyboard.release('a');
            } else {
              Keyboard.press('b');
              Keyboard.release('b');
            }
            isSelectionTool = !isSelectionTool;
          }
          break;
        case 4:
          if (state){
            Keyboard.press('j');
          } else {
            Keyboard.release('j');
          }
          break;
        case 5:
          if (state){
            Keyboard.press('k');
          } else {
            Keyboard.release('k');
          }
          break;
        case 6:
          if (state){
            Keyboard.press('l');
          } else {
            Keyboard.release('l');
          }
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Edit 2");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 3:
    // FUSION TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Fusion 3");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 4:
    // COLOR TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Color 4");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 5:
    // FAIRLIGHT TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Fairlight 5");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 6:
    // DELIVER TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - tab Deliver 6");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
    case 7:
    // SETTINGS TAB - MUX 1
      switch (buttonNumber) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          break;
        case 3:
          break;
        case 4:
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          break;
        case 9:
          break;
        case 10:
          break;
        case 11:
          break;
        case 12:
          break;
        case 13:
          break;
        case 14:
          break;
        case 15:
          if (state){
            Serial.println("button 15 Encoder - Settings 7");
            if (encoderControl == 0){
              Serial.println("ctrl 0, going to 1");
              encoderControl = 1;  
            } else {
              Serial.println("ctrl 1, going to 0");
              encoderControl = 0;
            }
          }
          break;
      }
      break;
   }
  } else if (muxNumber == 2) {
    switch (currentTabIndex) {
      case 0:
      // MEDIA TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
      case 1:
      // CUT TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
      case 2:
      // EDIT TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            if (state){
              Keyboard.press(KEY_RETURN);
            } else {
              Keyboard.release(KEY_RETURN);
            }
            break;
          case 1:
            if (state){
              Keyboard.press(KEY_BACKSPACE);
            } else {
              Keyboard.release(KEY_BACKSPACE);
            }
            break;
          case 2:
            if (state){
              Keyboard.press(KEY_F9);
            } else {
              Keyboard.release(KEY_F9);
            }
            break;
          case 3:
            if (state){
              Keyboard.press('q');
              Keyboard.press(KEY_LEFT_GUI);
              if (isTimelineNotSource) {
                Keyboard.press('2');  
              } else {
                Keyboard.press('5');
              }
              
            } else {
              Keyboard.release('q');

              Keyboard.release(KEY_LEFT_GUI);
              if (isTimelineNotSource) {
                Keyboard.release('2');  
              } else {
                Keyboard.release('5');
              }
              isTimelineNotSource = !isTimelineNotSource;
            }
            break;
          case 4:
            if (state){
              Keyboard.press('i');
            } else {
              Keyboard.release('i');
            }
            break;
          case 5:
            if (state){
              Keyboard.press('o');
            } else {
              Keyboard.release('o');
            }
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            // Trim Start 
            if (state){
              Keyboard.press(KEY_LEFT_SHIFT);
              Keyboard.press('[');
            } else {
              Keyboard.release(KEY_LEFT_SHIFT);
              Keyboard.release('[');
            }
            break;
          case 11:
            // Trim End
            if (state){
              Keyboard.press(KEY_LEFT_SHIFT);
              Keyboard.press(']');
            } else {
              Keyboard.release(KEY_LEFT_SHIFT);
              Keyboard.release(']');
            }
            break;
          case 12:
            // Undo
            if (state) {
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press('z');
            } else {
              Keyboard.release(KEY_LEFT_GUI);
              Keyboard.release('z');
            }
            break;
          case 13:
            // Redo
            if (state) {
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(KEY_LEFT_SHIFT);
              Keyboard.press('z');
            } else {
              Keyboard.release(KEY_LEFT_GUI);
              Keyboard.release(KEY_LEFT_SHIFT);
              Keyboard.release('z');
            }
            break;
          case 14:
            // Copy 
            if (state) {
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press('c');
            } else {
              Keyboard.release(KEY_LEFT_GUI);
              Keyboard.release('c');
            }
            break;
          case 15:
            // Paste
            if (state) {
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press('v');
            } else {
              Keyboard.release(KEY_LEFT_GUI);
              Keyboard.release('v');
            }
            break;
        }
        break;
      case 3:
      // FUSION TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
      case 4:
      // COLOR TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
      case 5:
      // FAIRLIGHT TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
      case 6:
      // DELIVER TAB - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
      case 7:
      // SETTINGS - MUX 2
        switch (buttonNumber) {
          case 0:
            break;
          case 1:
            break;
          case 2:
            break;
          case 3:
            break;
          case 4:
            break;
          case 5:
            break;
          case 6:
            break;
          case 7:
            break;
          case 8:
            break;
          case 9:
            break;
          case 10:
            break;
          case 11:
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;
        }
        break;
     }
  }
}
