
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "projdefs.h"
#include "portable.h"
#include "FreeRTOS.h"
//#include "list.h"
#include "task.h"

/*************** enable printf in simulate mode for debug *******************/
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

//半主机模式必须定义这个
struct __FILE
{
    int handle;
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
    if (DEMCR & TRCENA)
    {
        while (ITM_Port32(0) == 0);

        ITM_Port8(0) = ch;
    }
    return(ch);
}

/********************************************************************/
/*
*************************************************************************
*                              全局变量
*************************************************************************
*/
uint32_t flag_1 = 0;
uint32_t flag_2 = 0;
uint32_t flag_3 = 0;

extern List_t pxReadyTaskLists[configMAX_PRIORITIES];

/*
*************************************************************************
*                        任务控制块 & STACK 
*************************************************************************
*/
TCB_t Task1_TCB;
TaskHandle_t Task1_Handle;
#define TASK1_STACK_SIZE                    20
StackType_t Task1_Stack[TASK1_STACK_SIZE];

TCB_t Task2_TCB;
TaskHandle_t Task2_Handle;
#define TASK2_STACK_SIZE                    20
StackType_t Task2_Stack[TASK2_STACK_SIZE];

TCB_t Task3_TCB;
TaskHandle_t Task3_Handle;
#define TASK3_STACK_SIZE                    20
StackType_t Task3_Stack[TASK3_STACK_SIZE];

/* 定义空闲任务栈 */
TCB_t IdleTask_TCB;
TaskHandle_t IdleTask_Handle;
StackType_t IdleTask_Stack[configMINIMAL_STACK_SIZE];

/*
 * 打印测试函数
 * 过多打印会阻塞模拟
 */
void print_func(int *rate, const char *func)
{
    if(*rate >= 10000)
    {
        *rate = 0;
        //printf("exec %s \n", (func == NULL) ? "..." : func);//一直打印会导致其他线程无法切入
    }
}
/*
 * 软件延时
 * 让 CPU 空等来实现延时，并未放弃 CPU 占有权，即 CPU 还是被占用着
 */
void delay(uint32_t count)
{
    for (; count != 0; count--);
}

/*
 * 任务0， 空闲任务
 */
//static UBaseType_t idle_cnt = (UBaseType_t)0UL;
//static UBaseType_t print_rate = (UBaseType_t)0xffff;

void Task0_Idle_Entry(void *p_arg)
{
    while(1)
    {
        #if 0
        idle_cnt++;
        if(idle_cnt % print_rate == 0)
        {
            printf("%s, exec for %u times agagin \n", __func__, print_rate);
            idle_cnt = 0;
        }
        #else
        ;
        #endif
    }
}

#define TASK_TIME_SLICING_TEST_ON    1
/*
 * 任务 1
 */
void Task1_Entry(void *p_arg)
{
    int rate = 0;
	while(1)
	{
        rate++;
        //print_func(&rate, __func__);

#if TASK_TIME_SLICING_TEST_ON
        int cnt1 = 0;
        while(cnt1 <= 100)
        {
            flag_1 = 1;
            delay(50);
            flag_1 = 0;
            delay(50);
            cnt1++;
        }
        //vTaskDelay(1);
        /* 任务切换（手动），触发 PendSV_Handler，进入 xPortPendSVHandler，内部执行了 vTaskSwitchContext */
        //taskYIELD();
#else
        flag_1 = 1;
        vTaskDelay(1);
        flag_1 = 0;
        vTaskDelay(1);
#endif
	}
}
/*
 * 任务 2
 */
void Task2_Entry(void *p_arg)
{
    int rate = 0;
	while(1)
	{
        rate++;
        //print_func(&rate, __func__);
#if TASK_TIME_SLICING_TEST_ON
        int cnt2 = 0;
        while(cnt2 <= 100)
        {
            flag_2 = 1;
            delay(100);
            flag_2 = 0;
            delay(100);
            cnt2++;
        }
        //vTaskDelay(1);
        /* 任务切换（手动），触发 PendSV_Handler，进入 xPortPendSVHandler，内部执行了 vTaskSwitchContext */
        //taskYIELD();
#else
        flag_2 = 1;
        vTaskDelay(1);
        flag_2 = 0;
        vTaskDelay(1);
#endif
	}
}
/*
 * 任务 3
 */
void Task3_Entry(void *p_arg)
{
    int rate = 0;
	while(1)
	{
        rate++;
        //print_func(&rate, __func__);
        /* 观察波形，可以确认任务3先执行 */
        {
            flag_3 = 1;
            delay(2000);
            flag_3 = 0;
            delay(2000);
            flag_3 = 1;
            delay(2000);
            flag_3 = 0;
            delay(2000);
        }

        flag_3 = 1;
        vTaskDelay(1);
        flag_3 = 0;
        vTaskDelay(1);
	}
}
/*
 * 主函数
 */
int main(void)
{
    /* 硬件初始化 */
	/* 将硬件相关的初始化放在这里，如果是软件仿真则没有相关初始化代码 */
    
    /* 初始化与任务相关的列表，如就绪列表 */
	//prvInitialiseTaskLists();
    
    /* 创建任务 */
    IdleTask_Handle = xTaskCreateStatic((TaskFunction_t)Task0_Idle_Entry,
                                     (char *)"IDLE",
                                     (uint32_t)configMINIMAL_STACK_SIZE,
                                     (void *)NULL,
                                     taskIDLE_PRIORITY,
                                     (StackType_t *)IdleTask_Stack,
                                     (TCB_t *)(&IdleTask_TCB));

    Task1_Handle = xTaskCreateStatic((TaskFunction_t)Task1_Entry,
                                     (char *)"Task1",
                                     (uint32_t)TASK1_STACK_SIZE,
                                     (void *)NULL,
                                     1,
                                     (StackType_t *)Task1_Stack,
                                     (TCB_t *)(&Task1_TCB));

    /* 优先级与 Task1 相同，测试时间片轮转 */
    Task2_Handle = xTaskCreateStatic((TaskFunction_t)Task2_Entry,
                                     (char *)"Task2",
                                     (uint32_t)TASK2_STACK_SIZE,
                                     (void *)NULL,
#if TASK_TIME_SLICING_TEST_ON
                                     1,
#else
                                     2,
#endif
                                     (StackType_t *)Task2_Stack,
                                     (TCB_t *)(&Task2_TCB));

    /* 任务1和2是死循环，任务3优先级必须高于任务1/2，才有机会运行 */
    Task3_Handle = xTaskCreateStatic((TaskFunction_t)Task3_Entry,
                                     (char *)"Task3",
                                     (uint32_t)TASK3_STACK_SIZE,
                                     (void *)NULL,
                                     3,
                                     (StackType_t *)Task3_Stack,
                                     (TCB_t *)(&Task3_TCB));
#if 0 // 根据优先级自动添加到就绪列表
    /* 将任务添加到就绪列表 */
    vListInsertEnd(&(pxReadyTaskLists[0]), &(((TCB_t *)(&IdleTask_TCB))->xStateListItem)); 
    vListInsertEnd(&(pxReadyTaskLists[1]), &(((TCB_t *)(&Task1_TCB))->xStateListItem));  
    vListInsertEnd(&(pxReadyTaskLists[2]), &(((TCB_t *)(&Task2_TCB))->xStateListItem));  
#endif
    /* 启用调度前关闭中断 */
    portDISABLE_INTERRUPTS();
    /* 启动调度器，开始多任务调度，启动完成不返回 */                                     
    vTaskStartScheduler();
         
    while(1)
    {
        /*系统启动成功不会进入这里 */
    }
}
