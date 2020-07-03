#ifndef __TASK_H__
#define __TASK_H__

#include "list.h"

#define taskIDLE_PRIORITY                   ((UBaseType_t)0U)
#define taskYIELD()                         portYIELD()

#define taskENTER_CRITICAL()                portENTER_CRITICAL()
#define taskENTER_CRITICAL_FROM_ISR()       portSET_INTERRUPT_MASK_FROM_ISR()

#define taskEXIT_CRITICAL()                 portEXIT_CRITICAL()
#define taskEXIT_CRITICAL_FROM_ISR(x)       portCLEAR_INTERRUPT_MASK_FROM_ISR(x)


/* ÈÎÎñ¾ä±ú */
typedef void* TaskHandle_t;

#if(configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void* const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t* const puxStakBuffer,
                               TCB_t* const pxTaskBLock);                                                          
#endif /* configSUPPORT_STATIC_ALLOCATION */

void prvInitialiseTaskLists(void);
void vTaskStartScheduler(void);
void vTaskSwitchContext(void);
void vTaskDelay(const TickType_t xTicksToDelay);
void xTaskIncrementTick(void);

#endif /* __TASK_H__ */
