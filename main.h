#include "iostm8s103f3.h"

#define rim() asm("rim")
#define sim() asm("sim")
#define nop() asm("nop")


#define PORT_iic_ODR       PD_ODR
#define PORT_iic_DDR       PD_DDR
#define PORT_iic_IDR       PD_IDR
#define PORT_iic_CR1       PD_CR1
#define PORT_iic_CR2       PD_CR2
#define pin_sda        2
#define bit_sda                           (1<<pin_sda)
#define pin_clk        3
#define bit_clk                           (1<<pin_clk)
#define WRITE 0
#define READ  1

#define addr_iic_BME_280 0x76 

#define PORT_OUT_DDR PB_DDR
#define PORT_OUT_ODR PB_ODR
#define PORT_OUT_IDR PB_IDR
#define PORT_OUT_CR1 PB_CR1
#define PORT_OUT_CR2 PB_CR2
#define PIN_OUT_RELAY 5 
#define PIN_IN_SWITHER 4 
#define BIT_OUT_RELAY  (1<<PIN_OUT_RELAY)
#define BIT_IN_SWITHER  (1<<PIN_IN_SWITHER)

#define PORT_ENCODER_DDR PA_DDR
#define PORT_ENCODER_ODR PA_ODR
#define PORT_ENCODER_IDR PA_IDR
#define PORT_ENCODER_CR1 PA_CR1
#define PORT_ENCODER_CR2 PA_CR2
#define pin_enc_l      1
#define bit_enc_l                       (1<<pin_enc_l)
#define pin_enc_r      2
#define bit_enc_r                       (1<<pin_enc_r)
#define pin_enc_b      3
#define bit_enc_b                       (1<<pin_enc_b)

typedef enum {off,on} switcher_t;
typedef enum {main_screen,T_setting,HIST_setting} mode_t;

typedef struct {
	char cnt;
	int cnt_button;
	char f_push :1;
	char f_long_push :1;
	
	int first_enc_data;
	int enc_data;
	int enc_data_lim_h;
	int enc_data_lim_l;
	char but_data;
        char but_data_long;
	char but_data_lim;
	char but_data_lim_long;
} encoder_t;

typedef struct {
	char t_preset_off_mode_HH;
        char t_preset_off_mode_L;
        char t_preset_on_mode_HH;
        char t_preset_on_mode_L;
        char histeresis_off_mode;
        char histeresis_on_mode;
        char on_off_mode;
}
eeprom_data_t;

typedef struct {
	char t_left;
        char t_right;
	char h;
        char value_left;
        char value_right;
}
data_buffer_t;


//in-out
void init_in_out(void);
char check_inp(void);
void set_out(switcher_t mode);

//encoder
#pragma vector = 15
__interrupt void TIM2Interrupt (void);
void init_tim2 (void);
void init_encoder(encoder_t* enc);
void encoder_handler(encoder_t* enc);
void encoder_setter(int lim_l,int lim_h,int first_value);

//iic
void sys_del_us(char del_us);
char start_iic (char adr, char read_write_bit);
char send_byte (char data_byte);
unsigned char receive_byte (char acknowledge);
void stop_iic (void);


void print_changeg_value_left(char* buf,char value, char x, char y);
void print_changeg_value_right(char* buf,char value, char x, char y );
void write_eeprom(char* addr_eeprom,char data);
void perform_action(int t, int t_preset,char hist);
void print_temperature(char* buf_t_left,int t, char y);
void print_humidity(char* buf,char h, char y);
void cm_f_setter(mode_t mode1,mode_t mode2);
void set_enc_to_eeprom_t(char* pointer_to_eeprom, int limit_l, int limit_h, char cm_f);
void set_enc_to_eeprom_h(char* pointer_to_eeprom, int limit_l, int limit_h, char cm_f);