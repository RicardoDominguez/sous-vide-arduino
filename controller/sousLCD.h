/*
Library sousLCD hader file
Author: Ricardo Dominguez Olmedo
Last updated: 09/2017
*/
#ifndef sousLCD_h
#define sousLCD_h

#include <Arduino.h>
#include <LiquidCrystal.h> //Library for LiquidCrystal screen
#include "sousEnv.h" //SousVide environment

class sousLCD
{
  public:
    sousLCD(sousEnv *in_sous);

    //Key input functions
    int read_LCD_buttons();
    int selectPressed();
    void selectParameters();

    //Safety functions
    void setUpError(int error_code);

    //Sous on/off display functions
    void sousOff();
    void sousOn();

    //LCD set up functions
    void setUpSelectParam();
    void setUpHeatingMode();
    void setUpCookingMode();

    //LCD update functions
    void updateHeatingTemp(float temp_current);
    void updateCookingTime(int runTime);
    void updateCookingTemp(float temp_current);
    void updateHeatingDone();
    void updateCookingDone();

  private:
    sousEnv* sous;

    //Displayed values - {temparature, time}
    int displayed_values[2];
    int max_vals[2];
    int min_vals[2];
};

#endif
