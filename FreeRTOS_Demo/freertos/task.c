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
List_t pxReadyTaskLists[configMAX_PRIORITIES];

static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;

/*
*************************************************************************
*                               函数声明
*************************************************************************
*/
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,          /* 任务入口 */
                                 const char * const pcName,          /* 任务名称 */
                                 const uint32_t ulStackDepth,        /* 任务栈大小，单位字节 */
                                 void * const pvParameters,          /* 任务形参 */
                                 TaskHandle_t * const pxCreatedTask, /* 创建成功后返回的任务句柄 */
                                 TCB_t *pxNewTCB);
/*
*************************************************************************
*                               宏定义
*************************************************************************
*/


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
                               StackType_t* const puxStakBuffer,   /* 任务栈起始地址 */
                               TCB_t* const pxTaskBLock)          /* 任务控制块指针 */
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
                             &xReturn,     /* 指针的指针 */
                             pxNewTCB);
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
        vListInitialise(&(pxReadyTaskLists[uxPriority]));
    }
}

/* 启动任务调度 */
extern TCB_t Task1_TCB;
extern TCB_t Task2_TCB;

void vTaskStartScheduler(void)
{
    /* 手动指定第一个运行的任务 */
    pxCurrentTCB = &Task1_TCB;
    
    /* 启动调度器 */
    if(xPortStartScheduler() != pdFALSE)
    {
        /* 调度器启动成功，则不会返回到这里 */
    }
}

/* 任务切换 */
void vTaskSwitchContext(void)
{
    /* 两个任务轮流切换 */
    if(pxCurrentTCB == &Task1_TCB)
    {
        pxCurrentTCB = &Task2_TCB;
    }
    else
    {
        pxCurrentTCB = &Task1_TCB;
    }
}














