#include "FreeRTOS.h"
#include "task.h"

/*
*************************************************************************
*                               任务控制块
*************************************************************************
*/
/* 当前正在运行的任务的控制块 */
TCB_t * volatile pxCurrentTCB = NULL;

/* 任务就绪列表 */
List_t pxReadyTasksLists[configMAX_PRIORITIES];

static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;
static volatile TickType_t xTickCount              = (TickType_t)0U;
static volatile UBaseType_t uxTopReadyPriority     = taskIDLE_PRIORITY;
static UBaseType_t uxTaskNumber                    = (UBaseType_t)0U;

/*
*************************************************************************
*                               函数声明
*************************************************************************
*/
static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB);
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,          /* 任务入口 */
                                 const char * const pcName,          /* 任务名称 */
                                 const uint32_t ulStackDepth,        /* 任务栈大小，单位字节 */
                                 void * const pvParameters,          /* 任务形参 */
                                 UBaseType_t uxPriority,             /* 任务优先级 */
                                 TaskHandle_t * const pxCreatedTask, /* 创建成功后返回的任务句柄 */
                                 TCB_t *pxNewTCB);
/*
*************************************************************************
*                               宏定义
*************************************************************************
*/
/* 将任务添加到就绪列表 */
#define prvAddTaskToReadyList( pxTCB )																   \
	taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												   \
	vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) ); \

/* 查找最高优先级的就绪任务：通用方法 */
#if ( configUSE_PORT_OPTIMISED_TASK_SELECTION == 0 )
	/* uxTopReadyPriority 存的是就绪任务的最高优先级 */
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
		/* 寻找包含就绪任务的最高优先级的队列 */                                                          \
		while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )							\
		{																								\
			--uxTopPriority;																			\
		}																								\
																										\
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */							            \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );			\
		/* 更新uxTopReadyPriority */                                                                    \
		uxTopReadyPriority = uxTopPriority;																\
	} /* taskSELECT_HIGHEST_PRIORITY_TASK */

	/*-----------------------------------------------------------*/

	/* 这两个宏定义只有在选择优化方法时才用，这里定义为空 */
	#define taskRESET_READY_PRIORITY( uxPriority )
	#define portRESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )

/* 查找最高优先级的就绪任务：根据处理器架构优化后的方法 */
#else /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

	#define taskRECORD_READY_PRIORITY( uxPriority )	portRECORD_READY_PRIORITY( uxPriority, uxTopReadyPriority )

	/*-----------------------------------------------------------*/

	#define taskSELECT_HIGHEST_PRIORITY_TASK()														    \
	{																								    \
        UBaseType_t uxTopPriority;																		    \
																							    \
		/* 寻找最高优先级 */								                            \
		portGET_HIGHEST_PRIORITY( uxTopPriority, uxTopReadyPriority );								    \
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */                                       \
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
*                               静态任务创建函数
*************************************************************************
*/
#if(configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,          /* 任务入口 */
                               const char *const pcName,           /* 任务名称，字符串形式 */
                               const uint32_t ulStackDepth,        /* 任务栈大小，相应的栈指针移动 4 * ulStackDepth 个字节 */
                               void* const pvParameters,           /* 任务形参 */
                               UBaseType_t uxPriority,             /* 任务优先级，数值越大，优先级越高 */
                               StackType_t* const puxStakBuffer,   /* 任务栈起始地址 */
                               TCB_t* const pxTaskBLock)           /* 任务控制块指针 */
{
    TCB_t *pxNewTCB = NULL;
    TaskHandle_t xReturn;
    
    if((pxTaskBLock != NULL) && (puxStakBuffer != NULL))
    {
        pxNewTCB = (TCB_t *)pxTaskBLock;
        /* 任务控制块中的栈地址 pxStack 关联到传入的任务栈起始地址 puxStakBuffer */
        pxNewTCB->pxStack = (StackType_t *)puxStakBuffer;
        /* 创建新的任务 */
        prvInitialiseNewTask(pxTaskCode,
                             pcName,
                             ulStackDepth,
                             pvParameters,
                             uxPriority,
                             &xReturn,     /* 指针的指针 */
                             pxNewTCB);
        /* 将任务添加到就绪列表 */
        prvAddNewTaskToReadyList(pxNewTCB);
    }
    else
    {
        xReturn = NULL;
    }

    /* 返回任务句柄，如果任务创建成功，此时xReturn应该指向任务控制块 */
    return xReturn;
}                                   
#endif /* configSUPPORT_STATIC_ALLOCTION */

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,          /* 任务入口 */
                                 const char * const pcName,          /* 任务名称 */
                                 const uint32_t ulStackDepth,        /* 任务栈大小，相应的栈指针移动 4 * ulStackDepth 个字节 */
                                 void * const pvParameters,          /* 任务形参 */
                                 UBaseType_t uxPriority,             /* 任务优先级，数值越大，优先级越高 */
                                 TaskHandle_t * const pxCreatedTask, /* 创建成功后返回的任务句柄，指针的指针，
                                                                        最后的地址即经过初始化之后pxNewTCB控制块的地址，初始化失败则指向NULL*/
                                 TCB_t *pxNewTCB)
{
    StackType_t *pxTopOfStack = NULL;
    UBaseType_t x;

    /* 获取栈顶地址 */
    pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
    /* 向下做8字节对齐*/
    pxTopOfStack = (StackType_t *)(((uint32_t)pxTopOfStack) & (~((uint32_t)0x7)));

    /* 将任务的名字存储在TCB中 */
    for(x = (UBaseType_t)0; x < (UBaseType_t)configMAX_TASK_NAME_LEN; x++)
    {
        pxNewTCB->pcTaskName[x] = pcName[x];
        if(pcName[x] == '\0')
        {
            break;
        }
    }
    /* 任务名字的长度不能超过configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';

    /*
     * 初始化TCB中的xStateListItem节点，
     * 即将 xStateListItem->pvContainer 置 NULL，表示尚未加入任何链表，
     * 可在后续的 insert 操作中（如 vListInsertEnd ）和具体的链表关联
    */
    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    /* 设置xStateListItem节点的拥有者 */
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);

    /* 初始化优先级 */
    if(uxPriority >= (UBaseType_t)configMAX_PRIORITIES)
    {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
    }
    pxNewTCB->uxPriority = uxPriority;

    /* 初始化任务栈，此时 pxNewTCB->pxTopOfStack 指向空闲栈 */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);

    /* 让任务句柄指向任务控制块 */
    if((void *)pxCreatedTask != NULL)
    {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB;
    }
}    
   
