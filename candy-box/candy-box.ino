/*
  -user turns on switch to power on Arduino
  -user hits power button on IR remote
  -arduino displays a message on LED: "Hello"
  -user enters pin
  -servo moves to open door on top of box
  -sound plays on speaker

*/
/*
   IRreceiveDemo.cpp

   Demonstrates receiving IR codes with IRrecv

    This file is part of Arduino-IRremote https://github.com/z3t0/Arduino-IRremote.

*/

#include <IRremote.h> // https://www.circuitbasics.com/arduino-ir-remote-receiver-tutorial/
#include <LiquidCrystal.h> // https://create.arduino.cc/projecthub/najad/interfacing-lcd1602-with-arduino-764ec4 uses a 3.3V connecttion, but this one uses a resister https://arduinogetstarted.com/tutorials/arduino-lcd)
#include <Servo.h>
#include "pitches.h" // for legend of zelda sound clips

// secret pin codes for family
const String rhysPin = "4431", riversPin = "8872", mommysPin = "3210", daddysPin = "0000";
const String maintStart = "9999", maintEnd = "9998";

Servo servoLid; // create object to control servo to open lid
const int servoLidPin = 10; // servo to open lid

const int IR_RECEIVE_PIN = 7; //IR receiver pin
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; // LCD pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // LCD setup

const int piezoPin = 9;

String pin; //store pin

void setup() {
  // logging setup
  //pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200); // don't forget to set COM baud to 115200
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.

  // LCD display setup
  lcd.begin(16, 2); // initializes the interface to the LCD screen, and specifies the dimensions (width and length) of the display. needs to be called before any other LCD library commands.
  lcd.print("Hello!");

  // piezo (speaker) setup
  pinMode(piezoPin, OUTPUT);

  playZeldaSound();

  // servo setup
  servoLid.attach(servoLidPin);
  servoLid.write(180); // closed position will be 180
  // temporarily open and close door on startup
  //servoLid.write(25); // temp start
  //moveServoWithDelay(170, 20); delay(1000); moveServoWithDelay(25, 20);

  // IR receiver setup
  Serial.println("Enabling IRin");
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver, enable feedback LED, take LED feedback pin from the internal boards definition

  Serial.print(F("Ready to receive IR signals at pin "));
  Serial.println(IR_RECEIVE_PIN);

  // give illusion more work is happening than actually is by delaying half a second
  delay(500);
  displayPin();
}

void loop() {
  monitorIRReceiver();
}

void playZeldaSound() {
  // todo: add code here to play some sound
}

void monitorIRReceiver() {
  if (IrReceiver.decode()) {
    // Print a short summary of received data
    //IrReceiver.printIRResultShort(&Serial);

    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      // We have an unknown protocol, print more info
      //IrReceiver.printIRResultRawFormatted(&Serial, true);
      Serial.println("unknown protocol");
      displayError("Please try again.");
    }

    String button = translateIrRemoteButton(IrReceiver.decodedIRData.command);
    // if button value can be converted to int, add it to pin value
    if (button == "power") {
      Serial.println("power button:");
      removeLastNumFromPin();
    }
    else if (isValidNumber(button)) {
      Serial.print(button);
      Serial.println(" button:");
      addToPin(button.toInt());
    }
    else {
      Serial.println(F("unrecognized button: "));
      Serial.println(IrReceiver.decodedIRData.command);
    }

    delay(400);
    IrReceiver.resume(); // Enable receiving of the next value
  }
}

// borrowed from https://forum.arduino.cc/index.php?topic=209407.msg1538544#msg1538544
boolean isValidNumber(String str) {
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i))) return true;
  }
  return false;
}

String translateIrRemoteButton(int command) {
  if (IrReceiver.decodedIRData.command == 0x45) {
    Serial.println("hitting power button removes last character");
    return "power";
  }
  else if (IrReceiver.decodedIRData.command == 0x16) {
    return "0";
  }
  else if (IrReceiver.decodedIRData.command == 0xC) {
    return "1";
  }
  else if (IrReceiver.decodedIRData.command == 0x18) {
    return "2";
  }
  else if (IrReceiver.decodedIRData.command == 0x5E) {
    return "3";
  }
  else if (IrReceiver.decodedIRData.command == 0x8) {
    return "4";
  }
  else if (IrReceiver.decodedIRData.command == 0x1C) {
    return "5";
  }
  else if (IrReceiver.decodedIRData.command == 0x5A) {
    return "6";
  }
  else if (IrReceiver.decodedIRData.command == 0x42) {
    return "7";
  }
  else if (IrReceiver.decodedIRData.command == 0x52) {
    return "8";
  }
  else if (IrReceiver.decodedIRData.command == 0x4A) {
    return "9";
  }
  else {
    return "other";
  }
}

// remove last number from pin
void removeLastNumFromPin() {
  int pinLen = pin.length(); // global var for pin, String type

  if (pinLen > 0) {
    pin = pin.substring(0, pinLen - 1);
    Serial.println(String("remove last char from pin"));
    displayPin();
  }
}

