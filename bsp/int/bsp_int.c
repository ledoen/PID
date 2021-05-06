#include "bsp_int.h"

/* 中断嵌套计数器 */
static unsigned int irqNesting;

/* 中断处理函数表 */
static sys_irq_handle_t irqTable[NUMBER_OF_INT_VECTORS];

/**
 * default_irqhandler() - 默认的中断服务处理函数
 * 
 * @giccIar: 中断号
 * @userParam: 中断服务处理函数参数
 * 
 * @return: 无
 */
void default_irqhandler(unsigned int giccIar, void *userParam)
{
    while (1) {

    }
}

/**
 * system_irqhandler() - 中断服务处理函数
 * 汇编中的IRQ中断服务函数将调用该函数，该
 * 函数通过在中断服务列表中查找指定中断号所
 * 对应的中断处理函数并执行
 * 
 * @giccIar: 中断号
 * 
 * @return: 无
 */
void system_irqhandler(unsigned int giccIar)
{
    unsigned int intNum = giccIar & 0x3FFUL; /* 获取中断号 */

    /* 判断中断号是否符合要求 */
    if (intNum >= NUMBER_OF_INT_VECTORS)
        return;
    
    irqNesting++;   /* 中断嵌套计数器加1 */

    /* 根据传递的中断号，在中断列表中调用对应的外设中断处理函数 */
    irqTable[intNum].irqHandler(intNum, irqTable[intNum].userParam);

    irqNesting--;   /* 中断嵌套计数器减1 */
}

/**
 * system_register_irqhandler() - 注册中断服务函数
 * 
 * @irq: 要注册的中断号
 * @handler: 要注册的中断处理函数
 * @userParam: 中断处理函数参数
 * 
 * @return: 无
 */
void system_register_irqhandler(IRQn_Type irq,
    system_irq_handler_t handler, void *userParam)
{
    irqTable[irq].irqHandler = handler;
    irqTable[irq].userParam = userParam;
}

/**
 * system_irqtable_init() - 初始化中断服务函数列表
 * 
 * @param: 无
 * @return: 无
 */
void system_irqtable_init(void)
{
    unsigned int i;
    irqNesting = 0;

    /* 将所有的中断处理服务函数设置为默认值 */
    for (i = 0; i < NUMBER_OF_INT_VECTORS; i++) {
        system_register_irqhandler((IRQn_Type)i,
            default_irqhandler, 0);
    }
}

/**
 * interrupt_init() - 中断初始化函数
 * 
 * @param: 无
 * @return: 无
 */
void interrupt_init(void)
{
    GIC_Init();                             /* 初始化GIC中断控制器 */
    system_irqtable_init();                 /* 初始化中断表 */
    __set_VBAR((unsigned int)0x87800000);   /* 设置中断向量表偏移地址 */
}