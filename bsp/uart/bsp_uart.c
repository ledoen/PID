#include "bsp_uart.h"

/* 初始化函数 uart1_init() */

void uart1_init(void)
{
	uart_config_t config;
	/*
	* config.baudRate_Bps = 115200U;
	* config.parityMode = kUART_ParityDisabled;
	* config.dataBitsCount = kUART_EightDataBits;
	* config.stopBitCount = kUART_OneStopBit;
	* config.txFifoWatermark = 2;
	* config.rxFifoWatermark = 1;
	* config.enableTx = false;
	* config.enableRx = false;
	*/
	UART_GetDefaultConfig(&config);
	config.baudRate_Bps = 115200U;
	config.enableTx = true;
	config.enableRx = true;
	UART_Init(UART1, &config, 80000000);	/* UART默认时钟为PLL3，80MHz */
	uint8_t txbuff[] = "hello world";
	
	UART_WriteBlocking(UART1, txbuff, sizeof(txbuff) - 1);
/*	UART_WriteByte(UART1, '\r');
	UART_WriteByte(UART1, '\n');
	UART_WriteNum(0x8008abcd);
*/	
}

void UART_GetDefaultConfig(uart_config_t *config)
{
    //assert(config);

    config->baudRate_Bps = 115200U;
    config->parityMode = kUART_ParityDisabled;
    config->dataBitsCount = kUART_EightDataBits;
    config->stopBitCount = kUART_OneStopBit;
    config->txFifoWatermark = 2;
    config->rxFifoWatermark = 1;
    config->enableAutoBaudRate = false;
    config->enableTx = false;
    config->enableRx = false;
}

status_t UART_Init(UART_Type *base, const uart_config_t *config, uint32_t srcClock_Hz)
{
    /* Check argument */
    //assert(!((NULL == base) || (NULL == config) || (0U == srcClock_Hz)));

    /* Disable UART Module. */
    UART_Disable(base);
    /* Reset the transmit and receive state machines, all FIFOs and register
     * USR1, USR2, UBIR, UBMR, UBRC, URXD, UTXD and UTS[6-3]. */
    UART_SoftwareReset(base);

    /* Set UART Module Register content to default value */
    base->UCR1 = 0x0;
    base->UCR2 = UART_UCR2_SRST_MASK;
    base->UCR3 = UART_UCR3_DSR_MASK | UART_UCR3_DCD_MASK | UART_UCR3_RI_MASK;
    base->UCR4 = UART_UCR4_CTSTL(32);
    base->UFCR = UART_UFCR_TXTL(2) | UART_UFCR_RXTL(1);
    base->UESC = UART_UESC_ESC_CHAR(0x2B);
    base->UTIM = 0x0;
    base->ONEMS = 0x0;
    base->UTS = UART_UTS_TXEMPTY_MASK | UART_UTS_RXEMPTY_MASK;
    base->UMCR = 0x0;

    /* Set UART data word length, stop bit count, parity mode and communication
     * direction according to uart init struct, disable RTS hardware flow control.
     */
    base->UCR2 |=
        ((uint32_t)UART_UCR2_WS(config->dataBitsCount) | (uint32_t)UART_UCR2_STPB(config->stopBitCount) |
         (uint32_t)(((config->parityMode) << UART_UCR2_PROE_SHIFT) & (UART_UCR2_PREN_MASK | UART_UCR2_PROE_MASK)) |
         (uint32_t)UART_UCR2_TXEN(config->enableTx) | (uint32_t)UART_UCR2_RXEN(config->enableRx) | UART_UCR2_IRTS_MASK);

    /* For imx family device, UARTs are used in MUXED mode, so that this bit should always be set.*/

    base->UCR3 |= UART_UCR3_RXDMUXSEL_MASK;

    /* Set TX/RX fifo water mark */
    /*UART_SetTxFifoWatermark(base, config->txFifoWatermark);
    UART_SetRxFifoWatermark(base, config->rxFifoWatermark);*/

    if (config->enableAutoBaudRate)
    {
        /* Start automatic baud rate detection */
        UART_EnableAutoBaudRate(base, true);
    }
    else if (config->baudRate_Bps)
    {
        /* Stop automatic baud rate detection */
        UART_EnableAutoBaudRate(base, false);
        /* Set BaudRate according to uart initialize struct. Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)) */
        if (kStatus_Success != UART_SetBaudRate(base, config->baudRate_Bps, srcClock_Hz))
        {
            return 0;
        }
    }
    else
    {
        /* Stop automatic baud rate detection */
        UART_EnableAutoBaudRate(base, false);
    }

    /* Enable UART module */
    UART_Enable(base);

    return kStatus_Success;
}

/* This UART instantiation uses a slightly different baud rate calculation.
 * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
 * To get a baud rate, three register need to be writen, UFCR,UBMR and UBIR
 * At first, find the approximately maximum divisor of src_Clock and baudRate_Bps.
 * If the numerator and denominator are larger then register maximum value(0xFFFF),
 * both of numerator and denominator will be divided by the same value, which
 * will ensure numerator and denominator range from 0~maximum value(0xFFFF).
 * Then calculate UFCR and UBIR value from numerator, and get UBMR value from denominator.
 */
