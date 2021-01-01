#include "iostm8s103f3.h"
#include "wh0802.h"


void del_WH(unsigned int del)
{
  for(unsigned int j=0;j<=del;j++){}
}

void init1604(void)
{
  PORT_WH_DATALINE_ODR&=~(MASK_db);//b00001111
  PORT_WH_DATALINE_DDR|=(MASK_db); //b11110000
  PORT_WH_DATALINE_CR1|=(MASK_db); //b11110000
  PORT_WH_DATALINE_CR2|=(MASK_db); //b11110000
  PORT_WH_RS_ODR&=~BIT_rs;
  PORT_WH_EN_ODR&=~BIT_en;
  PORT_WH_RS_DDR|=BIT_rs; 
  PORT_WH_EN_DDR|=BIT_en;
  PORT_WH_RS_CR1|=BIT_rs; 
  PORT_WH_EN_CR1|=BIT_en;   
  PORT_WH_RS_CR2&=~BIT_rs; 
  PORT_WH_EN_CR2&=~BIT_en;
  del_WH(30000);
  write_wh_byte(0,8);
  write_wh(0,0x30); //arg: char rs, char db7_db4
  del_WH(50);
  write_wh_byte(0,32|N_F_code<<2);
  write_wh_byte(0,32|N_F_code<<2);
  write_wh_byte(0,8|(D_C_B_code));
  clear();
  write_wh_byte(0,4|ID_SH_code);
  clear();
  set_cursor(1,1);
}

void clear(void)
{
  write_wh_byte(0,1);
  del_WH(2000);
}

void set_cursor(char x,char y)
{
  x--;
  switch(y)
{
case 1: {write_wh_byte(0,0x80|0x0|x);break;} 
case 2: {write_wh_byte(0,0x80|0x40|x);break;}
case 3: {write_wh_byte(0,0x80|0x10|x);break;}
case 4: {write_wh_byte(0,0x80|0x50|x);break;}
}
}

void write_wh(char rs,char db7_db4)// rs=0-instruction; rs=1-data (write half of byte)
{
  PORT_WH_DATALINE_ODR&=~MASK_db;
  PORT_WH_RS_ODR&=~BIT_rs;
  PORT_WH_RS_ODR|=rs<<PIN_WH_RS;
  PORT_WH_EN_ODR|=BIT_en;
  PORT_WH_DATALINE_ODR|=(db7_db4&0xF0);
  asm("nop");
  PORT_WH_EN_ODR&=~BIT_en;
}

void write_wh_byte(char rs,char db7_db0)
{
  write_wh(rs,db7_db0);
  write_wh(rs,db7_db0<<4);
  PORT_WH_DATALINE_ODR&=~(MASK_db);
  del_WH(20);
}

void write_wh_digit(char n)
{
write_wh_byte(1,0x30+n);
}

void print_char(char ch,char x, char y) 
{
  set_cursor(x,y);
  write_wh_byte(1,ch);
}

void print_string(char* pointer, char x, char y) //print eng_litters string
{
set_cursor(x,y);
while (*pointer!=0)
{
write_wh_byte(1,*pointer);
pointer++;
}
}


char print_n_01(unsigned long int n, char x, char y ) //n=0..65535 right align x-right position
{ 

do
{
print_char((0x30+(n%10)),x,y); 
n/=10;
x--;
}
while(n>0); 
return (x);

}
      
char print_n_10(unsigned int n, char x, char y ) //n=0..65535 left align x-left position
{ 
unsigned int tn=0;
char fz=0;
if ((n%10)==0)  {n++;fz=1;}
if(n==1) print_char(0x30,x,y); 
  else 
  {
while(n>0) {tn=(tn*10)+(n%10); n/=10;}
while(tn>9) {print_char(0x30+(tn%10),x,y); tn/=10;x++;}
if(fz) print_char(0x30,x,y); else print_char(0x30+tn,x,y);
  }
x++;
return (x);
}

