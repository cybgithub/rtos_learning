#ifndef __PORTABLE_H__
#define __PORTABLE_H__

#include "portmacro.h"
#include "projdefs.h"

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                   TaskFunction_t pxCode,
                                   void *pvParameters);
BaseType_t xPortStartScheduler(void);

#endif /* __PORTABLE_H__ */