/* 初始化任务相关的列表 */
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
    /* 进入临界段 */
    taskENTER_CRITICAL();

    /* 全局任务计数器自增 */
    uxCurrentNumberOfTasks++;

    /* 如果当前 pxCurrentTCB 为空，则将其指向新创建的任务 */
    if(pxCurrentTCB == NULL)
    {
        pxCurrentTCB = pxNewTCB;
        /* 如果是第一次创建任务，则需要初始化任务相关列表 */
        if(uxCurrentNumberOfTasks == (UBaseType_t)1U)
        {
            prvInitialiseTaskLists();
        }
    }
    else
    {
        /* 如果 pxCurrentTCB 不为空，则根据任务优先级将 pxCurrentTCB 指向最高优先级任务的 TCB */
        if(pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
        {
            pxCurrentTCB = pxNewTCB;
        }
    }
    uxTaskNumber++;

    /* 将任务添加到就绪列表 */
    prvAddTaskToReadyList(pxNewTCB);

    /* 退出临界段 */
    taskEXIT_CRITICAL();
}

/* 启动任务调度 */
extern TCB_t IdleTask_TCB;
extern TCB_t Task1_TCB;
extern TCB_t Task2_TCB;

void vTaskStartScheduler(void)
{
#if 0 // 根据优先级自动指定
    /* 手动指定第一个运行的任务 */
    pxCurrentTCB = &Task1_TCB;
#endif

    /* 初始化系统时基计数器 */
    xTickCount = (TickType_t)0U;

    /* 启动调度器 */
    if(xPortStartScheduler() != pdFALSE)
    {
        /* 调度器启动成功，则不会返回到这里 */
    }
}

/* 任务切换 */
void vTaskSwitchContext(void)
{
#if 0
    /* 两个任务轮流切换 */
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
    /* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */
    taskSELECT_HIGHEST_PRIORITY_TASK();
    #else
    /* 如果当前线程是空闲线程，那么就去尝试执行线程1或线程2，
     * 看看它们的延时时间是否结束，如果未到期，则继续运行空闲线程
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
        /* 同样的逻辑去检查线程1或线程2 */
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
                return ; /* 任务1和任务2都处于延时中 */
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
                return ; /* 任务1和任务2都处于延时中 */
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
 * 阻塞性延时，释放 CPU 使用权，CPU 可以去处理其他事情
 */
void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    /* 获取当前任务的 TCB */
    pxTCB = pxCurrentTCB;

    /* 设置延时时间 */
    pxTCB->xTicksToDelay = xTicksToDelay;

    /* 暂时未将任务从就绪列表删除，否则时基更新函数 xTaskIncrementTick 扫描会有问题 */
    //uxListRemove(&(pxTCB->xStateListItem));

    /* 根据优先级将优先级位图表 uxTopReadyPriority 中对应的位清零*/
    taskRESET_READY_PRIORITY(pxTCB->uxPriority);

    /* 任务切换 */
    taskYIELD();
}

void xTaskIncrementTick(void)
{
    TCB_t *pxTCB = NULL;
    BaseType_t i = 0;

    const TickType_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;

    /* 扫描就绪列表中所有线程的 xTickToDelay，如果不为0，则减1 */
    for(i = 0; i < configMAX_PRIORITIES; i++)
    {
        pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
        if(pxTCB->xTicksToDelay > 0)
        {
            pxTCB->xTicksToDelay--;

            /* 延时时间到，将任务就绪 */
            if(pxTCB->xTicksToDelay == 0)
            {
                taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
            }
        }
    }

    /* 任务切换 */
    portYIELD();
}












