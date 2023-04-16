#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "HX711.cpp"
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 3;
const int TARE_PIN = 2;

// lcd setup
// A4: SDA, A5: SCL
LiquidCrystal_I2C lcd(0x27,16,2);


//hx711 setup
HX711 scale;

double lastDisplayed;
double reading;

volatile unsigned long lastTareTime=0;

void onTareClick(){
  if(lastTareTime-millis()<1000) return;

  
  scale.tare();
  lastTareTime=millis();
}

template<class T>
void sort(T* array){
  
  int n = 7;
  for(int i=0;i<n;i++){
    T smallest = array[i];
    int smallest_at = i;
    for(int j=i;j<n;j++){
      
      if(array[j]<smallest){
        smallest = array[j];
        smallest_at = j;
      } 
    }
    array[smallest_at] = array[i];
    array[i] = smallest;
  }
}

uint8_t numberLength(double number){
  number = abs(number);
  if(number<10) return 0;
  uint8_t i=0;
  while(number/=10){
    i++;
  }
  return i;
}

void setup() {
  // Serial.begin(9600);
  lcd.init();                      
  lcd.backlight();

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(1000.0);
  scale.set_offset(125000);
  // scale.set_gain(128);
  pinMode(TARE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TARE_PIN), onTareClick, FALLING);

}

void loop() {
  if(scale.is_ready()){
    // reading = scale.get_units(5);
    long int arr[] = {scale.read(),scale.read(),scale.read(),scale.read(),scale.read(),scale.read(),scale.read()};
    sort(arr);
    reading = float(arr[3]-scale.get_offset())/float(scale.get_scale());
    // Serial.print("Result: ");
    // Serial.print(reading);
    // Serial.println("g");


    
    //for detecting decimal place change
    uint8_t t1 = numberLength(lastDisplayed);
    uint8_t t2 = numberLength(reading);
    if(t1-t2 || (lastDisplayed>0 && reading<0) || (lastDisplayed<0 && reading>0)){
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print(String(reading)+"g"); // floor(reading*10)/10.0
    }

    //if a value has been changed change the operating value
    if(lastDisplayed!=reading){
      lcd.setCursor(0,1);
      lcd.print(String(reading)+"g"); // floor(reading*10)/10.0
    }
    lastDisplayed = reading;
  }
  //delay(200);
}