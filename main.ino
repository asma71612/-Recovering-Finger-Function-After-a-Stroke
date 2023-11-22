// V2.2 Changed lines 66, 70, 73, 123 for motors/circuits that don't stop when given the stop command or when the simulation ends
// BEFORE STARTING: Go to Tools > Serial Monitor and have the serial monitor opened. Read through the code and change the fields marked with "TO CHANGE" in the comments. The servo motor only operates in 4-6V range.
// remember that you can restart the Arduino code at anytime by pressing the white button on the Arduino
/*
PURPOSE: Measure the voltage of the rubber wire within a sampling duration and calculate the resistance. Now with a servo motor code integrated.
INPUTS: Reads the resistance of the rubber wire
OUTPUTS: Data in the serial monitor (voltage of rubber wire in volts, resistance of rubber wire in ohms, and sample time in seconds). Now also controls a servo motor 
Easily copy-paste the data into a spreadsheet by following the steps below:
    1) Start the Serial Monitor by going to "Tools > Serial Monitor" in the toolbar visible at the top of the IDE
    2) Click and select the entire selection of data in the serial monitor, press Ctrl+C to copy
    3) A necessary intermediary step for the data to paste into individual cells is to open up a notepad application and paste the data into the blank notepad and save it as a text file
    4) Open Excel. In the toolbar, navigate to Data > From Text/CSV or Data > From Text file and select the text file from step 3
*/
#include <Servo.h> // necessary library to code servos
Servo servo1; // creates a motor object. Naming it servo1 by convention.

#define STOP_TIME 15  // TO CHANGE: set the length of time you wish to record the data for (in seconds). Both motor motions and rubber resistance measurements will stop at STOP_TIME

const int pinNumber = 5; // pin the motor's yellow wire (control signal) is connected to, can be any digital pin

// TO CHANGE: the array below is used to control the servo motor's direction and speed
// cw = clockwise, ccw = counter-clockwise, stop = stops the motor
// To add more motions, increase the number of the first index (default set to 4)
String mtr_motions[4][2] = {{"cw","slow"},{"ccw","fast"},{"stop","na"},{"cw","fast"}};

// TO CHANGE: the array below is used by the move_mtr() function to specify the time t where the motor's motion in the mtr_motions[] array will stop
// The first motion will stop at time t = a where a is the 1st index value (time is in seconds). The second motion will start when the first motion ends. The second motion will end at time t = b which is the 2nd index value. And so on
// For example with the default code, the motor will:
//            (1) rotate clockwise slowly between time 0-3 seconds
//            (2) rotate counter-clockwise fast between time 3-7 seconds
//            (3) stop moving between time 7-12 seconds
//            (4) rotate clockwise fast between time 12-15 seconds
// *IMPORTANT* the last index's value (default 15 below) must equal STOP_TIME in line 16. Otherwise, one of the error messages will keep triggering in the serial monitor.
int mtr_stop_times[4] = {3,7,12,15}; // default 4 stop time intervals of the motor's motions (in seconds)
int j = 0; // counter variable for motor movement


int RubberPin = A0;                // input pin that reads the voltage drop across the rubber wire, can be any of the analog pins A0-A5
const int Resistor = 2200;         // TO CHANGE: this is the fixed value of the other resistor in the voltage divider circuit
const int sample_interval = 1000;  // TO CHANGE: interval of time between resistance measurements (in milliseconds)

float R_voltage;     // measured voltage drop value of the rubber wire
float R_resistance;  // calculated resistance value of the rubber wire
float sample_time;     // sample time the resistances are sampled at, increments by sample_interval every loop
int i=0;             // counter for calculating sample time
bool written = false;  // true once all data has been outputed to serial monitor



void setup() {
  // startup the serial monitor for data to be displayed
  Serial.begin(9600);  // initialize serial communication, 9600 is the baud rate
  servo1.attach(pinNumber); // assigns pinNumber as the servo motor's control signal pin
  Serial.print('\n');  // starts the program on a new line in the serial monitor
  Serial.println("Sample Time,Voltage(V),Resistance(Ω)"); // prints the column headings
}


