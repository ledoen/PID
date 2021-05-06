#include "bsp_epit.h"
#include "bsp_int.h"
#include "bsp_gpio.h"

/**
 * epit1_init() - EPIT1定时器初始化函数
 * 
 * @prescalar: 分频值，能设置的范围为1~4096
 * @value: 装载值 
 * 
 * @return: 无
 */
void epit1_init(unsigned int prescalar, unsigned int value)
{
    if (prescalar > 4096)
        prescalar = 4096;
    
    EPIT1->CR = 0;  /* 将CR寄存器清0 */

    /**
     * 配置EPIT1的CR控制寄存器 
     * bit [25:24]: 01 EPIT1时钟选择Peripheral Clock=66MHz
     * bit [15:4]: prescalar-1 分频器值
     * bit [3]: 1 计数器向下计数到0后从LR重新加载计数器
     * bit [2]: 1 比较中断使能
     * bit [1]: 1 初始计数值来于LR寄存器值 
     * bit [0]: 0 先关闭EPIT1定时器
     */
    EPIT1->CR = (1 << 24 | (prescalar - 1) << 4 | 0xe << 0);
    EPIT1->LR = value;  /* 定时器加载寄存器 */
    EPIT1->CMPR = 0;    /* 定时器比较寄存器 */

    GIC_EnableIRQ(EPIT1_IRQn);  /* 使能GIC相应的EPIT1中断 */

    /* 注册EPIT1的中断服务函数 */
    system_register_irqhandler(EPIT1_IRQn,
        (system_irq_handler_t)epit1_irqhandler, 0);
    
    EPIT1->CR |= 1 << 0; /* 使能EPIT1定时器 */
}

/**
 * epit1_irqhandler() - EPIT1中断服务处理函数
 * 
 * @giccIar: 中断号
 * @userParam: 用户参数 
 * 
 * @return: 无
 */
void epit1_irqhandler(unsigned int giccIar, void *userParam)
{
	/*static unsigned int led_status;
	led_status = !led_status;
	int led_state = led_status & 0x1;
	
	if (EPIT1->SR & (1 << 0))
        gpio_output((int)led_state);
	*/
	/* 产生触发测距的信号：产生高电平 */
	gpio1X2_output(1);

    EPIT1->SR |= 1 << 0;    /* 清除EPIT1中断标志 */   
}