#include "iostm8s103f3.h"
#include "BME_280.h"
#include "main.h"

void delay(unsigned int del){
  int i;
  while(del!=0) {
    del--;
    for(i=0;i<100;i++){
      nop();
    }
  }
}

void init_BME280()
{
    // reset the device using soft-reset
    // this makes sure the IIR is off, etc.
    write8(BME280_REGISTER_SOFTRESET, 0xB6);
    // wait for chip to wake up.
    delay(300);
    // if chip is still reading calibration, delay
    while (isReadingCalibration())
          delay(100);
    readCoefficients(); // read trimming parameters, see DS 4.2.2
    setSampling(MODE_NORMAL,SAMPLING_X16,SAMPLING_X16,SAMPLING_X16,FILTER_OFF, STANDBY_MS_0_5); // use defaults
    delay(100);

}

void setSampling(sensor_mode       mode,
		 sensor_sampling   tempSampling,
		 sensor_sampling   pressSampling,
		 sensor_sampling   humSampling,
		 sensor_filter     filter,
		 standby_duration  duration) {
 extern    ctrl_meas _measReg; 
 extern    ctrl_hum _humReg;
 extern    config _configReg;
    _measReg.mode     = mode;
    _measReg.osrs_t   = tempSampling;
    _measReg.osrs_p   = pressSampling;
        
    
    _humReg.osrs_h    = humSampling;
    _configReg.filter = filter;
    _configReg.t_sb   = duration;

    
    // you must make sure to also set REGISTER_CONTROL after setting the
    // CONTROLHUMID register, otherwise the values won't be applied (see DS 5.4.3)
    write8(BME280_REGISTER_CONTROLHUMID, (_humReg.osrs_h));
    write8(BME280_REGISTER_CONFIG, (_configReg.t_sb << 5) | (_configReg.filter << 3) | _configReg.spi3w_en);
    write8(BME280_REGISTER_CONTROL, (_measReg.osrs_t << 5) | (_measReg.osrs_p << 3) | _measReg.mode);
}

void write8(char reg, char value) {

        start_iic (addr_iic_BME_280, WRITE);
        send_byte (reg);
        send_byte (value);
        stop_iic ();
  
}

char read8(char reg) {
    char value;
    start_iic (addr_iic_BME_280, WRITE);
    send_byte (reg);
    stop_iic ();
    start_iic (addr_iic_BME_280, READ);
    value=receive_byte (0);
    stop_iic ();
    return value;
}

unsigned int read16(char reg)
{
    unsigned int value;

    start_iic (addr_iic_BME_280, WRITE);
    send_byte (reg);
    stop_iic ();
    start_iic (addr_iic_BME_280, READ);
    value=receive_byte (1)<<8;
    value|=receive_byte (0);
    stop_iic ();
    return value;
}


unsigned int read16_LE(char reg) {
    unsigned int temp = read16(reg);
    return (temp >> 8) | (temp << 8);
}


int readS16(char reg)
{
    return (int)read16(reg);
}



int readS16_LE(char reg)
{
    return (int)read16_LE(reg);
}

void takeForcedMeasurement()
{ 
 extern    ctrl_meas _measReg; 
 
    // If we are in forced mode, the BME sensor goes back to sleep after each
    // measurement and we need to set it to forced mode once at this point, so
    // it will take the next measurement and then return to sleep again.
    // In normal mode simply does new measurements periodically.
    if (_measReg.mode == MODE_FORCED) {
        // set to forced mode, i.e. "take next measurement"
        write8(BME280_REGISTER_CONTROL, (_measReg.osrs_t << 5) | (_measReg.osrs_p << 3) | _measReg.mode);
        // wait until measurement has been completed, otherwise we would read
        // the values from the last measurement
        while (read8(BME280_REGISTER_STATUS) & 0x08)
		delay(1);
    }
}

void readCoefficients(void)
{
  extern bme280_calib_data _bme280_calib;
  
    _bme280_calib.dig_T1 = read16_LE(BME280_REGISTER_DIG_T1);
    _bme280_calib.dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
    _bme280_calib.dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

    _bme280_calib.dig_P1 = read16_LE(BME280_REGISTER_DIG_P1);
    _bme280_calib.dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
    _bme280_calib.dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
    _bme280_calib.dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
    _bme280_calib.dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
    _bme280_calib.dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
    _bme280_calib.dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
    _bme280_calib.dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
    _bme280_calib.dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

    _bme280_calib.dig_H1 = read8(BME280_REGISTER_DIG_H1);
    _bme280_calib.dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
    _bme280_calib.dig_H3 = read8(BME280_REGISTER_DIG_H3);
    _bme280_calib.dig_H4 = (read8(BME280_REGISTER_DIG_H4) << 4) | (read8(BME280_REGISTER_DIG_H4+1) & 0xF);
    _bme280_calib.dig_H5 = (read8(BME280_REGISTER_DIG_H5+1) << 4) | (read8(BME280_REGISTER_DIG_H5) >> 4);
    _bme280_calib.dig_H6 = (signed char)read8(BME280_REGISTER_DIG_H6);
}
char isReadingCalibration(void)
{
  char rStatus = read8(BME280_REGISTER_STATUS);

  return (rStatus & (1 << 0)) != 0;
}

int readTemperature(void)
{
   extern long int t_fine; 
   extern bme280_calib_data _bme280_calib;
   long int var1, var2, T;
   long int adc_T = (long int) read16(BME280_REGISTER_TEMPDATA)<<8;
    adc_T |= read8(BME280_REGISTER_TEMPDATA+2);

    adc_T >>= 4;

    var1 = ((((adc_T>>3) - ((long int)_bme280_calib.dig_T1 <<1))) *
            ((long int)_bme280_calib.dig_T2)) /2048;
             
    var2 = (((((adc_T>>4) - ((long int)_bme280_calib.dig_T1)) *
              ((adc_T>>4) - ((long int)_bme280_calib.dig_T1))) >> 12) *
            ((long int)_bme280_calib.dig_T3)) / 16384;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;
    return T/10;

}



int readHumidity(void) {
    extern long int t_fine;
    extern bme280_calib_data _bme280_calib;
    readTemperature(); // must be done first to get t_fine

    long int adc_H = read16(BME280_REGISTER_HUMIDDATA);
    
        
    long int v_x1_u32r;

    v_x1_u32r = (t_fine - ((long int)76800));

    v_x1_u32r = (((((adc_H << 14) - (((long int)_bme280_calib.dig_H4) << 20) -\
                    (((long int)_bme280_calib.dig_H5) * v_x1_u32r)) + ((long int)16384)) >> 15) *\
                 (((((((v_x1_u32r * ((long int)_bme280_calib.dig_H6)) >> 10) *\
                      (((v_x1_u32r * ((long int)_bme280_calib.dig_H3)) >> 11) + ((long int)32768))) >> 10) +\
                    ((long int)2097152)) * ((long int)_bme280_calib.dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((long int)_bme280_calib.dig_H1)) >> 4));

    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    long int h = (v_x1_u32r>>12);
    return  h / 1024;
}

