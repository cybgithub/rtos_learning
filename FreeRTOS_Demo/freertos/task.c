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
List_t pxReadyTasksLists[configMAX_PRIORITIES];

static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;
static volatile TickType_t xTickCount              = (TickType_t)0U;
static volatile UBaseType_t uxTopReadyPriority     = taskIDLE_PRIORITY;
static UBaseType_t uxTaskNumber                    = (UBaseType_t)0U;

/*
*************************************************************************
*                               ��������
*************************************************************************
*/
static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB);
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,          /* ������� */
                                 const char * const pcName,          /* �������� */
                                 const uint32_t ulStackDepth,        /* ����ջ��С����λ�ֽ� */
                                 void * const pvParameters,          /* �����β� */
                                 UBaseType_t uxPriority,             /* �������ȼ� */
                                 TaskHandle_t * const pxCreatedTask, /* �����ɹ��󷵻ص������� */
                                 TCB_t *pxNewTCB);
/*
*************************************************************************
*                               �궨��
*************************************************************************
*/
/* ��������ӵ������б� */
#define prvAddTaskToReadyList( pxTCB )																   \
	taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												   \
	vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) ); \

/* ����������ȼ��ľ�������ͨ�÷��� */
#if ( configUSE_PORT_OPTIMISED_TASK_SELECTION == 0 )
	/* uxTopReadyPriority ����Ǿ��������������ȼ� */
	#define taskRECORD_READY_PRIORITY( uxPriority )														\
	{																									\
		if( ( uxPriority ) > uxTopReadyPriority )														\
		{																								\
			uxTopReadyPriority = ( uxPriority );														\
		}																								\
	} /* taskRECORD_READY_PRIORITY */

	/*-----------------------------------------------------------*/

	#define taskSELECT_HIGHEST_PRIORITY_TASK()															\
	{																									\
	UBaseType_t uxTopPriority = uxTopReadyPriority;														\
																										\
		/* Ѱ�Ұ������������������ȼ��Ķ��� */                                                          \
		while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )							\
		{																								\
			--uxTopPriority;																			\
		}																								\
																										\
		/* ��ȡ���ȼ���ߵľ��������TCB��Ȼ����µ�pxCurrentTCB */							            \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );			\
		/* ����uxTopReadyPriority */                                                                    \
		uxTopReadyPriority = uxTopPriority;																\
	} /* taskSELECT_HIGHEST_PRIORITY_TASK */

	/*-----------------------------------------------------------*/

	/* �������궨��ֻ����ѡ���Ż�����ʱ���ã����ﶨ��Ϊ�� */
	#define taskRESET_READY_PRIORITY( uxPriority )
	#define portRESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )

/* ����������ȼ��ľ������񣺸��ݴ������ܹ��Ż���ķ��� */
#else /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

	#define taskRECORD_READY_PRIORITY( uxPriority )	portRECORD_READY_PRIORITY( uxPriority, uxTopReadyPriority )

	/*-----------------------------------------------------------*/

	#define taskSELECT_HIGHEST_PRIORITY_TASK()														    \
	{																								    \
        UBaseType_t uxTopPriority;																		    \
																							    \
		/* Ѱ��������ȼ� */								                            \
		portGET_HIGHEST_PRIORITY( uxTopPriority, uxTopReadyPriority );								    \
		/* ��ȡ���ȼ���ߵľ��������TCB��Ȼ����µ�pxCurrentTCB */                                       \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );		    \
	} /* taskSELECT_HIGHEST_PRIORITY_TASK() */

	/*-----------------------------------------------------------*/
#if 0
	#define taskRESET_READY_PRIORITY( uxPriority )														\
	{																									\
		if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 )	\
		{																								\
			portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );							\
		}																								\
	}
#else
    #define taskRESET_READY_PRIORITY( uxPriority )											            \
    {																							        \
            portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );					        \
    }
#endif

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/*
*************************************************************************
*                               ��̬���񴴽�����
*************************************************************************
*/
#if(configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,          /* ������� */
                               const char *const pcName,           /* �������ƣ��ַ�����ʽ */
                               const uint32_t ulStackDepth,        /* ����ջ��С����Ӧ��ջָ���ƶ� 4 * ulStackDepth ���ֽ� */
                               void* const pvParameters,           /* �����β� */
                               UBaseType_t uxPriority,             /* �������ȼ�����ֵԽ�����ȼ�Խ�� */
                               StackType_t* const puxStakBuffer,   /* ����ջ��ʼ��ַ */
                               TCB_t* const pxTaskBLock)           /* ������ƿ�ָ�� */
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
                             uxPriority,
                             &xReturn,     /* ָ���ָ�� */
                             pxNewTCB);
        /* ��������ӵ������б� */
        prvAddNewTaskToReadyList(pxNewTCB);
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
                                 UBaseType_t uxPriority,             /* �������ȼ�����ֵԽ�����ȼ�Խ�� */
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
     * ���� xStateListItem->pvContainer �� NULL����ʾ��δ�����κ�����
     * ���ں����� insert �����У��� vListInsertEnd ���;�����������
    */
    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    /* ����xStateListItem�ڵ��ӵ���� */
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);

    /* ��ʼ�����ȼ� */
    if(uxPriority >= (UBaseType_t)configMAX_PRIORITIES)
    {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
    }
    pxNewTCB->uxPriority = uxPriority;

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
        vListInitialise(&(pxReadyTasksLists[uxPriority]));
    }
}

