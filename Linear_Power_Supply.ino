#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#define clk1 2 
#define clk2 3
#define dt1  4
#define dt2  5
const int CS_PIN = 10;
LiquidCrystal_I2C lcd(0x27,20,4); 
unsigned long t = 0;
unsigned long on=0;
unsigned long re1Press,re2Press,t1,t2;
volatile int re1=1200,re2=100;
int iRes=1,vRes=5,v;

volatile bool currentClk1,currentClk2,lastClk1,lastClk2;
bool s1,s2,currentS1=HIGH,currentS2=HIGH,lastS1,lastS2;

void setup() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

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
  
  lcd.init();
  lcd.backlight();
  lcd.clear();

  attachInterrupt(digitalPinToInterrupt(clk1), encoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(clk2), encoder2, RISING);

  buzzer(250);
}

void buzzer(int time){
  analogWrite(6, 100);
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
  digitalWrite(CS_PIN, LOW);
  SPI.transfer16(cmd);
  digitalWrite(CS_PIN, HIGH);
  
}

void encoder1(){
  currentClk1 = digitalRead(clk1);

  if(currentClk1!=lastClk1 && currentClk1==1){
    if(digitalRead(dt1) != currentClk1 ){
      re1+=vRes;
    }
    else{
      re1-=vRes;
    }
    re1 = constrain(re1, 0, 4095);
  }
  lastClk1 = currentClk1;
}

void encoder2(){
  currentClk2 = digitalRead(clk2);

  if(currentClk2!=lastClk2 && currentClk2==1){
    if(digitalRead(dt2) != currentClk2){
      re2+=iRes;
    }
    else{
      re2-=iRes;
    }
    re2 = constrain(re2, 0, 4095);
  }
  lastClk2 = currentClk2;
}


void loop() {

  v = analogRead(A2);
  int out = analogRead(A0);

  if(out>1000){
    digitalWrite(9,LOW);
    on  = millis();
  }
  if(millis()-on >30000){
    on = 0;
    digitalWrite(9,HIGH);
  }

  currentS1 = digitalRead(7);

  if(currentS1==0 && lastS1==1){
    t1 = millis();
  }
  if(currentS1==1 && lastS1==0){
    if(millis()-t1>1000){
      vRes = 15;
    }
    else{
      vRes = 5;
    }
    buzzer(250);
  }
  lastS1 = currentS1;

  currentS2 = digitalRead(8);
  if(currentS2==0 && lastS2==1){
    t2 = millis();
  }
  if(currentS2==1 && lastS2==0){
    if(millis()-t2>1000){
      iRes = 15;
    }
    else{
      iRes = 1;
    }
    buzzer(250);
  }
  lastS2 = currentS2;
 
  
  double vset = ((5.0 * v * 21.0)/ 1023.0);
  vset = vset>1.0? vset+1.0:vset;
  double iset = (25.0 * re2)/4095.0;
  
 
  writeDAC(re1, false); 
  writeDAC(re2, true);   

  if((millis() - t) >= 1000){
    t = millis();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("V Set");
    lcd.setCursor(5,0);
    lcd.print(":");
    lcd.print(vset,3);
    lcd.setCursor(12,0);
    lcd.print("V");

    lcd.setCursor(0,1);
    lcd.print("I-Set");
    lcd.setCursor(5,1);
    lcd.print(":");
    lcd.setCursor(6,1);
    lcd.print(iset,3);
    lcd.setCursor(12,1);
    lcd.print("A");

 
  }
  
  

  delay(10);
}
