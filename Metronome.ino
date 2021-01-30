#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "notes.h"

//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int lcdRow=2;
const int lcdCol=16;
const int lcdNextLineRow=1;
const int lcdNextLineCol=0;

// declare the constants for the five LEDS
const int LED1 = 2;
const int LED2 = 3;
const int LED3 = 4;
const int LED4 = 5;
const int BTN_NEXT_PIN=6;
const int BTN_PREV_PIN=7;
const int BTN_SWITCH_PIN=8;
const int BTN_MODE_PIN=9;
const int BUZZER = 10; //Buzzer to arduino pin 9

const int indicatorAmount=4;

int ledArray[indicatorAmount] = {LED1, LED2, LED3, LED4};

float oneMinute=60000; //1000 milliseconds is equal to 1 second. 1000*60 milliseconds is equal to 60 seconds(1 minute).

int bpm=60;
float tempoDelay;
float switchDelay=200;
float indicatorDuration=100;//We do not turn on the buzzer for whole beat, only for the beginning of the beat. Since the sound is just an indicator which shows the beat changes to the artist, 100ms is enough to indicate.

int buzzerAFirstTone=220;//Note A.
int buzzerASecondTone=440;//Note A.
int buzzerTones[indicatorAmount]={buzzerASecondTone, buzzerAFirstTone, buzzerAFirstTone, buzzerAFirstTone};
int noteFrequency=buzzerASecondTone;//Default note

int resetCounter=0;
unsigned long timeNow=resetCounter;
int i=resetCounter;
int beginningTime=resetCounter;
int btnSwitchState=resetCounter;
int btnModeState=resetCounter;
int btnPrevState=resetCounter;
int btnNextState=resetCounter;
int metronomeMode=0;
int tunerMode=1;
int modeState=metronomeMode; //Defaultly, it is on metronome mode.
bool turnOnSystem=true;
bool turnOnIndicators=true;
bool playFrequency;


void setup() {
 Serial.begin(9600); // initialize Serial connection
 lcd.begin(lcdCol, lcdRow);
 lcd.backlight();//To Power ON the back light
//lcd.noBacklight();// To Power OFF the back light

 pinMode(LED1, OUTPUT);
 pinMode(LED2, OUTPUT);
 pinMode(LED3, OUTPUT);
 pinMode(LED4, OUTPUT);
 pinMode(BUZZER, OUTPUT); // Set buzzer - pin 8 as an output
 pinMode(BTN_NEXT_PIN, INPUT);
 Serial.begin(9600);
 
 tempoDelay=oneMinute/bpm;
 Serial.println(tempoDelay);
 DisplayBPM(bpm);
}

void loop() {
  CheckButtonStates();//Checking the button states in every loop.

  if(turnOnSystem==true){//TRY TO IMPROVE THIS LINE.
   if(modeState==metronomeMode){
    if(turnOnIndicators==true){
      tone(BUZZER, buzzerTones[i],indicatorDuration);
      
      digitalWrite(ledArray[i],HIGH);
      turnOnIndicators=false;//It is enough to turn it on once before quarterNote has been reached.
    }
    if(millis()>=tempoDelay+timeNow){
      timeNow+=tempoDelay;
      //noTone(BUZZER);
      digitalWrite(ledArray[i],LOW);  
      i++;
      turnOnIndicators=true; 
      
      if(i>=indicatorAmount)
        i=resetCounter;
    } 
   }
   else{//Tuner Mode //IT SHOULD BE ENOUGH TO PLAY THE NOTE JUST ONCE. NO NEED FOR THAT IN EVERY LOOP. FIX IT!!
    if(playFrequency==true){
      tone(BUZZER,noteFrequency);
      
      Serial.print("Note Frequency: ");
      Serial.println(noteFrequency);
      DisplayNoteFrequency(noteFrequency);
      playFrequency=false;//It is enough to play the frequency once otherwise it will be running the code "tone(BUZZER,noteFrequency);" every loop.
    }
   }
 }
}
void CheckButtonStates(){
  btnSwitchState=digitalRead(BTN_SWITCH_PIN);
  btnModeState=digitalRead(BTN_MODE_PIN);
  btnPrevState=digitalRead(BTN_PREV_PIN);
  btnNextState=digitalRead(BTN_NEXT_PIN);

  if(btnSwitchState==HIGH){
    TurnOnOffSystem();
    delay(switchDelay);
  }

  else if(btnModeState==HIGH){//THIS IS FORE MODE SELECTOR PIN
    ChangeMode();
    delay(switchDelay);
  }
  
  else if(btnNextState==HIGH){
    if(modeState==metronomeMode){
       bpm++;
       CalculateTempoDelay();
    }

    else{// If the mode state is tuner, then deal with the note frequency instead of not value.
      noteFrequency++;
      PlayFrequency();
    }
    /*This is for preventing fast button clicking.
    This will run only one time when you click the button.
    Meanwhile, the tempo will be also delayed for 200ms because of swichDelay time.
    But since we don't press the button all the time, there won't be any effect for tempo in a normal condition.*/
    delay(switchDelay);
  }

  else if(btnPrevState==HIGH){
    if(modeState==metronomeMode){
       bpm--;
       CalculateTempoDelay();
    }
    
    else{// If the mode state is tuner, then deal with the note frequency instead of not value.
      noteFrequency--;
      PlayFrequency();
    }
    
    delay(switchDelay);
  }
}

void CalculateTempoDelay(){
   tempoDelay=oneMinute/bpm;
   DisplayBPM(bpm);
}

void PlayFrequency(){
  playFrequency=true;
  Serial.println(playFrequency);
}
void ChangeMode(){
  if(modeState==metronomeMode){
    modeState=tunerMode;
    playFrequency=true;
  }
  else{
    modeState=metronomeMode;
    DisplayBPM(bpm); //Metronome mode shows the note values on the LCD screen.
  }
}

void TurnOnOffSystem(){
  if(turnOnSystem==true){
    turnOnSystem=false;
    noTone(BUZZER);
    lcd.noBacklight();
  }      
  else{
    turnOnSystem=true;
    playFrequency=true;// If the mode was in tuner when the system was turned off, then change the value of variable "playFrequency" with true to play the frequency again.
    lcd.backlight();
  }  
}
void DisplayBPM(int bpm){
  lcd.clear();
  lcd.print("BPM: ");
  lcd.print(bpm);
}

void DisplayNoteFrequency(int noteFrequency){
  lcd.clear();
  lcd.print("Note Frequency:");
  lcd.setCursor(lcdNextLineCol,lcdNextLineRow);
  lcd.print(noteFrequency);
}
