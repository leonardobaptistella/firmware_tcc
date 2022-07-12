//--------------------------------------------------------------------------------------------
// Author: Leonardo J. R. Baptistella
// Data: 05/07/2022
// - Implementa o gerenciamento dos estados de teste falta e repouso, 
// - Leitura dos sensores de tensão e corrente
// - Atuação nos relés de acoplamento
//--------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <Project.h>

int bat_name = 0, state = 0;
float v_eod = 0, load_current = 0;

void setup(){
  system_init(); 
}

void loop() {
  // int bat_name = 0, state = 0;
  // float v_eod = 0, load_current = 0;

  if(actual_state == falta){
    state_falta();
  }

  if (Serial.available()) { // if there is data comming
    String state_str = "", bat_name_str = "", v_eod_str = "",load_c_str = "";
    String command = Serial.readStringUntil('\n');
    
    state_str = command.substring(0,1);
    bat_name_str = command.substring(2,3);
    v_eod_str = command.substring(4,9);
    load_c_str = command.substring(10,14);
    
    state = state_str.toInt();
    bat_name = bat_name_str.toInt();
    v_eod = v_eod_str.toFloat();
    load_current = load_c_str.toFloat();
  }

  if((actual_state == repouso)&&(state == teste)){
    actual_state == teste;
    state_teste(bat_name, v_eod, load_current);
  }

  send_data(12.00, 12.32, 2.52, 1.02);
  
  delay(500);

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

double measure_current(int pin, float sensibility, float v_ref){
  double i_acs = 0, v_acs = 0;
  v_acs = measure_voltage(pin);
  i_acs = (v_acs - (v_ref * 0.5)) / sensibility;

  return i_acs;
}

// - Control -
void load_control(float load_current, int pin){
  float v_adc = 0;
  int adc_value = 0, adc_r = 0;

  v_adc = load_current / 2;
  
  adc_value = v_adc/(dac_v_ref/(dac_res+1));
  adc_value = round(adc_value);

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

//- System manage -
void system_init(){
  Serial.begin(9600);
  pinMode(led_debug, OUTPUT);
  pinMode(relay_bat1_pin, OUTPUT);
  pinMode(relay_bat2_pin, OUTPUT);
  digitalWrite(led_debug, LOW);
  digitalWrite(relay_bat1_pin, LOW);
  digitalWrite(relay_bat2_pin, LOW);
  dacWrite(load_ctr_pin, 0); 
}

void verifica_estado(int estado_atual, int bat_teste){
  // apenas entra e sai do estado falta

  // Estado atual repouso
  if(estado_atual == repouso){
    
    float i_bat1 = 0, i_bat2 = 0;
    i_bat1 = measure_current(i_bat1_pin, acs_sensibility, v_ref_acs); 
    i_bat2 = measure_current(i_bat2_pin, acs_sensibility, v_ref_acs);
    
    if ((i_bat1 >= i_limit)||(i_bat2 >= i_limit)){
      actual_state = falta;
    } 
  }

  // Estado atual teste
  if(estado_atual ==  teste){
    float i_bat = 0;
    int bat_backup = 0;

    if(bat_teste == 1){
      bat_backup = 2;
    }else if (bat_teste == 2){
      bat_backup = 1;
    }
    
    i_bat = measure_current(bat_backup,acs_sensibility, v_ref_acs); 

    if (i_bat >= i_limit){
      actual_state = falta;
    }
  }

  // Estado atual falta
  if(estado_atual == falta){
    float i_bat1 = 0, i_bat2 = 0;
    i_bat1 = measure_current(i_bat1_pin,acs_sensibility, v_ref_acs); 
    i_bat2 = measure_current(i_bat2_pin,acs_sensibility, v_ref_acs);
    
    if ((i_bat1 <= i_limit)&&(i_bat2 <= i_limit)){
      actual_state = repouso;
    }    
  }
}

bool state_falta(){
  float i_bat1 = 0, i_bat2 = 0, v_pack = 0;
  
  verifica_estado(actual_state, 0);

  while (actual_state == falta){
    i_bat1 = measure_current(i_bat1_pin,acs_sensibility, v_ref_acs); 
    i_bat2 = measure_current(i_bat2_pin,acs_sensibility, v_ref_acs);
    v_pack = measure_voltage(v_bat_pin);
    send_data(0,v_pack, i_bat1, i_bat2);
    verifica_estado(actual_state, 0);
  }
  return true;
}

bool state_teste(int bat_name, int v_eod, int load_current){
  float v_bat_teste = 0, v_bat_backup = 0, i_bat1 = 0, i_bat2 = 0;
  //int bat_pin = 0;
  
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
    i_bat1 = measure_current(i_bat1_pin,acs_sensibility, v_ref_acs); //func do acs
    i_bat2 = measure_current(i_bat2_pin,acs_sensibility, v_ref_acs);
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
  send_data(0, 0, 0, 0);
}
