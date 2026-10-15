#include "ui_app.h"

#include "lvgl.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define UI_SCREEN_WIDTH       240
#define UI_SCREEN_HEIGHT      296
#define UI_PAGE_COUNT           5

#define UI_COLOR_BG       0x07111FU
#define UI_COLOR_CARD     0x101E32U
#define UI_COLOR_BORDER   0x203552U
#define UI_COLOR_TEXT     0xF4F8FFU
#define UI_COLOR_MUTED    0x8FA3BFU
#define UI_COLOR_BLUE     0x38BDF8U
#define UI_COLOR_GREEN    0x35D07FU
#define UI_COLOR_YELLOW   0xFFB020U
#define UI_COLOR_RED      0xFF4D5AU
#define UI_COLOR_PURPLE   0xB783FFU

static lv_obj_t *s_system_state_label;             /* 保存系统页状态大字标签。 */
static lv_obj_t *s_system_armed_label;             /* 保存系统页布防标签。 */
static lv_obj_t *s_system_detail_label;            /* 保存系统页报警原因标签。 */
static lv_obj_t *s_dht_temperature_label;          /* 保存DHT11页温度标签。 */
static lv_obj_t *s_dht_humidity_label;             /* 保存DHT11页湿度标签。 */
static lv_obj_t *s_dht_status_label;               /* 保存DHT11页读取状态标签。 */
static lv_obj_t *s_distance_value_label;           /* 保存超声波页距离标签。 */
static lv_obj_t *s_distance_status_label;          /* 保存超声波页状态标签。 */
static lv_obj_t *s_motion_state_label;             /* 保存MPU6050页运动状态标签。 */
static lv_obj_t *s_motion_accel_label;             /* 保存MPU6050页三轴加速度标签。 */
static lv_obj_t *s_motion_gyro_label;              /* 保存MPU6050页三轴角速度标签。 */
static lv_obj_t *s_remote_command_label;           /* 保存红外遥控页命令码标签。 */
static lv_obj_t *s_remote_device_label;            /* 保存红外遥控页设备地址标签。 */
static lv_obj_t *s_remote_hint_label;              /* 保存红外遥控页操作提示标签。 */
static lv_obj_t *s_system_card;                    /* 保存系统页状态卡片，用于改变强调色。 */
static lv_obj_t *s_pages[UI_PAGE_COUNT];           /* 保存五个独立页面对象地址。 */
static uint32_t s_current_page;                    /* 保存当前正在显示的页面序号。 */
static uint32_t s_last_version = UINT32_MAX;       /* 保存上次显示版本，确保第一次更新。 */
static int8_t s_pending_navigation;                /* 保存按钮提交、等待主循环执行的切页方向。 */

static lv_obj_t *UI_CreateLabel(
    lv_obj_t *parent,                              /* parent指定标签所属父对象。 */
    const char *text,                              /* text指定标签初始文字。 */
    uint32_t color);                               /* color指定文字颜色。 */

/**
 * @brief 执行UI_PreviousButtonEvent相关功能。
 * @param event 函数输入或输出参数。
 * @return None
 */
static void UI_PreviousButtonEvent(lv_event_t *event)
{
    if(lv_event_get_code(event) == LV_EVENT_CLICKED) {
        s_pending_navigation = -1;
    }
}

/**
 * @brief 执行UI_NextButtonEvent相关功能。
 * @param event 函数输入或输出参数。
 * @return None
 */
static void UI_NextButtonEvent(lv_event_t *event)
{
    if(lv_event_get_code(event) == LV_EVENT_CLICKED) {
        s_pending_navigation = 1;
    }
}

static lv_obj_t *UI_CreateNavigationButton(        /* 创建统一样式的翻页按钮。 */
    lv_obj_t *page,                                /* page指定按钮所属页面。 */
    const char *text,                              /* text指定按钮文字。 */
    int32_t x,                                     /* x指定按钮左上角横坐标。 */
    lv_event_cb_t event_cb)                        /* event_cb指定点击回调函数。 */
{
    lv_obj_t *button = lv_button_create(page);     /* 在当前页面创建LVGL按钮。 */
    lv_obj_t *label;                               /* 保存按钮内部文字对象。 */

    lv_obj_set_size(button, 100, 44);
    lv_obj_set_pos(button, x, 244);
    lv_obj_set_style_radius(button, 8, 0);
    lv_obj_set_style_bg_color(button,
                              lv_color_hex(UI_COLOR_BORDER),
                              0);
    lv_obj_set_style_bg_color(button,
                              lv_color_hex(UI_COLOR_BLUE),
                              LV_STATE_PRESSED);
    lv_obj_add_event_cb(button, event_cb,
                        LV_EVENT_CLICKED, NULL);
    label = UI_CreateLabel(button, text, UI_COLOR_TEXT);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *UI_CreateLabel(                   /* 创建统一风格的文字标签。 */
    lv_obj_t *parent,                              /* parent指定标签所属的父对象。 */
    const char *text,                              /* text指定标签初始文字。 */
    uint32_t color)                                /* color指定0xRRGGBB文字颜色。 */
{
    lv_obj_t *label = lv_label_create(parent);     /* 在父对象中创建标签。 */

    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label,
                                lv_color_hex(color),
                                0);
    lv_obj_remove_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return label;
}

