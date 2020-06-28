#ifndef __FREERTOS_H__
#define __FREERTOS_H__

#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portable.h"
#include "list.h"

typedef struct tskTaskControlBlock
{
    volatile StackType_t *pxTopOfStack; /* ջ��������8�ֽڶ��� */
    ListItem_t           xStateListItem; /* ����ڵ� */
    StackType_t          *pxStack; /* ����ջ��ʼ��ַ */
    char pcTaskName[configMAX_TASK_NAME_LEN]; /* ����ջ���� */
    TickType_t xTicksToDelay; /* ������������ʱ */
}tskTCB;

typedef tskTCB TCB_t;

#endif /* __FREERTOS_H__ */
