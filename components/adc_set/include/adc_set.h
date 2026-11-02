#ifndef ADC_SET_H
#define ADC_SET_H

#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// ADC 采集引脚（GPIO1 对应 ADC1_CH0）
#define ADC_BATTERY_CHANNEL  0

// 分压电阻（单位：KΩ），新板 BAT -> R9(18K) -> IO1 -> R10(68K) -> GND
// 满电 4.2V 时 IO1 处约 4.2 x 68/(18+68) ≈ 3.32V，在 12dB 衰减量程内
#define BATTERY_R_SERIES   18.0f   // 串联电阻（上，R9）
#define BATTERY_R_PARALLEL 68.0f   // 并联电阻（下，R10）

// 电池满电电压
#define BATTERY_FULL_VOLTAGE  4.2f

// 新板分压网络直接接在 BAT 上，无 MOS 压降，补偿系数为 1
#define BATTERY_MOS_COMPENSATION   1.0f

/**
 * @brief 初始化 ADC 电池电压采集
 */
void adc_init(void);

/**
 * @brief 读取电池电压
 *
 * 通过 ADC 采集分压后的电压，换算为实际电池电压
 * 电路有背对背AO3406 N-MOS开关，需补偿MOS压降
 *
 * @return 电池电压（单位：V）
 */
float adc_get_battery_voltage(void);

/**
 * @brief 计算电池电量百分比
 *
 * @param voltage 电池电压
 * @return 电量百分比（0~100）
 */
uint8_t adc_get_battery_percent(float voltage);

#endif 
