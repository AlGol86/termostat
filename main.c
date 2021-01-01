#include "iostm8s103f3.h"
#include "wh0802.h"
#include "main.h"
#include "BME_280.h"


cyr_eeprom_struct s_cyr_eeprom[8] @0x4010; //cyr_alphabet
eeprom_data_t eeprom_data @0x4100;         //store setting variables (in EEPROM)()
encoder_t 	encoder;
//global block for BME_280 sensor
long int t_fine;
bme280_calib_data _bme280_calib;
ctrl_meas _measReg; 
ctrl_hum _humReg;
config _configReg;
int t;
char h;
////////////////////////////////
unsigned long int global_counter;
mode_t mode ;
data_buffer_t db;
char cm_f;//changed munu flag
char state_of_out_f;//state_of_out flag
   
int main( void )
{
   //send_cyr_to_eeprom();
  init1604();
  init_in_out();
  init_BME280();
  init_encoder(&encoder);
  

  while(1){
   // measurement
   t=readTemperature();
   h=readHumidity();
   //check switcher on-off mode
   if(check_inp()!=eeprom_data.on_off_mode) {cm_f=1;if(check_inp())write_eeprom(&eeprom_data.on_off_mode,on); else write_eeprom(&eeprom_data.on_off_mode,off);}
   if(eeprom_data.on_off_mode==off) 
     perform_action(t,eeprom_data.t_preset_off_mode_HH*10+eeprom_data.t_preset_off_mode_L,eeprom_data.histeresis_off_mode);
   else perform_action(t,eeprom_data.t_preset_on_mode_HH*10+eeprom_data.t_preset_on_mode_L,eeprom_data.histeresis_on_mode);
   //mode setting 
   if(encoder.but_data==0 && encoder.but_data_long==0 ) {cm_f_setter(mode,main_screen);mode=main_screen;}
   else if(encoder.but_data==1 && encoder.but_data_long==0 ) {cm_f_setter(mode,T_setting);mode=T_setting;}
   else if(encoder.but_data_long==1 ) {cm_f_setter(mode,HIST_setting);mode=HIST_setting;}
   /******************************/
   /////////menu//////////////////
   /******************************/
  //...main screen..............//
   if(mode==main_screen) {
     if(cm_f) {  
       cm_f=0;  
       clear();
       db.t_left=0;
       db.t_right=10;
       db.h=0;}
     print_temperature(&db.t_left,t,1);
     print_humidity(&db.h,h,2);
   }
   //...T_setting screen..............//
   else if (mode==T_setting){
     if (eeprom_data.on_off_mode==off) set_enc_to_eeprom_t(&eeprom_data.t_preset_off_mode_HH,50,300, cm_f);
     else set_enc_to_eeprom_t(&eeprom_data.t_preset_on_mode_HH,50,300, cm_f);
     if(cm_f) {  
       cm_f=0;  
       clear();
       print_symb(&s_cyr_eeprom[0].Uu,1,1,1);
       print_string("CT.",2,1);
       db.t_left=0;
       db.t_right=10;
     }
     print_temperature(&db.t_left,encoder.enc_data,2);
     
   }
   //...HISTERESIS_setting screen
   else if (mode==HIST_setting){
     if (eeprom_data.on_off_mode==off) set_enc_to_eeprom_h(&eeprom_data.histeresis_off_mode,2,50, cm_f);
     else set_enc_to_eeprom_h(&eeprom_data.histeresis_on_mode,2,50, cm_f);
     if(cm_f) {  
       cm_f=0;  
       clear();
       print_symb(&s_cyr_eeprom[0].Ge,1,1,0);
       print_symb(&s_cyr_eeprom[0].II,2,1,1);
       print_string("CTEP.",3,1);
       db.h=0;

     }
     print_changeg_value_right(&db.h,encoder.enc_data/10, 3, 2 );
     print_char(0x2e,4,2);
     print_changeg_value_right(&db.h,encoder.enc_data%10, 5, 2 );
   }

  }
  return 0;
}

void print_changeg_value_left(char* buf,char value, char x, char y ){
 char pos;
  if(*buf!=value) {
    *buf=value;
    pos=print_n_01(value, x, y);
    if (value<10) print_char(0x20,pos,y);
  }
 
}
void print_changeg_value_right(char* buf,char value, char x, char y ){
 if (value>9) return;
  if(*buf!=value) {
    *buf=value;
    print_n_01(value, x, y);
    
  }
 
}

void write_eeprom(char* addr_eeprom,char data)
{ sim();
  	if (!FLASH_IAPSR_DUL)
    {  // unlock EEPROM
       FLASH_DUKR = 0xAE;
       FLASH_DUKR = 0x56;
    }
  rim();
       while (!FLASH_IAPSR_DUL) {;}
       *addr_eeprom=data;
       FLASH_IAPSR_DUL=0; // lock EEPROM
}

void perform_action(int t, int t_preset,char hist){
  if (state_of_out_f){ 
    if(t>=t_preset+hist) 
      set_out(off);
  }
  else{
    if(t<=(t_preset)) 
      set_out(on);
  }
  
}

void print_temperature(char* buf_t_left,int t, char y){
   print_char(0x54,1,y);//"T= "
   print_char(0x3d,2,y);
   
   print_changeg_value_left(buf_t_left,t/10, 5, y);
   print_char(0x2e,6,y);
   print_changeg_value_right(buf_t_left+1,t%10, 7, y);
   print_symb(&s_cyr_eeprom[0].deg,8,y,0);
}

void print_humidity(char* buf,char h, char y){
   print_char(0x68,1,y);//"h= "
   print_char(0x3d,2,y);
   
   print_changeg_value_left(buf,h, 5, y);
   print_char(0x25,6,y);

}
void cm_f_setter(mode_t mode1,mode_t mode2){
 extern char cm_f;
 if (mode1!=mode2) 
   cm_f=1;
}
void set_enc_to_eeprom_t(char* pointer_to_eeprom, int limit_l, int limit_h, char cm_f){
  extern encoder_t 	encoder;
  if(cm_f) encoder_setter(limit_l,limit_h,(int)(*pointer_to_eeprom)*10+*(pointer_to_eeprom+1));
  if((encoder.enc_data/10)!=*pointer_to_eeprom) write_eeprom(pointer_to_eeprom,encoder.enc_data/10);
  if((encoder.enc_data%10)!=*(pointer_to_eeprom+1)) write_eeprom(pointer_to_eeprom+1,encoder.enc_data%10);
  
}

void set_enc_to_eeprom_h(char* pointer_to_eeprom, int limit_l, int limit_h, char cm_f){
  extern encoder_t 	encoder;
  if(cm_f) encoder_setter(limit_l,limit_h,*pointer_to_eeprom);
  if(encoder.enc_data!=*pointer_to_eeprom) write_eeprom(pointer_to_eeprom,encoder.enc_data);
   
}