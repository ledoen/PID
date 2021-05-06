#ifndef __BSP_INT_H
#define __BSP_INT_H

#include "imx6ul.h"

/* 中断处理函数形式 */
typedef void (*system_irq_handler_t)(unsigned int giccIar, void *param);

/* 中断处理函数结构体 */
struct _sys_irq_handle {
    system_irq_handler_t irqHandler;    /* 中断处理函数指针 */
    void *userParam;                    /* 中断处理函数参数 */
};

typedef struct _sys_irq_handle sys_irq_handle_t;

/* 相关函数声明 */
void system_irqhandler(unsigned int giccIar);
void default_irqhandler(unsigned int giccIar, void *userParam);
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam);
void system_irqtable_init(void);
void interrupt_init(void);

#endif