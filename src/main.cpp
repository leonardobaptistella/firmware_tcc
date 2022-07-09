//--------------------------------------------------------------------------------------------
// Author: Leonardo J. R. Baptistella
// Data: 05/07/2022
// - Implementa o gerenciamento dos estados de teste falta e repouso, 
// - Leitura dos sensores de tensão e corrente
// - Atuação nos relés de acoplamento
//--------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <ACS712.h>
#include <Project.h>


void setup(){

  Serial.begin(9600);

  // SET - Pinout config.
  pinMode(2, OUTPUT);
  pinMode(relay_bat1_pin, OUTPUT);
  pinMode(relay_bat2_pin, OUTPUT);
 
}

void loop() {
  v_bateria = measure_voltage(v_load_pin);
  load_control(3, 1);
  
  digitalWrite(2, HIGH);
  sleep(1);
  digitalWrite(2, LOW);
  sleep(1);
}


//--------------- Function Body ------------------------
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
  // Volt. divider cte; ADC resolution (V) - set in macros
  double r_cte = (r1_voltimeter/r2_voltimeter), adc_resol = (adc_v_ref/adc_resol);
  
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

void load_control(float load_current, int pin){
  float v_adc = 0;
  int adc_value = 0, adc_r = 0;

  v_adc = load_current / 2;
  adc_value = map(v_adc,0,dac_v_ref,0,dac_res);

  dacWrite(pin, adc_value); 
}