status_t UART_SetBaudRate(UART_Type *base, uint32_t baudRate_Bps, uint32_t srcClock_Hz)
{
    uint32_t numerator = 0u;
    uint32_t denominator = 0U;
    uint32_t divisor = 0U;
    uint32_t refFreqDiv = 0U;
    uint32_t divider = 1U;
    uint64_t baudDiff = 0U;
    uint64_t tempNumerator = 0U;
    uint32_t tempDenominator = 0u;

    /* get the approximately maximum divisor */
    numerator = srcClock_Hz;
    denominator = baudRate_Bps << 4;
    divisor = 1;

    while (denominator != 0)
    {
        divisor = denominator;
        denominator = numerator % denominator;
        numerator = divisor;
    }

    numerator = srcClock_Hz / divisor;
    denominator = (baudRate_Bps << 4) / divisor;

    /* numerator ranges from 1 ~ 7 * 64k */
    /* denominator ranges from 1 ~ 64k */
    if ((numerator > (UART_UBIR_INC_MASK * 7)) || (denominator > UART_UBIR_INC_MASK))
    {
        uint32_t m = (numerator - 1) / (UART_UBIR_INC_MASK * 7) + 1;
        uint32_t n = (denominator - 1) / UART_UBIR_INC_MASK + 1;
        uint32_t max = m > n ? m : n;
        numerator /= max;
        denominator /= max;
        if (0 == numerator)
        {
            numerator = 1;
        }
        if (0 == denominator)
        {
            denominator = 1;
        }
    }
    divider = (numerator - 1) / UART_UBIR_INC_MASK + 1;

    switch (divider)
    {
        case 1:
            refFreqDiv = 0x05;
            break;
        case 2:
            refFreqDiv = 0x04;
            break;
        case 3:
            refFreqDiv = 0x03;
            break;
        case 4:
            refFreqDiv = 0x02;
            break;
        case 5:
            refFreqDiv = 0x01;
            break;
        case 6:
            refFreqDiv = 0x00;
            break;
        case 7:
            refFreqDiv = 0x06;
            break;
        default:
            refFreqDiv = 0x05;
            break;
    }
    /* Compare the difference between baudRate_Bps and calculated baud rate.
     * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
     * baudDiff = (srcClock_Hz/divider)/( 16 * ((numerator / divider)/ denominator).
     */
    tempNumerator = srcClock_Hz;
    tempDenominator = (numerator << 4);
    divisor = 1;
    /* get the approximately maximum divisor */
    while (tempDenominator != 0)
    {
        divisor = tempDenominator;
        tempDenominator = tempNumerator % tempDenominator;
        tempNumerator = divisor;
    }
    tempNumerator = srcClock_Hz / divisor;
    tempDenominator = (numerator << 4) / divisor;
    baudDiff = (tempNumerator * denominator) / tempDenominator;
    baudDiff = (baudDiff >= baudRate_Bps) ? (baudDiff - baudRate_Bps) : (baudRate_Bps - baudDiff);

    if (baudDiff < (baudRate_Bps / 100) * 3)
    {
        base->UFCR &= ~UART_UFCR_RFDIV_MASK;
        base->UFCR |= UART_UFCR_RFDIV(refFreqDiv);
        base->UBIR = UART_UBIR_INC(denominator - 1);
        base->UBMR = UART_UBMR_MOD(numerator / divider - 1);
        base->ONEMS = UART_ONEMS_ONEMS(srcClock_Hz / (1000 * divider));

        return kStatus_Success;
    }
    else
    {
        return 0;
    }
}

void UART_WriteBlocking(UART_Type *base, const uint8_t *data, size_t length)
{
    //assert(data);

    while (length--)
    {
        /* Wait for TX fifo valid.
         * This API can only ensure that the data is written into the data buffer but can't
         * ensure all data in the data buffer are sent into the transmit shift buffer.
         */
        while (!(base->USR2 & UART_USR2_TXDC_MASK))
        {
        }
        UART_WriteByte(base, *(data++));
    }
}

void UART_WriteNum(const uint32_t data)
{
	/* 从高到低取出每四位进行处理，共进行8次 */
	uint8_t ByteData;
	for(int i=0; i<8; i++){
		ByteData = ((data >> (28-i*4)) & 0xf);	/* 取出每4位 */
		switch(ByteData){
			case 0x0:{
				UART_WriteByte(UART1,'0');
				break;
			}
			case 0x1:{
				UART_WriteByte(UART1,'1');
				break;
			}
			case 0x2:{
				UART_WriteByte(UART1,'2');
				break;
			}
			case 0x3:{
				UART_WriteByte(UART1,'3');
				break;
			}
			case 0x4:{
				UART_WriteByte(UART1,'4');
				break;
			}
			case 0x5:{
				UART_WriteByte(UART1,'5');
				break;
			}
			case 0x6:{
				UART_WriteByte(UART1,'6');
				break;
			}
			case 0x7:{
				UART_WriteByte(UART1,'7');
				break;
			}
			case 0x8:{
				UART_WriteByte(UART1,'8');
				break;
			}case 0x9:{
				UART_WriteByte(UART1,'9');
				break;
			}
			case 0xa:{
				UART_WriteByte(UART1,'A');
				break;
			}
			case 0xb:{
				UART_WriteByte(UART1,'B');
				break;
			}
			case 0xc:{
				UART_WriteByte(UART1,'C');
				break;
			}
			case 0xd:{
				UART_WriteByte(UART1,'D');
				break;
			}
			case 0xe:{
				UART_WriteByte(UART1,'E');
				break;
			}
			case 0xf:{
				UART_WriteByte(UART1,'F');
				break;
			}
		}
	}
}
