//--- PROJECT MACROS ---
//- PIN MAP
//- output -
#define led_debug 2
#define relay_bat1_pin 5
#define relay_bat2_pin 6
#define load_ctr_pin 25
//- input -
#define v_load_pin 12
#define v_bat_pin 13
#define i_bat1_pin 36  
#define i_bat2_pin 4

//- CONFIG
//- adc config -
#define adc_resolution 4095
#define adc_v_ref 3.3
//- dac config -
#define dac_v_ref 3.3
#define dac_res 255
//- instrumentation config -
#define div_res_cte 0.5
#define seg_coef 0.05
#define i_limit 0.1

//- GENERAL
#define coupling 1
#define decoupling 0
#define repouso 0
#define teste 1
#define falta 2


//--- VARIABLES ---
double v_bateria = 0;
int actual_state = 0; // 0 - repouso; 1 - Teste; 2 - Falta
int prev_state = 0;


//--- FUNCTIONS ---
// - instrumentation -
double adc_calibration(float x);
double measure_voltage (int pin);

// - actuation -
void load_control(float load_current, int pin);
void relay_control(int bat_name, int action);

// - Data manage
void send_data(float v_bat_teste, float v_bat_backup, float i_bat1, float i_bat2);
void receive_data();

// - system operation -
void verifica_estado(int estado_atual, int bat_teste);
void state_repouso();
void state_falta();
bool state_teste(int bat_name, int v_eod, int load_current); //0 = test unfinished(falta; 1 = test finished


