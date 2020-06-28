
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

//������ģʽ���붨�����
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
*                              ȫ�ֱ���
*************************************************************************
*/
uint32_t flag_1 = 0;
uint32_t flag_2 = 0;

extern List_t pxReadyTaskLists[configMAX_PRIORITIES];

/*
*************************************************************************
*                        ������ƿ� & STACK 
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

/*
 *�����ʱ
 */
void delay(uint32_t count)
{
    for (; count != 0; count--);
}
/*
 *���� 1
 */
void Task1_Entry(void *p_arg)
{
	while(1)
	{
        flag_1 = 1;
        delay(100);
        flag_1 = 0;
        delay(100);
        
        /* �����л����ֶ��������� PendSV_Handler������ xPortPendSVHandler���ڲ�ִ���� vTaskSwitchContext */
        taskYIELD();
	}
}
/*
 *���� 2
 */
void Task2_Entry(void *p_arg)
{
	while(1)
	{
        flag_2 = 1;
        delay(100);
        flag_2 = 0;
        delay(100);		
        
        /* �����л����ֶ��������� PendSV_Handler������ xPortPendSVHandler���ڲ�ִ���� vTaskSwitchContext */
        taskYIELD();
	}
}
/*
 * ������
 */
int main(void)
{
    /* Ӳ����ʼ�� */
	/* ��Ӳ����صĳ�ʼ�����������������������û����س�ʼ������ */
    
    /* ��ʼ����������ص��б�������б� */
	prvInitialiseTaskLists();
    
    /* �������� */
    Task1_Handle = xTaskCreateStatic((TaskFunction_t)Task1_Entry,
                                     (char *)"Task1",
                                     (uint32_t)TASK1_STACK_SIZE,
                                     (void *)NULL,
                                     (StackType_t *)Task1_Stack,
                                     (TCB_t *)(&Task1_TCB));
                                     
    Task2_Handle = xTaskCreateStatic((TaskFunction_t)Task2_Entry,
                                     (char *)"Task2",
                                     (uint32_t)TASK2_STACK_SIZE,
                                     (void *)NULL,
                                     (StackType_t *)Task2_Stack,
                                     (TCB_t *)(&Task2_TCB));
    /* ��������ӵ������б� */
    vListInsertEnd(&(pxReadyTaskLists[1]), &(((TCB_t *)(&Task1_TCB))->xStateListItem));  
    vListInsertEnd(&(pxReadyTaskLists[2]), &(((TCB_t *)(&Task2_TCB))->xStateListItem));  

    /* ��������������ʼ��������ȣ�������ɲ����� */                                     
    vTaskStartScheduler();
         
    while(1)
    {
        /*ϵͳ�����ɹ������������ */
    }
}
