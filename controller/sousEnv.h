/*
Library sousEnv header file
Author: Ricardo Dominguez Olmedo
Last updated: 09/2017
*/

#ifndef sousEnv_h
#define sousEnv_h

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class sousEnv
{
  public:
    sousEnv();

    //Functions to change cooking parameters
    void changeTempTarget(int new_target);
    void changeCookingTime(int new_time);

    //Temperature control functions
    void setUpRF();
    void heatingOFF();
    void heatingON();
    int switchHeating(int tempState);
    int isHeatingON();

    //Time control functions
    void updateCookStartTime(float temp);
    int doneCooking(int mins_passed);
    int getMinsCooking();

    //Functions to read temperature from sensors
    float readTemp();

    //Safety functions
    int getSafetyFlag(float temp);
    void safetyTrigger();

    //Functions to access variables
    int getTempTarget();
    int getCookingTime();
    int getMinTemp();
    int getMaxTemp();
    int getMinTime();
    int getMaxTime();

  private:
    //Secure minimum and maximum temperatures
    int min_temp;
    int max_temp;

    //Minimum and maximum operating time
    int min_time;
    int max_time;

    //Temperature and time of cooking
    int temp_target;
    int cooking_time;
    unsigned long cookStartTime;

    //RF pins to switch on/off power to sous vide
    const int PWR_PIN_OTHERS   = 12; //RF POWER D0, D1, D2 ENCODERS
    const int PWR_PIN4         = 13; //RF POWER D3, ENCODER
    const int PWR_MOD          = 11; //ENABLE OR DISABLE MODULATOR

    //Variables for temperature control
    const int ON  = 1;
    const int OFF = 0;
    int isON = 0; //TRUE if heating is ON, false otherwise
};

#endif