// Reads voltage drop of rubber wire, calculates resistance and outputs sample time, voltage and resistance to the serial monitor
// void loop() runs indefinitely, follow the steps in lines 5-9 of this code for pasting data into a spreadsheet
int maxRes = 0;
int minRes = 10000;
void loop() {

  // calls different motor motions based on comparing the current time of the program to the stop time's outlined in mtr_stop_times array
  // the current motion of the motor will be stopped and the next motion will begin. This is done by comparing against millis() 
  // millis() returns the number of milliseconds passed since the Arduino board ran the program
  servo1.attach(pinNumber); // NEW CHANGE
  move_mtr(mtr_motions[j][0], mtr_motions[j][1]);
  if(millis() > mtr_stop_times[j]*1000){
    j++;
    servo1.detach(); // NEW CHANGE, 95 was not stopping the motor for some groups, so it is now being hard-stopped using the detach() method
  }

  if(millis() > STOP_TIME*1000){  // NEW CHANGE
    servo1.detach();
  }
  // process the input values
  R_voltage = analogRead(RubberPin) * (5.0 / 1023.0);     // analogRead() reads the voltage and returns a number betweenn 0 and 1023 that is proportional to the voltage. Therefore, to reverse this and get voltage readings, we must divide by 1023 and multiply by the input voltage of the circuit which is 5V
  R_resistance = Resistor * R_voltage / (5 - R_voltage);  // voltage divider equation re-arranged to find R2, R2 = R1*Vout/(Vin - Vout), where R2 is the rubber wire
  sample_time = sample_interval * i / 1000;               // computes the sample time the sample is taken at (in seconds)

  // print calculated values to serial monitor
  if(!written) {
    // compute and output the sample time into rows on the serial monitor each time void loop() runs
    Serial.print(sample_time);
    Serial.print(',');  // separating the samples by a comma such that the row can later be copy+pasted into individual spreadsheet cells
    Serial.print(R_voltage);
    Serial.print(',');
    Serial.print(R_resistance);
    //  checks if sample time has exceeded the desired stop time, if true, stop outputting data to the serial monitor
    if (sample_time >= STOP_TIME) {
      written = true;  // when true, no more data will be written to the serial monitor
    }
  }
  i++;
  delay(sample_interval);  // the loop is paused for the sample interval time
  if(R_resistance>maxRes){
    maxRes=R_resistance;
  }
  if(R_resistance<minRes){
    minRes=R_resistance;
  }
    Serial.print(',');  
    Serial.print(minRes);
    Serial.print(',');
    Serial.print(maxRes);
    Serial.print(',');
    Serial.println("endofloop");
}


// PURPOSE: Rotates the motor at different directions and speeds based on pre-set values from mtr_motions and mtr_stop_times
//          servo.write() is used to control the speed and direction of the motor, it takes in an integer between 0-180 as its input. 
//          Different brand servo motors may have different speeds assigned to the integers between 0-180.
// INPUTS: the motor direction and motor speed written by the student on line 23 (mtr_motions string)
// RETURNS: nothing

void move_mtr(String direction, String speed){
  if(direction == "cw"){
    if(speed == "slow"){
      servo1.write(93); // TO CHANGE: 93 is the slowest speed (90-93 are varying slow speeds that you can use)
    }else if(speed == "fast"){
      servo1.write(30); // 30 is the fastest speed (50 RPM)
    }else{
      Serial.println("ERROR: incorrect speed input");
    }
  }else if(direction == "ccw"){
    if(speed == "slow"){
      servo1.write(96); // TO CHANGE: 96 is the slowest speed (96-100 are varying slow speeds)
    }else if(speed == "fast"){
      servo1.write(160);  // 160 is the fastest speed
    }else{
      Serial.println("ERROR: incorrect speed input");
    }
  }else if(direction == "stop"){
    servo1.detach();  // NEW CHANGE
  }else{
    Serial.println("ERROR: incorrect direction input");
  }
}
