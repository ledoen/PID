#include "bsp_pwm.h"

void pwm3_init(unsigned int prescalar, uint16_t period, uint16_t duty)
{
	if (prescalar > 4096)
        prescalar = 4096;
	/* 设置GPIO1_IO04为PWM3OUT */
	IOMUXC_SetPinMux(IOMUXC_GPIO1_IO04_PWM3_OUT, 0);
	
	/* 将PWM3控制寄存器清零 */
	PWM3->PWMCR = 0;
	
	/* 配置PWM3控制寄存器CR
	* bit [17:16]: 01 PWM3时钟选择Peripheral Clock=66MHz
	* bit [15:4]: prescalar-1 分频器值
	*/
	PWM3->PWMCR = ((1 << 16) | (prescalar << 4));
	
	/* 设置IR寄存器，禁止所有使能 */
	PWM3->PWMIR = 0;
	
	/* 设置周期 */
	PWM3->PWMPR = period;
	
	/* 设置占空比 */
	pwm3_setduty(duty);
	
	/* 使能PWM3 */
	PWM3->PWMCR |= (1 << 0);
}

void pwm3_setduty(uint16_t value)
{
	for(int i=0; i<3; i++){
		PWM3->PWMSAR = value;
	}
}