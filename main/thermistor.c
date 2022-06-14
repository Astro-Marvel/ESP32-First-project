#include "tasks_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "thermistor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "math.h"
#include "esp_adc_cal.h"

static const char* TAG = "THERMISTOR";


adc_channel_t THMP_channel = ADC_CHANNEL_4;
adc_unit_t THMP_unit = ADC_UNIT_1;
adc_atten_t THMP_atten = ADC_ATTEN_DB_11;

esp_adc_cal_characteristics_t *thmp_adc_chars;
esp_adc_cal_value_t thmp_val_type;

float THMPtemp_C = 0.;
float THMPvoltage_mV = 0.;
int THMPgpio = 4;


/**
 * @brief Sets the thermistor GPIO PIN
 * 
 */
void setgpio(int gpio)
{
    THMPgpio = gpio;
}

// == get temp & hum =============================================

float getvoltage(void) { 
    return THMPvoltage_mV; 
    }
float getTemperature(void) { 
    return THMPtemp_C; 
    }

/**
 * @brief Configure ADC_UNIT_1 and ADC_UNIT_2
 * 
 */
void THMP_ADC_Configure(void)
{
    if(THMP_unit==ADC_UNIT_1){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten((adc1_channel_t)THMP_channel, THMP_atten);
}
else
{
    adc2_config_channel_atten((adc2_channel_t)THMP_channel, THMP_atten);
}
}

/**
 * @brief Convert ADC raw data to mV
 * 
 * @param adc_raw ADC bits 
 * @return uint32_t voltage in mV

double THMP_convert_adc_raw_to_mV(uint32_t adc_raw)
{
    return esp_adc_cal_raw_to_voltage(adc_raw, thmp_adc_chars);
}
 */


/**
 * @brief Convert the voltage to temperature
 * 
 * @param vol_mV voltage in mV
 * @return double temp_C temperature in degreesC
 */
double THMP_convert_mV_to_temp_C(uint32_t vol_mV)
{
    double vol_V = vol_mV / 1000.0f;
    double Resistance = 10*vol_V/(3.3-vol_V);
    double temp_K = 1/(1/(273.15+25) + log(Resistance/10)/3950.0); 
    double temp_C = temp_K - 273.15;
    return temp_C;
}
/*
void check_efuse(void)
{
 //Check TP is burned into eFuse
 if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
 printf("eFuse Two Point: Supported\n");
 }
 else {
 printf("eFuse Two Point: NOT supported\n");
 }
 //Check Vref is burned into eFuse
 if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
 printf("eFuse Vref: Supported\n");
 }
 else {
 printf("eFuse Vref: NOT supported\n");
 }
}
void print_char_val_type(esp_adc_cal_value_t val_type)
{
 if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
 printf("Characterized using Two Point Value\n");
 }
 else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
 printf("Characterized using eFuse Vref\n");
 }
 else {
 printf("Characterized using Default Vref\n");
 }
}
*/
/**
 * @brief Thermistor task to be pinned to the coreID. Sampling and logging the output
 * 
 */
static void THMP_task(void *pvParameters)
{
setgpio(THMP_GPIO);
//check_efuse();
THMP_ADC_Configure();
thmp_adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));

thmp_val_type = esp_adc_cal_characterize(THMP_unit,THMP_atten,ADC_WIDTH_BIT_12,DEFAULT_VREF,thmp_adc_chars);

//print_char_val_type(thmp_val_type);

for(;;)
{
 uint32_t adc_reading =0;
 //Multisampling
 for(int i=0;i<NO_OF_SAMPLES;i++)
 {
     if(THMP_unit == ADC_UNIT_1)
     {
         adc_reading += adc1_get_raw((adc1_channel_t) THMP_channel);
     }
     else{
         int raw;
         adc2_get_raw((adc2_channel_t)THMP_channel, ADC_WIDTH_BIT_12, &raw);
         adc_reading +=raw;
     }
 }
adc_reading /= NO_OF_SAMPLES; 
THMPvoltage_mV = esp_adc_cal_raw_to_voltage(adc_reading, thmp_adc_chars);
THMPtemp_C = THMP_convert_mV_to_temp_C(THMPvoltage_mV);


ESP_LOGI(TAG, "ADC value : %d,\t Voltage : %.1fmV, \t Temperature: %.1fC\n", adc_reading, getvoltage(), getTemperature());

vTaskDelay(4000/portTICK_RATE_MS);

}
}


void THMP_start_task(void)
{
    xTaskCreatePinnedToCore(&THMP_task,"Thermistor Task",THMP_Task_STACK_SIZE,NULL,THMP_Task_PRIORITY,NULL,THMP_Task_CORE_ID);
}