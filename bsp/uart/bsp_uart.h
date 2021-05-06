#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "imx6ul.h"

typedef enum _uart_parity_mode
{
    kUART_ParityDisabled = 0x0U, /*!< Parity disabled */
    kUART_ParityEven = 0x2U,     /*!< Even error check is selected */
    kUART_ParityOdd = 0x3U,      /*!< Odd error check is selected */
} uart_parity_mode_t;

typedef enum _uart_data_bits
{
    kUART_SevenDataBits = 0x0U, /*!< Seven data bit */
    kUART_EightDataBits = 0x1U, /*!< Eight data bit */
} uart_data_bits_t;

typedef enum _uart_stop_bit_count
{
    kUART_OneStopBit = 0x0U, /*!< One stop bit */
    kUART_TwoStopBit = 0x1U, /*!< Two stop bits */
} uart_stop_bit_count_t;

/*! @brief UART configuration structure. */
typedef struct _uart_config
{
    uint32_t baudRate_Bps;              /*!< UART baud rate. */
    uart_parity_mode_t parityMode;      /*!< Parity error check mode of this module. */
    uart_data_bits_t dataBitsCount;     /*!< Data bits count, eight (default), seven */
    uart_stop_bit_count_t stopBitCount; /*!< Number of stop bits in one frame. */
    uint8_t txFifoWatermark;            /*!< TX FIFO watermark */
    uint8_t rxFifoWatermark;            /*!< RX FIFO watermark */
    bool enableAutoBaudRate;            /*!< Enable automatic baud rate detection */
    bool enableTx;                      /*!< Enable TX */
    bool enableRx;                      /*!< Enable RX */
} uart_config_t;

void uart1_init(void);
void UART_GetDefaultConfig(uart_config_t *config);
status_t UART_Init(UART_Type *base, const uart_config_t *config, uint32_t srcClock_Hz);

/*!
 * @brief This function is used to Disable the UART Module.
 *
 * @param base UART base pointer.
 */
static inline void UART_Disable(UART_Type *base)
{
    base->UCR1 &= ~UART_UCR1_UARTEN_MASK;
}

/*!
 * @brief Resets the UART using software.
 *
 * This function resets the transmit and receive state machines, all FIFOs and register
 * USR1, USR2, UBIR, UBMR, UBRC , URXD, UTXD and UTS[6-3]
 *
 * @param base UART peripheral base address.
 */
static inline void UART_SoftwareReset(UART_Type *base)
{
    base->UCR2 &= ~UART_UCR2_SRST_MASK;
    while ((base->UCR2 & UART_UCR2_SRST_MASK) == 0)
    {
    }
}

/*!
 * @brief This function is used to set the watermark of UART Tx FIFO.
 *        A maskable interrupt is generated whenever the data level in
 *        the TxFIFO falls below the Tx FIFO watermark.
 *
 * @param base UART base pointer.
 * @param watermark The Tx FIFO watermark.
 */
static inline void UART_SetTxFifoWatermark(UART_Type *base, uint8_t watermark)
{
    //assert((watermark >= 2) && (watermark <= FSL_FEATURE_IUART_FIFO_SIZEn(base)));
    base->UFCR = (base->UFCR & ~UART_UFCR_TXTL_MASK) | UART_UFCR_TXTL(watermark);
}

/*!
 * @brief This function is used to set the watermark of UART Rx FIFO.
 *        A maskable interrupt is generated whenever the data level in
 *        the RxFIFO reaches the Rx FIFO watermark.
 *
 * @param base UART base pointer.
 * @param watermark The Rx FIFO watermark.
 */
static inline void UART_SetRxFifoWatermark(UART_Type *base, uint8_t watermark)
{
    //assert(watermark <= FSL_FEATURE_IUART_FIFO_SIZEn(base));
    base->UFCR = (base->UFCR & ~UART_UFCR_RXTL_MASK) | UART_UFCR_RXTL(watermark);
}

/*!
 * @brief This function is used to set the enable condition of
 *        Automatic Baud Rate Detection feature.
 *
 * @param base UART base pointer.
 * @param enable Enable/Disable Automatic Baud Rate Detection feature.
 *               - true: Enable Automatic Baud Rate Detection feature.
 *               - false: Disable Automatic Baud Rate Detection feature.
 */
static inline void UART_EnableAutoBaudRate(UART_Type *base, bool enable)
{
    if (enable)
    {
        /* When ADET=0 and ADBR=1, automatic baud rate detection starts */
        /* Enable automatic baud rate detection */
        base->UCR1 |= UART_UCR1_ADBR_MASK;
        /* Clear ADET brfore start automatic baud rate detection*/
        base->USR2 |= UART_USR2_ADET_MASK;
    }
    else
    {
        /* Disable automatic baud rate detection */
        base->UCR1 &= ~UART_UCR1_ADBR_MASK;
    }
}

status_t UART_SetBaudRate(UART_Type *base, uint32_t baudRate_Bps, uint32_t srcClock_Hz);
/*!
 * @brief This function is used to Enable the UART Module.
 *
 * @param base UART base pointer.
 */
static inline void UART_Enable(UART_Type *base)
{
    base->UCR1 |= UART_UCR1_UARTEN_MASK;
}

void UART_WriteBlocking(UART_Type *base, const uint8_t *data, size_t length);

static inline void UART_WriteByte(UART_Type *base, uint8_t data)
{
    base->UTXD = data & UART_UTXD_TX_DATA_MASK;
}

void UART_WriteNum(const uint32_t data);

#endif