#ifndef __TASK_H__
#define __TASK_H__

//#include "list.h"

#define taskYIELD()    portYIELD()

/* ÈÎÎñ¾ä±ú */
typedef void* TaskHandle_t;

#if(configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void* const pvParameters,
                               StackType_t* const puxStakBuffer,
                               TCB_t* const pxTaskBLock);                                                          
#endif /* configSUPPORT_STATIC_ALLOCATION */

void prvInitialiseTaskLists(void);
void vTaskStartScheduler(void);
void vTaskSwitchContext(void);

#endif /* __TASK_H__ */
