#include <EEPROM.h>
#include <RCSwitch.h>
#include <Convert.h>
RCSwitch mySwitch = RCSwitch();
Convert convert;

const int LED = 12;
const int LED2 = 13;
const int BUTTON_PIN = 10;
const int BUTTON2_PIN = 11;
const int LONG_PRESS_TIME  = 5000;

int lastState = LOW;  
int lastState2 = HIGH;
int currentState;
int currentState2 = HIGH;
unsigned long pressedTime  = 0;
bool isPressing = false;
bool isLongDetected = false;
int leitura=0;
int valor;
int receive=0;
int transmit=0;

bool blinking = false;
unsigned long blinkInterval = 100; 
unsigned long currentMillis;
unsigned long previousMillis;

void scan(){ 
  if (transmit==1){
    mySwitch.disableTransmit();
    transmit=0;
  }
  if (receive==0){
    mySwitch.enableReceive(0);
    receive=1;
  }
  if (mySwitch.available()){
    writeLongIntoEEPROM(100, mySwitch.getReceivedValue());
    writeLongIntoEEPROM(80, mySwitch.getReceivedDelay());
    EEPROM.write(60, mySwitch.getReceivedProtocol());
    EEPROM.write(50, mySwitch.getReceivedBitlength());
    mySwitch.resetAvailable();
    isLongDetected=false;
    blinking=false;
    digitalWrite(LED, HIGH);
    delay(600);
  }
}

void send(){
  if (receive==1){
    mySwitch.disableReceive();
    receive=0;
  }
  if (transmit==0){
    mySwitch.enableTransmit(8);
    transmit=1;
  }
  long micros = readLongFromEEPROM(80);
  long decimal = readLongFromEEPROM(100);
  long binary = convert.decimalToBinary(decimal).toInt();
  int protocol = EEPROM.read(60);
  int length = EEPROM.read(50);
  mySwitch.setPulseLength(length);
  mySwitch.setProtocol(protocol);
  mySwitch.send(binary, length);
  isLongDetected=LOW;
  //Serial.print("decimal: ");
  //Serial.println(decimal);
  //Serial.print("binary: ");
  //Serial.println(convert.decimalToBinary(decimal));
  //Serial.print("milliseconds: ");
  //Serial.println(milliseconds);
  //Serial.print("protocol: ");
  //Serial.println(EEPROM.read(60));
  digitalWrite(LED2,HIGH);
  delay(500);
  digitalWrite(LED2,LOW);
  delay(500);
  
}

void blinkLed(){
  if (isLongDetected==HIGH) // buttons with pull-up are pressed when LOW
    blinking=true; // start blinking
  else
    blinking=false; // stop blinking
}

void writeLongIntoEEPROM(int address, long number)
{ 
  EEPROM.write(address, (number >> 24) & 0xFF);
  EEPROM.write(address + 1, (number >> 16) & 0xFF);
  EEPROM.write(address + 2, (number >> 8) & 0xFF);
  EEPROM.write(address + 3, number & 0xFF);
}

long readLongFromEEPROM(int address)
{
  return ((long)EEPROM.read(address) << 24) +
         ((long)EEPROM.read(address + 1) << 16) +
         ((long)EEPROM.read(address + 2) << 8) +
         (long)EEPROM.read(address + 3);
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  mySwitch.enableReceive(0);
  receive=1;
  }

void loop() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);
  currentState2 = digitalRead(BUTTON2_PIN);

  if(lastState == HIGH && currentState == LOW) {        // button is pressed
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  } else if(lastState == LOW && currentState == HIGH) { // button is released
    isPressing = false;
  }

  if(isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if( pressDuration > LONG_PRESS_TIME ) {
      Serial.println("A long press is detected");
      isLongDetected = true;
    }
  }




  // save the the last state
  lastState = currentState;
  if (blinking) {
    currentMillis = millis();
    if ((unsigned long)(currentMillis - previousMillis) >= blinkInterval) {
      digitalWrite(LED, !digitalRead(LED)); 
      previousMillis = currentMillis;
    }
  } else {
    digitalWrite(LED, LOW); // desliga led
  }
  delay(50); // de-bouncing

  blinkLed();

  if (isLongDetected==true) {
    scan();
  }
  if (currentState2==LOW){
    send();
  }
}