#ifndef __BSP_EPIT_H
#define __BSP_EPIT_H

#include "imx6ul.h"

/* 相关函数声明 */
void epit1_init(unsigned int prescalar, unsigned int value);
void epit1_irqhandler(unsigned int giccIar, void *userParam);

#endif