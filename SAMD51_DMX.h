#pragma once

class SAMD51DMX {
public:
  SAMD51DMX();
  ~SAMD51DMX();

  void begin(uint16_t channels);
  void set(uint16_t chan, uint8_t data);
  void set(uint8_t *data, uint16_t n);
  void flush(uint16_t n);
  void flush();

  void irq();
  
private:
  void baud(uint32_t baud);
  
  void txComplete(void);
  void dataRegEmpty(void);

  uint16_t channels;
  uint8_t *write_buf;
  uint8_t *tx_buf;

  bool busy;
  uint8_t state;
  
  uint16_t i;
  uint16_t n;
};

extern SAMD51DMX DMX;
