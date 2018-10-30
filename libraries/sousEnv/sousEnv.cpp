/*
Library sousLCD source file
Author: Ricardo Dominguez Olmedo
Last updated: 09/2017
*/

#include "sousEnv.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Set up temperature sensor
const int ONE_WIRE_BUS = 2;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp_sensor(&oneWire);

sousEnv::sousEnv(){
  //Initialize critical safety variables
  min_temp = 20;  //°C
  max_temp = 85;  //°C
  min_time = 30;  //min
  max_time = 600; //min

  //Initialize cooking variables to defaults
  temp_target = 60; //°C
  cooking_time = 60; //min
  cookStartTime = 0; //ms

  //Initialize temperature sensor
  temp_sensor.begin();

  //Initialize RF for switch control
  setUpRF();

  //heatingOFF(); //For safety, start with switch OFF
}

//------------------------------------------------------------------------------
//Functions to change cooking parameters
//------------------------------------------------------------------------------

//Change temperature
void sousEnv::changeTempTarget(int new_target){
  if(new_target < min_temp){ //Check over minimum bound
    temp_target = min_temp;
  } else if (new_target > max_temp){ //Check under maximum bound
    temp_target = max_temp;
  } else { //If within bounds
    temp_target = new_target;
  }
}

//Change cooking time
void sousEnv::changeCookingTime(int new_time){
  if(new_time < min_time){ //Check over minimum bound
    cooking_time = min_time;
  } else if (new_time > max_time){
    cooking_time = max_time; //Check under maximum bound
  } else { //If within bounds
    cooking_time = new_time;
  }
}

//------------------------------------------------------------------------------
//Temperature control functions
//------------------------------------------------------------------------------

//Set UP the RF switch controller
void sousEnv::setUpRF(){
  //Set all RF pins as outputs
  pinMode(PWR_PIN_OTHERS, OUTPUT);
  pinMode(PWR_PIN4, OUTPUT);
  pinMode(PWR_MOD, OUTPUT);

  digitalWrite(PWR_MOD, LOW); //Disable the modulator
  //Set encoder to 0000
  digitalWrite(PWR_PIN_OTHERS, LOW);
  digitalWrite(PWR_PIN4, LOW);
}

//Turn switch off
void sousEnv::heatingOFF(){
  //delayMicroseconds is used to avoid colluding with the LCD
  digitalWrite(PWR_PIN4, LOW);
  digitalWrite(PWR_PIN_OTHERS, HIGH);
  delay(100);
  //delayMicroseconds(100000);
  digitalWrite(PWR_MOD, HIGH); //Modulator ON
  //delayMicroseconds(200000);
  delay(200);
  digitalWrite(PWR_MOD, LOW); //Modulator OFF
  isON = 0;
}

//Turn switch on
void sousEnv::heatingON(){
  digitalWrite(PWR_PIN4, HIGH);
  digitalWrite(PWR_PIN_OTHERS, HIGH);
  delay(100);
  //delayMicroseconds(100000);
  digitalWrite(PWR_MOD, HIGH); //Modulator on
  delay(200);
  //delayMicroseconds(200000);
  digitalWrite(PWR_MOD, LOW); //Modulator off
  isON = 1;
}

//Turn switch on/off as stated in tempState if the switch is not currently in
//that state
int sousEnv::switchHeating(int tempState){
  if (tempState != isHeatingON()){ //If states differ
    if(tempState == ON) {
      heatingON();
    } else {
      heatingOFF();
    }
  }
}

//Return TRUE if the switch is on, FALSE otherwise
int sousEnv::isHeatingON(){
  return isON;
}

//------------------------------------------------------------------------------
//Time control functions
//------------------------------------------------------------------------------

void sousEnv::updateCookStartTime(float temp){
  //Only update it if cookStartTime has not been initialized (cookStartTime == 0)
  //and the temperature is close to the target temperature (within 0.5 degrees)
  if ((cookStartTime == 0)&&((temp_target - temp) < 1.0)){
    cookStartTime = millis();
  }
}

//Return TRUE if the time that the sousvide has been on is higher than the
//cooking time
int sousEnv::doneCooking(int mins_passed){
  if(mins_passed > cooking_time){
    return 1;
  }
  return 0;
}

//Return the time in minutes that the sousvide has been cooking
int sousEnv::getMinsCooking(){
  unsigned long now = millis();
  //If cookStartTime has not been initialized, 0 minutes cooking
  if(cookStartTime == 0){
    return 0;
  } else {
    Serial.print((now - cookStartTime) / 60000);
    return (now - cookStartTime) / 60000;
  }
}

//------------------------------------------------------------------------------
//Functions to read temperature from sensor
//------------------------------------------------------------------------------
float sousEnv::readTemp(){
  temp_sensor.requestTemperatures();
  return temp_sensor.getTempCByIndex(0);
}

//------------------------------------------------------------------------------
//Safety functions
//------------------------------------------------------------------------------

//Return a value other than 0 if some flag must be raised
int sousEnv::getSafetyFlag(float temp){
  if (temp > max_temp){ //Check if temperature is over the max temp
    return 1;
  }

  if (temp < min_temp) { //Check if temperature is under min temp
    return 2;
  }

  return 0;
}

//Function to be called in case of malfunction
void sousEnv::safetyTrigger(){
  heatingOFF(); //Power off heat
  while(1); //Keep everything off
}

//------------------------------------------------------------------------------
//Functions to access variables
//------------------------------------------------------------------------------

int sousEnv::getTempTarget(){
  return temp_target;
}

int sousEnv::getCookingTime(){
  return cooking_time;
}

int sousEnv::getMinTemp(){
  return min_temp;
}

int sousEnv::getMaxTemp(){
  return max_temp;
}

int sousEnv::getMinTime(){
  return min_time;
}

int sousEnv::getMaxTime(){
  return max_time;
}
