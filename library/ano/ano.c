/*
 * Copyright (c) 2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-26     sogwms       The first version
 */

#include "ano.h"

#define DBG_SECTION_NAME  "ano"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define BYTE0(dwTemp) (*((uint8_t *)(&dwTemp)))
#define BYTE1(dwTemp) (*((uint8_t *)(&dwTemp) + 1))
#define BYTE2(dwTemp) (*((uint8_t *)(&dwTemp) + 2))
#define BYTE3(dwTemp) (*((uint8_t *)(&dwTemp) + 3))

#define PID_PARAM_FACTOR       100.0f

// Thread
#define THREAD_STACK_SIZE      512
#define THREAD_PRIORITY        ((RT_THREAD_PRIORITY_MAX / 3) + 2)
#define THREAD_TICK            10

static rt_thread_t tid_ano = RT_NULL;
static rt_device_t dev_ano = RT_NULL;
static rt_sem_t rx_sem = RT_NULL;

static int (*__request_user_pid_params)(struct ano_pid pid[]) = RT_NULL;
static int (*__request_user_version_info)(struct ano_version *version) = RT_NULL;
static int (*__request_user_reset_pid_params)(void) = RT_NULL;
static int (*__request_user_set_pid_params)(struct ano_pid *pid) = RT_NULL;

void ano_register_get_pid_cb(int (*func)(struct ano_pid pid[]))
{
    __request_user_pid_params = func;
}
void ano_register_get_version_cb(int (*func)(struct ano_version *version))
{
    __request_user_version_info = func;
}
void ano_register_reset_pid_cb(int (*func)(void))
{
    __request_user_reset_pid_params = func;
}
void ano_register_set_pid_cb(int (*func)(struct ano_pid *pid))
{
    __request_user_set_pid_params = func;
}

static int _send_data(uint8_t *buffer, uint8_t length)
{
    if (dev_ano != RT_NULL)
    {
        return rt_device_write(dev_ano, 0, buffer, length);
    }

    return RT_ERROR;
}

#define _GET_PID_PARAM(buffer, offset)	(float)((1/PID_PARAM_FACTOR) * ((int16_t)(*(buffer + offset) << 8) | *(buffer + (offset + 1))));

static void _get_pid_param(uint8_t *buffer, float *kpid)
{
    for(int i = 0; i < 9; i++)
    {
        kpid[i] = _GET_PID_PARAM(buffer, (i + 2) * 2); 
    }
}

static void ano_send_check(uint8_t head, uint8_t check_sum)
{
    uint8_t data_to_send[7];

    data_to_send[0] = 0xAA;
    data_to_send[1] = 0xAA;
    data_to_send[2] = 0xEF;
    data_to_send[3] = 2;
    data_to_send[4] = head;
    data_to_send[5] = check_sum;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < 6; i++)
        sum += data_to_send[i];
    data_to_send[6] = sum;

    _send_data(data_to_send, 7);
}

