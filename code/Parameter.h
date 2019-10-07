#include <LiquidCrystal_74HC595.h>
#include <EEPROMex.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

#define pinTrig 4
#define pinEcho 5

#define pinInfrared 3

#define useVib 0
#define ignoreVtime 1
#define pinVibration 2
#define pinSound 2  //must be 2 or 3

#define pinSpeaker 6

#define pinServo 7

#define pinLatch 12
#define pinClock 13
#define pinData 11

#define pinLCD_A 10

//const parameter
#define EEPORM_SIZE 1024
#define ROOF_HEIGHT 191  //cm
#define Name_Display_Time 100000

//parater concerning verifying
#define MAX_PASSWORD_LENGTH 20
#define ValidDistance 200  //A Parameter Used in getHeight()
#define MAX_KNOCKS_TIMES 10
#define KnockStopsTime 2000
#define RedAlert_Time 10  //s
#define WORKTIME 30000  //Working Time after Someone coming
#define REFRESHING_PERIOD 2000  //Refreshing Perion of the Infrared Motion Sensor
#define UNLOCK_TIME 5000  //The time during which the door is unlocked, but you need to put the latch back mannually
#define MAX_VALID_DIFF_HEIGHT 3  //cm, the max difference between the recorded height and the measured height
#define MAX_VALID_DIFF_Vibration_Duration 5
#define MAX_VALID_DIFF_NonVibration_Duration 50
#define MIN_KnockSound 300  //of 1024
#define MAX_NAME_SIZE 10

const int c = 261;  //used in doorbell music
const int d = 294;
const int e = 329;
const int f = 349;
const int g = 391;
const int gS = 415;
const int a = 440;
const int aS = 455;
const int b = 466;
const int cH = 523;
const int cSH = 554;
const int dH = 587;
const int dSH = 622;
const int eH = 659;
const int fH = 698;
const int fSH = 740;
const int gH = 784;
const int gSH = 830;
const int aH = 880;

typedef struct
{
	_Bool isInUse;  //So as you can delete a info that you dont want and cover it with new info
	char name[MAX_NAME_SIZE + 1];
	byte height;
#if useVib
	int knock[2 * MAX_KNOCKS_TIMES - 1]; //knock[0] stands for the first duration of vibration
#else
	int knock[MAX_KNOCKS_TIMES - 1];
#endif
} info;


