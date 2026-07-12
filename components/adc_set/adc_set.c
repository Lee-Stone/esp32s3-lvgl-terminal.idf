#include <stdio.h>
#include "adc_set.h"

// 基准电压 3.3V 对应的 ADC 满量程值（12 位 = 4095）
#define ADC_MAX_MV     3300
#define ADC_RESOLUTION 4095.0f

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t cali_handle = NULL;

void adc_init(void)
{
    // 1. 创建 ADC 单元 
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    // 2. 配置 ADC 通道（GPIO1 = ADC1_CH0）
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,     // 衰减 12dB，量程约 0~3.3V
        .bitwidth = ADC_BITWIDTH_12,  // 12 位分辨率
    };
    adc_oneshot_config_channel(adc_handle, ADC_BATTERY_CHANNEL, &chan_cfg);

    // 3. 创建校准句柄
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle);
}

float adc_get_battery_voltage(void)
{
    // 读取 ADC 原始值
    int raw = 0;
    adc_oneshot_read(adc_handle, ADC_BATTERY_CHANNEL, &raw);

    // 通过校准转换为毫伏
    int voltage_mv = 0;
    adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv);

    // 分压比：MOS后电压 = 采集电压 × (R串联 + R并联) / R并联
    float ratio = (BATTERY_R_SERIES + BATTERY_R_PARALLEL) / BATTERY_R_PARALLEL;
    float voltage_after_mos = (voltage_mv / 1000.0f) * ratio;

    // 补偿MOS管压降：电池实际电压 = MOS后电压 × 补偿系数
    float battery_v = voltage_after_mos * BATTERY_MOS_COMPENSATION;

    return battery_v;
}

uint8_t adc_get_battery_percent(float voltage)
{
    if (voltage >= BATTERY_FULL_VOLTAGE) {
        return 100;
    }
    if (voltage <= 3.3f) {
        return 0;
    }
    // 3.3V~4.2V 线性映射到 0%~100%
    uint8_t percent = (uint8_t)((voltage - 3.3f) / (BATTERY_FULL_VOLTAGE - 3.3f) * 100.0f);
    return percent;
}