static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB)
{
    /* �����ٽ�� */
    taskENTER_CRITICAL();

    /* ȫ��������������� */
    uxCurrentNumberOfTasks++;

    /* �����ǰ pxCurrentTCB Ϊ�գ�����ָ���´��������� */
    if(pxCurrentTCB == NULL)
    {
        pxCurrentTCB = pxNewTCB;
        /* ����ǵ�һ�δ�����������Ҫ��ʼ����������б� */
        if(uxCurrentNumberOfTasks == (UBaseType_t)1U)
        {
            prvInitialiseTaskLists();
        }
    }
    else
    {
        /* ��� pxCurrentTCB ��Ϊ�գ�������������ȼ��� pxCurrentTCB ָ��������ȼ������ TCB */
        if(pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
        {
            pxCurrentTCB = pxNewTCB;
        }
    }
    uxTaskNumber++;

    /* ��������ӵ������б� */
    prvAddTaskToReadyList(pxNewTCB);

    /* �˳��ٽ�� */
    taskEXIT_CRITICAL();
}

/* ����������� */
extern TCB_t IdleTask_TCB;
extern TCB_t Task1_TCB;
extern TCB_t Task2_TCB;

void vTaskStartScheduler(void)
{
#if 0 // �������ȼ��Զ�ָ��
    /* �ֶ�ָ����һ�����е����� */
    pxCurrentTCB = &Task1_TCB;
#endif

    /* ��ʼ��ϵͳʱ�������� */
    xTickCount = (TickType_t)0U;

    /* ���������� */
    if(xPortStartScheduler() != pdFALSE)
    {
        /* �����������ɹ����򲻻᷵�ص����� */
    }
}

/* �����л� */
void vTaskSwitchContext(void)
{
#if 0
    /* �������������л� */
    if(pxCurrentTCB == &Task1_TCB)
    {
        pxCurrentTCB = &Task2_TCB;
    }
    else
    {
        pxCurrentTCB = &Task1_TCB;
    }
#else
    #if 1
    /* ��ȡ���ȼ���ߵľ��������TCB��Ȼ����µ�pxCurrentTCB */
    taskSELECT_HIGHEST_PRIORITY_TASK();
    #else
    /* �����ǰ�߳��ǿ����̣߳���ô��ȥ����ִ���߳�1���߳�2��
     * �������ǵ���ʱʱ���Ƿ���������δ���ڣ���������п����߳�
     */
    if(pxCurrentTCB == &IdleTask_TCB)
    {
        if(Task1_TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task1_TCB;
        }
        else if(Task2_TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task2_TCB;
        }
        else
        {
            return ;
        }
    }
    else
    {
        /* ͬ�����߼�ȥ����߳�1���߳�2 */
        if(pxCurrentTCB == &Task1_TCB)
        {
            if(Task2_TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task2_TCB;
            }
            else if(pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &IdleTask_TCB;
            }
            else
            {
                return ; /* ����1������2��������ʱ�� */
            }
        }
        else if(pxCurrentTCB == &Task2_TCB)
        {
            if(Task1_TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task1_TCB;
            }
            else if(pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &IdleTask_TCB;
            }
            else
            {
                return ; /* ����1������2��������ʱ�� */
            }
        }
        else
        {
            return ;
        }
    }
    #endif
#endif
}

/*
 * ��������ʱ���ͷ� CPU ʹ��Ȩ��CPU ����ȥ������������
 */
void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    /* ��ȡ��ǰ����� TCB */
    pxTCB = pxCurrentTCB;

    /* ������ʱʱ�� */
    pxTCB->xTicksToDelay = xTicksToDelay;

    /* ��ʱδ������Ӿ����б�ɾ��������ʱ�����º��� xTaskIncrementTick ɨ��������� */
    //uxListRemove(&(pxTCB->xStateListItem));

    /* �������ȼ������ȼ�λͼ�� uxTopReadyPriority �ж�Ӧ��λ����*/
    taskRESET_READY_PRIORITY(pxTCB->uxPriority);

    /* �����л� */
    taskYIELD();
}

void xTaskIncrementTick(void)
{
    TCB_t *pxTCB = NULL;
    BaseType_t i = 0;

    const TickType_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;

    /* ɨ������б��������̵߳� xTickToDelay�������Ϊ0�����1 */
    for(i = 0; i < configMAX_PRIORITIES; i++)
    {
        pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
        if(pxTCB->xTicksToDelay > 0)
        {
            pxTCB->xTicksToDelay--;

            /* ��ʱʱ�䵽����������� */
            if(pxTCB->xTicksToDelay == 0)
            {
                taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
            }
        }
    }

    /* �����л� */
    portYIELD();
}












