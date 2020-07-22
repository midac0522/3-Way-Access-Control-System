/*
  // These are the initials and the project makers
  AUP = Adventist University of the Philippines
  ED = Engineering Department
  C = Caparaz, Michael David Q.
  A = Akinpelu, Israel O.
  G = Gonzaga, Noennyl Jeif M.
  G = Gabut, Rhoda I.
  E = Espinosa, Dia luna A.
  AC = Access Control

  [] = An array
*/

// Includes (Libraries used)
#include <Wire.h>                // We Include Wire.h to control I2C
#include <EEPROM.h>              // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>                 // Since RC522 Module uses SPI protocol
#include <MFRC522.h>             // Library for Mifare RC522 Devices
#include <Keypad.h>              // Keypad Library (4x3 and 4x4)
#include <LiquidCrystal_I2C.h>   // LCD I2c Library

// Variables and Constants
// Reader Pins (MEGA)
/*
  SDA = 53
  SCK = 52
  MOSI = 51
  MISO = 50
  IRQ = OPEN
  RST = 2
  GND = GND
  3.3V = 3.3V
*/

#define SS_PIN 53                // SDA PIN
#define RST_PIN 2                // RST PIN

// LED Output Pins (B G R)
int ledPin[] = {7, 6, 5};
int doorLED = 13;

// Relay Pin
int relayPin = 11;

// Button and key pins
int btnKey = 3;
int btnBuzz = 18;
int btnOpen = 19;
int btnKeyState = 0;
int btnOpenState = 0;
int btnBuzzState = 0;

// Buzzer
int Buzz = 12;                   // Passive Buzzer

// Sensors
int ProxSensor = A0;

// Defining how many rows and columns our keypad have
const byte numRows = 4;
const byte numCols = 4;

// Keypad pin map
char keymap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char keypressed;                       // Where the keys are stored it changes very often
char code[] = {'0', '5', '2', '2'};    // The default code (We can customized this)
char code_buff1[sizeof(code)];         // Where the new key is stored in 1st_array
char code_buff2[sizeof(code)];         // Where the new key is stored again but in 2nd_array then later on be compared to the values in 1st array

short a = 0, i = 0, s = 0, j = 0;      // Variables used later

// Initializing pins for keypad
byte rowPins[numRows] = {45, 43, 41, 39};
byte colPins[numCols] = {44, 42, 40, 38};

// Others (For configs and misc)
byte readCard[4];
char* myTags[100] = {};
int tagsCount = 0;
String tagID = "";
boolean successRead = false;
boolean correctTag = false;
int proximitySensor;
boolean doorOpened = false;

// Create instances (RFID, LCD I2C, KYPD(4x4))
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);


// Events
// Door opens
void doorOpen() {
  ledSuccess();
  DoorOPNmsgs();
  delay(500);
  digitalWrite(relayPin, LOW);
  delay(5000);
  printNormalModeMessage();
}

// Normal Mode RFID
void printNormalModeMessage() {
  ledStandby();
  delay(1500);
  lcd.clear();
  Serial.println("-Access Control AUPED-CAGGE v1-");
  lcd.print("-AUPED-CAGGE AC-");
  lcd.setCursor(0, 1);
  lcd.print("-Logics Project-");
  delay(2000);
  lcd.clear();
  Serial.println("Modes RFID||KYPD");
  lcd.print("Modes RFID||KYPD");
  lcd.setCursor(0, 1);
  Serial.println("Press :B  ||:C  ");
  lcd.print("Press :B  ||:C  ");
}

// Scroll Up (B)
void scrollUP() {
  delay(2000);
  lcd.clear();
  Serial.println("Direct ScanRFID-");
  lcd.print("Direct ScanRFID-");
  lcd.setCursor(0, 1);
  Serial.println("(D) Alt Scan   A");
  lcd.print("(D) Alt Scan   A");
}

// Scroll Down (C)
void scrollDN() {
  delay(2000);
  lcd.clear();
  Serial.println("(*) Type Code  A");
  lcd.print("(*) Type Code  A");
  lcd.setCursor(0, 1);
  Serial.println("(#) Change Code-");
  lcd.print("(#) Change Code-");
}

