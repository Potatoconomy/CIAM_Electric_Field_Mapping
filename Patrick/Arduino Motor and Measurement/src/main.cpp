#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_ADS1X15.h"

Adafruit_ADS1115 ads1115; //Construct ads1115 class


//*=====================*//
//* Function Prototypes *//
//*=====================*//
void isr_readEncoder();
void control_motor_automatically();
void control_motor_manually(int user_input);
void set_motor_signals(int direction, int speed);
void adc_bit_return(int16_t&, int16_t&);
void adc_voltage_convert(int16_t&, int16_t&, float&, float&);
void print_adc_values(float, float);
void print_adc_values(int16_t, int16_t);
void get_adc_measurements();
void perform_round_trip();

//*=================*//
//* Pin Definitions *//
//*=================*//
#define RELAY_PIN 7
#define ENCA 2
#define ENCB 3
#define PWM_MOTOR_FWD 9
#define PWM_MOTOR_RVS 10


//*====================*//
//* Useful Definitions *//
//*====================*//
#define FWD 1
#define RVS -1
#define MAX_MOTOR_SPEED 200    //for automatic operation mode; do not exceed 255!
#define MIN_MOTOR_SPEED 20    //for operation mode: between 0 and 255


//*===============*//
//* Relay Options *//
//*===============*//
bool use_relay = 0; // 0 avoids relay switching of corrosion, 1 has relay switching
const int n_samples = 1000;   // number of samples to collect between relay switches


//*=====================*//
//* ADS1115 Definitions *//
//*=====================*//

// NOTE: This program is only setup for Differential measurements of channel 1 vs 0, and channel 3 vs 1.
// m_gain: GAIN_ + {TWO_THIRDS; ONE; TWO; FOUR; EIGHT; SIXTEEN}
// output: 0 for digits, 1 for voltage values from ADC
adsGain_t m_gain = GAIN_TWOTHIRDS;  
bool output = 0;  

  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV


//*========================*//
//* Motor Global Variables *//
//*========================*//
int operation_mode = 0;     //0=select operation mode, 1=automatic operation, 2=manual operation
long target_pos = 0;
long current_pos = 0;


void setup() {
  Serial.begin(9600);

  //*=============*//
  //* Relay Setup *//
  //*=============*//  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  //*===========*//
  //* ADS Setup *//
  //*===========*// 
  ads1115.setDataRate(RATE_ADS1115_860SPS); //Actual Rate: 1/rate + time to send signal to Serial 
  
  if (!ads1115.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }


  //*=============*//
  //* Motor Setup *//
  //*=============*// 
  pinMode(ENCA,INPUT);
  pinMode(ENCB,INPUT);
  pinMode(PWM_MOTOR_FWD,OUTPUT);
  pinMode(PWM_MOTOR_RVS,OUTPUT);
  //configure interrupt that tracks the current position
  attachInterrupt(digitalPinToInterrupt(ENCA), isr_readEncoder, RISING);
  //TBD initialise HW Timer to be used to activate serial communication only every 333ms
}

class Relay {
  public: 
    void set_high() {
      Serial.println("HIGH");
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
    }

    void set_low() {
      Serial.println("LOW");
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }
};

void adc_bit_return(int16_t& a, int16_t& b) {
  a = ads1115.readADC_Differential_0_1();
  b = ads1115.readADC_Differential_2_3();
}

void adc_voltage_convert(int16_t& a, int16_t& b, float& a_volts, float& b_volts) {
  a_volts = ads1115.computeVolts(a);
  b_volts = ads1115.computeVolts(b);
}

void print_adc_values(float a, float b)  {
    Serial.print(a, 10);
    Serial.print(",");
    Serial.print(b, 10);
    Serial.println(" ");
}

void print_adc_values(int16_t a, int16_t b)  {
  // To be called before motor values
  Serial.print(a);
  Serial.print(",");
  Serial.print(b);
  Serial.print(",");
}

void print_motor_values(long position) {
  // To be called after adc values
  Serial.print(position);
  Serial.println(" ");
}

void get_adc_measurements() {
  // This function calls 
  //    -adc_bit_return
  //    -adc_voltage_convert
  //    -print_adc_values (overloaded)
  // This function is a process that retrieves digital values from the ADC, converts if necessary, and prints to serial monitor.
  // This function exists purely to keep the primary loop function clean.
  // Need to find out if better to access global variabls or to redefine variables each function loop.    
  int16_t adc_1_0; 
  int16_t adc_3_2;
  float adc_1_0_volts; 
  float adc_3_2_volts;  

  adc_bit_return(adc_1_0, adc_3_2);

  if (output == 1) {
    adc_voltage_convert(adc_1_0, adc_3_2, adc_1_0_volts, adc_3_2_volts);
    print_adc_values(adc_1_0_volts, adc_3_2_volts);
  } else {
    print_adc_values(adc_1_0, adc_3_2);
  }

  //delay(1000);
}

