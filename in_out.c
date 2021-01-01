#include "iostm8s103f3.h"
#include "main.h"
void init_in_out(){
  PORT_OUT_ODR|=BIT_OUT_RELAY;
  PORT_OUT_CR1&=~BIT_OUT_RELAY;
  PORT_OUT_DDR|=BIT_OUT_RELAY;
  
  
  PORT_OUT_DDR&=~BIT_IN_SWITHER;
  PORT_OUT_CR1|=BIT_IN_SWITHER;
    
}

switcher_t check_inp(){
  if (PORT_OUT_IDR & BIT_IN_SWITHER) return off;
  else return on;
}

void set_out(switcher_t mode){
  extern char state_of_out_f;
  if(mode) {PORT_OUT_ODR&=~BIT_OUT_RELAY;state_of_out_f=1;}
  else {PORT_OUT_ODR|=BIT_OUT_RELAY;state_of_out_f=0;}
}