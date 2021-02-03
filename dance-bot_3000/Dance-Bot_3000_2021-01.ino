/*
  - servo ref: <http://people.interaction-ivrea.it/m.rinott>
  - light sensor ref: https://create.arduino.cc/projecthub/DCamino/ambient-light-sensor-using-photo-resistor-and-led-lights-9b7f39
  - avoidance sensor ref: https://osoyoo.com/2017/07/24/arduino-lesson-obstacle-avoidance-sensor/

  Analog Pins:
  A0 - does not work
  A1 - works as expected
  A2 - reads from physical A4 input
  A3 - does not work
  A4 - reads from physical A2 input
  A5 - works as expected
*/

#include <Servo.h>
//#include <IRremote.h>

// Demo mode
bool demoMode = false; //set to true to bypass sensors and keep robot dancing

// servo objects
Servo servoLeftArm;  // create servo object to control left arm servo
Servo servoRightArm;  // create servo object to control right arm servo
Servo servoLeftLeg;  // create servo object to control left leg servo
Servo servoRightLeg;  // create servo object to control right leg servo

// servo pins
int servoLeftArmPin = 9; //left arm pin for servo
int servoRightArmPin = 12; //right arm pin for servo
int servoLeftLegPin = 10; //left leg pin for servo
int servoRightLegPin = 11; //right leg pin for servo

// sensor pins
int sensorMotionPin = 3; // store pin for motion sensor (obstacle avoidance)
word sensorLightPin = "A4"; // store pin for light sensor

// output pins
int ledPin = 5; // 7-color LED

// sensor values
word sensorMotionValue; // HIGH means obstacle, LOW means no obstacle
int sensorLightValue; // under 450 is dark

// action triggers
bool motionDetected = false; // store true/false if motion was detected
bool lowLightDetected = false; // store true/false if lights are off (room is dark)
bool lightsOn = false; // store true/flase for if lights were on when dancing started (lights will be kept on even if lights are turned on after dancing started)
bool justDance = false; // store true/false if criteria met to start/continue dancing

// track dancing time
int coolDownTime = 7500; // number of milliseconds to keep dancing after motion was last detected
unsigned long lastMotion = 0; // store number of milliseconds since Arduino started up as "dance start time" so can stop dancing after period of time. default to before ardeuino started.

// set delay time between dance moves
int standardDelay = 1000; // milliseconds to delay between dance moves

void setup() {
  // attach servos
  servoLeftArm.attach(servoLeftArmPin);  // attaches the servo pin to the servo object
  servoRightArm.attach(servoRightArmPin);  // attaches the servo pin to the servo object
  servoLeftLeg.attach(servoLeftLegPin);  // attaches the servo pin to the servo object
  servoRightLeg.attach(servoRightLegPin);  // attaches the servo pin to the servo object

  // output pins
  pinMode(ledPin, OUTPUT); // input for 7-color LED
  //digitalWrite(ledPin, HIGH); // high turns LED off

  // input pins
  pinMode(sensorMotionPin, INPUT);
  pinMode(sensorLightPin, INPUT);

  // logging
  Serial.begin(9600); Serial.flush();
}

void loop() {

  //// START: Read sensors ////

  // check sensors
  sensorMotionValue = digitalRead(sensorMotionPin); // read and save value from sensor
  sensorLightValue = analogRead(sensorLightPin); // read and save value from sensor

  //// END: Read sensors ////

  //// START: Check for conditions ////

  // check motion sensor value
  if (sensorMotionValue == LOW)
  {
    Serial.println("Motion detected"); Serial.flush();
    motionDetected = true;
  }
  else // HIGH
  {
    motionDetected = false;
  }

  // check light sensor value
  if (sensorLightValue < 450) // If is dark
  {
    //Serial.println(sensorLightValue); Serial.flush();
    lowLightDetected = true;
  }
  else
  { // >= 450
    //Serial.println(sensorLightValue); Serial.flush();
    lowLightDetected = false;
  }

  //// END: Check for conditions ////


  //// START: Run through logic ////

  //if motion detected (or demo mode enabled), keep dancing!
  if (motionDetected == true || demoMode == true)
  {
    // turn lights on only if low light detected when dancing, and keep them on even if lights are turned off later.
    if (lowLightDetected == true) {
      lightsOn = true;
      Serial.println("The lights are low... start the light show!"); Serial.flush();
    }

    Serial.println("light sensor value"); Serial.flush();
    Serial.println(sensorLightValue); Serial.flush();

    lastMotion = millis(); // set/update dance start time to milliseconds since Arduino started
    justDance = true; // keep dancing while motion detected
    Serial.println("I am dancing"); Serial.flush();
  }
  else
  {
    if (justDance == true)
    {
      // if no motion detected while dancing, keep dancing for coolDownTime milliseconds
      if ((lastMotion + coolDownTime) < millis()) {
        justDance = false; // stop dancing
        lastMotion = 0; // reset to 0
        lightsOn = false; // turn off lights when dancing stops

        Serial.println("Taking a break"); Serial.flush();
      }
      else
      {
        Serial.println("Still dancing for "); Serial.flush();
        // [current time] - [last motion detected time] - [how long to keep dancing after motion detected]) / 1000 to get seconds
        Serial.println(coolDownTime + lastMotion - millis()); Serial.flush();
        Serial.println("ms"); Serial.flush();

        //Serial.println(coolDownTime); Serial.flush();
        //Serial.println(lastMotion); Serial.flush();
        //Serial.println(millis()); Serial.flush();
      }
    } //else justDance == false
  }

  //// END: Run through logic ////


  //// START: Execute actions ////

  // keep lights on while dancing, if lights were off when dancing started, even if lights are turned on while dancing
  if (lightsOn == true)
  {
    digitalWrite(ledPin, LOW); // LOW turns LED on
    //Serial.println("LED on"); Serial.flush();
  }
  else
  {
    digitalWrite(ledPin, HIGH); // HIGH turns LED off
    //Serial.println("LED off"); Serial.flush();
  }

  // just dance is true when there is motion detected, or if motion was detected in the last coolDownTime milliseconds
  if (justDance == true) {
    // move legs then arms
    servoLeftLeg.write(random(80, 160)); // move to random position between 80 (front) and 160 (back)
    //delay(200);
    servoRightLeg.write(random(30, 110)); // move to random position between 30 (back) and 110 (front)
    delay(200);
    servoLeftArm.write(random(0, 150)); // move to random position between 0 (front) and 150 (back)
    //delay(200);
    servoRightArm.write(random(30, 180)); // move to random position between 30 (back) and 180 (front)
  }

  //// END: Execute actions ////


  //// START: Pause before next loop ////

  // dance faster (i.e. reduce delay between dance moves) if low light was detected while dancing
  if (justDance == true && lightsOn == true)
  {
    delay(standardDelay / 2); // speed up by 2X if lights are on
    Serial.println("...fast!!!"); Serial.flush();
  }
  else if (justDance == true && lightsOn == false)
  {
    delay(standardDelay);
    Serial.println("...slow..."); Serial.flush();
  }
  else
  {
    delay(200); //since no dancing, check more often for activity
    digitalWrite(ledPin, HIGH); // HIGH turns LED off
    Serial.println("chilling"); Serial.flush();
  }

  //// END: Pause before next loop ////

}// end loop
