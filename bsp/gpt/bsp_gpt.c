#include "bsp_gpt.h"
#include "bsp_int.h"
#include "bsp_gpio.h"
#include "bsp_uart.h"

/**
 * gpt_init() - GPT1定时器初始化函数
 * 
 * @prescalar: 分频值，能设置的范围为1~4096
 * @value: 装载值 
 * 
 * @return: 无
 */
void gpt_init(unsigned int prescalar, unsigned int value)
{
    if (prescalar > 4096)
        prescalar = 4096;
    GPT1->CR = 0;           /* GPT1_CR寄存器清0 */
    GPT1->CR |= 1 << 15;    /* GPT1定时器进入软复位状态 */
    while((GPT1->CR >> 15) & 0x01);    /* 等待软复位完成 */
	/*配置GPT1-capture1管脚*/
	IOMUXC_SetPinMux(IOMUXC_UART2_TX_DATA_GPT1_CAPTURE1, 0);


    /**
     * 配置GPT1_CR寄存器相关位
     * bit[22:20]: 000 输出比较功能关闭
	 * bit[17:16]: 11 capture方式为双边沿检测
     * bit[9]: 0 GPT1定时器工作于Restart模式
     * bit[8:6]: 001 GPT1时钟源选择ipg_clk=66MHz
     */
    GPT1->CR |= ((1 << 6) | (0x3 << 16));

    /**
     * 配置GPT1_PR寄存器，设置时钟分频系数
     * bit[11:0]: 分频值，0x000~0xFFF代表1~4096分频
     */
    GPT1->PR = prescalar - 1;

    /**
     * 配置GPT1_OCR1寄存器，输出比较1的计数值
     */
    GPT1->OCR[0] = value;

    /**
     * 配置GPT1_IR寄存器，使能输出比较1中断
     * bit[0]: 0 输出比较1中断使能
	 * bit[3]: 1 capture使能
     */
    GPT1->IR |= (1 << 3);

    GIC_EnableIRQ(GPT1_IRQn);    /* 使能GIC相应的GPT1中断 */

    /* 注册GPT1的中断服务函数 */
    system_register_irqhandler(GPT1_IRQn,
        (system_irq_handler_t)gpt_irqhandler, 0);

    GPT1->CR |= 1 << 0; /* 使能GPT1定时器 */

}

/**
 * gpt_irqhandler() - GPT1中断服务处理函数
 * 
 * @giccIar: 中断号
 * @userParam: 用户参数 
 * 
 * @return: 无
 */
void gpt_irqhandler(unsigned int giccIar, void *userParam)
{
	UART_WriteByte(UART1, '\r');
	UART_WriteByte(UART1, '\n');
	uint32_t time = GPT1->ICR[0];
	UART_WriteNum(time);			

	GPT1->SR |= (1 << 3);
}