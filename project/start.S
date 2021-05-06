.global _start

/**
 *  _start函数，程序先在这开始执行，用来设置C语言运行环境
 */
_start:
    /* 创建中断向量表 */
    ldr pc, =Reset_Handler      /* 复位中断 */
    ldr pc, =Undefined_Handler  /* 未定义指令中断 */
    ldr pc, =SVC_Handler        /* Supervisor中断 */
    ldr pc, =PrefAbort_Handler  /* 预取终止中断 */
    ldr pc, =DataAbort_Handler  /* 数据终止中断 */
    ldr pc, =NotUsed_Handler    /* 未使用中断 */
    ldr pc, =IRQ_Handler        /* IRQ中断 */
    ldr pc, =FIQ_Handler        /* FIQ中断 */

/* 复位中断服务函数 */
Reset_Handler:
    cpsid i                     /* 关闭全局IRQ中断 */

    /* 关闭I/D Cache以及MMU */
    mrc p15, 0, r0, c1, c0, 0   /* 读取CP15协处理器的c1寄存器值到R0中 */
    bic r0, r0, #(1 << 12)      /* 清除c1的I位，关闭I Cache */
    bic r0, r0, #(1 << 2)       /* 清除c1的C位，关闭D Cache */
    bic r0, r0, #(1 << 1)       /* 清除c1的A位，关闭对齐检查 */
    bic r0, r0, #(1 << 11)      /* 清除c1的Z位，关闭分支预测 */
    bic r0, r0, #(1 << 0)       /* 清除c1的M位，关闭MMU */
    mcr p15, 0, r0, c1, c0, 0   /* 将R0的值写入到CP15协处理器的c1寄存器中 */

#if 0
    /* 汇编版本设置中断向量表偏移 */
    ldr r0, =0x87800000

    dsb
    isb
    mcr p15, 0, r0, c12, c0, 0  /* 将R0的值写入到CP15协处理器的c12寄存器中 */
    dsb
    isb
#endif

    /* SoC进入IRQ运行模式 */
    mrs r0, cpsr            /* 将cpsr寄存器的值读取到R0中 */
    bic r0, r0, #0x1f       /* 将cpsr寄存器的M[4:0]清0 */
    orr r0, r0, #0x12       /* SoC进入到IRQ运行模式 */
    msr cpsr, r0            /* 将R0的值回写到cpsr寄存器 */
    ldr sp, =0x80600000     /* 设置IRQ模式下栈指针0x80600000，大小为2MB */

    /* SoC进入System运行模式 */
    mrs r0, cpsr            /* 将cpsr寄存器的值读取到R0中 */
    bic r0, r0, #0x1f       /* 将cpsr寄存器的M[4:0]清0 */
    orr r0, r0, #0x1f       /* SoC进入到SYS运行模式 */
    msr cpsr, r0            /* 将R0的值回写到cpsr寄存器 */
    ldr sp, =0x80400000     /* 设置IRQ模式下栈指针0x80400000，大小为2MB */

    /* SoC进入Supervisor运行模式 */
    mrs r0, cpsr            /* 将cpsr寄存器的值读取到R0中 */
    bic r0, r0, #0x1f       /* 将cpsr寄存器的M[4:0]清0 */
    orr r0, r0, #0x13       /* SoC进入到SVC运行模式 */
    msr cpsr, r0            /* 将R0的值回写到cpsr寄存器 */
    ldr sp, =0x80200000     /* 设置IRQ模式下栈指针0x80200000，大小为2MB */

    cpsie i                 /* 打开全局IRQ中断 */

#if 0
    /* 使能IRQ中断 */
    mrs r0, cpsr            /* 将cpsr寄存器的值读取到R0中 */
    bic r0, r0, #0x80       /* 将R0寄存器中的bit7清0，允许IRQ中断 */
    msr cpsr, r0            /* 将R0的值回写到cpsr寄存器中 */
#endif

    b main                  /* 跳转到main函数运行 */

/* 未定义指令中断函数 */
Undefined_Handler:
    ldr r0, =Undefined_Handler
    bx  r0

/* Supervisor中断服务函数 */
SVC_Handler:
    ldr r0, =SVC_Handler
    bx  r0

/* 预取终止中断服务函数 */
PrefAbort_Handler:
    ldr r0, =PrefAbort_Handler
    bx  r0

/* 数据终止中断服务函数 */
DataAbort_Handler:
    ldr r0, =DataAbort_Handler
    bx  r0

/* 未使用的中断服务函数 */
NotUsed_Handler:
    ldr r0, =NotUsed_Handler
    bx  r0

/* IRQ中断服务函数 */
IRQ_Handler:
    /* 中断发生时保护现场 */
    push {lr}           /* 保存lr寄存器(中断发生时保存PC的值) */
    push {r0-r3, r12}   /* 保存r0-r3,r12寄存器中的值 */

    mrs r0, spsr        /* 读取spsr寄存器的值到R0寄存器中 */
    push {r0}           /* 保存spsr寄存器 */

    mrc p15, 4, r1, c15, c0, 0  /* 读取CP15协处理器的c15寄存器值(保存GIC基地址) */
    /* r1=r1+0x2000，得到GIC的CPU Interface端基地址 */
    add r1, r1, #0x2000

    /**
     * CPU Interface端基地址加上0x0C得到
     * GICC_IAR寄存器地址，该寄存器保存着当前
     * 发生IRQ中断的中断号，需要根据这个中断号
     * 调用相应的中断服务函数。
     */
    ldr r0, [r1, #0x0C]     /* 将GICC_IAR寄存器中的值保存到R0 */
    push {r0, r1}           /* 保存r0(发生IRQ的中断号)，r1(GIC的CPU Interface基地址) */

    cps #0x13               /* SoC进入到SVC模式，允许其它中断再次进入 */

    push {lr}               /* 保存SVC模式下的lr寄存器 */
    ldr r2, =system_irqhandler  /* 将C中断服务处理函数地址加载到R2寄存器 */
    blx r2                  /* 调用C中断服务处理函数(参数为r0) */

    pop {lr}                /* 执行完C中断服务处理函数，lr出栈 */
    cps #0x12               /* SoC进入到IRQ模式 */
    pop {r0, r1}
    str r0, [r1, #0x10]     /* 中断执行完成，写GICC_EOIR寄存器 */

    /* 中断执行完成需要恢复现场 */
    pop {r0}
    msr spsr_cxsf, r0   /* 恢复spsr寄存器 */

    pop {r0-r3, r12}    /* r0-r3,r12出栈 */
    pop {lr}            /* lr出栈 */
    subs pc, lr, #4     /* 将lr-4赋给pc寄存器 */

/* FIQ中断服务函数 */
FIQ_Handler:
    ldr r0, =FIQ_Handler
    bx  r0
