#include "Parameter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
   Function
*/

//Basic Indentification
void startKnock(void);
void getVibration(void);
void getSound(void);
byte getHeight(void);
//Doorbell
void firstSection(void);
void beep(int note, int duration);
void ringDoorbell(void);
void stopDoorbell(void);
void keypadEvent(KeypadEvent key);
//Administrator Identification
char* getPasswordInput(void);  //return a string<u> starts and<\u> ends with '#'
_Bool isRightPassword(char* input);
void wrongPasswordAlert(void);
//Reaction
byte compareVibrationAndHeight(void);  //If found, return the address of the info in EEPROM starting from 0. Otherwise, return -1.
void displayOrStoreInfo(void);
char* findName(byte address_EEPROM);
void projectToLCD(char *pName);
void unlock(void);
void redAlert(void);

/*
   Global Variables
*/
char password[] = "123ABCD#";
byte validNumber_EEPROM;
char keypadInput[MAX_PASSWORD_LENGTH + 1] = {0};
int counter;  //music
info thisInfo;  //store info. of current vistor
_Bool noKnock = 1;
_Bool isKnocking = 0;  //Used in Interrupt
_Bool passDoubleIdentification = 0;
_Bool isHoldKey = 0;
byte address_First_Unused_Unit;  //***likes 0 1 2 3 4 5 6
unsigned int tempT;


/*
   Sensor and Worker Settings
*/
Servo myServo;
LiquidCrystal_74HC595 myLcd(11, 13, 12, 1, 3, 4, 5, 6, 7);  //rs, e, d4-d7
char keys[4][4] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
byte rowPins[4] = {8, 9, A5, A4};
byte colPins[4] = {A3, A2, A1, A0};
Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
   Functional Function
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void startKnock(void) {
  detachInterrupt(digitalPinToInterrupt(pinSound));  //Prevent the recording processing for being interrupted. Also detach after first knock
  Serial.println("Sound dectected");
  isKnocking = 1;  //make the loop() reaches the process of recording height as quick as possible(jump getHeight())
  tempT = millis();  //keep the time before being interrupted
}
void getVibration() {
  if (!noKnock) return;  //Prevent from overwriting data, or recording the vibration of opening the door
  noKnock = 0;
  int tempInitTime = millis() % 65536, tempLastTime = tempInitTime;
  byte p;
  _Bool Vstatue = 1; //1 for V, 0 for nonV
  int knockInfo[2 * MAX_KNOCKS_TIMES - 1] = {0};

  int tempT = millis() % 65536;
  while ( (tempT > tempLastTime ? tempT - tempLastTime : 65536 - tempLastTime + tempT) > KnockStopsTime && p < MAX_KNOCKS_TIMES) {
    if (Vstatue == 0 && digitalRead(pinVibration) == 1) {  //Starts vibration
      knockInfo[p++] = tempT > tempLastTime ? tempT - tempLastTime : 65536 - tempLastTime + tempT;
      tempLastTime = tempT;
      Vstatue = 1;
    }
    else if (Vstatue == 1 && digitalRead(pinVibration) == 0) {  //Stops vibration
      knockInfo[p++] = tempT > tempLastTime ? tempT - tempLastTime : 65536 - tempLastTime + tempT;
      tempLastTime = tempT;
      Vstatue = 0;
    }
    tempT = millis() % 65536;
  }
}

void getSound(void) {
  if (!noKnock) return;  //Prevent from overwriting data, or recording the vibration of opening the door
  noKnock = 0;
  unsigned int tempLastTime = tempT, dT = 0;
  byte p = 0;
  byte count = 0;
  _Bool Vstatue = 0; //1 for V, 0 for nonV
  int knockInfo[MAX_KNOCKS_TIMES - 1] = {0};
#if ignoreVtime
  delay(100);  //prevent from recording the PWM
  while ( dT < KnockStopsTime && count < MAX_KNOCKS_TIMES - 1) {
    if (digitalRead(pinSound) == 0 && dT > 100) { //Starts vibration
      knockInfo[count++] = dT;
      tempLastTime = tempT;
    }
    tempT = millis() % 65536;
    dT = tempT > tempLastTime ? tempT - tempLastTime : 65536 - tempLastTime + tempT;
  }
#else
  while ( dT < KnockStopsTime && count < MAX_KNOCKS_TIMES) {
    if (Vstatue == 0 && digitalRead(pinSound) == 1) {  //Starts vibration
      knockInfo[count++] = dT;
      Vstatue = 1;
      count++;
    }
    else if (Vstatue == 1 && digitalRead(pinSound) == 0) {  //Stops vibration
      tempLastTime = tempT;
      Vstatue = 0;
    }
    tempT = millis() % 65536;
    dT = tempT > tempLastTime ? tempT - tempLastTime : 65536 - tempLastTime + tempT;
  }
#endif

  for (int i = 0; i < MAX_KNOCKS_TIMES - 1/* && knockInfo[i]*/; i++) thisInfo.knock[i] = knockInfo[i];  //overwrite all unit in the array
  for (int i = 0; i < MAX_KNOCKS_TIMES - 1 && knockInfo[i]; i++) Serial.println(knockInfo[i]);
  isKnocking = 0;
  // attachInterrupt(digitalPinToInterrupt(pinSound), getSound, RISING);  //If allowed multiple times of knocking

  if (thisInfo.knock[0] == 0) {  //Prevent the case that the recorder being activated by noise
    noKnock = 1;
    attachInterrupt(digitalPinToInterrupt(pinSound), startKnock, FALLING);
  }
}


