#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "imx6ul.h"

void pwm3_init(unsigned int prescalar, uint16_t period, uint16_t duty);
void pwm3_setduty(uint16_t value);

#endif