#ifndef __PORTMACRO_H__
#define __PORTMACRO_H__

#include "stdint.h"
#include "stddef.h" 

/* 数据类型重定义 */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	uint32_t
#define portBASE_TYPE	long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if (configUse_16_BIT_TICKS == 1)
    typedef uint16_t TickType_t;
    #define portMAX_DELAY (TickType_t)0xffff;
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY (TickType_t)0xffffffffUL
#endif

/*
 * PendSV是为系统级服务提供的中断驱动。在一个操作系统环境中，当没有其他异常正在执行时，可以使用PendSV来进行上下文的切换；
 * 由于PendSV在系统中被设置为最低优先级，因此只有当没有其他异常或者中断在执行时才会被执行。
 */
/* 中断控制状态寄存器：0xe000ed04
 * Bit 28 PENDSVSET: PendSV 悬起位
 */
#define portNVIC_INT_CTRL_REG (*((volatile uint32_t *)0xe000ed04))
#define portNVIC_PENDSVSET_BIT (1UL << 28UL)
#define portSY_FULL_READ_WRITE (15)

#define portYIELD()																\
{																				\
	/* 触发PendSV，产生上下文切换 */								                \
	portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;								\
	__dsb( portSY_FULL_READ_WRITE );											\
	__isb( portSY_FULL_READ_WRITE );											\
}


#endif /* __PORTMACRO_H__ */
