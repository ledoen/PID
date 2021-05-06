#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_epit.h"
#include "bsp_uart.h"
#include "bsp_gpt.h"

/**
 * main() - 主函数
 */
int main(void)
{
    clk_enable();   /* 外设时钟使能 */
    gpio_init();    /* GPIO初始化 */
	interrupt_init();       /* 中断初始化 */
    epit1_init(66, 3000);/* EPIT1定时器初始化，1000000为1s */
	uart1_init();
	gpt_init(66, 0x8000000);
	UART_WriteByte(UART1, '\r');
	UART_WriteByte(UART1, '\n');
	UART_WriteNum(0x333);
    while (1) {
		/* 产生触发测距的信号：拉低在epit.c中产生的高电平，持续时间为3000-2950*/
		if(EPIT1->CNR  == 2950)
			gpio1X2_output(0);
    }

    return 0;
}