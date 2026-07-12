#ifndef ADC_SET_H
#define ADC_SET_H

#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// ADC 采集引脚（GPIO1 对应 ADC1_CH0）
#define ADC_BATTERY_CHANNEL  0

// 分压电阻（单位：KΩ）
#define BATTERY_R_SERIES   10.0f   // 串联电阻（上）
#define BATTERY_R_PARALLEL 5.1f    // 并联电阻（下）

// 电池满电电压
#define BATTERY_FULL_VOLTAGE  4.2f

// MOS管压降补偿系数（背对背AO3406 N-MOS导致的压降）
// 电路结构：BAT -> [MOS] -> 分压电阻 -> ADC
// 实测满电4.2V时，MOS后分压前电压约为2.7V（两个AO3406的Vds压降约1.5V）
// 补偿系数 = 电池满电电压 / MOS后满电电压 = 4.2 / 2.7 ≈ 1.556
#define BATTERY_MOS_V_AFTER        2.7f
#define BATTERY_MOS_COMPENSATION   (BATTERY_FULL_VOLTAGE / BATTERY_MOS_V_AFTER)

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
