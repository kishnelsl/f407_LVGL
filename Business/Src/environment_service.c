#include "environment_service.h"

#include <stddef.h>
#include <string.h>

/**
 * @brief 根据最新采样和布防状态重新计算系统状态。
 * @param service 函数输入或输出参数。
 * @return None
 */
static void EnvironmentService_Recalculate(
    EnvironmentService_t *service)                    /* service是要被更新的业务对象。 */
{
    bool sensor_error = false;                        /* sensor_error记录核心传感器是否异常。 */
    bool startup = !service->has_environment_sample || /* startup表示核心传感器是否还未完成首次上报。 */
                   !service->has_motion_sample;
    bool risk_detected = false;                       /* risk_detected记录是否检测到安全风险。 */
    EnvAlertReason_t reason = ENV_ALERT_NONE;         /* reason保存优先级最高的报警原因。 */

    if(service->has_environment_sample &&
       !service->snapshot.environment.valid) {
        sensor_error = true;
    }
    if(service->has_motion_sample &&
       !service->snapshot.motion.valid) {
        sensor_error = true;
    }

    if(service->has_distance_sample &&
       service->snapshot.distance.valid &&
       service->snapshot.distance.distance_cm <= ENV_DISTANCE_ALARM_CM) {
        risk_detected = true;
        reason = ENV_ALERT_DISTANCE;
    }
    else if(service->has_motion_sample &&
            service->snapshot.motion.motion_detected) {
        risk_detected = true;
        reason = ENV_ALERT_MOTION;
    }

    if(service->alarm_latched) {
        service->snapshot.state = ENV_STATE_ALARM;
    }
    else if(startup) {
        service->snapshot.state = ENV_STATE_STARTUP;
    }
    else if(sensor_error) {
        service->snapshot.state = ENV_STATE_SENSOR_ERROR;
    }
    else if(risk_detected && service->snapshot.armed) {
        service->alarm_latched = true;
        service->snapshot.alert_reason = reason;
        service->snapshot.state = ENV_STATE_ALARM;
    }
    else if(risk_detected) {
        service->snapshot.state = ENV_STATE_WARNING;
    }
    else {
        service->snapshot.state = ENV_STATE_NORMAL;
        service->snapshot.alert_reason = ENV_ALERT_NONE;
    }

    if(service->snapshot.state == ENV_STATE_ALARM) {
        service->snapshot.indicator.color_rgb888 = 0xFF2020U;
        service->snapshot.indicator.blink = true;
    }
    else if(service->snapshot.state == ENV_STATE_WARNING) {
        service->snapshot.indicator.color_rgb888 = 0xFFB020U;
        service->snapshot.indicator.blink = false;
    }
    else if(service->snapshot.state == ENV_STATE_SENSOR_ERROR) {
        service->snapshot.indicator.color_rgb888 = 0xA855F7U;
        service->snapshot.indicator.blink = false;
    }
    else if(service->snapshot.state == ENV_STATE_STARTUP) {
        service->snapshot.indicator.color_rgb888 = 0x2490FFU;
        service->snapshot.indicator.blink = false;
    }
    else {
        service->snapshot.indicator.color_rgb888 = 0x35D07FU;
        service->snapshot.indicator.blink = false;
    }
}

/**
 * @brief 初始化环境监测业务状态。
 * @param service 函数输入或输出参数。
 * @return None
 */
void EnvironmentService_Init(EnvironmentService_t *service)
{
    if(service == NULL) {
        return;
    }

    memset(service, 0, sizeof(*service));
    service->snapshot.state = ENV_STATE_STARTUP;
    service->snapshot.alert_reason = ENV_ALERT_NONE;
    service->snapshot.indicator.color_rgb888 = 0x2490FFU;
    service->snapshot.indicator.blink = false;
}

/**
 * @brief 应用一份温湿度采样结果。
 * @param service 函数输入或输出参数。
 * @param sample 函数输入或输出参数。
 * @return None
 */
void EnvironmentService_ApplyEnvironmentSample(
    EnvironmentService_t *service,                      /* service是要修改的业务对象。 */
    const EnvSample_t *sample)                          /* sample是传入的DHT11数据。 */
{
    if(service == NULL || sample == NULL) {
        return;
    }

    service->snapshot.environment = *sample;
    service->has_environment_sample = true;
    service->snapshot.version++;
    EnvironmentService_Recalculate(service);
}

/**
 * @brief 应用一份运动传感器采样结果。
 * @param service 函数输入或输出参数。
 * @param sample 函数输入或输出参数。
 * @return None
 */
void EnvironmentService_ApplyMotionSample(
    EnvironmentService_t *service,                       /* service是要修改的业务对象。 */
    const EnvMotionSample_t *sample)                    /* sample是传入的MPU6050数据。 */
{
    if(service == NULL || sample == NULL) {
        return;
    }

    service->snapshot.motion = *sample;
    service->has_motion_sample = true;
    service->snapshot.version++;
    EnvironmentService_Recalculate(service);
}

/**
 * @brief 应用一份超声波测距结果。
 * @param service 函数输入或输出参数。
 * @param sample 函数输入或输出参数。
 * @return None
 */
void EnvironmentService_ApplyDistanceSample(
    EnvironmentService_t *service,                       /* service是要修改的业务对象。 */
    const EnvDistanceSample_t *sample)                  /* sample是传入的距离数据。 */
{
    if(service == NULL || sample == NULL) {
        return;
    }

    service->snapshot.distance = *sample;
    service->has_distance_sample = true;
    service->snapshot.version++;
    EnvironmentService_Recalculate(service);
}

/**
 * @brief 应用一条红外遥控按键命令。
 * @param service 函数输入或输出参数。
 * @param key 函数输入或输出参数。
 * @return None
 */
void EnvironmentService_ApplyIrKey(
    EnvironmentService_t *service,                       /* service是要修改的业务对象。 */
    const EnvIrKey_t *key)                               /* key是输入任务解码出的按键。 */
{
    if(service == NULL || key == NULL) {
        return;
    }

    service->snapshot.ir_key = *key;

    if(key->command == ENV_IR_CMD_POWER) {
        if(service->snapshot.armed) {
            service->snapshot.armed = false;
            service->alarm_latched = false;
            service->snapshot.alert_reason = ENV_ALERT_NONE;
        }
        else {
            service->snapshot.armed = true;
        }
    }
    else if(key->command == ENV_IR_CMD_PLAY_PAUSE) {
        service->alarm_latched = false;
        service->snapshot.alert_reason = ENV_ALERT_NONE;
    }

    service->snapshot.version++;
    EnvironmentService_Recalculate(service);
}

const EnvSnapshot_t *EnvironmentService_GetSnapshot(    /* 返回当前业务快照的只读地址。 */
    const EnvironmentService_t *service)                /* service是要读取的业务对象。 */
{
    if(service == NULL) {
        return NULL;
    }

    return &service->snapshot;
}
