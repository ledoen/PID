#ifndef __BSP_GPT_H
#define __BSP_GPT_H

#include "imx6ul.h"
void gpt_init(unsigned int prescalar, unsigned int value);
void gpt_irqhandler(unsigned int giccIar, void *userParam);


#endif