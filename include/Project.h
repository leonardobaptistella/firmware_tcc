// Project Macros
//- output
#define led_debug 2
#define relay_bat1_pin 5
#define relay_bat2_pin 6
#define load_ctr_pin 25
//- input
#define v_load_pin 12
#define v_bat_pin 13
#define i_bat1_pin 36  
#define i_bat2_pin 4
//- adc config
#define adc_resolution 4095
#define adc_v_ref 3.3
//- dac config
#define dac_v_ref 3.3
#define dac_res 255
//- Instrumentation config
#define r1_voltimeter 300 
#define r2_voltimeter 300

// Variable declaration
double v_bateria = 0;
int actual_state = 0; // 0 - repouso; 1 - Teste; 2 - Falta
int prev_state = 0;

// Function declaration
double adc_calibration(float x);
double measure_voltage (int pin);
void load_control(float load_current, int pin);