byte getHeight(void) {
  //Activate Ultrasonic Sensor
  digitalWrite(pinTrig, 0);
  delayMicroseconds(10);
  digitalWrite(pinTrig, 1);
  delayMicroseconds(10);
  digitalWrite(pinTrig, 0);
  int duration = pulseIn(pinEcho, 1);  //Records Ultrasonic Wave's propagation in μs
  byte distance = (byte)(duration * 0.0174);  //Compute distance from roof in cm
  if (distance < ROOF_HEIGHT && distance > 0)
    return ROOF_HEIGHT - distance; //Return valid height
  else return 0;  //Return Error Mark
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firstSection(void)
{
  beep(a, 500);
  beep(a, 500);
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);

  delay(500);

  beep(eH, 500);
  beep(eH, 500);
  beep(eH, 500);
  beep(fH, 350);
  beep(cH, 150);
  beep(gS, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);

  delay(500);
}

void beep(int note, int duration)
{
  tone(pinSpeaker, note);  //Play tone on pinSpeaker
  delay(duration);
  noTone(pinSpeaker);  //Stop tone on pinSpeaker
  delay(50);
  counter++;
}

void ringDoorbell(void) {
  counter = 0;
  firstSection();
}

void stopDoorbell(void) {
  digitalWrite(pinSpeaker, 0);
}