void print_n_00(unsigned long int n, char nn, char x, char y ) //n=0..65535 fixed format - nn diges x-left pos
{
char pos;  
pos=print_n_01(n,x+nn-1,y);
while(pos>=x) { print_char(0x30,pos,y);pos--;}
}
  

char print_symb(unsigned char* ch_cyr_pointer,char x,char y,char n_CGRAM) //if LED don't provide CYR letters
{             
 cyr_eeprom_struct smth[2];
 char step=(&smth[1].Be-&smth[0].Be);
  
 write_wh_byte(0,64|(n_CGRAM<<3)); //set address CGRAM_segment
 for (char j=0;j<8;j++) //no.string pix in character
        {  
        write_wh_byte(1,*ch_cyr_pointer);  //send data[0..7] to CGRAM_segment
        ch_cyr_pointer+=step;
        }
 
set_cursor(x,y);
write_wh_byte(1,n_CGRAM);
n_CGRAM++; 
return n_CGRAM;
}

void print_n_10_format(unsigned int n, char x, char y, char format ) //n=0..65535 left align x-left position, format-nomber of patterns
{
  char pos;
  pos=print_n_10(n,x,y);
  while(pos<(x+format))
   {
    write_wh_byte(1,' ');
    pos++;
   }
}

void send_cyr_to_eeprom(void)
{
extern cyr_eeprom_struct s_cyr_eeprom[8];
cyr_eeprom_struct data_input[8]=\
{{0x1f,0x1f,0x0e,0x12,0x15,0x1e,0x11,0x0a,0x1c,0x1f,0x11,0x04,0x12,0x11,0x15,0x15,0x18,0x11,0x10,0x0e,0x12,0x0f,0x18},
 {0x11,0x11,0x0a,0x00,0x15,0x01,0x11,0x04,0x14,0x11,0x11,0x0e,0x12,0x11,0x15,0x15,0x08,0x11,0x10,0x11,0x15,0x11,0x18},
 {0x10,0x10,0x0a,0x1e,0x15,0x01,0x13,0x11,0x12,0x11,0x11,0x15,0x12,0x11,0x15,0x15,0x08,0x11,0x10,0x01,0x15,0x11,0x00},
 {0x1e,0x10,0x0a,0x10,0x0e,0x0e,0x15,0x13,0x11,0x11,0x0a,0x15,0x12,0x0f,0x15,0x15,0x0e,0x19,0x1e,0x07,0x1d,0x0f,0x07},
 {0x11,0x10,0x0a,0x1c,0x15,0x01,0x19,0x15,0x11,0x11,0x04,0x15,0x12,0x01,0x15,0x15,0x09,0x15,0x11,0x01,0x15,0x05,0x04},
 {0x11,0x10,0x1f,0x10,0x15,0x01,0x11,0x19,0x11,0x11,0x08,0x0e,0x12,0x01,0x15,0x15,0x09,0x15,0x11,0x11,0x15,0x09,0x04},
 {0x1e,0x10,0x11,0x1e,0x15,0x1e,0x11,0x11,0x11,0x11,0x10,0x04,0x1f,0x01,0x1f,0x1f,0x0e,0x19,0x1e,0x0e,0x12,0x11,0x07},
 {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};
 //be   ge   de   yo  zhe   ze   II   ii   le   pe   uu   fe   ce   che  sha scha  tb   Ib   mb   ee   yu   ya   deg

asm("sim");
  	if (!((FLASH_IAPSR) & (MASK_FLASH_IAPSR_DUL)))
    {  // unlock EEPROM
       FLASH_DUKR = 0xAE;
       FLASH_DUKR = 0x56;
    }
  asm("rim");
  
       while (!((FLASH_IAPSR) & (MASK_FLASH_IAPSR_DUL))) {;}
       for(char i=0;i<8;i++){ s_cyr_eeprom[i]=data_input[i];}
       FLASH_IAPSR &= ~(MASK_FLASH_IAPSR_DUL); // lock EEPROM
}