static void ano_parse_frame(uint8_t *buffer, uint8_t length)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < (length - 1); i++)
        sum += *(buffer + i);
    if (!(sum == *(buffer + length - 1)))
        return;
    if (!(*(buffer) == 0xAA && *(buffer + 1) == 0xAF))
        return;

    if (*(buffer + 2) == 0X01)
    {
        if (*(buffer + 4) == 0X01)
        {
            // acc calibrate
        }
        else if (*(buffer + 4) == 0X02)
        {
            // gyro calibrate
        }
        else if (*(buffer + 4) == 0X04)
        {
            // mag calibrate
        }
    }
    else if (*(buffer + 2) == 0X02)
    {
        if (*(buffer + 4) == 0X01)
        {
            // request pid paramters
            struct ano_pid pid[4] = {0};
            int amount = 0;
            if (__request_user_pid_params != RT_NULL)
                amount = __request_user_pid_params(pid);
            for (int i=0; i<amount; i++)
            {
                ano_send_pid(pid[i].group,
                    pid[i].kp1, pid[i].ki1, pid[i].kd1,
                    pid[i].kp2, pid[i].ki2, pid[i].kd2,
                    pid[i].kp3, pid[i].ki3, pid[i].kd3);
            }
        }
        else if (*(buffer + 4) == 0XA0)
        {
            struct ano_version version;
            // request version info 
            if (__request_user_version_info != RT_NULL)
                if (__request_user_version_info(&version) == RT_EOK)
                    ano_send_version(version.hardware_type, 
                        version.hardware_ver, 
                        version.software_ver, 
                        version.protocol_ver, 
                        version.bootloader_ver);
        }
        else if (*(buffer + 4) == 0XA1)
        {
            // request resetting pid parameters 
            if (__request_user_reset_pid_params != RT_NULL)
                __request_user_reset_pid_params();
        }
    }
    else if (*(buffer + 2) == 0X10) //PID1
    {
        float kpid[9];
        _get_pid_param(buffer, kpid);

        if (__request_user_set_pid_params != RT_NULL)
        {
            struct ano_pid anopid;
            anopid.group = *(buffer + 2) - 0x10 + 1;
            anopid.kp1 = kpid[0];
            anopid.ki1 = kpid[1];
            anopid.kd1 = kpid[2];
            anopid.kp2 = kpid[3];
            anopid.ki2 = kpid[4];
            anopid.kd2 = kpid[5];
            anopid.kp3 = kpid[6];
            anopid.ki3 = kpid[7];
            anopid.kd3 = kpid[8];
            __request_user_set_pid_params(&anopid);
        }

        ano_send_check(*(buffer + 2), sum);
    }
    else if (*(buffer + 2) == 0X11) //PID2
    {
        float kpid[9];
        _get_pid_param(buffer, kpid);
        
        if (__request_user_set_pid_params != RT_NULL)
        {
            struct ano_pid anopid;
            anopid.group = *(buffer + 2) - 0x10 + 1;
            anopid.kp1 = kpid[0];
            anopid.ki1 = kpid[1];
            anopid.kd1 = kpid[2];
            anopid.kp2 = kpid[3];
            anopid.ki2 = kpid[4];
            anopid.kd2 = kpid[5];
            anopid.kp3 = kpid[6];
            anopid.ki3 = kpid[7];
            anopid.kd3 = kpid[8];
            __request_user_set_pid_params(&anopid);
        }

        ano_send_check(*(buffer + 2), sum);
    }
    else if (*(buffer + 2) == 0X12) //PID3
    {
        // struct cmd_pid pid;
        // float kpid[9];
        // _get_pid_param(buffer, kpid);
        
        // pid.id = 7;
        // pid.kp = kpid[0];
        // pid.ki = kpid[1];
        // pid.kd = kpid[2];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 8;
        // pid.kp = kpid[3];
        // pid.ki = kpid[4];
        // pid.kd = kpid[5];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 9;
        // pid.kp = kpid[6];
        // pid.ki = kpid[7];
        // pid.kd = kpid[8];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));

        ano_send_check(*(buffer + 2), sum);
    }
    else if (*(buffer + 2) == 0X13) //PID4
    {
        // struct cmd_pid pid;
        // float kpid[9];
        // _get_pid_param(buffer, kpid);
        
        // pid.id = 10;
        // pid.kp = kpid[0];
        // pid.ki = kpid[1];
        // pid.kd = kpid[2];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 11;
        // pid.kp = kpid[3];
        // pid.ki = kpid[4];
        // pid.kd = kpid[5];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 12;
        // pid.kp = kpid[6];
        // pid.ki = kpid[7];
        // pid.kd = kpid[8];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));

        ano_send_check(*(buffer + 2), sum);
    }
    else if (*(buffer + 2) == 0X14) //PID5
    {
        // struct cmd_pid pid;
        // float kpid[9];
        // _get_pid_param(buffer, kpid);
        
        // pid.id = 13;
        // pid.kp = kpid[0];
        // pid.ki = kpid[1];
        // pid.kd = kpid[2];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 14;
        // pid.kp = kpid[3];
        // pid.ki = kpid[4];
        // pid.kd = kpid[5];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 15;
        // pid.kp = kpid[6];
        // pid.ki = kpid[7];
        // pid.kd = kpid[8];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));

        ano_send_check(*(buffer + 2), sum);
    }
    else if (*(buffer + 2) == 0X15) //PID6
    {
        // struct cmd_pid pid;
        // float kpid[9];
        // _get_pid_param(buffer, kpid);
        
        // pid.id = 16;
        // pid.kp = kpid[0];
        // pid.ki = kpid[1];
        // pid.kd = kpid[2];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 17;
        // pid.kp = kpid[3];
        // pid.ki = kpid[4];
        // pid.kd = kpid[5];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));
        // pid.id = 18;
        // pid.kp = kpid[6];
        // pid.ki = kpid[7];
        // pid.kd = kpid[8];
        // command_handle(COMMAND_SET_PID, &pid, sizeof(struct cmd_pid));

        ano_send_check(*(buffer + 2), sum);
    }
}

