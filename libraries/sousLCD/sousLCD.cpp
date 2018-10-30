/*
Library sousLCD source file
Author: Ricardo Dominguez Olmedo
Last updated: 09/2017
*/

#include "sousLCD.h"
#include <Arduino.h>
#include <LiquidCrystal.h> //Library for LiquidCrystal screen
#include <sousEnv.h> //SousVide environment

//Initialize screen object
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //Pins used by the screen

//lcd buttons definitions
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

sousLCD::sousLCD(sousEnv *in_sous){
  lcd.begin(16, 2); //start screen object, 16x2

  sous = in_sous;   //copy incoming sousvide object address

  //Copy max/min values for display
  max_vals[0] = (*sous).getMaxTemp();
  max_vals[1] = (*sous).getMaxTime();
  min_vals[0] = (*sous).getMinTemp();
  min_vals[1] = (*sous).getMinTime();
}

//------------------------------------------------------------------------------
//Key input functions
//------------------------------------------------------------------------------

//Function to read the buttons state
//Returns the button currently pushed
//Extracted from LiquidCrystal library example
int sousLCD::read_LCD_buttons(){
   int adc_key_in = analogRead(0);  //read analog input
   // the buttons when read are approximately centered at these values: 0, 144, 329, 504, 741
   // we add approx 50 to those values and check to see if we are close
   if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for
        //speed reasons since it will be the most likely result
   if (adc_key_in < 50)   return btnRIGHT;
   if (adc_key_in < 195)  return btnUP;
   if (adc_key_in < 380)  return btnDOWN;
   if (adc_key_in < 555)  return btnLEFT;
   if (adc_key_in < 790)  return btnSELECT;
   return btnNONE; //if all others fail
}

//Returns TRUE if the select button is being pressed, FALSE otherwise
int sousLCD::selectPressed(){
  int key = read_LCD_buttons();
  return key == btnSELECT;
}

//Lets user select the cooking temperature and time using the screen buttons
//Finishes loop when user presses select three consecutive times
void sousLCD::selectParameters(){
  setUpSelectParam();

  int loop_flag = 1;
  int select_presses = 0; //Number of straight times SELECT has been pressed
  int indx_blink = 6, line_blink = 0; //Start at first line
  while(loop_flag){
    //Set cursor where digit should blink
    lcd.setCursor(indx_blink,line_blink);

    //Read buttons, and act accordingly
    int lcd_key = read_LCD_buttons(); //Return the button currently pushed
    int new_val = -1; //Wether user changed anything
    switch (lcd_key) //Act based on key input
    {
      case btnRIGHT: //If possible, shift the blinking digit right
       {
          //Rightmost blinking pixel for temperature (line 0) is 7 (7+0)
          //Rightmost blinking pixel for temperature (line 1) is 8 (7+1)
          if (indx_blink<(7+line_blink)){
            indx_blink++;
          }
          select_presses = 0;
          break;
       }
      case btnLEFT: //If possible, shift the blinking digit left
       {
          //Leftmost blinking pixel is 6
          if (indx_blink>6){
            indx_blink--;
          }
          select_presses = 0;
          break;
       }
      //UP and DOWN changes the values for the blinking digit
      //The new value has to be the current displayed value for the line (displayed_values[line_blink]) +- something
      //We change the value in increments of 1, 10, or 100, depending the pixel selected (in the units, tens, hundreds)
      //If we are in the digit in position 0 (units), we add/substract 10^0=1
      //If we are in the digit in position 1 (tens), we add/substract 10^1=10
      //If we are in the digit in position 2 (hundreds), we add/substract 10^2=100
      //For temperature (line 0), the position of the digit is 0 in pixel 7, 1 in pixel 6 (7-indx_blink)
      //For temperature (line 1), the position of the digit is 0 in pixel 6, 1 in pixel 7, 2 in pixel 8 (7-indx_blink+line_blink)
      case btnUP:
       {
          new_val = displayed_values[line_blink] + pow(10,(7-indx_blink)+line_blink);
          select_presses = 0;
          break;
       }
      case btnDOWN:
       {
          new_val = displayed_values[line_blink] - pow(10,(7-indx_blink)+line_blink);
          select_presses = 0;
          break;
       }
      //Change current line (from 0 to 1, from 1 to 0)
      case btnSELECT:
       {
          line_blink = !line_blink;
          indx_blink = 6; //Blink the first digit in the new line
          select_presses++;
          break;
       }
    }

    //If the new value is between appropiate limits, set it as the displayed value
    //Otherwise, either set it to the minimum or maximum value
    //This does not print the value to the screen
    if (new_val!=-1){ //If new_val was changed
      if (new_val>max_vals[line_blink]){
        displayed_values[line_blink] = max_vals[line_blink];
      } else if (new_val<min_vals[line_blink]){
        displayed_values[line_blink] = min_vals[line_blink];
      } else {
        displayed_values[line_blink] = new_val;
      }
    }

    //Make the digit which can be changed blink
    //Blink consists of making the pixel blank, wait, print is value, wait
    lcd.setCursor(indx_blink,line_blink);
    lcd.print(" "); //Make the pixel blank
    delay(100);     //Wait
    lcd.setCursor(6, line_blink);
    //If setting temperature (line = 1), and its value is less than 100, print the 0 for 099
    if ((line_blink == 1)&&(displayed_values[1]<100)){
      lcd.print(0);
    }
    lcd.print(displayed_values[line_blink]); //Display the value
    delay(100); //Wait
    if (select_presses>2){
      loop_flag = 0;
    }
  }
  //Update environment values
  (*sous).changeTempTarget(displayed_values[0]);
  (*sous).changeCookingTime(displayed_values[1]);
}

