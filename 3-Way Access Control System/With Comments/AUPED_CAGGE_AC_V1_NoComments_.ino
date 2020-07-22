#include <Wire.h>                
#include <EEPROM.h>              
#include <SPI.h>                 
#include <MFRC522.h>             
#include <Keypad.h>              
#include <LiquidCrystal_I2C.h>   

#define SS_PIN 53                
#define RST_PIN 2                

int ledPin[] = {7, 6, 5};
int doorLED = 13;

int relayPin = 11;

int btnKey = 3;
int btnBuzz = 18;
int btnOpen = 19;
int btnKeyState = 0;
int btnOpenState = 0;
int btnBuzzState = 0;

int Buzz = 12;                   

int ProxSensor = A0;

const byte numRows = 4;
const byte numCols = 4;

char keymap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char keypressed;                       
char code[] = {'0', '5', '2', '2'};    
char code_buff1[sizeof(code)];         
char code_buff2[sizeof(code)];         

short a = 0, i = 0, s = 0, j = 0;      

byte rowPins[numRows] = {45, 43, 41, 39};
byte colPins[numCols] = {44, 42, 40, 38};

byte readCard[4];
char* myTags[100] = {};
int tagsCount = 0;
String tagID = "";
boolean successRead = false;
boolean correctTag = false;
int proximitySensor;
boolean doorOpened = false;

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);


void doorOpen() {
  ledSuccess();
  DoorOPNmsgs();
  delay(500);
  digitalWrite(relayPin, LOW);
  delay(5000);
  printNormalModeMessage();
}

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

void scrollUP() {
  delay(2000);
  lcd.clear();
  Serial.println("Direct ScanRFID-");
  lcd.print("Direct ScanRFID-");
  lcd.setCursor(0, 1);
  Serial.println("(D) Alt Scan   A");
  lcd.print("(D) Alt Scan   A");
}

void scrollDN() {
  delay(2000);
  lcd.clear();
  Serial.println("(*) Type Code  A");
  lcd.print("(*) Type Code  A");
  lcd.setCursor(0, 1);
  Serial.println("(#) Change Code-");
  lcd.print("(#) Change Code-");
}

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

void ledAccess () {
  byte nums[] = {0, 2, 0, 2, 0, 2};
  for (byte i = 0; i < 6; i++)
  {
    displayBinary(nums[i]);
    delay(250);
  }
}

void ledDenied () {
  byte nums[] = {0, 4, 0, 4, 0, 4, 0, 4, 0, 4};
  for (byte i = 0; i < 10; i++)
  {
    displayBinary(nums[i]);
    delay(500);
  }
}

void ledStandby () {
  byte nums[] = {0, 1};
  for (byte i = 0; i < 2; i++)
  {
    displayBinary(nums[i]);
    delay(100);
  }
}

void ledProgrammer () {
  byte nums[] = {0, 3};
  for (byte i = 0; i < 2; i++)
  {
    displayBinary(nums[i]);
    delay(100);
  }
}

void ledSuccess () {
  byte nums[] = {0, 5, 0, 5, 0, 5};
  for (byte i = 0; i < 6; i++)
  {
    displayBinary(nums[i]);
    delay(250);
  }
}

void ledFailed () {
  byte nums[] = {4, 7, 4, 7, 4, 7};
  for (byte i = 0; i < 6; i++)
  {
    displayBinary(nums[i]);
    delay(250);
  }
}

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


void ShowReaderDetails() {
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
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    ledFailed ();
    while (true); 
  }
}

uint8_t getID() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {            
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {              
    return 0;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) {                   
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); 
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();                                
  return 1;
}