static int ano_receive_byte(uint8_t data)
{    
    static uint8_t RxBuffer[50];
    static uint8_t _data_len = 0, _data_cnt = 0;
    static uint8_t state = 0;

    if (state == 0 && data == 0xAA)
    {
        state = 1;
        RxBuffer[0] = data;
    }
    else if (state == 1 && data == 0xAF)
    {
        state = 2;
        RxBuffer[1] = data;
    }
    else if (state == 2 && data < 0XF1)
    {
        state = 3;
        RxBuffer[2] = data;
    }
    else if (state == 3 && data < 50)
    {
        state = 4;
        RxBuffer[3] = data;
        _data_len = data;
        _data_cnt = 0;
    }
    else if (state == 4 && _data_len > 0)
    {
        _data_len--;
        RxBuffer[4 + _data_cnt++] = data;
        if (_data_len == 0)
            state = 5;
    }
    else if (state == 5)
    {
        state = 0;
        RxBuffer[4 + _data_cnt] = data;
        ano_parse_frame(RxBuffer, _data_cnt + 5);
        return 1;
    }
    else
        state = 0;
    
    return 0;
}

int ano_send_version(uint8_t hardware_type, uint16_t hardware_ver, uint16_t software_ver, uint16_t protocol_ver, uint16_t bootloader_ver)
{
    uint8_t data_to_send[14];
    uint8_t _cnt = 0;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0;

    data_to_send[_cnt++] = hardware_type;
    data_to_send[_cnt++] = BYTE1(hardware_ver);
    data_to_send[_cnt++] = BYTE0(hardware_ver);
    data_to_send[_cnt++] = BYTE1(software_ver);
    data_to_send[_cnt++] = BYTE0(software_ver);
    data_to_send[_cnt++] = BYTE1(protocol_ver);
    data_to_send[_cnt++] = BYTE0(protocol_ver);
    data_to_send[_cnt++] = BYTE1(bootloader_ver);
    data_to_send[_cnt++] = BYTE0(bootloader_ver);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_status(float angle_rol, float angle_pit, float angle_yaw, int32_t alt, uint8_t fly_model, uint8_t armed)
{
    uint8_t data_to_send[17];
    uint8_t _cnt = 0;
    volatile int16_t _temp;
    volatile int32_t _temp2 = alt;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x01;
    data_to_send[_cnt++] = 0;

    _temp = (int)(angle_rol * 100);
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = (int)(angle_pit * 100);
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = (int)(angle_yaw * 100);
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);

    data_to_send[_cnt++] = BYTE3(_temp2);
    data_to_send[_cnt++] = BYTE2(_temp2);
    data_to_send[_cnt++] = BYTE1(_temp2);
    data_to_send[_cnt++] = BYTE0(_temp2);

    data_to_send[_cnt++] = fly_model;

    data_to_send[_cnt++] = armed;

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_senser(int16_t a_x, int16_t a_y, int16_t a_z, int16_t g_x, int16_t g_y, int16_t g_z, int16_t m_x, int16_t m_y, int16_t m_z, int32_t bar)
{
    uint8_t data_to_send[23];
    uint8_t _cnt = 0;
    volatile int16_t _temp;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x02;
    data_to_send[_cnt++] = 0;

    _temp = a_x;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = a_y;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = a_z;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);

    _temp = g_x;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = g_y;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = g_z;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);

    _temp = m_x;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = m_y;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = m_z;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_rcdata(uint16_t thr, uint16_t yaw, uint16_t rol, uint16_t pit, uint16_t aux1, uint16_t aux2, uint16_t aux3, uint16_t aux4, uint16_t aux5, uint16_t aux6)
{
    uint8_t data_to_send[25];
    uint8_t _cnt = 0;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x03;
    data_to_send[_cnt++] = 0;
    data_to_send[_cnt++] = BYTE1(thr);
    data_to_send[_cnt++] = BYTE0(thr);
    data_to_send[_cnt++] = BYTE1(yaw);
    data_to_send[_cnt++] = BYTE0(yaw);
    data_to_send[_cnt++] = BYTE1(rol);
    data_to_send[_cnt++] = BYTE0(rol);
    data_to_send[_cnt++] = BYTE1(pit);
    data_to_send[_cnt++] = BYTE0(pit);
    data_to_send[_cnt++] = BYTE1(aux1);
    data_to_send[_cnt++] = BYTE0(aux1);
    data_to_send[_cnt++] = BYTE1(aux2);
    data_to_send[_cnt++] = BYTE0(aux2);
    data_to_send[_cnt++] = BYTE1(aux3);
    data_to_send[_cnt++] = BYTE0(aux3);
    data_to_send[_cnt++] = BYTE1(aux4);
    data_to_send[_cnt++] = BYTE0(aux4);
    data_to_send[_cnt++] = BYTE1(aux5);
    data_to_send[_cnt++] = BYTE0(aux5);
    data_to_send[_cnt++] = BYTE1(aux6);
    data_to_send[_cnt++] = BYTE0(aux6);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];

    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_power(uint16_t votage, uint16_t current)
{
    uint8_t data_to_send[9];
    uint8_t _cnt = 0;
    uint16_t temp;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x05;
    data_to_send[_cnt++] = 0;

    temp = votage;
    data_to_send[_cnt++] = BYTE1(temp);
    data_to_send[_cnt++] = BYTE0(temp);
    temp = current;
    data_to_send[_cnt++] = BYTE1(temp);
    data_to_send[_cnt++] = BYTE0(temp);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];

    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_motorpwm(uint16_t m_1, uint16_t m_2, uint16_t m_3, uint16_t m_4, uint16_t m_5, uint16_t m_6, uint16_t m_7, uint16_t m_8)
{
    uint8_t data_to_send[21];
    uint8_t _cnt = 0;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x06;
    data_to_send[_cnt++] = 0;

    data_to_send[_cnt++] = BYTE1(m_1);
    data_to_send[_cnt++] = BYTE0(m_1);
    data_to_send[_cnt++] = BYTE1(m_2);
    data_to_send[_cnt++] = BYTE0(m_2);
    data_to_send[_cnt++] = BYTE1(m_3);
    data_to_send[_cnt++] = BYTE0(m_3);
    data_to_send[_cnt++] = BYTE1(m_4);
    data_to_send[_cnt++] = BYTE0(m_4);
    data_to_send[_cnt++] = BYTE1(m_5);
    data_to_send[_cnt++] = BYTE0(m_5);
    data_to_send[_cnt++] = BYTE1(m_6);
    data_to_send[_cnt++] = BYTE0(m_6);
    data_to_send[_cnt++] = BYTE1(m_7);
    data_to_send[_cnt++] = BYTE0(m_7);
    data_to_send[_cnt++] = BYTE1(m_8);
    data_to_send[_cnt++] = BYTE0(m_8);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];

    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_pid(uint8_t group, float k1_p, float k1_i, float k1_d, float k2_p, float k2_i, float k2_d, float k3_p, float k3_i, float k3_d)
{
    uint8_t data_to_send[23];
    uint8_t _cnt = 0;
    volatile int16_t _temp;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0x10 + group - 1;
    data_to_send[_cnt++] = 0;

    _temp = k1_p * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k1_i * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k1_d * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k2_p * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k2_i * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k2_d * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k3_p * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k3_i * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);
    _temp = k3_d * PID_PARAM_FACTOR;
    data_to_send[_cnt++] = BYTE1(_temp);
    data_to_send[_cnt++] = BYTE0(_temp);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];

    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

