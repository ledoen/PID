#ifndef __BSP_GPT_H
#define __BSP_GPT_H

#include "imx6ul.h"

/* 定义状态量，用于判断每次中断是上升沿触发还是下降沿触发
* 方法是每次产生突发电平后，将该值置0
* 之后每次触发中断，该值加1
* 则：当为1时，为上升沿
*		为2时，为下降沿
*/
extern uint8_t edgeStatus;
extern uint32_t startTime;
extern uint32_t flightTime;

void gpt_init(unsigned int prescalar, unsigned int value);
void gpt_irqhandler(unsigned int giccIar, void *userParam);


#endif