void keypadEvent(KeypadEvent key) {
  switch (myKeypad.getState()) {
  case PRESSED:
    isHoldKey = 1;
    break;

  case RELEASED:
    isHoldKey = 0;
    break;

  case HOLD:
    break;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* getPasswordInput(void) {
  for (int i = 0; i < MAX_PASSWORD_LENGTH + 1; i++) keypadInput[i] = 0;
  int i = 0;
  for (; i < MAX_PASSWORD_LENGTH + 1;) {
    char key;
    while (1) {
      key = myKeypad.getKey();
      if (key != 0) break;
    }
    Serial.println(key);
    keypadInput[i] = key;
    if (keypadInput[i] == '#') {
      return keypadInput;
    }
    else i++;
  }
  Serial.println(keypadInput[0]);
  return keypadInput;
}

_Bool isRightPassword(char* input) {
  _Bool b = 1;
  int i;
  for (i = 0; password[i] != '#'; i++) {
    if (password[i] != keypadInput[i]) {
      b = 0;
      break;
    }
  }
  if (password[i] == '#' && keypadInput[i] != '#') {  //判断
    b = 0;
  }
  for (int i = 0; i < MAX_PASSWORD_LENGTH + 1; i++) keypadInput[i] = 0;
  if (b)Serial.println("RIGHTPASSWORD");
  return b;
}

void wrongPasswordAlert(void) {
  for (int i = 1200; i > 500; i--) {
    tone(pinSpeaker, i);
    delay(2);
  }
  noTone(pinSpeaker);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
byte compareVibrationAndHeight(void) {  //If found, return the address of the info in EEPROM starting from 0. Otherwise, return -1.
  _Bool isFirstUnused = 1;
  byte tempAddress = 0;
  for (int i = 0; i < validNumber_EEPROM; tempAddress++) {  //i is a counter
    info loopInfo;
    EEPROM.readBlock(sizeof(info) * tempAddress + 1, loopInfo);
    if (!loopInfo.isInUse) {  //avoid invalid EEPROM unit
      if (isFirstUnused) {
        address_First_Unused_Unit = tempAddress;
        isFirstUnused = 0;
      }
      continue;
    }
    i++;
    _Bool b = 1;
    //compare height
    if (abs(loopInfo.height - thisInfo.height) > MAX_VALID_DIFF_HEIGHT) {
      b = 0;
      Serial.println(tempAddress);
      Serial.println("height not match");
    }

    //compare knock
    if (b) {
#if ignoreVtime
      for (int j = 0; j < MAX_KNOCKS_TIMES - 1 && loopInfo.knock[j]; j++) {
        if ( abs(thisInfo.knock[j] - loopInfo.knock[j]) > MAX_VALID_DIFF_NonVibration_Duration) {  //Non-Vibration
          b = 0;
          break;
        }
        Serial.println("knock match");
      }
#else
      for (int j = 0; j < 2 * MAX_KNOCKS_TIMES - 1; j++) {
        if ( j % 2 == 0 && abs(thisInfo.knock[j] - loopInfo.knock[j]) > MAX_VALID_DIFF_Vibration_Duration ) {  //Vibration
          b = 0;
          break;
        }
        if (j % 2 == 1 && abs(thisInfo.knock[j] - loopInfo.knock[j]) > MAX_VALID_DIFF_NonVibration_Duration) {  //Non-Vibration
          b = 0;
          break;
        }
      }
#endif
      if (b) {
        passDoubleIdentification = 1;
        return tempAddress;
      }
      Serial.println(tempAddress);
      Serial.println("Knocks not match");
    }
  }  //for loop ends here
  if (isFirstUnused) address_First_Unused_Unit = (byte) - 1;  //Unused info unit not found
  return (byte) - 1;
}

void displayOrStoreInfo(void) {
  validNumber_EEPROM = EEPROM.read(0);
  Serial.print("validNumber_EEPROM = ");
  Serial.println(validNumber_EEPROM);
  int tempAddress_targetInfo = compareVibrationAndHeight();  //1,2,3,4...

  if (tempAddress_targetInfo != (byte) - 1) {
    projectToLCD(findName(tempAddress_targetInfo));
  }

  if (tempAddress_targetInfo == (byte) - 1) {
    projectToLCD("Unknown Guest");
    thisInfo.name[0] = '\0';
    thisInfo.isInUse = 1;
    if (address_First_Unused_Unit != (byte) - 1)
      EEPROM.writeBlock( (address_First_Unused_Unit) * sizeof(info) + 1 , thisInfo);
    else {
      EEPROM.writeBlock( (validNumber_EEPROM++) * sizeof(info) + 1 , thisInfo);
      EEPROM.write(0, validNumber_EEPROM);
    }
  }

  Serial.print("tempAddress_targetInfo = ");
  Serial.println(tempAddress_targetInfo);

  Serial.print("address_First_Unused_Unit = ");
  Serial.println(address_First_Unused_Unit);

  Serial.print("validNumber_EEPROM = ");
  Serial.println(validNumber_EEPROM);
}

char* findName(byte address_EEPROM) {
  static info temp;  //let it be static so that its name can be read from outside
  EEPROM.readBlock(sizeof(info) * address_EEPROM + 1, temp);  // !!! NOT SURE IF THIS IS OK>>  Done.
  return temp.name;
}

void projectToLCD(char *pName) {
  myLcd.clear();
  digitalWrite(pinLCD_A, 1);
  if (pName[0]) myLcd.print(pName);
  else myLcd.print("Known Guest");  //case that the vistor has been record but has not been named
}


void unlock(void) {
  myServo.attach(pinServo);
  for (int i = 160; i >= 20; i--) {
    myServo.write(i);
    delay(5);
  }
  myServo.detach();
  delay(UNLOCK_TIME);
  myServo.attach(pinServo);
  for (int i = 20; i <= 160; i++) {
    myServo.write(i);
    delay(5);
  }
  myServo.detach();
}

void redAlert(void) {
  for (int i = 0; i < RedAlert_Time; i++) {
    tone(pinSpeaker, 500);
    delay(500);
    noTone(pinSpeaker);
    delay(500);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
   Main Function
*/

void setup() {
  Serial.begin(9600);  //For testing, disable for official version
  myLcd.begin(16, 2);
  pinMode(pinInfrared, 0);
  pinMode(pinSound, 0);
  pinMode(pinLatch, 1);
  pinMode(pinData, 1);
  pinMode(pinClock, 1);
  pinMode(pinLCD_A, 1);
  pinMode(pinSpeaker, 1);
  pinMode(pinTrig, 1);
  pinMode(pinEcho, 0);
  pinMode(pinServo, 1);
  //  myKeypad.addEventListener(keypadEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {  //Initialize
  _Bool canOpen = 0;
  _Bool hasUnlocked = 0;
  _Bool hasANewPassWordInput = 0;
  char *keypadInputAddress;
  for (int i = 0; i < MAX_PASSWORD_LENGTH + 1; i++) keypadInput[i] = 0;
  _Bool firstInto_displayOrStoreInfo = 1;  //Only record the first knock and the corresponding height. Prevent from writing on the EEPROM too frequently and damage it
  noKnock = 1;
  isKnocking = 0;
  passDoubleIdentification = 0;
  address_First_Unused_Unit = (byte) - 1;
  for (int i = 0; i < MAX_NAME_SIZE + 1; i++) thisInfo.name[i] = 0;
  for (int i = 0; i < MAX_KNOCKS_TIMES - 1; i++) thisInfo.knock[i] = 0;
  thisInfo.height = 0;
  myServo.attach(pinServo);
  myServo.write(160);
  delay(500);
  myServo.detach();

  //Wait for one's coming
  while (1) {
    delay(REFRESHING_PERIOD);
    if (digitalRead(pinInfrared))break;
  }

  //Initialize'
  unsigned int countTime = 0;
  unsigned int initTime = millis() % 65536;


  Serial.println("Human motion dectected");
  //Use Interrupt to Detect Kncok
#if useVib
  attachInterrupt(digitalPinToInterrupt(pinVibration), startKnock, FALLING);
#else
  attachInterrupt(digitalPinToInterrupt(pinSound), startKnock, FALLING);
#endif

  //Main Loop
  while (countTime < 30000 /*&& !hasUnlocked*/) {  //hasUnlocked 没用上， 看看需不需要admin的LCD显示很久
    if (!isKnocking) { //after the knock sensor being activated, shorten the time it takes to enter the recording function
      //Height
      if (!isKnocking) {
        int tempH = getHeight();  //Mid time-consuming
        // Serial.print("tempH = ");
        // Serial.println(tempH);
        if (tempH > thisInfo.height && tempH != -1) thisInfo.height = tempH;
      }

      //Keypad
      if (!isKnocking) {
        char tempK = myKeypad.getKey();
        switch (tempK) {
        case 0:
          stopDoorbell();
          break;

        case '#':
          keypadInputAddress = getPasswordInput();  //High time-consuming
          hasANewPassWordInput = 1;
          break;

        default:
          detachInterrupt(digitalPinToInterrupt(pinSound));
          ringDoorbell();  //High time-consuming
          attachInterrupt(digitalPinToInterrupt(pinSound), startKnock, FALLING);
        }
      }

      //Verify password input
      if (!isKnocking && hasANewPassWordInput) {
        if (isRightPassword(keypadInputAddress)) {
          if (passDoubleIdentification)
            unlock();
          break;
        }
        else {
          detachInterrupt(digitalPinToInterrupt(pinSound));
          wrongPasswordAlert();  //High time-consuming
          attachInterrupt(digitalPinToInterrupt(pinSound), startKnock, FALLING);
          for (int i = 0; i < MAX_PASSWORD_LENGTH + 1; i++) keypadInput[i] = 0;
        }
        hasANewPassWordInput = 0;
      }

      //Modify data in EEPROM
      if (!isKnocking && !noKnock && firstInto_displayOrStoreInfo) {  // !!! Assume that when someone got inside the area and knock, his height has been correctly recorded. WILL DISCUSS ABOUT THIS LATER>>
        displayOrStoreInfo();
        firstInto_displayOrStoreInfo = 0;
      }
    }

    //record knocking info
    if (isKnocking) {
#if useVib
      getVibration();
#else
      getSound();
#endif
      isKnocking = 0;
    }

    //Modify countTime in the End of a while() loop
    unsigned int tempT = millis() % 65536;
    countTime = tempT > initTime ? tempT - initTime : tempT + 65536 - initTime;
  }
  //After 30 seconds with no knock
  if (noKnock && digitalRead(pinInfrared) == 1) {
    redAlert();
  }

  myLcd.clear();
  digitalWrite(pinLCD_A, 0);  //stop the power supply of LCD after display so as to save energy
}




#if 0

待解决：
是否对密码进行编码操作
字母是否可用 6bits 而不是 1Byte 来存储？可以 前三bit 表示姓名长度，后面的表示字母
键盘和振动的两个外部中断是否可行，不可行该怎么办
**按键是否会引起振动导致两个函数一起被外部中断调用？？可通过优先调用键盘解决
595驱动LCD：链接：https://blog.csdn.net/tonyiot/article/details/89212560
***如果改用模拟的麦克风，则外部中断测敲门不可用了，只能改变中断方式。并且由于测距较耗时，可以将getHeight()改为定时中断。

#endif

