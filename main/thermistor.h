#ifndef __THERMISTOR_H__
#define __THERMISTOR_H__


#include "driver/gpio.h"


#define THMP_GPIO   32       //A14, ADC1_CHANNEL_4
#define DEFAULT_VREF 1100   //Default vref
#define NO_OF_SAMPLES 64    //Multisampling 


#define THMP_OK 0
#define THMP_TIMEOUT_ERROR -1
#define THMP_CHECKSUM_ERROR -2
/**
 * @brief Sets the thermistor GPIO PIN
 * 
 */
void setgpio(int gpio);

float getvoltage();
float getTemperature();
/**
 * @brief Configure ADC_UNIT_1 and ADC_UNIT_2
 * 
 */
void THMP_ADC_Configure();



/**
 * @brief Convert ADC raw data to mV
 * 
 * @param adc_raw ADC bits 
 * @return uint32_t voltage in mV

double THMP_convert_adc_raw_to_mV(uint32_t);
 */

/**
 * @brief Convert the voltage to temperature
 * 
 * @param vol_mV voltage in mV
 * @return double temp_C temperature in degreesC
 */
double THMP_convert_mV_to_temp_C(uint32_t);

//void check_efuse(void);


//void print_char_val_type(esp_adc_cal_value_t);




/**
 * @brief STarts thermistor sensor task
 * 
 */
void THMP_start_task();


#endif // __THERMISTOR_H__