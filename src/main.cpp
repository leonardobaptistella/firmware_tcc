//--------------------------------------------------------------------------------------------
// Author: Leonardo J. R. Baptistella
// Data: 05/07/2022
// - Implementa o gerenciamento dos estados de teste falta e repouso, 
// - Leitura dos sensores de tensão e corrente
// - Atuação nos relés de acoplamento
//--------------------------------------------------------------------------------------------

#include <Arduino.h>


// Project Macros
//- output
#define led_debug 2
#define relay_bat1_pin X
#define relay_bat2_pin X
#define load_ctr_pin 25
//- input
#define v_load_pin 12
#define v_bat_pin 13
#define i_bat1_pin 36  
#define i_bat2_pin 4
//- adc config
#define adc_resolution 12
#define adc_reference 3.3

// Function declaration
double adc_calibration(float x);
double measure_voltage (int pin);
double measure_current(int pin);

// Variable declaration
double v_bateria = 0;

void setup(){
  Serial.begin(9600);
  pinMode(led_debug, OUTPUT);
}

void loop() {
  v_bateria = measure_voltage(v_load_pin);
  Serial.println(v_bateria);
  digitalWrite(led_debug, HIGH);
  sleep(1);
  digitalWrite(led_debug, LOW);
  sleep(1);
}

double adc_calibration(float x){
  return   2.202196968876e+02
           +   3.561383996027e-01 * x
           +   1.276218788985e-04 * x * x
           +  -3.470360275448e-07 * x * x * x
           +   2.082790802069e-10 * x * x * x * x
           +  -5.306931174991e-14 * x * x * x * x * x
           +   4.787659214703e-18 * x * x * x * x * x * x;
  
}

double measure_voltage (int pin){
  // SET - Volt. divider cte; ADC resolution (V)
  double r_cte = 0.5000000, adc_resol = 0.000805861;
  
  // SET - Periodo de amostragem (s); Nº de amostras
  double  period_amostra = 0.0002;
  int n_amostras = 10;

  int analog_val = 0;
  double v_in = 0, analog_val_acc = 0;
 
  //Adc read and correction
  for(int i = 0; i < n_amostras; i++){
    analog_val =  analogRead(pin);
    //analog_val =  analog_val + adc_calibration(analog_val);
    analog_val_acc += analog_val;
    analog_val = 0; 
  }

  analog_val_acc = analog_val_acc / n_amostras;

  v_in = (adc_resol * analog_val_acc) /(r_cte);

  return v_in; 
}

double measure_current(int pin){
  double v_sensor = 0, i_sensor = 0;

  v_sensor = measure_voltage(pin);



  return i_sensor;
}