#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include "ADS1X15.h"

#define clk1 2 
#define clk2 3
#define dt1  4
#define dt2  5
#define voltageGain 105.0
ADS1115 ADS(0x48);
LiquidCrystal_I2C lcd(0x27,20,4);

const int CS_PIN = 10; 
volatile int re1=1200,re2=100,re1C=0,re2C=0;
int iRes=1,vRes=5,v=0;
unsigned long t = 0;
unsigned long on=0;
unsigned long t1=0,t2=0;
volatile unsigned long tc1=0,tc2=0;
float voltage,current,iset,power,c=0;
bool s1=0,s2=0,currentS1=1,currentS2=1,lastS1=0,lastS2=0;

void setup() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();

  pinMode(clk1,INPUT_PULLUP);
  pinMode(clk2,INPUT_PULLUP);
  pinMode(dt1,INPUT_PULLUP);
  pinMode(dt2,INPUT_PULLUP);
  pinMode(7,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(A0,INPUT);
  pinMode(6,OUTPUT);
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  
  ADS.begin();
  ADS.setGain(2);
  ADS.setDataRate(860);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  attachInterrupt(digitalPinToInterrupt(clk1), encoder1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(clk2), encoder2, CHANGE);
  
  lcd.setCursor(0, 0);
  lcd.print("LINEAR PSU");
  buzzer(128,250);
  delay(1000);
  lcd.clear();
}

void buzzer(int freq,int time){
  analogWrite(6, freq);
  delay(time);
  analogWrite(6,0);
  delay(5);
}

void writeDAC(uint16_t val, bool ch) {
  val &= 0x0FFF;  
  uint16_t cmd = 0;
  cmd |= (ch ? 1 : 0) << 15;  
  cmd |= 0 << 14;                  
  cmd |= 1 << 13;                 
  cmd |= 1 << 12;                 
  cmd |= val;  
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));                  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer16(cmd);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  
}

void encoder1(){
  unsigned long ct = micros();
  if(ct-micros()<4000) return;
  tc1 = ct;
  bool clk = digitalRead(clk1);
  bool dt = digitalRead(dt1);
  if(clk==dt){
    re1-=vRes;
  }
  else{
    re1+=vRes;
  }
  re1 = constrain(re1, 0, 4095); 
}

void encoder2(){
  unsigned long ct = micros();
  if(ct-micros()<4000) return;
  tc2 = ct;
  bool clk = digitalRead(clk2);
  bool dt = digitalRead(dt2);
  if(clk==dt){
    re2-=iRes;
  }
  else{
    re2+=iRes;
  }
  re2 = constrain(re2, 0, 4095);
}



void loop() {

  currentS1 = digitalRead(7);
  currentS2 = digitalRead(8);

  if(currentS1==0 && lastS1==1){
    t1 = millis();
  }
  if(currentS1==1 && lastS1==0){
    if(millis()-t1>1000){
      vRes = 15;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("VRes: 15Turns");
    }
    else{
      vRes = 5;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("VRes: 5 Turns");
    }
    buzzer(128,250);
    delay(100);
    lcd.clear();
  }
  lastS1 = currentS1;

  
  if(currentS2==0 && lastS2==1){
    t2 = millis();
  }
  if(currentS2==1 && lastS2==0){
    if(millis()-t2>1000){
      iRes = 16;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("IRes: 100mA");
    }
    else{
      iRes = 1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("IRes: 1mA");
      
    }
    buzzer(128,250);
    delay(100);
    lcd.clear();
  }
  lastS2 = currentS2;

  v=0;c=0;
  for(int i=0;i<50;i++){
    v += analogRead(A2);
    c += ADS.toVoltage(ADS.readADC(0)); 
  }
  v/=50; c/=50;
  
  voltage = (v * voltageGain)/ 1023.0;
  voltage = voltage>1.0? voltage+1.0:voltage;
  iset = (25.0 * re2)/4095.0;
  current = c*10;
  if(voltage<0) voltage=0.0;
  if(iset<0) iset = 0.0;
  if(current<0) current = 0.0;
  power = voltage*current;

  noInterrupts();
  re1C = re1;
  re2C = re2;
  writeDAC(re1C, 0); 
  writeDAC(re2C, 1);
  interrupts();   

  if(current>=iset && iset!=0.0){
    digitalWrite(9,LOW);
    analogWrite(6, 128);
  }
  else{
    digitalWrite(9, HIGH);
    analogWrite(6, 0);
  }

  if((millis() - t) >= 250){
    t = millis();
    lcd.setCursor(0, 0);
    if(voltage>10){
      lcd.print(voltage,3);
    }
    else{
      lcd.print(voltage,4);
    }
    lcd.print("  ");
    lcd.setCursor(5, 0);
    lcd.print("V");

    lcd.setCursor(7, 0);
    if(power>10){
      lcd.print(power,3);
    }
    else{
      lcd.print(power,4);
    }
    lcd.print("  ");
    lcd.setCursor(12, 0);
    lcd.print("W");

    lcd.setCursor(0, 1);
    if(current>10){
      lcd.print(current,3);
    }
    else{
      lcd.print(current,4);
    }
    lcd.print("  ");
    lcd.setCursor(5, 1);
    lcd.print("A");

    lcd.setCursor(7, 1);
    if(iset>10){
      lcd.print(iset,3);
    }
    else{
      lcd.print(iset,4);
    }
    lcd.print("  ");
    lcd.setCursor(12, 1);
    lcd.print("A");

    if(current>=iset){
      lcd.setCursor(14,0);
      lcd.print("  ");
      lcd.setCursor(14,1);
      lcd.print("CC");
    }
    else{
      lcd.setCursor(14,0);
      lcd.print("CV");
      lcd.setCursor(14,1);
      lcd.print("  ");
    }
 
  }


  delay(10);
}
