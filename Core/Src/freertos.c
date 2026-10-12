/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_runtime.h"
#include "app_config.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* configSUPPORT_STATIC_ALLOCATION=1时，FreeRTOS要求应用提供Idle和
 * Timer任务的静态TCB与栈。CMSIS包装层曾隐式提供它们；改用原生API后
 * 由本文件显式提供，避免依赖cmsis_os2.c。 */
static StaticTask_t s_idle_task_tcb;
static StackType_t s_idle_task_stack[configMINIMAL_STACK_SIZE];

#if (configUSE_TIMERS == 1)
static StaticTask_t s_timer_task_tcb;
static StackType_t s_timer_task_stack[configTIMER_TASK_STACK_DEPTH];
#endif

volatile TaskHandle_t g_stack_overflow_task;
volatile uint32_t g_freertos_malloc_failed;

/* USER CODE END Variables */
/* Definitions for UiTask */
TaskHandle_t defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the task(s) */
  /* USER CODE BEGIN RTOS_THREADS */
  /* 先创建应用队列、ControlTask和SensorTask。 */
  if (App_RuntimeInit() != 0)
  {
    Error_Handler();
  }

  /* CubeMX原defaultTask现在作为UiTask运行。
   * xTaskCreate()的栈参数单位为StackType_t数量，不是字节。 */
  if (xTaskCreate(StartDefaultTask,
                  "UiTask",
                  APP_UI_TASK_STACK_WORDS,
                  NULL,
                  tskIDLE_PRIORITY + 1U,
                  &defaultTaskHandle) != pdPASS)
  {
    Error_Handler();
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  (void)argument;

  /* CubeMX生成的defaultTask现在作为唯一UiTask运行。 */
  App_UiTaskRun();
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void vApplicationGetIdleTaskMemory(StaticTask_t **idle_tcb,
                                   StackType_t **idle_stack,
                                   uint32_t *idle_stack_size)
{
  *idle_tcb = &s_idle_task_tcb;
  *idle_stack = s_idle_task_stack;
  *idle_stack_size = (uint32_t)configMINIMAL_STACK_SIZE;
}

#if (configUSE_TIMERS == 1)
void vApplicationGetTimerTaskMemory(StaticTask_t **timer_tcb,
                                    StackType_t **timer_stack,
                                    uint32_t *timer_stack_size)
{
  *timer_tcb = &s_timer_task_tcb;
  *timer_stack = s_timer_task_stack;
  *timer_stack_size = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}
#endif

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
  (void)task_name;
  g_stack_overflow_task = task;
  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}

void vApplicationMallocFailedHook(void)
{
  g_freertos_malloc_failed++;
  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}

/* USER CODE END Application */

