#include "Parameter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeName(char *dest, char *source) {
  int i;
  for (i = 0; * (source + i); i++) {
    *(dest + i) = *(source + i);
  }
  *(dest + i) = *(source + i);
}

void setup() {
  _Bool workMode;
  _Bool modiMode;
  Serial.begin(9600);
start:

  int validInfoNum = EEPROM.read(0);
  Serial.println("Please select woring mode, 1 for get vistors data, 0 for modify guests data.");
  Serial.println();
  while (Serial.available() == 0);
  workMode = Serial.read() == '1' ? 1 : 0;
  delay(50);
  if (workMode == 1) {
    Serial.print("validInfoNum = ");
    Serial.println(validInfoNum);
    int ads = 0;
    for (int i = 0; i < validInfoNum; ads++) {  //i is the counter, ads is the actual address
      info loopInfo;
      EEPROM.readBlock(sizeof(info) * ads + 1, loopInfo);
      if (loopInfo.isInUse) {
        i++;
        Serial.println();
        Serial.print("Address = ");
        Serial.println(ads);

        Serial.print("Name = ");
        for (int j = 0; loopInfo.name[j]; j++) Serial.print(loopInfo.name[j]);
        Serial.println();

        Serial.print("Height = ");
        Serial.println(loopInfo.height);

        Serial.print("Knock: ");
        for (int j = 0; j < MAX_KNOCKS_TIMES - 1 && loopInfo.knock[j]; j++) {
          Serial.print(loopInfo.knock[j]);
          Serial.print(' ');
        }
        Serial.println();
        Serial.println();
      }
    }
    Serial.println();
  }
  if (workMode == 0) {
    //    char tempName[MAX_NAME_SIZE + 1]={0};

    Serial.println("Please select modification mode, 1 for change name, 0 for delete data.");
    Serial.println();
    while (Serial.available() == 0);
    modiMode = Serial.read() == '1' ? 1 : 0;

    if (modiMode == 1) {
      Serial.println("Please input the Address of the data you want to modify.");
      Serial.println();
      while (Serial.available() == 0);
      int no = 0;
      while (Serial.available() > 0)
      {
        no = no * 10 + Serial.read() - '0';
        delay(2);
      }
      Serial.println("Please input the name");
      Serial.println();
      while (Serial.available() == 0);
      char tempName[MAX_NAME_SIZE] = {0};
      int i = 0;
      while (Serial.available() > 0)
      {
        tempName[i++] = char(Serial.read());
        delay(2);
      }
      EEPROM.writeBlock(sizeof(info) * no + 2, tempName);
    }

    if (modiMode == 0) {
      Serial.println("Please input the Address of the data you want to delete.");
      Serial.println();
      while (Serial.available() == 0);
      int no = 0;
      while (Serial.available() > 0)
      {
        no = no * 10 + Serial.read() - '0';
        delay(2);
      }
      EEPROM.write(0, --validInfoNum);
      EEPROM.writeBlock(sizeof(info) * no + 1, (_Bool)0);

    }

  }
  Serial.println("Still need to read or write? 1 for yes, 0 for no");
  Serial.println();
  while (Serial.available() == 0);
  if (Serial.read() == '1') {
    delay(5);
    goto start;
  }

  delay(5);
}

void loop() {}
