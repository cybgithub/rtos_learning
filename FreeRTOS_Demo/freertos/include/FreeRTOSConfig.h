#ifndef __FREERTOS_CONFIG_H__
#define __FREERTOS_CONFIG_H__

#define configUSE_16_BIT_TICKS            0
#define configMAX_TASK_NAME_LEN           16
#define configSUPPORT_STATIC_ALLOCATION   1
#define configMAX_PRIORITIES              5

#define configKERNEL_INTERRUPT_PRIORITY       255 /* 高四位有效，即等于0xf，或者是15 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY  191   /* 高四位有效，即等于0xb0，或者是11，>= 11的中断会被屏蔽，不会被响应， < 11的中断会被响应 */

#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
#define vPortSVCHandler     SVC_Handler

#endif /* __FREERTOS_CONFIG_H__ */
