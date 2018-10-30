/*
sousVide control script
Author: Ricardo Dominguez Olmedo
Last updated: 07/2017
*/

//TO DO:
//  -Limit within bounds
//When heating keep on

#include <sousLCD.h>
#include <sousEnv.h>
#include <PID_v1.h>

#define ON  1
#define OFF 0

//Initialize necessary classes
sousEnv sous; //sousVide environment
sousLCD disp(&sous); //LCD display

//PID variables
const float Kp = 80/3.0; //Proportional gain
const float Ki = 0.01; //Integral gain
const float Kd = 0; //Derivative gain
double pid_tgt, pid_in, pid_out; //Variables for PID target/input/output
PID myPID(&pid_in, &pid_out, &pid_tgt, Kp, Ki, Kd, DIRECT); //PID object
#define PID_lower_limit 0.0
#define PID_upper_limit 100.0

//Window variables
const int window_length = 10000;  //In ms
unsigned long windowStartTime;

int onTime = 0;

unsigned long glob_start;

void setup(){
  Serial.begin(9600);
  sous.heatingOFF();

  //1.Select parameters screen
  disp.selectParameters(); //Finishes when parameters are selected

  ///2. Heating to selected temperature
  //disp.setUpHeatingMode();
  glob_start = millis();
  //heatControl(); //Finishes when desired temperature is reached

  //3. Cooking for given time
  disp.setUpCookingMode();
  cookControl();
}

void loop(){ }

//------------------------------------------------------------------------------
// Heating mode control functions
//------------------------------------------------------------------------------

//Turns heating on until the target temperature is surpassed, then maintains
//that temperature through a bang-bang controller until user presses the select
//button in the screen
void heatControl(){
  float heatTarget = sous.getTempTarget(); //Compute target temperature
  disp.setUpHeatingMode(); //Set up display for heating mode

  int tempState = ON;

   //Keep heating on until target temperature is reached
  float temp = sous.readTemp();
  while(temp < heatTarget){
    sous.heatingON(); //Turn heating on
    temp = sous.readTemp();
    disp.updateHeatingTemp(temp); //Update screen
    Serial.print((millis()-glob_start)/1000);
    Serial.print(" ");
    Serial.println(temp);
  }
  disp.updateHeatingDone();

  //Maintain the target temperature through a bang-bang controller
  while(!disp.selectPressed()){ //Until SELECT is pressed by user
    temp = sous.readTemp();
    Serial.print((millis()-glob_start)/1000);
    Serial.print(" ");
    Serial.println(temp);
    disp.updateHeatingTemp(temp); //Update screen

    //Bang-bang controller
    if(temp < heatTarget){
      tempState = ON;
    } else {
      tempState = OFF;
    }
    sous.switchHeating(tempState);
  }
}

//------------------------------------------------------------------------------
// Cooking mode control functions
//------------------------------------------------------------------------------

//Maintains temperature at a setpoint through PID control, until cooking is done,
//then turns heat off but still updates the temperature/cookign time to screen
void cookControl(){
  //PID control
  pid_tgt = sous.getTempTarget();
  myPID.SetMode(AUTOMATIC);  //Turn PID on
  //Apply PID control until the cooking is done
  int done = 0;
  while(!done){
    done = cookControlLoop();
  }

  //Turn heat off but still updates the screen
  sous.heatingOFF();
  disp.updateCookingDone(); //Set screen to cooking done mode
  float temp;
  int mins_passed;
  while(1){
    //Get temperature and time passed
    temp = sous.readTemp();
    mins_passed = sous.getMinsCooking();
    Serial.print((millis()-glob_start)/1000);
    Serial.print(" ");
    Serial.println(temp);
    //Update screen
    disp.updateCookingTemp(temp);
    disp.updateCookingTime(mins_passed);
  }
}

//PID temperature control loop, by turning heat either ON or OFF in windows of
//time proportional to the value outputted by the PID controller
int cookControlLoop(){
  unsigned long now = millis();
  //Check if current window has ended
  int done = 0;
  if ((now - windowStartTime) > window_length){
    windowStartTime += window_length;
    float temp = sous.readTemp(); //Read current temperature
    Serial.print((millis()-glob_start)/1000);
    Serial.print(" ");
    Serial.print(temp);
    Serial.print(" ");
    Serial.println(onTime);

    //Check for malfunction
    int flag = sous.getSafetyFlag(temp);
    checkSafetyFlag(flag); //Check that there is no malfunction

    //Compute new ON time from PID
    updateOnTime(temp);

    //Computations related to time passed since start
    sous.updateCookStartTime(temp);
    int mins_passed = sous.getMinsCooking();
    done = sous.doneCooking(mins_passed);

    //Update display
    disp.updateCookingTemp(temp); //Print to screen
    disp.updateCookingTime(mins_passed);
  }

  //State is OFF if past onTime, ON otherwise
  int tempState;
  if ((now - windowStartTime) > onTime) {
    tempState = OFF;
  } else {
    tempState = ON;
  }
  sous.switchHeating(tempState);

  return done;
}

//Compute the heat ON time window (proportional to the PID output value)
int updateOnTime(float temp){
  //Update ON time
  pid_in = temp; //PID input is current temperature
  myPID.Compute(); //Compute PID output
  //Compute the % of the time sousVide is powered ON, where 1 is at all times
  // and 0 is always powered OFF
  Serial.print(pid_out);
  if(pid_out>PID_upper_limit){
    pid_out = PID_upper_limit;
  } else if(pid_out<PID_lower_limit){
    pid_out = PID_lower_limit;
  }
  float percentON = (pid_out - PID_lower_limit) / (PID_upper_limit - PID_lower_limit);
  //Scale the % ON with window_length
  onTime = percentON * window_length;
}

//------------------------------------------------------------------------------
// Safety functions
//------------------------------------------------------------------------------

void checkSafetyFlag(int flag){
  if (flag != 0){ //If some error was raised
    disp.setUpError(flag); //Warn user of error
    sous.safetyTrigger(); //Perform relevant actions (such as shut off)
  }
}
