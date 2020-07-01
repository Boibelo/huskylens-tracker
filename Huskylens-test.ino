//////////////////////////////////////////
// 
//          Huskylens Tracker 
//      Frank Sobel - 22 June 2020
//      version 1.0
//


// Librairies for the Huskylens Tracker system including servo control for rotation
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include "Servo.h"

// User Parameters 
const bool simul=1;

// Program Parameters
int redled = 2;
int greenled = 3;
int blueled=4;
int servopin=9;

// Camera FOV steetings;
int posx = 160;
int alphapos=90; // nominal value for central kick-off
long randposx;
int huskyfov=70; // assuming FOV of OV6540 is 70°
int n=1; //increment for scanning

Servo panphone;
HUSKYLENS huskylens;
SoftwareSerial mySerial(10, 11); // RX, TX

void printResult(HUSKYLENSResult result);

void setup() {

    pinMode(redled, OUTPUT);
    pinMode(greenled, OUTPUT);
    pinMode(blueled, OUTPUT);
    digitalWrite(redled, LOW);
    digitalWrite(greenled,LOW);
    digitalWrite(blueled,LOW);
    
    panphone.attach(servopin);
    Serial.begin(115200);
    mySerial.begin(115200);
    if(!simul) {
      while (!huskylens.begin(mySerial)) {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2.Please recheck the connection."));
        Lightwarning(redled, 450, 50);
        }
    }
}

void loop() {
    if (!simul) {
      if (!huskylens.request()) {
        Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
        Lightwarning(redled, 100, 400);
        }      
      else if(!huskylens.isLearned()) {
        Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
        Lightwarning(blueled, 200, 300);
      }
      else if(!huskylens.available()) {
        Serial.println(F("No block or arrow appears on the screen!"));
        // scanning to find the ball
        while (!huskylens.available()){
          alphapos=ScanObject(alphapos,n);
      Lightwarning(greenled, 50, 750);
          n++;
          delay(2000);
        }   
      }

      else {
        Serial.println(F("###########"));
        while (huskylens.available()){
            HUSKYLENSResult result = huskylens.read();
            printResult(result);
            alphapos=TrackingObject(result.xCenter,alphapos,huskyfov);
            delay(20);
        }
      }
    }
    else {
  
  // Simulation of ball tracking
      for (posx =0;posx<50;posx++) {
        randposx=random(10,310);
        alphapos=TrackingObject(randposx,alphapos,huskyfov);
        Lightwarning(greenled, 50, 450);
      }
  // Simulation of ball scanning
    for (n=1;n<10;n++) {
      alphapos=ScanObject(alphapos,n);
      Lightwarning(redled, 50, 750);
    }

    }
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}

int ScanObject(int alpha,int n) {
    panphone.write(alpha);
    Serial.println(String()+n+F(", Scanning at ")+alpha);
    if (alpha==90) {alphapos=alpha+10;n+=1;}
    else if (alpha>90) alphapos=alpha-10*n;
    else if (alpha<90) alphapos=alpha+10*n;
    if (alpha<0) alphapos=90;
    else if (alpha>179) alphapos=90;
    return(alphapos);
}

int TrackingObject(int resultposx,int alphapos,int huskyfov) {
    alphapos+=map(resultposx-160,-160,159,-0.5*huskyfov,0.5*(huskyfov-1));
    if (alphapos<0) {
      alphapos=0;
      Serial.println("maximum angle reached to right (0)");
    } 
    else if (alphapos>179) {
      alphapos=179;
      Serial.println("maximum angle reached to left (179)");
    }
    if (resultposx<107) {
      panphone.write(alphapos);
      Serial.println(String()+F("camera moved to right by ")+ alphapos);
    }
    else if (resultposx>213) {
      panphone.write(alphapos);
      Serial.println(String()+F("camera moved to left by ")+ alphapos);
    }
    else Serial.println("camera remains in position");
    return(alphapos);
}

void Lightwarning(int Cled, int delayon, int delayoff) {
        digitalWrite(Cled, HIGH);
        delay(delayon);
        digitalWrite(Cled,LOW);
        delay(delayoff);
}