//------------------------------------------------------------------------------
//Safety functions
//------------------------------------------------------------------------------

void sousLCD::setUpError(int error_code){
  //Set up the screen with an error message in case of device malfunction
  //ERROR        OFF
  //error_code
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ERROR        OFF");
  lcd.setCursor(0,1);
  lcd.print(error_code);
}

//------------------------------------------------------------------------------
//Sous on/off display functions
//------------------------------------------------------------------------------

void sousLCD::sousOff(){
  //let user know that the sous vide is off
  //prints OFF in the top right corner
  lcd.setCursor(13,0);
  lcd.print("OFF");
}

void sousLCD::sousOn(){
  //let user know that the sous vide is on
  //prints ON in the top right corner
  lcd.setCursor(13,0);
  lcd.print(" ON");
}

//------------------------------------------------------------------------------
//LCD set up functions
//------------------------------------------------------------------------------

void sousLCD::setUpSelectParam(){
  //Set up the screen for user to input cooking parameters (temperature, time)
  //Temp: 50  °C OFF
  //Time: 060 min

  //Get temperature and time values
  displayed_values[0] = (*sous).getTempTarget();
  displayed_values[1] = (*sous).getCookingTime();
  //First line
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.setCursor(6,0);
  lcd.print(displayed_values[0]); //default temp value
  lcd.setCursor(9,0);
  lcd.print(" ");
  lcd.print((char)223); //degree symbol
  lcd.print("C ");
  sousOff();
  //Second line
  lcd.setCursor(0,1);
  lcd.print("Time: ");
  lcd.setCursor(6,1);
  if (displayed_values[1]<100){
    lcd.print(0); //If the initial value is less than 100, ie. print 0 for 099
  }
  lcd.print(displayed_values[1]); //default time value
  lcd.setCursor(9,1);
  lcd.print(" min   ");
}

void sousLCD::setUpHeatingMode(){
  //Set up the screen - when heating to set temperature
  //Heating       ON
  //WAIT 59.9/60°C
  //First line
  //Second line
  lcd.setCursor(0,0); //First line
  lcd.print("HEATING       ");
  sousOn();
  lcd.setCursor(0,1); //Second line
  lcd.print("WAIT     ");
  lcd.setCursor(9,1);
  lcd.print("/");
  lcd.print((*sous).getTempTarget());
  lcd.print((char)223); //degree symbol
  lcd.print("C  ");
  delay(1000);
}

void sousLCD::setUpCookingMode(){
  //Set up the screen - when cooking at constant temperature
  //Temp: 50.0/60 ON
  //Time: 001/060min
  //First line
  lcd.setCursor(0,0);
  lcd.print("Temp:     ");
  lcd.setCursor(10,0);
  lcd.print("/");
  lcd.print((*sous).getTempTarget());
  sousOn();
  //Second line
  lcd.setCursor(0,1);
  lcd.print("Time:    /");
  lcd.print((*sous).getCookingTime());
  lcd.print("min");
}

//------------------------------------------------------------------------------
//LCD update functions
//------------------------------------------------------------------------------

void sousLCD::updateHeatingTemp(float temp_current){
  //Update screen with current sousvide temperature (heating mode)
  lcd.setCursor(5,1);
  lcd.print(temp_current,1);
}

void sousLCD::updateCookingTime(int runTime){
  //Update screen with current cooking time (cooking mode)
  //Check that the cooking time has no more than 3 digits
  if (runTime>999){
    runTime = 999; //Set to maximum 3 digits int
  }
  //Set cursor position depending on number of digits
  if (runTime>99){
    lcd.setCursor(6,1);
  } else if (runTime>9){
    lcd.setCursor(7,1);
  } else {
    lcd.setCursor(8,1);
  }
  lcd.print(runTime, DEC); //Print run time
  Serial.print(runTime);
}

void sousLCD::updateCookingTemp(float temp_current){
  //Update screen with current sousvide temperature (cooking mode)
  lcd.setCursor(6,0);

  lcd.print(temp_current,1);
  Serial.print(temp_current);
}

void sousLCD::updateHeatingDone(){
  //Update screen to show that sousvide has been heated to set temperature
  //Heated         ON
  //Go   59.9/60°C
  lcd.setCursor(0,0);
  lcd.write("Heated  ");
  lcd.setCursor(0,1);
  lcd.write("Go  ");
}

void sousLCD::updateCookingDone(){
  //Update screen to show that cooking has finished
  //Print DONE in the top right corner
  lcd.setCursor(12,0);
  lcd.print("DONE");
}
