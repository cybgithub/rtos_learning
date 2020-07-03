#ifndef __FREERTOS_CONFIG_H__
#define __FREERTOS_CONFIG_H__

#define configUSE_16_BIT_TICKS            0
#define configMAX_TASK_NAME_LEN           16
#define configSUPPORT_STATIC_ALLOCATION   1
#define configMAX_PRIORITIES              5

#define configKERNEL_INTERRUPT_PRIORITY       255 /* ����λ��Ч��������0xf��������15 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY  191   /* ����λ��Ч��������0xb0��������11��>= 11���жϻᱻ���Σ����ᱻ��Ӧ�� < 11���жϻᱻ��Ӧ */

#define configMINIMAL_STACK_SIZE              128
#define configCPU_CLOCK_HZ                    ((unsigned long)25000000)
#define configTICK_RATE_HZ                    ((TickType_t)100)

#define configUSE_PREEMPTION                   1
//#define configUSE_TIME_SLICING                 1

#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
#define vPortSVCHandler     SVC_Handler

#endif /* __FREERTOS_CONFIG_H__ */