static lv_obj_t *UI_CreatePage(                    /* 创建一个独立的完整页面。 */
    lv_obj_t *parent,                              /* parent是根屏幕。 */
    uint32_t index,                                /* index指定页面在容器中的序号。 */
    const char *title,                             /* title指定模块名称。 */
    const char *subtitle)                          /* subtitle指定模块简短说明。 */
{
    lv_obj_t *page = lv_obj_create(parent);        /* 创建页面根对象。 */
    lv_obj_t *label;                               /* 复用变量创建页头文字。 */
    char page_number[16];                          /* 保存右上角页码文字。 */

    lv_obj_set_size(page, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_bg_color(page,
                              lv_color_hex(UI_COLOR_BG),
                              0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_remove_flag(page,
                       LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    if(index != 0U) {
        lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);
    }
    s_pages[index] = page;

    label = UI_CreateLabel(page, title, UI_COLOR_TEXT);
    lv_obj_set_pos(label, 16, 14);
    lv_obj_set_style_text_letter_space(label, 1, 0);

    label = UI_CreateLabel(page, subtitle, UI_COLOR_MUTED);
    lv_obj_set_pos(label, 16, 38);

    (void)snprintf(page_number,
                   sizeof(page_number),
                   "%lu / %u",
                   (unsigned long)(index + 1U),
                   UI_PAGE_COUNT);
    label = UI_CreateLabel(page, page_number, UI_COLOR_MUTED);
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -16, 16);

    label = UI_CreateNavigationButton(page,
                                      "< PREV",
                                      16,
                                      UI_PreviousButtonEvent);
    if(index == 0U) {
        lv_obj_add_state(label, LV_STATE_DISABLED);
    }
    label = UI_CreateNavigationButton(page,
                                      "NEXT >",
                                      124,
                                      UI_NextButtonEvent);
    if(index + 1U == UI_PAGE_COUNT) {
        lv_obj_add_state(label, LV_STATE_DISABLED);
    }
    return page;
}

static lv_obj_t *UI_CreateMainCard(                /* 创建每页唯一的主数据卡片。 */
    lv_obj_t *page)                                /* page指定卡片所属页面。 */
{
    lv_obj_t *card = lv_obj_create(page);          /* 在页面上创建卡片。 */

    lv_obj_set_size(card, 208, 166);
    lv_obj_set_pos(card, 16, 70);
    lv_obj_set_style_bg_color(card,
                              lv_color_hex(UI_COLOR_CARD),
                              0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card,
                                  lv_color_hex(UI_COLOR_BORDER),
                                  0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 18, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_remove_flag(card,
                       LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return card;
}

/**
 * @brief 执行UI_SetLabel相关功能。
 * @param label 函数输入或输出参数。
 * @param text 函数输入或输出参数。
 * @param color 函数输入或输出参数。
 * @return None
 */
static void UI_SetLabel(
    lv_obj_t *label,                               /* label是要更新的标签。 */
    const char *text,                              /* text是期望显示的新字符串。 */
    uint32_t color)                                /* color是期望的新文字颜色。 */
{
    if(strcmp(lv_label_get_text(label), text) != 0) {
        lv_label_set_text(label, text);
    }

    if(!lv_color_eq(lv_obj_get_style_text_color(label, 0),
                    lv_color_hex(color))) {
        lv_obj_set_style_text_color(label,
                                    lv_color_hex(color),
                                    0);
    }
}

static const char *UI_StateText(EnvState_t state)  /* 将系统状态转换成简短文字。 */
{
    switch(state) {
    case ENV_STATE_NORMAL: return "SYSTEM NORMAL";
    case ENV_STATE_WARNING: return "SAFETY WARNING";
    case ENV_STATE_ALARM: return "ALARM ACTIVE";
    case ENV_STATE_SENSOR_ERROR: return "SENSOR OFFLINE";
    case ENV_STATE_STARTUP:
    default: return "SYSTEM STARTING";
    }
}

/**
 * @brief 执行UI_StateColor相关功能。
 * @param state 函数输入或输出参数。
 * @return 函数处理结果或计算值。
 */
static uint32_t UI_StateColor(EnvState_t state)
{
    switch(state) {
    case ENV_STATE_NORMAL: return UI_COLOR_GREEN;
    case ENV_STATE_WARNING: return UI_COLOR_YELLOW;
    case ENV_STATE_ALARM: return UI_COLOR_RED;
    case ENV_STATE_SENSOR_ERROR: return UI_COLOR_PURPLE;
    case ENV_STATE_STARTUP:
    default: return UI_COLOR_BLUE;
    }
}

static const char *UI_AlertText(EnvAlertReason_t reason) /* 将报警来源转换成说明。 */
{
    switch(reason) {
    case ENV_ALERT_DISTANCE: return "Object is too close";
    case ENV_ALERT_MOTION: return "Motion detected";
    case ENV_ALERT_NONE:
    default: return "Monitoring all modules";
    }
}

/**
 * @brief 执行UI_AppCreate相关功能。
 * @return None
 */
void UI_AppCreate(void)
{
    lv_obj_t *screen = lv_screen_active();         /* 获取LVGL当前活动屏幕。 */
    lv_obj_t *page;                                /* 复用变量保存当前页面。 */
    lv_obj_t *card;                                /* 复用变量保存当前主卡片。 */
    lv_obj_t *label;                               /* 复用变量创建卡片标题。 */

    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen,
                              lv_color_hex(UI_COLOR_BG),
                              0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    page = UI_CreatePage(screen, 0U, "SENTINEL", "SYSTEM & RGB ALARM");
    card = UI_CreateMainCard(page);
    s_system_card = card;
    label = UI_CreateLabel(card, "CURRENT STATUS", UI_COLOR_MUTED);
    lv_obj_set_pos(label, 16, 16);
    s_system_state_label = UI_CreateLabel(card, "SYSTEM STARTING", UI_COLOR_BLUE);
    lv_obj_set_width(s_system_state_label, 176);
    lv_obj_set_style_text_align(s_system_state_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(s_system_state_label, &lv_font_montserrat_28, 0);
    lv_obj_align(s_system_state_label, LV_ALIGN_CENTER, 0, -24);
    s_system_armed_label = UI_CreateLabel(card, "DISARMED", UI_COLOR_MUTED);
    lv_obj_align(s_system_armed_label, LV_ALIGN_CENTER, 0, 30);
    s_system_detail_label = UI_CreateLabel(card, "Monitoring all modules", UI_COLOR_MUTED);
    lv_obj_set_width(s_system_detail_label, 176);
    lv_obj_set_style_text_align(s_system_detail_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_system_detail_label, LV_ALIGN_BOTTOM_MID, 0, -16);

    page = UI_CreatePage(screen, 1U, "DHT11", "TEMPERATURE & HUMIDITY");
    card = UI_CreateMainCard(page);
    label = UI_CreateLabel(card, "TEMPERATURE", UI_COLOR_MUTED);
    lv_obj_set_pos(label, 18, 18);
    s_dht_temperature_label = UI_CreateLabel(card, "--.- C", UI_COLOR_BLUE);
    lv_obj_set_style_text_font(s_dht_temperature_label, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(s_dht_temperature_label, 18, 42);
    label = UI_CreateLabel(card, "HUMIDITY", UI_COLOR_MUTED);
    lv_obj_set_pos(label, 18, 82);
    s_dht_humidity_label = UI_CreateLabel(card, "--.- %", UI_COLOR_BLUE);
    lv_obj_set_style_text_font(s_dht_humidity_label, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(s_dht_humidity_label, 18, 102);
    s_dht_status_label = UI_CreateLabel(card, "Waiting for first sample", UI_COLOR_MUTED);
    lv_obj_align(s_dht_status_label, LV_ALIGN_BOTTOM_LEFT, 18, -14);

    page = UI_CreatePage(screen, 2U, "HC-SR04", "ULTRASONIC RANGE");
    card = UI_CreateMainCard(page);
    label = UI_CreateLabel(card, "DISTANCE", UI_COLOR_MUTED);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 22);
    s_distance_value_label = UI_CreateLabel(card, "-- cm", UI_COLOR_BLUE);
    lv_obj_set_style_text_font(s_distance_value_label, &lv_font_montserrat_28, 0);
    lv_obj_align(s_distance_value_label, LV_ALIGN_CENTER, 0, -8);
    s_distance_status_label = UI_CreateLabel(card, "Waiting for echo", UI_COLOR_MUTED);
    lv_obj_set_width(s_distance_status_label, 176);
    lv_obj_set_style_text_align(s_distance_status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_distance_status_label, LV_ALIGN_BOTTOM_MID, 0, -24);

    page = UI_CreatePage(screen, 3U, "MPU6050", "MOTION SENSOR");
    card = UI_CreateMainCard(page);
    s_motion_state_label = UI_CreateLabel(card, "STARTING", UI_COLOR_BLUE);
    lv_obj_set_style_text_font(s_motion_state_label, &lv_font_montserrat_28, 0);
    lv_obj_align(s_motion_state_label, LV_ALIGN_TOP_MID, 0, 25);
    s_motion_accel_label = UI_CreateLabel(card, "ACC  X --  Y --  Z --", UI_COLOR_TEXT);
    lv_obj_set_pos(s_motion_accel_label, 14, 88);
    s_motion_gyro_label = UI_CreateLabel(card, "GYR  X --  Y --  Z --", UI_COLOR_TEXT);
    lv_obj_set_pos(s_motion_gyro_label, 14, 119);
    label = UI_CreateLabel(card, "mg / deg per second", UI_COLOR_MUTED);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -16);

    page = UI_CreatePage(screen, 4U, "IR REMOTE", "NEC RECEIVER");
    card = UI_CreateMainCard(page);
    label = UI_CreateLabel(card, "LAST COMMAND", UI_COLOR_MUTED);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 18);
    s_remote_command_label = UI_CreateLabel(card, "0x--", UI_COLOR_BLUE);
    lv_obj_set_style_text_font(s_remote_command_label, &lv_font_montserrat_28, 0);
    lv_obj_align(s_remote_command_label, LV_ALIGN_CENTER, 0, -20);
    s_remote_device_label = UI_CreateLabel(card, "DEVICE 0x--", UI_COLOR_TEXT);
    lv_obj_align(s_remote_device_label, LV_ALIGN_CENTER, 0, 22);
    s_remote_hint_label = UI_CreateLabel(card, "POWER: ARM   PLAY: CLEAR", UI_COLOR_MUTED);
    lv_obj_align(s_remote_hint_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    s_current_page = 0U;
    s_pending_navigation = 0;
    s_last_version = UINT32_MAX;
}

/**
 * @brief 执行UI_AppProcessNavigation相关功能。
 * @return None
 */
void UI_AppProcessNavigation(void)
{
    int direction = (int)s_pending_navigation;     /* 读取按钮回调提交的方向。 */
    uint32_t next_page = s_current_page;           /* 默认保持当前页面不变。 */

    s_pending_navigation = 0;

    if(direction > 0 &&
       s_current_page + 1U < UI_PAGE_COUNT) {
        next_page = s_current_page + 1U;
    }
    else if(direction < 0 &&
            s_current_page > 0U) {
        next_page = s_current_page - 1U;
    }

    if(next_page != s_current_page) {
        lv_obj_add_flag(s_pages[s_current_page],
                        LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_pages[next_page],
                           LV_OBJ_FLAG_HIDDEN);
        s_current_page = next_page;
    }
}

/**
 * @brief 执行UI_AppUpdate相关功能。
 * @param snapshot 函数输入或输出参数。
 * @return None
 */
void UI_AppUpdate(const EnvSnapshot_t *snapshot)
{
    char text[96];                                 /* 复用缓冲区格式化各模块数据。 */
    uint32_t state_color;                          /* 保存系统状态强调色。 */

    if(snapshot == NULL ||
       snapshot->version == s_last_version) {
        return;
    }

    s_last_version = snapshot->version;
    state_color = UI_StateColor(snapshot->state);
    UI_SetLabel(s_system_state_label,
                UI_StateText(snapshot->state),
                state_color);
    UI_SetLabel(s_system_armed_label,
                snapshot->armed ? "ARMED" : "DISARMED",
                snapshot->armed ? UI_COLOR_RED : UI_COLOR_MUTED);
    UI_SetLabel(s_system_detail_label,
                snapshot->state == ENV_STATE_ALARM ?
                UI_AlertText(snapshot->alert_reason) :
                "Monitoring all modules",
                snapshot->state == ENV_STATE_ALARM ? UI_COLOR_RED : UI_COLOR_MUTED);
    lv_obj_set_style_border_color(s_system_card,
                                  lv_color_hex(state_color),
                                  0);

    if(snapshot->environment.valid) {
        (void)snprintf(text, sizeof(text), "%d.%d C",
                       snapshot->environment.temperature_x10 / 10,
                       snapshot->environment.temperature_x10 < 0 ?
                       -(snapshot->environment.temperature_x10 % 10) :
                       snapshot->environment.temperature_x10 % 10);
        UI_SetLabel(s_dht_temperature_label, text, UI_COLOR_TEXT);
        (void)snprintf(text, sizeof(text), "%u.%u %%",
                       snapshot->environment.humidity_x10 / 10U,
                       snapshot->environment.humidity_x10 % 10U);
        UI_SetLabel(s_dht_humidity_label, text, UI_COLOR_TEXT);
        UI_SetLabel(s_dht_status_label, "Sensor online", UI_COLOR_GREEN);
    }
    else {
        UI_SetLabel(s_dht_temperature_label, "--.- C", UI_COLOR_MUTED);
        UI_SetLabel(s_dht_humidity_label, "--.- %", UI_COLOR_MUTED);
        (void)snprintf(text, sizeof(text), "Waiting / error %ld",
                       (long)snapshot->environment.read_status);
        UI_SetLabel(s_dht_status_label, text, UI_COLOR_PURPLE);
    }

    if(snapshot->distance.valid) {
        (void)snprintf(text, sizeof(text), "%lu cm",
                       (unsigned long)snapshot->distance.distance_cm);
        UI_SetLabel(s_distance_value_label, text,
                    snapshot->distance.distance_cm <= ENV_DISTANCE_ALARM_CM ?
                    UI_COLOR_RED : UI_COLOR_BLUE);
        UI_SetLabel(s_distance_status_label,
                    snapshot->distance.distance_cm <= ENV_DISTANCE_ALARM_CM ?
                    "Object is inside alarm zone" :
                    "Echo captured without blocking",
                    snapshot->distance.distance_cm <= ENV_DISTANCE_ALARM_CM ?
                    UI_COLOR_RED : UI_COLOR_GREEN);
    }
    else {
        UI_SetLabel(s_distance_value_label, "-- cm", UI_COLOR_MUTED);
        UI_SetLabel(s_distance_status_label,
                    "No echo: check PB1 and divider",
                    UI_COLOR_PURPLE);
    }

    if(snapshot->motion.valid) {
        UI_SetLabel(s_motion_state_label,
                    snapshot->motion.motion_detected ? "MOTION" : "STABLE",
                    snapshot->motion.motion_detected ? UI_COLOR_YELLOW : UI_COLOR_GREEN);
        (void)snprintf(text, sizeof(text), "ACC  X%d  Y%d  Z%d",
                       snapshot->motion.accel_x_mg,
                       snapshot->motion.accel_y_mg,
                       snapshot->motion.accel_z_mg);
        UI_SetLabel(s_motion_accel_label, text, UI_COLOR_TEXT);
        (void)snprintf(text, sizeof(text), "GYR  X%d  Y%d  Z%d",
                       snapshot->motion.gyro_x_dps,
                       snapshot->motion.gyro_y_dps,
                       snapshot->motion.gyro_z_dps);
        UI_SetLabel(s_motion_gyro_label, text, UI_COLOR_TEXT);
    }
    else {
        (void)snprintf(text, sizeof(text), "OFFLINE %ld",
                       (long)snapshot->motion.read_status);
        UI_SetLabel(s_motion_state_label, text, UI_COLOR_PURPLE);
        UI_SetLabel(s_motion_accel_label, "ACC  X --  Y --  Z --", UI_COLOR_MUTED);
        UI_SetLabel(s_motion_gyro_label, "GYR  X --  Y --  Z --", UI_COLOR_MUTED);
    }

    (void)snprintf(text, sizeof(text), "0x%02X",
                   snapshot->ir_key.command);
    UI_SetLabel(s_remote_command_label, text, UI_COLOR_BLUE);
    (void)snprintf(text, sizeof(text), "DEVICE 0x%02X",
                   snapshot->ir_key.device);
    UI_SetLabel(s_remote_device_label, text, UI_COLOR_TEXT);
    UI_SetLabel(s_remote_hint_label,
                snapshot->armed ?
                "ARMED   PLAY: CLEAR" :
                "POWER: ARM   PLAY: CLEAR",
                snapshot->armed ? UI_COLOR_RED : UI_COLOR_MUTED);
}
