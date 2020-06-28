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

#if (configUSE_16_BIT_TICKS == 1)
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

/* 临界区管理 */

/*  
 * not interrupt-safe
 * 未保存 BASEPRI 值，不关心当前中都安装该，不能在中断中使用，也不能嵌套使用
 */
extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

#define portDISABLE_INTERRUPTS()    vPortRaiseBASEPRI() // 屏蔽部分中断
#define portENABLE_INTERRUPTS()     vPortSetBASEPRI(0)  // 不关闭任何中断

#define portENTER_CRITICAL()        vPortEnterCritical()
#define portEXIT_CRITICAL()         vPortExitCritical()

/*  
 * interrupt-safe
 * 保存 BASEPRI 值，更新完 BASEPRI 新值后，将之前保存好的 BASEPRI 的值返回，能在中断中使用，也能嵌套使用
 */
#define portSET_INTERRUPT_MASK_FROM_ISR()    ulPortRaiseBASEPRI() // 进入临界区--屏蔽部分中断，并返回之前的中断状态
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)   // 退出临界区--开启被关闭的中断，并恢复之前的中断状态

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
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY; // >= 11的中断会被屏蔽，不会被响应， < 11的中断会被响应
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
