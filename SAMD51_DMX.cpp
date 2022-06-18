#include <Arduino.h>
//#include <string.h>
#include <SERCOM.h>
#include <Uart.h>
#include <variant.h>
#include <wiring_private.h> 

#include "SAMD51_DMX.h"

constexpr uint32_t DATA_BAUD  = 250000;
constexpr uint32_t BREAK_BAUD = 90000;

constexpr uint8_t STATE_BREAK = 0;
constexpr uint8_t STATE_START = 1;
constexpr uint8_t STATE_DATA = 2;
constexpr uint8_t STATE_IDLE = 3;

constexpr uint8_t PIN_SAMD51DMX_RX = A5;
constexpr SercomRXPad PAD_SAMD51DMX_RX = SERCOM_RX_PAD_2;

constexpr uint8_t PIN_SAMD51DMX_TX = A4;
constexpr SercomUartTXPad PAD_SAMD51DMX_TX = UART_TX_PAD_0;

SAMD51DMX DMX;

Uart SerialDMX(&sercom0,
  PIN_SAMD51DMX_RX, PIN_SAMD51DMX_TX,
  PAD_SAMD51DMX_RX, PAD_SAMD51DMX_TX);

void SERCOM0_0_Handler() { DMX.irq(); }
void SERCOM0_1_Handler() { DMX.irq(); }
void SERCOM0_2_Handler() { DMX.irq(); }
void SERCOM0_3_Handler() { DMX.irq(); }


SAMD51DMX::SAMD51DMX() {}
SAMD51DMX::~SAMD51DMX() {
  delete this->write_buf;
  delete this->tx_buf;
}

void SAMD51DMX::begin(uint16_t channels) {
  this->channels = channels;
  
  this->write_buf = new uint8_t[channels];
  this->tx_buf = new uint8_t[channels];
  memset(write_buf, 0, channels);
  
  SerialDMX.begin(DATA_BAUD, SERIAL_8N2);
  
  pinPeripheral(PIN_SAMD51DMX_RX, PIO_SERCOM_ALT);
  pinPeripheral(PIN_SAMD51DMX_TX, PIO_SERCOM_ALT);

  SERCOM0->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_RXC |
                                SERCOM_USART_INTENCLR_ERROR;

  this->busy = false;
  this->state = STATE_BREAK;
  this->i = channels;
}

void SAMD51DMX::set(uint16_t chan, uint8_t data) {
  this->write_buf[chan] = data;
}

void SAMD51DMX::set(uint8_t *data, uint16_t n) {
  memcpy(this->write_buf + 1, data, n);
}

void SAMD51DMX::flush(uint16_t n) {
  if (!this->busy) {
    this->busy = true;

    memcpy(tx_buf, write_buf, channels);

    this->n = n;
    this->state = STATE_BREAK;
    this->txComplete();
  }
}

void SAMD51DMX::flush() {
  this->flush(this->channels);
}

void SAMD51DMX::irq() { 
  if (SERCOM0->USART.INTFLAG.bit.TXC) {
    this->txComplete();
  } else if (SERCOM0->USART.INTFLAG.bit.DRE) {
    this->dataRegEmpty();
  }
}

void SAMD51DMX::txComplete() {
  if (this->state == STATE_BREAK) {
    this->baud(BREAK_BAUD);
    this->state = STATE_START;
    this->i = 0;
    SERCOM0->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
    SERCOM0->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC;
    SERCOM0->USART.DATA.reg = 0;
  } else if (this->state == STATE_IDLE) {
    SERCOM0->USART.INTFLAG.bit.TXC = 1;
    SERCOM0->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
    SERCOM0->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC;
    this->state = STATE_BREAK;
    this->busy = false;
  } else if (this->state == STATE_START) {
    this->baud(DATA_BAUD);
    this->state = STATE_DATA;
    SERCOM0->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC;
    SERCOM0->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;
    //SERCOM0->USART.DATA.reg = 0;
    SERCOM0->USART.DATA.reg = this->tx_buf[this->i++];
  }
}

void SAMD51DMX::dataRegEmpty(void) {
  if (this->i < this->n) {
    SERCOM0->USART.DATA.reg = this->tx_buf[this->i++];
  } else {
    this->state = STATE_IDLE;
    SERCOM0->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
    SERCOM0->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC;
  }
}

void SAMD51DMX::baud(uint32_t baud) {
  SERCOM0->USART.CTRLA.bit.ENABLE = 0;
  
  uint32_t baudTimes8 = (SERCOM_FREQ_REF * 8) / (16 * baud);
  SERCOM0->USART.BAUD.FRAC.FP   = baudTimes8 % 8;
  SERCOM0->USART.BAUD.FRAC.BAUD = baudTimes8 / 8;
  
  SERCOM0->USART.CTRLA.bit.ENABLE = 1;
}
