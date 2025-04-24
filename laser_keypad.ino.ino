#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "Keypad.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define RST_PIN 9
#define SS_PIN 10

#define BUZZER 6
#define LDR A0
#define PIR 7
#define LASER 4

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
};

// this may be different on different keypads - please check!
byte rowPins[ROWS] = {2, 3, 5, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 10, 11, 12}; //connect to the column pinouts of the keypad
// attach keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char password[4];
char initial_password[4],new_password[4];
char key_pressed=0;
int cnt=0, Check=0;

// Timer for non-blocking buzzer
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
const unsigned long buzzerDuration = 2000; // 2 seconds

void resetLCD();
void change();

void setup() {
  Serial.begin(9600);

  pinMode(LDR, INPUT);
  pinMode(PIR, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LASER, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(LASER, HIGH);
 
  lcd.init();// initialize the lcd
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" Password  Lock ");
  lcd.setCursor(0,1);
  lcd.print("Security  System");
  delay(2000); // Waiting for a while
  lcd.clear(); 
  lcd.print("  Press # for  ");
  lcd.setCursor(0,1);
  lcd.print("Change  Password");
  delay(2000); // Waiting for a while
  lcd.clear(); 
  lcd.print("Enter Password: "); //What's written on the lcd you can change
  if(EEPROM.read(100)==0){}
  else{
  for(int i=0;i<4;i++){EEPROM.write(i, i+49);}  
    EEPROM.write(100, 0);
  }
  for(int i=0;i<4;i++){ //read code from EEPROM
      initial_password[i]=EEPROM.read(i);
  }
}

void loop() {
  digitalWrite(LASER, HIGH);
  key_pressed = keypad.getKey();

  if (key_pressed) {
    digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);

    if (key_pressed == '#') {
      change();  // Trigger password change
      cnt = 0;
      memset(password, 0, sizeof(password));
      lcd.clear();
      lcd.print("Enter Password:");
      return;
    }

    if (key_pressed == '*') {
      if (cnt > 0) {
        cnt--;
        password[cnt] = 0;
        lcd.setCursor(5 + cnt, 1);
        lcd.print(" ");
        lcd.setCursor(5 + cnt, 1);
      }
      return;
    }

    if (key_pressed != NO_KEY) {
      Serial.print("Key Pressed: ");
      Serial.println(key_pressed);
    }

    if (cnt < 4 && key_pressed >= '0' && key_pressed <= '9') {
      password[cnt] = key_pressed;
      lcd.setCursor(cnt, 1);
      lcd.print("*");
      cnt++;
    }
  }

  // Only proceed to verify after 4 digits entered
  if (cnt == 4) {
    delay(200); // debounce

    for (int j = 0; j < 4; j++) {
      initial_password[j] = EEPROM.read(j);
    }

    if (strncmp(password, initial_password, 4) == 0) {
      Check = 0;
      lcd.clear();
      lcd.print("Access Granted");
      lcd.setCursor(0, 1);
      lcd.print("Laser OFF");

      digitalWrite(LASER, LOW);
      delay(5000);

      while(digitalRead(PIR) == HIGH) {
        digitalWrite(LASER, LOW);
        Serial.println("Motion detected - User inside");
      } 

      digitalWrite(LASER, HIGH);
      Serial.println("No motion - Laser active");

    } 
    else {
      Check++;
      lcd.clear();
      lcd.print("Wrong Password");
      lcd.setCursor(0, 1);
      lcd.print("Try Again");

      if (Check >= 3) {
        lcd.clear();
        lcd.print("Wait 10 Sec...");
        for (int i = 10; i > 0; i--) {
          lcd.setCursor(0, 1);
          lcd.print("Time: ");
          lcd.print(i);
          digitalWrite(BUZZER, HIGH); delay(500);
          digitalWrite(BUZZER, LOW); delay(500);
        }
        Check = 0;
      } else {
        digitalWrite(BUZZER, HIGH); delay(2000);
        digitalWrite(BUZZER, LOW);
      }
    }

    // Reset after check
    cnt = 0;
    memset(password, 0, sizeof(password));
    delay(1000);
    lcd.clear();
    lcd.print("Enter Password:");
  }

  // Laser interruption via LDR
  int ldrValue = analogRead(LDR);
  if (ldrValue < 800 && !buzzerActive) {
    buzzerActive = true;
    buzzerStartTime = millis();
    digitalWrite(BUZZER, HIGH);
    Serial.println("Laser interrupted - Possible intrusion!");
    Serial.println(ldrValue);
  }

  // Turn off buzzer after duration
  if (buzzerActive && millis() - buzzerStartTime >= buzzerDuration) {
    digitalWrite(BUZZER, LOW);
    buzzerActive = false;
  }
}

// Utility function to reset LCD prompt
void resetLCD() {
  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Enter password");
}

void change(){
  int j=0;
  lcd.clear();
  lcd.print("Current Password");
  lcd.setCursor(5,1);
  digitalWrite(BUZZER, HIGH); delay(100);
  digitalWrite(BUZZER, LOW);
  while(j<4){
   char key=keypad.getKey();
    if(key){
      digitalWrite(BUZZER, HIGH); delay(200);
      digitalWrite(BUZZER, LOW);
      new_password[j++]=key;
      lcd.print(key);   
    }
    key=0;
  }
  delay(500);
  if((strncmp(new_password, initial_password, 4))){
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0,1);
    lcd.print("Try Again");
    delay(1000);
  }
  else{
    j=0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(5,1);
    while(j<4){
      char key=keypad.getKey();
      if(key){
        digitalWrite(BUZZER, HIGH); delay(200);
        digitalWrite(BUZZER, LOW);
        initial_password[j]=key;
        lcd.print(key);
        EEPROM.write(j,key);
        j++;
      }
    }
    lcd.clear();
    lcd.print("Pass Changed");
    delay(1000);
  }
  
  lcd.clear();
  lcd.print("Enter Password:");
  key_pressed=0;
}
