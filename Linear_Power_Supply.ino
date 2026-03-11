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
int re1=1200,re2=100,v;
volatile bool currentClk1,currentClk2,lastClk1,lastClk2;

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
  lcd.init();
  lcd.backlight();
  lcd.clear();
  attachInterrupt(digitalPinToInterrupt(clk1), encoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(clk2), encoder2, RISING);
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
      re1+=5;
    }
    else{
      re1-=5;
    }
    re1 = constrain(re1, 0, 4095);
  }
  lastClk1 = currentClk1;
}

void encoder2(){
  currentClk2 = digitalRead(clk2);

  if(currentClk2!=lastClk2 && currentClk2==1){
    if(digitalRead(dt2) != currentClk2){
      re2+=1;
    }
    else{
      re2-=1;
    }
    re2 = constrain(re2, 0, 4095);
  }
  lastClk2 = currentClk2;
}

void loop() {

  v = analogRead(A2);
  
  
 
  
  double vset = ((5.0 * v * 21.0)/ 1023.0);
  vset = vset>1.0? vset+1.0:vset;
  double iset = (25.0 * re2)/4095.0;
  
  if(vset<2.0 && )
 
  writeDAC(re1, false); 
  writeDAC(re2, true);   

  if((millis() - t) >= 1000){
    t = millis();
    lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V Set");
  lcd.setCursor(6,0);
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