int ano_send_user_data(uint8_t number, float d0, float d1, float d2, float d3, float d4, float d5, int16_t d6, int16_t d7, int16_t d8)
{
    uint8_t data_to_send[35];
    uint8_t _cnt = 0;

    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xAA;
    data_to_send[_cnt++] = 0xF0 + number;
    data_to_send[_cnt++] = 0;

    data_to_send[_cnt++] = BYTE3(d0);
    data_to_send[_cnt++] = BYTE2(d0);
    data_to_send[_cnt++] = BYTE1(d0);
    data_to_send[_cnt++] = BYTE0(d0);

    data_to_send[_cnt++] = BYTE3(d1);
    data_to_send[_cnt++] = BYTE2(d1);
    data_to_send[_cnt++] = BYTE1(d1);
    data_to_send[_cnt++] = BYTE0(d1);

    data_to_send[_cnt++] = BYTE3(d2);
    data_to_send[_cnt++] = BYTE2(d2);
    data_to_send[_cnt++] = BYTE1(d2);
    data_to_send[_cnt++] = BYTE0(d2);

    data_to_send[_cnt++] = BYTE3(d3);
    data_to_send[_cnt++] = BYTE2(d3);
    data_to_send[_cnt++] = BYTE1(d3);
    data_to_send[_cnt++] = BYTE0(d3);

    data_to_send[_cnt++] = BYTE3(d4);
    data_to_send[_cnt++] = BYTE2(d4);
    data_to_send[_cnt++] = BYTE1(d4);
    data_to_send[_cnt++] = BYTE0(d4);

    data_to_send[_cnt++] = BYTE3(d5);
    data_to_send[_cnt++] = BYTE2(d5);
    data_to_send[_cnt++] = BYTE1(d5);
    data_to_send[_cnt++] = BYTE0(d5);

    data_to_send[_cnt++] = BYTE1(d6);
    data_to_send[_cnt++] = BYTE0(d6);
    data_to_send[_cnt++] = BYTE1(d7);
    data_to_send[_cnt++] = BYTE0(d7);
    data_to_send[_cnt++] = BYTE1(d8);
    data_to_send[_cnt++] = BYTE0(d8);

    data_to_send[3] = _cnt - 4;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < _cnt; i++)
        sum += data_to_send[i];

    data_to_send[_cnt++] = sum;

    return _send_data(data_to_send, _cnt);
}

