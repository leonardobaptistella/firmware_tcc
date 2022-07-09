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
}

void loop() {
  v_bateria = measure_voltage(v_load_pin);
  Serial.print("V bat. = ");
  Serial.println(v_bateria);
  //load_control(2, load_ctr_pin);
  
  digitalWrite(2, HIGH);
  sleep(1);
  digitalWrite(2, LOW);
  sleep(1);
}


//--------------- FUNCTION BODY ------------------------
//- Instrumentation -
double adc_calibration(float x){
  return   2.202196968876e+02
           +   3.561383996027e-01 * x
           +   1.276218788985e-04 * x * x
           +  -3.470360275448e-07 * x * x * x
           +   2.082790802069e-10 * x * x * x * x
           +  -5.306931174991e-14 * x * x * x * x * x
           +   4.787659214703e-18 * x * x * x * x * x * x;
  
}

double measure_voltage(int pin){
  // SET - Volt. divider cte; ADC resolution (V) - set in macros
  double r_cte = div_res_cte, adc_resol = (adc_v_ref/adc_resolution);
  
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

// - Control -
void load_control(float load_current, int pin){
  float v_adc = 0;
  int adc_value = 0, adc_r = 0;

  v_adc = load_current / 2;
  
  adc_value = v_adc/(dac_v_ref/(dac_res+1));
  adc_value = round(adc_value);

  Serial.print("An. Value = ");
  Serial.println(adc_value);

  dacWrite(pin, adc_value); 
}

void relay_control(int bat_name, int action){
  if((bat_name == 1)&&(action == 1)){
    digitalWrite(relay_bat1_pin, HIGH);
    digitalWrite(relay_bat2_pin, LOW); 
  }else if((bat_name == 2)&&(action == 1)){
    digitalWrite(relay_bat1_pin, LOW);
    digitalWrite(relay_bat2_pin, HIGH);
  }else if (action == 0){
    digitalWrite(relay_bat1_pin, LOW);
    digitalWrite(relay_bat2_pin, LOW);
  }else{
    digitalWrite(relay_bat1_pin, LOW);
    digitalWrite(relay_bat2_pin, LOW);
  }
}

// - Data manage -
void send_data(float v_bat_teste, float v_bat_backup, float i_bat1, float i_bat2){
  Serial.print(actual_state);
  Serial.print(";");
  Serial.print(v_bat_teste);
  Serial.print(";");
  Serial.print(v_bat_backup);
  Serial.print(";");
  Serial.print(i_bat1);
  Serial.print(";");
  Serial.println(i_bat2);
}

void receive_data(){

}

//- System manage -
void verifica_estado(int estado_atual, int bat_teste){
  
  if(estado_atual == repouso){
    float i_bat1 = 0, i_bat2 = 0;
    i_bat1 = 1; //sensor corrente
    i_bat2 = 1; //sensor corrente
    
    if ((i_bat1 >= i_limit)||(i_bat2 >= i_limit)){
      actual_state = falta;
    }    
  }

  if(estado_atual ==  teste){
    float i_bat = 0;
    int bat_backup = 0;

    if(bat_teste == 1){
      bat_backup = 2;
    }else if (bat_teste == 2){
      bat_backup = 1;
    }
    
    i_bat = 1; //sensor corrente (bat_backup)

    if (i_bat >= i_limit){
      actual_state = falta;
    }
  }
}

void state_falta(){


}

bool state_teste(int bat_name, int v_eod, int load_current){
  float v_bat_teste = 0, v_bat_backup = 0, i_bat1 = 0, i_bat2 = 0;
  int bat_pin = 0;
  
  // Verificação de seurança (não atingir totalmente o EoD)
  v_eod = v_eod + (v_eod*seg_coef);

  // Acopla a bateria
  relay_control(bat_name,coupling);  

  // Começa a descarga na corrente desejada
  load_control(load_current, load_ctr_pin);

  // Verifica o V_EoD, e o estado durante o teste
  while((v_bat_teste > v_eod)&&(actual_state == teste)){
    verifica_estado(actual_state, bat_name);
    prev_state = actual_state;
    v_bat_teste = measure_voltage(v_load_pin);
    v_bat_backup = measure_voltage(v_bat_pin);
    i_bat1 = 0; //func do acs
    i_bat2 = 0;
    send_data(v_bat_teste, v_bat_backup, i_bat1, i_bat2);    
    delay(1);
  }

  // Descopla a bateria
  relay_control(bat_name,decoupling);  
  
  if(actual_state == falta){
    return false;
  }else{ 
    actual_state = repouso;
    return true;
  }
}
