#include "FreeRTOS.h"
#include "task.h"

/*
*************************************************************************
*                               ������ƿ�
*************************************************************************
*/
/* ��ǰ�������е�����Ŀ��ƿ� */
TCB_t * volatile pxCurrentTCB = NULL;

/* ��������б� */
List_t pxReadyTaskLists[configMAX_PRIORITIES];

static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;

/*
*************************************************************************
*                               ��������
*************************************************************************
*/
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,          /* ������� */
                                 const char * const pcName,          /* �������� */
                                 const uint32_t ulStackDepth,        /* ����ջ��С����λ�ֽ� */
                                 void * const pvParameters,          /* �����β� */
                                 TaskHandle_t * const pxCreatedTask, /* �����ɹ��󷵻ص������� */
                                 TCB_t *pxNewTCB);
/*
*************************************************************************
*                               �궨��
*************************************************************************
*/


/*
*************************************************************************
*                               ��̬���񴴽�����
*************************************************************************
*/
#if(configSUPPORT_STATIC_ALLOCTION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,          /* ������� */
                               const char *const pcName,           /* �������ƣ��ַ�����ʽ */
                               const uint32_t ulStackDepth,        /* ����ջ��С����Ӧ��ջָ���ƶ� 4 * ulStackDepth ���ֽ� */
                               void* const pvParameters,           /* �����β� */
                               StackType_t* const puxStakBuffer,   /* ����ջ��ʼ��ַ */
                               TCB_t* const pxTaskBLock)          /* ������ƿ�ָ�� */
{
    TCB_t *pxNewTCB = NULL;
    TaskHandle_t xReturn;
    
    if((pxTaskBLock != NULL) && (puxStakBuffer != NULL))
    {
        pxNewTCB = (TCB_t *)pxTaskBLock;
        /* ������ƿ��е�ջ��ַ pxStack ���������������ջ��ʼ��ַ puxStakBuffer */
        pxNewTCB->pxStack = (StackType_t *)puxStakBuffer;
        /* �����µ����� */
        prvInitialiseNewTask(pxTaskCode,
                             pcName,
                             ulStackDepth,
                             pvParameters,
                             &xReturn,     /* ָ���ָ�� */
                             pxNewTCB);
    }
    else
    {
        xReturn = NULL;
    }
    
    /* ������������������񴴽��ɹ�����ʱxReturnӦ��ָ��������ƿ� */
    return xReturn;
}                                   
#endif /* configSUPPORT_STATIC_ALLOCTION */                              
                                 
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,          /* ������� */
                                 const char * const pcName,          /* �������� */
                                 const uint32_t ulStackDepth,        /* ����ջ��С����Ӧ��ջָ���ƶ� 4 * ulStackDepth ���ֽ� */
                                 void * const pvParameters,          /* �����β� */
                                 TaskHandle_t * const pxCreatedTask, /* �����ɹ��󷵻ص���������ָ���ָ�룬
                                                                        ���ĵ�ַ��������ʼ��֮��pxNewTCB���ƿ�ĵ�ַ����ʼ��ʧ����ָ��NULL*/
                                 TCB_t *pxNewTCB)
{
    StackType_t *pxTopOfStack = NULL;
    UBaseType_t x;
    
    /* ��ȡջ����ַ */
    pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
    /* ������8�ֽڶ���*/
    pxTopOfStack = (StackType_t *)(((uint32_t)pxTopOfStack) & (~((uint32_t)0x7)));
    
    /* ����������ִ洢��TCB�� */
    for(x = (UBaseType_t)0; x < (UBaseType_t)configMAX_TASK_NAME_LEN; x++)
    {
        pxNewTCB->pcTaskName[x] = pcName[x];
        if(pcName[x] == '\0')
        {
            break;
        }
    }
    /* �������ֵĳ��Ȳ��ܳ���configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
    
    /*
     * ��ʼ��TCB�е�xStateListItem�ڵ㣬
     * ���� xStateListItem->pvContainer �� NULL����ʾ��δ�����κ�������
     * ���ں����� insert �����У��� vListInsertEnd ���;������������
    */
    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    /* ����xStateListItem�ڵ��ӵ���� */
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);
    
    /* ��ʼ������ջ����ʱ pxNewTCB->pxTopOfStack ָ�����ջ */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);
    
    /* ��������ָ��������ƿ� */
    if((void *)pxCreatedTask != NULL)
    {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB;
    }
}    
   
/* ��ʼ��������ص��б� */
void prvInitialiseTaskLists(void)
{
    UBaseType_t uxPriority;
    for(uxPriority = (UBaseType_t)0U; uxPriority < (UBaseType_t)configMAX_PRIORITIES; uxPriority++)
    {
        vListInitialise(&(pxReadyTaskLists[uxPriority]));
    }
}

/* ����������� */
extern TCB_t Task1_TCB;
extern TCB_t Task2_TCB;

void vTaskStartScheduler(void)
{
    /* �ֶ�ָ����һ�����е����� */
    pxCurrentTCB = &Task1_TCB;
    
    /* ���������� */
    if(xPortStartScheduler() != pdFALSE)
    {
        /* �����������ɹ����򲻻᷵�ص����� */
    }
}

/* �����л� */
void vTaskSwitchContext(void)
{
    /* �������������л� */
    if(pxCurrentTCB == &Task1_TCB)
    {
        pxCurrentTCB = &Task2_TCB;
    }
    else
    {
        pxCurrentTCB = &Task1_TCB;
    }
}