static uint8_t ano_getbyte(void)
{
    uint8_t tmp;
    
    while (rt_device_read(dev_ano, -1, &tmp, 1) != 1)
        rt_sem_take(rx_sem, RT_WAITING_FOREVER);

    return tmp;
}

static rt_err_t ano_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(rx_sem);

    return RT_EOK;
}

int ano_set_device(const char *device_name)
{
    rt_device_t dev = RT_NULL;

    dev = rt_device_find(device_name);
    if (dev == RT_NULL)
    {
        LOG_E("Can not find device: %s\n", device_name);
        return RT_ERROR;
    }

    /* check whether it's a same device */
    if (dev == dev_ano) return RT_ERROR;
    /* open this device and set the new device in finsh shell */
    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)
    {
        if (dev_ano != RT_NULL)
        {
            /* close old finsh device */
            rt_device_close(dev_ano);
            rt_device_set_rx_indicate(dev_ano, RT_NULL);
        }

        dev_ano = dev;
        rt_device_set_rx_indicate(dev_ano, ano_rx_ind);
    }

    return RT_EOK;
}

static void ano_thread_entry(void *param)
{
    while(1)
    {
        ano_receive_byte(ano_getbyte());
    }
}

int ano_init(void *param)
{
    if (ano_set_device((char *)param) != RT_EOK)
    {
        LOG_E("Failed to find device");
        return RT_ERROR;
    }

    rx_sem = rt_sem_create("anoRx", 0, RT_IPC_FLAG_FIFO);
    if (rx_sem == RT_NULL)
    {
        LOG_E("Failed to create sem\n");
        return RT_ERROR;
    }

    tid_ano = rt_thread_create("ano", ano_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TICK);
    if (tid_ano == RT_NULL)
    {
        LOG_E("Failed to create thread\n");
        return RT_ERROR;
    }

    rt_thread_startup(tid_ano);

    LOG_D("ano thread started");

    return RT_EOK;
}