// Door Open Msgs
void DoorOPNmsgs() {
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println(" ");
  Serial.println(" Access Granted");
  lcd.print(" Access Granted");
  lcd.setCursor(0, 1);
  Serial.println("  Door Opened!");
  lcd.print("  Door Opened!");
}

// LED LIGHT CONFIGs
// Access Granted (Green LED)
void ledAccess () {
  byte nums[] = {0, 2, 0, 2, 0, 2};
  for (byte i = 0; i < 6; i++)
  {
    displayBinary(nums[i]);
    delay(250);
  }
}

// Accesss Denied (Red LED)
void ledDenied () {
  byte nums[] = {0, 4, 0, 4, 0, 4, 0, 4, 0, 4};
  for (byte i = 0; i < 10; i++)
  {
    displayBinary(nums[i]);
    delay(500);
  }
}

// Normalmode Standby (Blue LED)
void ledStandby () {
  byte nums[] = {0, 1};
  for (byte i = 0; i < 2; i++)
  {
    displayBinary(nums[i]);
    delay(100);
  }
}

// ProgramMode State (Light Blue LED)
void ledProgrammer () {
  byte nums[] = {0, 3};
  for (byte i = 0; i < 2; i++)
  {
    displayBinary(nums[i]);
    delay(100);
  }
}

// Master Tag Recorded OR Change Code Success (Purple LED)
void ledSuccess () {
  byte nums[] = {0, 5, 0, 5, 0, 5};
  for (byte i = 0; i < 6; i++)
  {
    displayBinary(nums[i]);
    delay(250);
  }
}

// Programmed Failed (Blinking MIX RGB and Red LED)
void ledFailed () {
  byte nums[] = {4, 7, 4, 7, 4, 7};
  for (byte i = 0; i < 6; i++)
  {
    displayBinary(nums[i]);
    delay(250);
  }
}

// Byte to LEDpin (B,G,R) show
void displayBinary(byte numToShow) {
  for (int i = 0; i < 3; i++)
  {
    if (bitRead(numToShow, i) == 1)
    {
      digitalWrite(ledPin[i], HIGH);
    }
    else
    {
      digitalWrite(ledPin[i], LOW);
    }
  }
}


// Functions
// Showing reader details for debugging purposes
void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    ledFailed ();
    while (true); // do not go further
  }
}

// Getting RFID
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) {            // If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {              // Since a PICC placed get Serial and continue
    return 0;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) {                   // The MIFARE PICCs that we use have 4 byte UID
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();                                // Stop reading
  return 1;
}