void scanRFID() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {            
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {              
    return;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) {                   
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); 
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();                                

  correctTag = false;
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

void typeCode() {
  keypressed = myKeypad.getKey();               
  if (keypressed == '*') {                      
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println("Enter code:");
    lcd.print("Enter code:");                   
    GetCode();                                  
    if (a == sizeof(code)) {                    
      ledAccess();
      doorOpen();                               
    }
    else {
      ledDenied();
      lcd.clear();
      Serial.println(" Access Denied! ");
      lcd.print(" Access Denied! ");
      lcd.setCursor(0, 1);
      Serial.println("   Wrong code   ");
      lcd.print("   Wrong code   ");           
    }
    delay(2000);
    printNormalModeMessage();                  
  }

  if (keypressed == '#') {                     
    ledProgrammer();
    ChangeCode();
    printNormalModeMessage();                  
  }


  if (keypressed == 'D') {                     
    Serial.println("Scan Master Card");
    Serial.println("To enter programming mode.");
    scanRFID();
  }

  if (keypressed == 'B') {                     
    scrollUP();
  }
  if (keypressed == 'C') {                     
    scrollDN();
  }

  if (keypressed == 'A') {                     
    printNormalModeMessage();
  }

}

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


void load() {
  lcd.clear();
  Serial.println("-No Master Tag!-");
  lcd.print("-No Master Tag!-");
  lcd.setCursor(0, 1);
  Serial.println("    SCAN NOW");
  lcd.print("    SCAN NOW");
  ledProgrammer();
  while (!successRead) {
    successRead = getID();
    if ( successRead == true) {
      myTags[tagsCount] = strdup(tagID.c_str());               
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

void GetCode() {
  i = 0;                                                      
  a = 0;
  j = 0;

  while (keypressed != 'A') {                                  
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {           
      lcd.setCursor(j, 1);                                      
      Serial.print("*");
      lcd.print("*");
      j++;
      if (keypressed == code[i] && i < sizeof(code)) {          
        a++;
        i++;
      }
      else
        a--;                                                    
    }
  }
  keypressed = NO_KEY;

}

void ChangeCode() {
  lcd.clear();
  Serial.println("Changing code:");
  lcd.print("Changing code:");
  delay(1000);
  lcd.clear();
  Serial.println("Enter old code:");
  lcd.print("Enter old code:");
  GetCode();                                                    

  if (a == sizeof(code)) {                                      
    lcd.clear();
    Serial.println("Changing code:");
    lcd.print("Changing code:");
    GetNewCode1();                                              
    GetNewCode2();                                              
    s = 0;
    for (i = 0 ; i < sizeof(code) ; i++) {                     
      if (code_buff1[i] == code_buff2[i])
        s++;                                                    
    }
    if (s == sizeof(code)) {                                    

      for (i = 0 ; i < sizeof(code) ; i++) {
        code[i] = code_buff2[i];                               
        EEPROM.put(i, code[i]);                                

      }
      lcd.clear();
      ledSuccess();
      Serial.println("Code Changed:");
      lcd.print("Code Changed:");
      delay(2000);
    }
    else {                                                      
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

  else {                                                        
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
  lcd.print("Enter new code");                                 
  lcd.setCursor(0, 1);
  Serial.println("and press A");
  lcd.print("and press A");
  delay(2000);
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
      code_buff1[i] = keypressed;                              
      i++;
      j++;
    }
  }
  keypressed = NO_KEY;
}

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


void setup() {
  Serial.begin(9600);                                   
  SPI.begin();                                          
  mfrc522.PCD_Init();                                   
  lcd.backlight();
  lcd.init();
  Serial.println(F("Access Control AUPED-CAGGE v1"));   
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("-AUPED-CAGGE v1-");
  ShowReaderDetails();                                 
  delay(3000);
  for (int i = 0; i < 3; i++)
  {
    pinMode(ledPin[i], OUTPUT);
  }

  pinMode(doorLED, OUTPUT);
  pinMode(Buzz, OUTPUT);

  pinMode(btnKey, INPUT);
  pinMode(btnBuzz, INPUT);
  pinMode(btnOpen, INPUT);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  load();
  printNormalModeMessage();
}

void loop() {
  int proximitySensor = analogRead(ProxSensor);
  if (proximitySensor < 200) {
    digitalWrite(doorLED, LOW);
    btnUserInput();                                 
    typeCode();                                     
    scanRFID();                                     
  }
  else {
    digitalWrite(doorLED, HIGH);
    DoorOPNmsgs();                                   
    while (!doorOpened) {
      proximitySensor = analogRead(A0);
      if (proximitySensor > 200) {
        doorOpened = true;
      }
    }
    doorOpened = false;
    delay(5000);
    digitalWrite(relayPin, HIGH);                   
    printNormalModeMessage();
  }
}
