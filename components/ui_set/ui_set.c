#include <stdio.h>
#include "ui_set.h"
#include "esp_log.h"

// static void btn_event_cb(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_CLICKED) {
//         ESP_LOGI("UI", "Button clicked!");
//     }
// }

// void ui_init(void)
// {
//     lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), 0);

//     lv_obj_t *btn = lv_btn_create(lv_scr_act());
//     lv_obj_set_size(btn, 150, 50);
//     lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);

//     lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_color(btn, lv_color_hex(0xCC0000), LV_PART_MAIN | LV_STATE_PRESSED);
//     lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
//     lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
//     lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);

//     lv_obj_t *label = lv_label_create(btn);
//     lv_label_set_text(label, "Hello LVGL");
//     lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
//     lv_obj_center(label);

//     lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
// }