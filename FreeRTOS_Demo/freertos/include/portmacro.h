#ifndef __PORTMACRO_H__
#define __PORTMACRO_H__

#include "stdint.h"
#include "stddef.h" 

/* ���������ض��� */
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
 * PendSV��Ϊϵͳ�������ṩ���ж���������һ������ϵͳ�����У���û�������쳣����ִ��ʱ������ʹ��PendSV�����������ĵ��л���
 * ����PendSV��ϵͳ�б�����Ϊ������ȼ������ֻ�е�û�������쳣�����ж���ִ��ʱ�Żᱻִ�С�
 */
/* �жϿ���״̬�Ĵ�����0xe000ed04
 * Bit 28 PENDSVSET: PendSV ����λ
 */
#define portNVIC_INT_CTRL_REG (*((volatile uint32_t *)0xe000ed04))
#define portNVIC_PENDSVSET_BIT (1UL << 28UL)
#define portSY_FULL_READ_WRITE (15)

#define portYIELD()																\
{																				\
	/* ����PendSV�������������л� */								                \
	portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;								\
	__dsb( portSY_FULL_READ_WRITE );											\
	__isb( portSY_FULL_READ_WRITE );											\
}


#endif /* __PORTMACRO_H__ */