// add number to pin
void addToPin(int num) {
  int pinLen = pin.length(); // global var for pin, String type

  // only accept 4 characters for pin
  if (pinLen > 3) {
    displayError("max 4 chars"); // need to change this because if 4 chars entered, verification should be done.
    Serial.println("error... over 4 chars for pin");
  }
  else {
    pin += num;
    lcd.print(String(num)); // temporarily show the num entered
    delay(200);
    displayPin(); // update display with pin
    Serial.println(String("pin is now: " + pin));
  }

  if (pin.length() == 4) {
    // verify pin and move on to next phase
    Serial.println("Verify pin now");
    verifyPin();
  }
}

// return specified number of astrisks
String hideChars(int numAstrisks) {
  String returnChars;

  for (int i = 0; i < numAstrisks; i++) {
    returnChars.concat("*");
  }

  return returnChars;
}

// return pin with all but last char replaced with asterisk
String securePin() {
  int pinLen = pin.length(); // global var for pin, String type
  String lastChar = pin.substring(pinLen - 1, pinLen);

  return String(hideChars(pinLen - 1) + lastChar);
}

// display error for 1 second, then re-display pin
void displayError(String msg) {
  lcd.clear();
  lcd.print(msg);
  delay(1000);
  displayPin();
}

// display pin
void displayPin() {
  //String pinForDisplay = securePin(); //don't need this now that just replacing all chars with asterisks
  lcd.clear(); // sets cursor to 0 [col] and 0 [row] (zero based counting)
  lcd.print("Enter PIN: ");
  lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
  //lcd.print(pinForDisplay);
  //delay(500); //show last char for half second

  // then show all all asterisks
  //lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
  String allAsterisks = hideChars(pin.length());
  lcd.print(allAsterisks);
  lcd.blink(); // resume blink if it was disabled
}

// logic for opening door and then closing it again.
void openDoor(int doorOpenSeconds = 30, int doorCloseDelay = 30, bool leaveOpen = false) {
  String doorMessage = "Door closing in"; // e.g. Door closing in 30 seconds
  int doorOpenDelay = 20;
  int doorClosed = 180;
  int doorOpen = 35;

  Serial.println("open door");
  moveServoWithDelay(doorOpen, doorOpenDelay);
  if (leaveOpen == false) {
    countdown(doorOpenSeconds, doorMessage); // countdown for 30 seconds
    Serial.println("close door");
    lcd.clear();
    lcd.print("Door closing now");
    lcd.setCursor(0, 1); // 2nd row, first character
    lcd.print("!Watch fingers!");
    moveServoWithDelay(doorClosed, doorCloseDelay);
  }
  else {
    Serial.println("maintenance mode");
    lcd.clear();
    lcd.print("Maintenance");
    lcd.setCursor(0, 1); // 2nd row, first character
    lcd.print("Door remain open");
  }
  displayPin();
}

void verifyPin() {
  if (pin == rhysPin) {
    lcd.clear();
    lcd.print("Hello Rhys!");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Have some candy!");
    delay(1000);
    openDoor();
  }
  else if (pin == riversPin) {
    lcd.clear();
    lcd.print("Hello Rivers!");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Have some candy!");
    delay(1000);
    openDoor();
  }
  else if (pin == mommysPin) {
    lcd.clear();
    lcd.print("Hello Mommy!");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Have some candy!");
    delay(1000);
    openDoor();
  }
  else if (pin == daddysPin) {
    lcd.clear();
    lcd.print("Hello Daddy!");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Have some candy!");
    delay(1000);
    openDoor(2, 10); // daddy only gets 2 seconds and door closes fast :-(
  }
  else if (pin == maintStart) {
    lcd.clear();
    lcd.print("Maintenance Mode");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Leave door open");
    delay(1000);
    openDoor(30, 20, true); // leave door open for maintenance
  }
  else if (pin == maintEnd) {
    lcd.clear();
    lcd.print("Maintenance Mode");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Complete");
    delay(1000);
    openDoor(1, 20); // close door now that maintenance is complete
  }
  else {
    lcd.clear();
    lcd.print("PIN not found:-(");
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("Please try again");
    delay(3000);
  }

  pin = ""; // clear pin after each verification, positive or negative
  displayPin();
}

void countdown(int secToCountdown, String message) {
  lcd.noBlink(); // stop blinking for countdown
  lcd.clear();
  lcd.print(message);

  String countdownMsg = "";
  Serial.print("count down from ");
  Serial.println(secToCountdown);

  // countdown from secToCountdown to 0
  for (int i = secToCountdown; i > 0; i--) {
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print("                "); // clear 2nd row using 16 space characters
    lcd.setCursor(0, 1); // (column, row) zero based counting, equates to 1st column 2nd row
    lcd.print(i);
    lcd.print(" seconds");
    delay(1000);
  }
}

void moveServoWithDelay(int toPosition, int delayMS) {
  int current = servoLid.read(); // get last write value from servo

  // if moving up higher position...
  if (current < toPosition) {
    for (int i = current; i <= toPosition; i++) {
      servoLid.write(i);
      delay(delayMS);
    }
  }
  else { // moving to lower position
    for (int i = current; i >= toPosition; i--) {
      servoLid.write(i);
      delay(delayMS);
    }
  }
}
