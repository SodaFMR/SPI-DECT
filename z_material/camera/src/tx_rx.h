#include <strings.h>

#ifndef TX_RX_H
#define TX_RX_H


int modem_rx(void);
int modem_tx(uint8_t* data, size_t length, uint8_t* header, size_t header_length);
void init(void);
void op_complete(void);
void rssi(void);
void rx_stop(void);
void pcc(void);
void pcc_crc_err(void);
void pdc(void);
void pdc_crc_err(void);
void link_config(void);
void time_get(void);
void capability_get(void);
void deinit(void);


#endif