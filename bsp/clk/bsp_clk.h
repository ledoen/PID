#ifndef __BSP_CLK_H
#define __BSP_CLK_H

#include "imx6ul.h"

/**
 *  SoC中CCM_CCGR寄存器地址
 */
#define CCM_CCGR0   *((volatile unsigned int *)0x020c4068)
#define CCM_CCGR1   *((volatile unsigned int *)0x020c406c)
#define CCM_CCGR2   *((volatile unsigned int *)0x020c4070)
#define CCM_CCGR3   *((volatile unsigned int *)0x020c4074)
#define CCM_CCGR4   *((volatile unsigned int *)0x020c4078)
#define CCM_CCGR5   *((volatile unsigned int *)0x020c407c)
#define CCM_CCGR6   *((volatile unsigned int *)0x020c4080)

/* clk_enable函数声明 */
void clk_enable(void);

#endif