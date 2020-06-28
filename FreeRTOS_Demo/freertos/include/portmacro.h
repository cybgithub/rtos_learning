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

#if (configUSE_16_BIT_TICKS == 1)
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

/* �ٽ������� */

/*  
 * not interrupt-safe
 * δ���� BASEPRI ֵ�������ĵ�ǰ�ж���װ�ã��������ж���ʹ�ã�Ҳ����Ƕ��ʹ��
 */
extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

#define portDISABLE_INTERRUPTS()    vPortRaiseBASEPRI() // ���β����ж�
#define portENABLE_INTERRUPTS()     vPortSetBASEPRI(0)  // ���ر��κ��ж�

#define portENTER_CRITICAL()        vPortEnterCritical()
#define portEXIT_CRITICAL()         vPortExitCritical()

/*  
 * interrupt-safe
 * ���� BASEPRI ֵ�������� BASEPRI ��ֵ�󣬽�֮ǰ����õ� BASEPRI ��ֵ���أ������ж���ʹ�ã�Ҳ��Ƕ��ʹ��
 */
#define portSET_INTERRUPT_MASK_FROM_ISR()    ulPortRaiseBASEPRI() // �����ٽ���--���β����жϣ�������֮ǰ���ж�״̬
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)   // �˳��ٽ���--�������رյ��жϣ����ָ�֮ǰ���ж�״̬

#define portINLINE __inline

#ifndef portFORCE_INLINE
#define portFORCE_INLINE __forceinline
#endif

static portFORCE_INLINE void vPortSetBASEPRI(uint32_t ulBASEPRI)
{
    __asm
    {
        /* Barrier instructions are not used as this function is only used to
		lower the BASEPRI value. */
        msr basepri, ulBASEPRI
    }
}


static portFORCE_INLINE void vPortRaiseBASEPRI(void)
{
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY; // >= 11���жϻᱻ���Σ����ᱻ��Ӧ�� < 11���жϻᱻ��Ӧ
    __asm
    {
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
}

static portFORCE_INLINE void vPortClearBASEPRIFromISR( void )
{
	__asm
	{
		/* Set BASEPRI to 0 so no interrupts are masked.  This function is only
		used to lower the mask in an interrupt, so memory barriers are not used. */
		msr basepri, #0
	}
}

static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI(void)
{
    uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    
    __asm
    {
        /* Save current BASEPRI and Set BASEPRI to the max syscall priority to effect a critical section */
        mrs ulReturn, basepri
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
    
    /* return previous BASEPRI */
    return ulReturn;
}




#endif /* __PORTMACRO_H__ */
