/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-01     sogwms       The first version
 */

#include "my_motor_speed.h"
#include <rtdevice.h>
#include <drv_config.h>

#define DBG_SECTION_NAME  "my_motor_speed"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define DEFAULT_SAMPLE_TIME     5

#define COUNTER_MIDDLE_VALUE    0x7FFF

static struct ev_motor_speed motor_speed_obj;

static rt_thread_t g_thread = RT_NULL;
static rt_int32_t  g_sample_time = DEFAULT_SAMPLE_TIME;
static rt_uint32_t g_maximum_speed[2];
static struct msg_motor g_mmotor = {0};

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);

static void thread_entry(void *param)
{
    rt_int32_t e1, e2;

    while (1)
    {
        rt_thread_mdelay(g_sample_time);
        e2 = __HAL_TIM_GET_COUNTER(&htim2) - COUNTER_MIDDLE_VALUE;
        e1 = __HAL_TIM_GET_COUNTER(&htim3) - COUNTER_MIDDLE_VALUE;
        __HAL_TIM_SET_COUNTER(&htim3, COUNTER_MIDDLE_VALUE);
        __HAL_TIM_SET_COUNTER(&htim2, COUNTER_MIDDLE_VALUE);
        
        e1 = e1 * ((float)EV_SCALE / g_maximum_speed[0]);
        e2 = e2 * ((float)EV_SCALE / g_maximum_speed[1]);
        if (e1 > EV_SCALE)
          e1 = EV_SCALE;
        else if (e1 < -EV_SCALE)
          e1 = -EV_SCALE;
        if (e2 > EV_SCALE)
          e2 = EV_SCALE;
        else if (e2 < -EV_SCALE)
          e2 = -EV_SCALE;

        g_mmotor.m1 = -e1;
        g_mmotor.m2 = e2;
        motor_speed_obj.up_notify(&motor_speed_obj);
    }
}

static void get_motor_speed(struct msg_motor *mmotor)
{
    mmotor->m1 = g_mmotor.m1;
    mmotor->m2 = g_mmotor.m2;
}

static const struct ev_motor_speed_ops _ops = {
    .get_motor_speed = get_motor_speed,
};

int my_motor_speed_install(int sample_time_ms, rt_uint32_t m1_maximum_speed, rt_uint32_t m2_maximum_speed, char *thread_name, rt_uint32_t stack_size, rt_uint8_t thread_priority)
{
    if (sample_time_ms != 0)
    {
        g_sample_time = sample_time_ms;
    }
    g_maximum_speed[0] = m1_maximum_speed;
    g_maximum_speed[1] = m2_maximum_speed;
    
    MX_TIM2_Init();
    MX_TIM3_Init();
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    motor_speed_obj.ops = &_ops;
    EV_CHECK_EOK_RVL(ev_motor_speed_install(&motor_speed_obj));

    g_thread = rt_thread_create(thread_name, thread_entry, RT_NULL, 256, thread_priority, 20);
    if (g_thread == RT_NULL)
    {
        LOG_E("Failed to create thread");
        return RT_ERROR;
    }
    rt_thread_startup(g_thread);

    return RT_EOK;
}

static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 3;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 3;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0xFFFF;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}
