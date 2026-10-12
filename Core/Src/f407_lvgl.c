#include "f407_lvgl.h"

#include "driver_st7789_f407.h"
#include "driver_cst816d_f407.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
#include "stm32f4xx_hal.h"

#define F407_LVGL_BUF_LINES 30U
#define F407_LVGL_BUF_BYTES (F407_ST7789_WIDTH * F407_LVGL_BUF_LINES * 2U)

LV_ATTRIBUTE_MEM_ALIGN
static uint8_t s_draw_buf[F407_LVGL_BUF_BYTES];

static lv_display_t *s_display;
static lv_indev_t *s_touch_indev;
static uint32_t s_rtos_tick_offset;
static uint16_t s_touch_last_x;
static uint16_t s_touch_last_y;
static uint8_t s_rtos_tick_synchronized;

/**
 * @brief 向LVGL提供当前触摸状态和坐标。
 * @param indev 函数输入或输出参数。
 * @param data 函数输入或输出参数。
 * @return None
 */
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    F407_CST816D_Point_t point;
    int result = F407_CST816D_Read(&point);

    (void)indev;
    if(result == 0) {
        s_touch_last_x = point.x;
        s_touch_last_y = (uint16_t)((F407_ST7789_HEIGHT - 1U) - point.y);
        data->point.x = (lv_coord_t)s_touch_last_x;
        data->point.y = (lv_coord_t)s_touch_last_y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->point.x = (lv_coord_t)s_touch_last_x;
        data->point.y = (lv_coord_t)s_touch_last_y;
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/**
 * @brief 获取连续递增的LVGL系统时基。
 * @return 函数处理结果或计算值。
 */
static uint32_t lvgl_tick_get(void)
{
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        TickType_t rtos_tick = xTaskGetTickCount();
        uint32_t rtos_ms =
            (uint32_t)(((uint64_t)rtos_tick * 1000U) / configTICK_RATE_HZ);

        if(!s_rtos_tick_synchronized) {
            s_rtos_tick_offset = HAL_GetTick() - rtos_ms;
            s_rtos_tick_synchronized = true;
        }
        return s_rtos_tick_offset + rtos_ms;
    }

    return HAL_GetTick();
}

/**
 * @brief 将LVGL绘制区域刷新到LCD。
 * @param disp 函数输入或输出参数。
 * @param area 函数输入或输出参数。
 * @param px_map 函数输入或输出参数。
 * @return None
 */
static void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    if(area->x2 >= 0 && area->y2 >= 0 &&
       area->x1 < (int32_t)F407_ST7789_WIDTH &&
       area->y1 < (int32_t)F407_ST7789_HEIGHT) {
        uint16_t x1 = (uint16_t)(area->x1 < 0 ? 0 : area->x1);
        uint16_t y1 = (uint16_t)(area->y1 < 0 ? 0 : area->y1);
        uint16_t x2 = (uint16_t)(area->x2 >= (int32_t)F407_ST7789_WIDTH ?
                                 (int32_t)F407_ST7789_WIDTH - 1 : area->x2);
        uint16_t y2 = (uint16_t)(area->y2 >= (int32_t)F407_ST7789_HEIGHT ?
                                 (int32_t)F407_ST7789_HEIGHT - 1 : area->y2);

        (void)F407_ST7789_Flush(x1, y1, x2, y2, (const uint16_t *)px_map);
    }

    lv_display_flush_ready(disp);
}

/**
 * @brief 初始化LVGL显示设备。
 * @return 函数处理结果或计算值。
 */
static int display_init(void)
{
    if(F407_LCD_Init() != 0) {
        return -1;
    }

    s_display = lv_display_create(F407_ST7789_WIDTH, F407_ST7789_HEIGHT);
    if(s_display == NULL) {
        return -1;
    }

    lv_display_set_color_format(s_display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(s_display, s_draw_buf, NULL, sizeof(s_draw_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_display, display_flush_cb);
    lv_display_set_default(s_display);
    return 0;
}

/**
 * @brief 初始化LVGL触摸输入设备。
 * @return 函数处理结果或计算值。
 */
static int touch_init(void)
{
    if(F407_CST816D_Init() != 0) {
        return -1;
    }

    s_touch_indev = lv_indev_create();
    if(s_touch_indev == NULL) {
        return -1;
    }
    s_touch_last_x = 0U;
    s_touch_last_y = 0U;
    lv_indev_set_type(s_touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_touch_indev, touch_read_cb);
    lv_indev_set_display(s_touch_indev, s_display);
    return 0;
}

/**
 * @brief 初始化LVGL显示和触摸输入设备。
 * @return 函数处理结果或计算值。
 */
int F407_LVGL_Init(void)
{
    lv_init();
    lv_tick_set_cb(lvgl_tick_get);

    if(display_init() != 0) {
        return -1;
    }

    (void)touch_init();

    return 0;
}

/**
 * @brief 执行一次LVGL定时器处理。
 * @return None
 */
void F407_LVGL_Loop(void)
{
    (void)lv_timer_handler();
}
