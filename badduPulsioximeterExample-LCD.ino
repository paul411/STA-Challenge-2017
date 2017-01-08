#include <EnableInterrupt.h>
#include <eHealth.h>
#include <LiquidCrystal.h>

//DATA Init
int SpO2Data [] = {98,98,98,98,98,97,97,96,96,96,95,95,95,95,93,93,93,92,92,92,91,91,91,90,90,90,90,89,86,86,85,84,83,
             83,83,82,80,78,77,76,76,76,76,76,76,76,75,73,73,73,72,72,71,71,71,71,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,59,59,64,63,62,61,60,58,58,57,55,55,56,57,57,57,57,57,58,61,62,62,61,61,64,65,68,68,68,70,71,
             73,73,74,74,74,74,74,75,75,74,74,75,75,75,77,77,77,77,77,77,78,79,79,79,79,80,82,81,82,83,83,83,84,85,
             86,86,86,86,86,86,86,86,86,87,88,88,88,90,90,90,91,90,90,91,91,91,91,92,92,92,92,92,93,93,93,93,94,94,
             94,94,95,95,95,95,95,95,96,96,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int BPMData[]  = {116,112,111,109,109,108,108,109,109,109,108,109,109,108,107,107,106,105,105,105,107,106,108,109,109,
              109,108,108,108,109,109,109,110,109,109,108,108,106,104,104,104,105,105,106,108,108,109,109,108,108,
              109,111,111,110,110,110,110,110,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,104,104,104,104,105,105,105,
              104,103,103,104,104,102,102,102,102,102,101,101,100,102,102,100,101,101,103,102,102,104,103,103,103,
              103,103,103,103,104,103,104,104,104,104,104,104,105,104,104,104,102,102,100,98,97,97,97,97,97,98,99,
              100,100,99,99,100,99,100,100,99,98,98,98,98,98,97,97,98,97,97,96,96,96,96,95,95,96,96,97,97,96,96,
              97,97,97,97,97,98,97,96,96,95,95,94,94,94,94,94,94,95,94,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


int cont = 0;
int nowSeconds;
int myBPM; // the heart rate
int mySpO2; // the pulse oximeter oxygen saturation
int lastSpO2;
int lastNote;
int myNoteLength = 100;


enum modes
{
  sim,
  pulseOx,
  data
};

modes myMode;

int myDelay = (60/(float)myBPM)*1000-myNoteLength;

int note = 50;
int noteON = 144;//144 = 10010000 in binary, note on command
int noteOFF = 128;//128 = 10000000 in binary, note off command
int pitchBendCommand = 224; //224 = 1110 0000 in binary, pitch bend on channel 0 command
int msb = 0;
int lsb = 0;
int velocity = 75;
float lastMills = 0-myDelay;

const int buttonPin = 50;
const int numModes = 3;
int lastButtonState;

int ledState = HIGH;         // the current state of the output pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

unsigned long t0 = millis();
LiquidCrystal lcd(53, 52, 5, 4, 3, 2);
byte heart[8] = {B00000, B00000, B01010, B11111, B11111, B01110, B00100, B00000};

void setup() {
  Serial.begin(115200);
  eHealth.initPulsioximeter();
  enableInterrupt(A15, readPulsioximeter, RISING);

  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  int nowSeconds;
  myMode = sim;

  lcd.begin(16, 2);   // set up the LCD's number of columns and rows
  lcd.print("PulseOx MIDI");
  
  lcd.createChar(0, heart);
  
  lastButtonState = 1;
}

void loop() {
    nowSeconds = millis()/1000;

    switch(myMode)
    {
      case sim:
        myBPM = map(analogRead(A1), 4, 1000, 200, 0);
        mySpO2 = map(analogRead(A0), 30, 1000, 70, 100);
        break;
      case pulseOx:
        //implement pOx stuff
        break;
      case data:
        myBPM = BPMData[nowSeconds];
        mySpO2 = SpO2Data[nowSeconds];
        break;
      
    }
    
    if(myBPM < 0) { myBPM = 0; }

    

    if(digitalRead(buttonPin) == 0 && lastButtonState == 1)
    {
       myMode = (myMode+1)%numModes;
       lastButtonState = 0;
       Serial.println(myMode);
    }

    if(digitalRead(buttonPin) == 1 && lastButtonState == 0)
    {
      lastButtonState = 1;
    }

    switch(myMode)
    {
      case sim: 
        lcd.setCursor(13,0);
        lcd.print("SIM");
        break;
      case pulseOx:
        lcd.setCursor(13,0);
        lcd.print("pOX");
        break;
      case data: 
        lcd.setCursor(13,0);
        lcd.print("dat");
        break;
    }
  
    //Serial.print("BPM: ");
    //Serial.print(eHealth.getBPM());
    //Serial.print(" SpO2: ");
    //Serial.println(eHealth.getOxygenSaturation());
    
    lcd.setCursor(0,1);
    lcd.print("HR ");
    
    lcd.setCursor(3,1);
    lcd.print("   ");
    lcd.setCursor(3,1);
    lcd.print(myBPM);
    
    lcd.setCursor(7,1);
    lcd.print("SpO2");
  
    lcd.setCursor(12,1);
    lcd.print("   ");
    lcd.setCursor(12,1);
    lcd.print(mySpO2);

    note = map(mySpO2, 70, 100, 50, 69); // piano keys 50-69
    myDelay = (60/(float)myBPM)*1000-myNoteLength;
    if(myBPM > 0) { 
      sendMIDINote();
    }
}

void sendMIDINote() {
  long mills = millis();
  if (mills > lastMills+myDelay+myNoteLength) {
    lcd.setCursor(15,1);
    lcd.write(byte(0));

    
    
    //MIDImessage(noteON, note, velocity);//turn note on
    
    
    if(mySpO2 < 75)
    {
      MIDImessage(noteON, note, velocity);//turn note on
      delay((myDelay+myNoteLength)*1/8);
      MIDImessage(noteOFF, note, velocity);//turn note off
      delay(myDelay*1/8);
      MIDImessage(noteON, note, velocity);
      delay(myDelay*1/8);
      MIDImessage(noteOFF, note, velocity);//turn note off
      delay(myDelay*5/8);
    }
    else
    {
      if(note > lastNote)
      {
        MIDImessage(noteON, lastNote, velocity);//play the last note
        for (msb=64;msb<=96;msb++){//increase pitchbend  msb 64 (no pitchbend) to 80 (one semitones)
          MIDImessage(pitchBendCommand, lsb, msb);//send pitchbend up message
          delay((myDelay-10)/32);
        }
        MIDImessage(176, 120, 0); // clear all sounds command
      }
      else {
        MIDImessage(noteON, note, velocity);//turn note on
        delay(myNoteLength);
        MIDImessage(noteOFF, note, velocity);//turn note off
      }
      
      //delay(myNoteLength);
      //MIDImessage(noteOFF, note, velocity);//turn note off
    
    }
    
    sendMIDIPitchBendMessage(64,0);
    lcd.setCursor(15,1);
    lcd.print(" ");

    lastSpO2 = mySpO2;
    lastNote = note;
    lastMills = mills;
  }  
}

//send MIDI message
void MIDImessage(int command, int x, int y) {
  Serial.write(command);//send note on or note off command 
  Serial.write(x);//send pitch data
  Serial.write(y);//send velocity data
//Serial.println();
}

void sendMIDIPitchBendMessage(int msb, int lsb)
{
  Serial.write(pitchBendCommand); //send Pitchbend command.
  Serial.write(lsb); //send lSB
  Serial.write(msb);
}

//Include always this code when using the pulsioximeter sensor
//=========================================================================
void readPulsioximeter(){  

  cont ++;

  if (cont == 50) { //Get only of one 50 measures to reduce the latency
    eHealth.readPulsioximeter();  
    cont = 0;
  }
}
