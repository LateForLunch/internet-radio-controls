#include <LiquidCrystal_I2C.h>

#define KEY1_PIN A6
#define KEY2_PIN A7
#define VOL_PIN_1 A0
#define VOL_PIN_2 A1
#define VOL_HIGH 350

LiquidCrystal_I2C lcd(0x27, 16, 2);
String displayLine1 = "";
String displayLine2 = "";
unsigned long displayTime = 0;
int displayPeriod = 0;
bool tempDisplay = false;

bool onState = true;
byte volPin1State = 0;
byte volPin2State = 0;
int key1LastButton = -1;
int key2LastButton = -1;

const String KEY1_NAMES[] = {
  "power",
  "function",
  "play_pause",
  "stop",
  "prev",
  "next"
};

const String KEY2_NAMES[] = {
  "dsgx",
  "tool",
  "up",
  "enter",
  "down",
  "eject"
};

const int KEY_THRESHOLDS[][2] = {
  {50, 80},
  {100, 130},
  {195, 225},
  {260, 290},
  {360, 390},
  {470, 500}
};

void toggleOnOff() {
  if (onState) {
    Serial.println("power_off");
    lcd.noDisplay();
    lcd.noBacklight();
  } else {
    Serial.println("power_on");
    lcd.display();
    lcd.backlight();
  }
  onState = !onState;
}

void readKey(int pin, String *buttonNames, int &lastButton) {
  int analogVal = analogRead(pin);
  for (int i = 0; i < 6; i++) {
    if (analogVal >= KEY_THRESHOLDS[i][0] && analogVal <= KEY_THRESHOLDS[i][1]) {
      if (lastButton != i) {
        lastButton = i;
        if (pin == KEY1_PIN && i == 0) {
          // Power button has been pressed
          toggleOnOff();
        } else {
          if (onState) {
            Serial.println(buttonNames[i]);
          }
        }
      }
      return;
    }
  }
  lastButton = -1;
}

void readVolume() {
  byte newVolPin1State = analogRead(VOL_PIN_1) >= VOL_HIGH ? 1 : 0;
  byte newVolPin2State = analogRead(VOL_PIN_2) >= VOL_HIGH ? 1 : 0;
  if (newVolPin1State != volPin1State) {
    volPin1State = newVolPin1State;
    if (newVolPin2State != volPin2State) {
      volPin2State = newVolPin2State;
      if (onState) {
        if (newVolPin1State == newVolPin2State) {
          Serial.println("volume_up");
        } else {
          Serial.println("volume_down");
        }
      }
    }
  }
}

void setDisplay(String line1, String line2) {
  displayLine1 = line1;
  displayLine2 = line2;
  if (!tempDisplay) {
    printDisplay(line1, line2);
  }
}

void setTempDisplay(String line1, String line2, int ms) {
  printDisplay(line1, line2);
  displayPeriod = ms;
  displayTime = millis();
  tempDisplay = true;
}

void resetTempDisplay() {
  if (tempDisplay) {
    if((unsigned long)(millis() - displayTime) > displayPeriod) {
      printDisplay(displayLine1, displayLine2);
      tempDisplay = false;
    }
  }
}

void printDisplay(String line1, String line2) {
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void serialEvent() {
  String line1 = Serial.readStringUntil('|');
  String line2 = Serial.readStringUntil('\n');
  int indexOfColon = line1.indexOf(':');
  if (indexOfColon == -1) {
    return;
  }
  String timeStr = line1.substring(0, indexOfColon);
  line1 = line1.substring(indexOfColon + 1);
  int ms = timeStr.toInt();
  if (ms > 0) {
    setTempDisplay(line1, line2, ms);
  } else {
    setDisplay(line1, line2);
  }
}

void setup() {
  volPin1State = analogRead(VOL_PIN_1) >= VOL_HIGH ? 1 : 0;
  volPin2State = analogRead(VOL_PIN_2) >= VOL_HIGH ? 1 : 0;

  lcd.init();
  lcd.clear();
  lcd.backlight();

  setDisplay("Booting", "Waiting for RPi");

  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  setDisplay("No Media", "");
}

void loop() {
  readKey(KEY1_PIN, KEY1_NAMES, key1LastButton);
  readKey(KEY2_PIN, KEY2_NAMES, key2LastButton);
  readVolume();
  resetTempDisplay();
}