// Scan RFID
void scanRFID() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {            // If a new PICC placed to RFID reader continue
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {              // Since a PICC placed get Serial and continue
    return;
  }
  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for ( uint8_t i = 0; i < 4; i++) {                   // The MIFARE PICCs that we use have 4 byte UID
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();                                // Stop reading

  correctTag = false;
  // Checks whether the scanned tag is the master tag
  if (tagID == myTags[0]) {
    lcd.clear();
    Serial.println("Program mode:");
    lcd.print("Program mode:");
    lcd.setCursor(0, 1);
    Serial.println("Add/Remove Tag");
    lcd.print("Add/Remove Tag");
    while (!successRead) {
      successRead = getID();
      if ( successRead == true) {
        for (int i = 0; i < 100; i++) {
          if (tagID == myTags[i]) {
            myTags[i] = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            Serial.println("  Tag Removed!");
            lcd.print("  Tag Removed!");
            printNormalModeMessage();
            return;
          }
        }
        myTags[tagsCount] = strdup(tagID.c_str());
        lcd.clear();
        lcd.setCursor(0, 0);
        Serial.println("   Tag Added!");
        lcd.print("   Tag Added!");
        printNormalModeMessage();
        tagsCount++;
        return;
      }
    }
  }
  successRead = false;
  // Checks whether the scanned tag is authorized
  for (int i = 0; i < 100; i++) {
    if (tagID == myTags[i]) {
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.println(" Access Granted!");
      lcd.print(" Access Granted!");
      printNormalModeMessage();
      correctTag = true;
    }
  }
  if (correctTag == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println(" Access Denied!");
    lcd.print(" Access Denied!");
    printNormalModeMessage();
  }
}

// Enter the code
void typeCode() {
  keypressed = myKeypad.getKey();               // Constantly waiting for a key to be pressed
  if (keypressed == '*') {                      // Press * to type the password required to open the lock
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println("Enter code:");
    lcd.print("Enter code:");                   // Default message to show
    GetCode();                                  // Getting code function
    if (a == sizeof(code)) {                    // The GetCode function assign a value to a (it's correct when it has the size of the code array)
      ledAccess();
      doorOpen();                               // Open lock function if code is correct
    }
    else {
      ledDenied();
      lcd.clear();
      Serial.println(" Access Denied! ");
      lcd.print(" Access Denied! ");
      lcd.setCursor(0, 1);
      Serial.println("   Wrong code   ");
      lcd.print("   Wrong code   ");           // Message to print when the code is wrong
    }
    delay(2000);
    printNormalModeMessage();                  // Return to standby mode it's the message do display when waiting
  }

  if (keypressed == '#') {                     // To change the code it calls the changecode function
    ledProgrammer();
    ChangeCode();
    printNormalModeMessage();                  // When done it returns to standby mode
  }


  if (keypressed == 'D') {                     // Alternative RFID Scan (Hold D to Scan)
    Serial.println("Scan Master Card");
    Serial.println("To enter programming mode.");
    scanRFID();
  }

  if (keypressed == 'B') {                     // Scroll Up message
    scrollUP();
  }
  if (keypressed == 'C') {                     // Scroll Down message
    scrollDN();
  }

  if (keypressed == 'A') {                     // Goes back to standby normal message
    printNormalModeMessage();
  }

}

// Inside Button and Key Access
void btnUserInput() {
  if ((digitalRead(btnKey) == HIGH) || (digitalRead(btnOpen) == HIGH)) {
    ledAccess();
    doorOpen();
  }

  if (digitalRead(btnBuzz) == HIGH) {
    digitalWrite(Buzz, HIGH);
  }

  if (digitalRead(btnBuzz) == LOW) {
    digitalWrite(Buzz, LOW);
  }
}


// Initialization
void load() {
  // Prints the initial message
  lcd.clear();
  Serial.println("-No Master Tag!-");
  lcd.print("-No Master Tag!-");
  lcd.setCursor(0, 1);
  Serial.println("    SCAN NOW");
  lcd.print("    SCAN NOW");
  // Waits until a master card is scanned
  ledProgrammer();
  while (!successRead) {
    successRead = getID();
    if ( successRead == true) {
      myTags[tagsCount] = strdup(tagID.c_str());                // Sets the master tag into position 0 in the array
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.println("Master Tag Set!");
      lcd.print("Master Tag Set!");
      tagsCount++;
      ledSuccess();
    }
  }
  successRead = false;
}

// Getting code sequence
void GetCode() {
  i = 0;                                                        // All variables set to 0
  a = 0;
  j = 0;

  while (keypressed != 'A') {                                   // The user press A to confirm the code otherwise the device will keep record user's typeings.
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {           // If the char typed isn't A and neither "nothing"
      lcd.setCursor(j, 1);                                      // This to write "*" on the LCD whenever a key is pressed it's position is controlled by j
      Serial.print("*");
      lcd.print("*");
      j++;
      if (keypressed == code[i] && i < sizeof(code)) {          // If the character typed is correct a and i increments to verify the next caracter
        a++;
        i++;
      }
      else
        a--;                                                    // If the character typed is wrong a decrements and cannot equal the size of code[]
    }
  }
  keypressed = NO_KEY;

}

// Change code sequence
void ChangeCode() {
  lcd.clear();
  Serial.println("Changing code:");
  lcd.print("Changing code:");
  delay(1000);
  lcd.clear();
  Serial.println("Enter old code:");
  lcd.print("Enter old code:");
  GetCode();                                                    // Verify the old code first, it's required for password change

  if (a == sizeof(code)) {                                      // And verifying again the a[] value
    lcd.clear();
    Serial.println("Changing code:");
    lcd.print("Changing code:");
    GetNewCode1();                                              // Get the new code
    GetNewCode2();                                              // Get the new code again to confirm it
    s = 0;
    for (i = 0 ; i < sizeof(code) ; i++) {                      // Compare codes in array 1 and array 2 from two previous functions
      if (code_buff1[i] == code_buff2[i])
        s++;                                                    // Again this how we verifiy, increment s whenever codes are matching
    }
    if (s == sizeof(code)) {                                    // Correct is always the size of the array

      for (i = 0 ; i < sizeof(code) ; i++) {
        code[i] = code_buff2[i];                                // The code array now receives the new code
        EEPROM.put(i, code[i]);                                 // And stores it in the EEPROM

      }
      lcd.clear();
      ledSuccess();
      Serial.println("Code Changed:");
      lcd.print("Code Changed:");
      delay(2000);
    }
    else {                                                      // In case the new codes aren't matching
      lcd.clear();
      ledFailed();
      Serial.println("Codes are not");
      lcd.print("Codes are not");
      lcd.setCursor(0, 1);
      Serial.println("matching !!");
      lcd.print("matching !!");
      delay(2000);
    }

  }

  else {                                                        // In case the old code is wrong
    lcd.clear();
    ledFailed();
    Serial.println("  Wrong code!!  ");
    lcd.print("  Wrong code!!  ");
    lcd.setCursor(0, 1);
    Serial.println("Please try again");
    lcd.print("Please try again");
    delay(2000);
  }
}

void GetNewCode1() {
  i = 0;
  j = 0;
  lcd.clear();
  Serial.println("Enter new code");
  lcd.print("Enter new code");                                 // Tell the user to enter a new code and press A
  lcd.setCursor(0, 1);
  Serial.println("and press A");
  lcd.print("and press A");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 1);
  Serial.println("and press A");
  lcd.print("and press A");                                    // Press A keep showing while the top row print ***

  while (keypressed != 'A') {                                  // A to confirm and quits the loop
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {
      lcd.setCursor(j, 0);
      Serial.print("*");
      lcd.print("*");                                          // On the new code you can show * or just change it to keypressed to show the keys
      code_buff1[i] = keypressed;                              // Store characters in the array
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

/*
  This is exactly like the GetNewCode1 function
  but this time the code is stored in another array
*/
void GetNewCode2() {
  i = 0;
  j = 0;

  lcd.clear();
  Serial.println("Confirm Code");
  lcd.print("Confirm code");
  lcd.setCursor(0, 1);
  Serial.println("and press A");
  lcd.print("and press A");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 1);
  Serial.println("and press A");
  lcd.print("and press A");

  while (keypressed != 'A') {
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {
      lcd.setCursor(j, 0);
      Serial.print("*");
      lcd.print("*");
      code_buff2[i] = keypressed;
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}


// Main Handler
void setup() {
  //Protocol Configuration
  Serial.begin(9600);                                   // Initialize serial communications with PC
  SPI.begin();                                          // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();                                   // Initialize MFRC522 Hardware
  lcd.backlight();
  lcd.init();
  Serial.println(F("Access Control AUPED-CAGGE v1"));   // For debugging purposes
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("-AUPED-CAGGE v1-");
  ShowReaderDetails();                                  // Show details of PCD - MFRC522 Card Reader details
  delay(3000);
  // Set Ledpins as output
  for (int i = 0; i < 3; i++)
  {
    pinMode(ledPin[i], OUTPUT);
  }

  // Outputs
  pinMode(doorLED, OUTPUT);
  pinMode(Buzz, OUTPUT);

  // Button and Key as inputs
  pinMode(btnKey, INPUT);
  pinMode(btnBuzz, INPUT);
  pinMode(btnOpen, INPUT);

  // Relay Configuration
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  // Initialization
  load();
  printNormalModeMessage();
}

void loop() {
  int proximitySensor = analogRead(ProxSensor);
  // If door is closed...
  if (proximitySensor < 200) {
    digitalWrite(doorLED, LOW);
    btnUserInput();                                 // Buttons' Functions (Key and Inside button to open the lock || A button for buzz)
    typeCode();                                     // Keypad's Functions (* = Enter Code, A = Enter/Back, B = Scroll UP, C = Scroll DN, # = Change Code, D = Alternative Scan)
    scanRFID();                                     // RFID Direct Scan
  }
  // If door is open...
  else {
    digitalWrite(doorLED, HIGH);
    DoorOPNmsgs();                                   // Door open message
    while (!doorOpened) {
      proximitySensor = analogRead(A0);
      if (proximitySensor > 200) {
        doorOpened = true;
      }
    }
    doorOpened = false;
    delay(5000);
    digitalWrite(relayPin, HIGH);                   // Closes the door
    printNormalModeMessage();
  }
}
