#ifndef __FREERTOS_H__
#define __FREERTOS_H__

#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portable.h"
#include "list.h"

typedef struct tskTaskControlBlock
{
    volatile StackType_t *pxTopOfStack; /* 栈顶，向下8字节对齐 */
    ListItem_t           xStateListItem; /* 任务节点 */
    StackType_t          *pxStack; /* 任务栈起始地址 */
    char pcTaskName[configMAX_TASK_NAME_LEN]; /* 任务栈名称 */
    TickType_t xTicksToDelay; /* 用于阻塞性延时 */
    UBaseType_t uxPriority; /* 任务优先级，数值越大，优先级越高，对应在全局 pxReadyTaskLists[] 链表数组的下标也越大 */
}tskTCB;

typedef tskTCB TCB_t;

#endif /* __FREERTOS_H__ */