void set_motor_mode() {
  int serial_buffer = 0;
  //select operation mode
  switch(operation_mode){
  case 0: //look if operational mode has been specified
    Serial.println("Please select operation mode: (1= automatic, 2=manual, 3=round-trip). The code currently only works with round-trip");
    while(Serial.available() == 0){
    delay(500);
    }
    if (Serial.available() > 0){
      serial_buffer = Serial.parseInt();
      //get rid of end line symbol
      Serial.read();
      if (serial_buffer == 1){
        operation_mode = 1;
        // report selection
        Serial.println("Automatic operation mode selected");
      }
      else if (serial_buffer == 2){
        operation_mode = 2;
        // report selection
        Serial.println("Manual operation mode selected");
        Serial.println("Please select motor speed: (between -255 and 255)");
      }
      else {
        operation_mode = 3;
        // report selection
        Serial.println("Round Trip mode selected");
      }
    }
    delay(100);
    break;
    // Operation Mode 1 (automatic mode)
  case 1:   
    //calculate and set the control signal for the motor
    control_motor_automatically();
    //look if new target position has been received
    if (Serial.available() > 0){
      target_pos = Serial.parseInt();
      //get rid of end line symbol
      Serial.read();
    }
    //report current position
    Serial.print("Current Position: ");
    Serial.println(current_pos);
    delay(1000);
    break;
    // Operation Mode 2 (manual mode)
  case 2:
    if (Serial.available() > 0){
      serial_buffer = Serial.parseInt();
      //get rid of end line symbol
      Serial.read();
      control_motor_manually(serial_buffer);
      Serial.println("Please select motor speed: (between -255 and 255)");
    }
    //report current position
    Serial.print("Current Position: ");
    Serial.println(current_pos);
    delay(1000);
    break;
  // Operation Mode 3 (Round Trip)
   case 3:
    perform_round_trip();
    operation_mode = 0;
  }
}

void loop() {

  set_motor_mode();
//  if (use_relay == 0) {
//    boolean done = false;
//    while (! done) {
//      get_adc_measurements();
//    }
//  } else {
//    boolean done = false;
//    Relay relay;
//    while (! done) {
//      relay.set_high();
//      for (int i=0; i<n_samples; i++) {
//        get_adc_measurements();
//      }
//      relay.set_low();
//      for (int i=0; i<n_samples; i++) {
//        get_adc_measurements();
//      }
//    }
//  }
}


void isr_readEncoder(){
  //interrupt service routine which is executed when a rising edge on ENCA is detected
  int b = digitalRead(ENCB);
  if(b>0){
  //if ENCB is high, the motor spins forward
    current_pos++;
  }
  else{
  //if ENCB is low, the motor spins backwards
    current_pos--;
  }
}

void control_motor_automatically()
{
  //define and initialize function variables
  long pos_error = 0;
  long abs_pos_error = 0;
  int motor_direction = FWD;
  int motor_speed = 0;
  //calculate mismatch between target position and current position
  pos_error = target_pos - current_pos;
  //define desired motor direction according to the position error
  if (pos_error >= 0){
  motor_direction = FWD;
  }
  else{
    motor_direction = RVS;
  }
  abs_pos_error = abs(pos_error);
  //define desired motor speed according to the absolute value of the position error
  if (abs_pos_error > (long)MAX_MOTOR_SPEED){
  motor_speed = MAX_MOTOR_SPEED;
  }
  //below a motor speed of 20, the motor not expected to move, therefore ensure a minimum value of 20
  else if ( (abs_pos_error > 0) && (abs_pos_error < (long)MIN_MOTOR_SPEED) ){
  motor_speed = MIN_MOTOR_SPEED;
  }
  else{
  motor_speed = (int)abs_pos_error;
  }
  //send the calculated control signals to the motor
  set_motor_signals(motor_direction, motor_speed);
  
  //Debug prints
  Serial.print("Motor Direction: ");
  Serial.println(motor_direction, DEC);
  Serial.print("Motor Speed: ");
  Serial.println(motor_speed, DEC); 
}

void control_motor_manually(int user_input)
{
  int motor_direction = FWD;
  int motor_speed = 0;
    //define desired motor direction
    if (user_input >= 0){
      motor_direction = FWD;
    }
    else{
      motor_direction = RVS;
    }
    motor_speed = abs(user_input);
    //send the calculated control signals to the motor
    set_motor_signals(motor_direction, motor_speed);
    
    //Debug prints
    Serial.print("Motor Direction: ");
    Serial.println(motor_direction, DEC);
    Serial.print("Motor Speed: ");
    Serial.println(motor_speed, DEC);
}

void perform_round_trip()
{
  // Must perform round trip whilst simultaneously sampling ADC
  // Right now, this only works without Relay function turned on
  if (use_relay == 1) {Serial.print("Code not setup to work with relay"); return;}

  //Motor must be set to position 0 in the tank
  set_motor_signals(FWD, 80);
  while(current_pos < 300)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(FWD, 255);
  while(current_pos < 9400)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(FWD, 80);
  while(current_pos < 9900)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(FWD, 30);
  while(current_pos < 10000)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(RVS, 80);
  while(current_pos > 9700)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(RVS, 255);
  while(current_pos > 600)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(RVS, 80);
  while(current_pos > 100)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(RVS, 30);
  while(current_pos > 0)
  {
    get_adc_measurements();
    print_motor_values(current_pos);
  }
  set_motor_signals(RVS, 0);
}

void set_motor_signals(int direction, int speed) 
{
  if (direction == FWD){
  analogWrite(PWM_MOTOR_RVS, 0);
  analogWrite(PWM_MOTOR_FWD, speed);
  }
  else{
  analogWrite(PWM_MOTOR_FWD, 0);
  analogWrite(PWM_MOTOR_RVS, speed);
  }
}

