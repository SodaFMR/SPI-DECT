/******************************************************************************
Copyright (c) 2023  Nordic Semiconductor ASA
SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************
Simple broadcast of DECT NR+ messages, for initial testing. 

2 Development Kits for 9161 needed
   1st transmits a counter value, needs only power, prints progress information
   2nd receives the value, needs terminal for printout

PC SW: nRF Connect for Desktop 
For development Zephyr and IDE (VSCode and nRF Connect extension)

Device first listens on hard-coded channel for 10secs, if no transmission detected, 
starts sending for-ever. Another device can start, listens and stays in listen 
mode, simple statistics are provided at end, when button is pressed. Reset 
board to start again

RX accepts first received counter value, and after that detects if messages
lost between receptions, increases a counter for missed messages
- RX calculates CRC error callbacks
- RX printouts basic data for each message received the to show progress
- RX will end if nothing received in 10 secs or if any button on DK pressed
- When RX ends, simple statistics shown

TX Mode: leds blink
RX mode: leds on
*******************************************************************************
PRINTOUT RX. NOTE that logging must be on and level INF should be used. 
Logging is faster than printk, so the progress uses logging output

RECEIVED DATA, 1386, rssi_2, -50,  DATA, 20314, missed/crc errors,  19
RECEIVED DATA, 1387, rssi_2, -50,  DATA, 20315, missed/crc errors,  19
...
PCC CRC ERROR, rssi_2, -52, crc error count,  7
RECEIVED DATA, 1389, rssi_2, -50,  DATA, 20318, missed/crc errors,  20
...
Exit on timeout or button
*********************************************
Received messages 1409
Missed messages 13
CRC errors  7
RSSI_2 AVG (rounded) -53
*********************************************

****************************************************************************
TODO:
- floats, config issue
****************************************************************************/

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_dect_phy.h>
#include "mac_header_field.h"


LOG_MODULE_REGISTER(app);

//state flags
//switch between RX or TX role
int RECEIVER_ROLE = 1;
int INIT_DONE = 0;
int EXIT = 0;

//handle values for API calls, separate tx and rx
int txHandle = 1;
int rxHandle = 31400;
// statistics collecting
int previous_received=-1;
int missing_errors=0;
int crc_errors=0;
int received_ok =0;
float rssi_average = 0;
int n=0;
int own_crc_errors=0;

//Note that the MCS impacts how much data can be fit into subslots/slots
//this is now hardcoded assuming MCS 1
#define MCS 4
// tx power, 0x0b == 19 dBm, maximum of the HW
#define POWER  0xb
//this depends on the MCS above
#define DATA_LEN 700
static uint8_t _txData[DATA_LEN];
static uint8_t _rxData[DATA_LEN];
//NOTE !!! defining  carrier to use, please modify according to region and scan results
#define CARRIER 1677

//semaphore for API calls, only 1 async operation at time in this sample 
K_SEM_DEFINE(modem, 1, 1);

mlcf_b_1t mac_header;


//TYPE 1 (SHORT BROADCAST) FORMAT 000 header for messages used
struct phy_ctrl_field_common
{
    uint32_t packet_length           : 4;
    uint32_t packet_length_type      : 1;
    uint32_t header_format           : 3;
    uint32_t short_network_id        : 8;
    uint32_t transmitter_id_hi : 8;
    uint32_t transmitter_id_lo : 8;
    uint32_t df_mcs         : 3;
    uint32_t reserved       : 1;
    uint32_t transmit_power : 4;
    uint32_t pad            : 24;
};
union nrf_modem_dect_phy_hdr phyHeader;

//new parameters for HARQ operation, not used in this sample
const struct nrf_modem_dect_phy_init_params init_params ={
	.harq_rx_expiry_time_us=5000000,
	.harq_rx_process_count=1
};

//Own function
uint8_t calcularChecksum(const uint8_t *buffer, size_t longitud) {
    uint16_t suma = 0;

    // Sumar todos los bytes del buffer
    for (size_t i = 2; i < longitud-2; ++i) {
        suma += buffer[i];
    }

    // Tomar los 8 bits menos significativos del resultado
    uint8_t checksum = (uint8_t)(suma & 0xFF);

    return checksum;
}



//timer is called if there is nothing received in receiver role in 10 seconds 
//if in init mode, decides then to start sending
//if init done, in receiver role, will exit
void timer_expiry_function(struct k_timer *dummy)
{
    //timeout after the initial role selection means exit
    if(INIT_DONE == 1) EXIT=1;
    //timeout on initial listen, go to tx mode, init is done
    else {
      INIT_DONE =1;
      RECEIVER_ROLE = 0;
    }
}
K_TIMER_DEFINE(my_timer, timer_expiry_function, NULL);

//exit on ANY button on the devkit
void button_handler(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & button_state & DK_BTN1_MSK) {
                LOG_INF("Button 1 pressed");
                LOG_INF("Initializing broadcast demo....");
                //demo();
        }
        if (has_changed & button_state & DK_BTN2_MSK) {
                LOG_INF("Button 2 pressed");
                LOG_INF("Initializing data transmission....");
                simple_tx();
        }
        if (has_changed & button_state & DK_BTN3_MSK) {
                LOG_INF("Button 3 pressed");
                LOG_INF("Initializing data reception....");
                simple_rx();
        }
        if (has_changed & button_state & DK_BTN4_MSK) {
                LOG_INF("Button 4 pressed");
                EXIT = 1;
        }
}

//ETSI TS 103 636-2  spec 8.3.3 RSSI is reported every 0.5dbm
//if successful reception, calculate the average 
int32_t calcRSSI(int16_t recrssi, int is_success){
  float resp =-20-((-recrssi-1)*0.5);
  //avg_new=avg_old+(value-avg_old)/n
  if(is_success) {
    n++;
    float new_average = rssi_average + (resp-rssi_average)/n;
    rssi_average = new_average;
  }
  return (int32_t)resp;
}

// Callback functions from PHY API
void init(const uint64_t *time, int16_t temp, enum nrf_modem_dect_phy_err err, const struct nrf_modem_dect_phy_modem_cfg *cfg)
{
  if(err==0) {
    LOG_INF("DECT Init done, temperature %d", temp);
  }
  else {
    LOG_ERR("INIT FAILED");
    printk("Init failed, Exit\n");
    EXIT=1;
  }
  k_sem_give(&modem);
}

void op_complete(const uint64_t *time, int16_t temperature, enum nrf_modem_dect_phy_err err, uint32_t handle)
{
  LOG_DBG("operation_complete_cb Status %d, Temp %d, Handle %d", err, temperature, handle);
  k_sem_give(&modem);
  return;
}

void rssi(const uint64_t *time, const struct nrf_modem_dect_phy_rssi_meas *status)
{   
  k_sem_give(&modem);
  return;
}


void rx_stop(const uint64_t *time, enum nrf_modem_dect_phy_err err, uint32_t handle)
{
  LOG_DBG("operation_stop_cb Status %d Handle %d", err, handle);
  k_sem_give(&modem);
  return;
}

//phy header receive
void pcc(
  const uint64_t *time,
  const struct nrf_modem_dect_phy_rx_pcc_status *status,
  const union nrf_modem_dect_phy_hdr *hdr)
{
  LOG_DBG("pcc_cb phy_header_valid %d rssi_2 %d", status->header_status, status->rssi_2);
  return;
}

void pcc_crc_err(const uint64_t *time, const struct nrf_modem_dect_phy_rx_pcc_crc_failure *crc_failure)
{
  crc_errors++;
  int16_t resp=calcRSSI(crc_failure->rssi_2, 0);
  LOG_INF("PCC CRC ERROR, rssi_2, %d, crc error count,  %d, continuing", resp, crc_errors);
}

//data paylod receive, statistics calculation and tracking previous received message number
void pdc(
  const uint64_t *time,
  const struct nrf_modem_dect_phy_rx_pdc_status *status,
  const void *data, uint32_t len)
{
  // printk(data);
  // printk("\n");
  // uint32_t recieved_data;
  // memcpy(&_rxData, data, DATA_LEN);
  
  // for(int i=0;i<DATA_LEN;i++)
  // {
  //   recieved_data += (uint32_t)(_rxData[i] << (8*(DATA_LEN-i-1)));
  // }
  // printk("Alphabet received: %d\n", recieved_data);
  
  
  // for(int i=0;i<DATA_LEN/8;i++)
  // {
  //   printk("%c", recieved_data >> (8*i) & 0xff);
  //   printk("\n");
  // } 

  int rx_counter=0;
  int32_t rx_rssi=calcRSSI(status->rssi_2, 1);
  memcpy(&_rxData, data, DATA_LEN);
  rx_counter = _rxData[0];
  //LOG_INF("%d", rx_counter);
  uint8_t calculatedChecksum = 0;
  uint8_t checksum = _rxData[1];

  // Uncomment to check CRC
  calculatedChecksum = calcularChecksum(_rxData, DATA_LEN);

  if(checksum != calculatedChecksum)
  {
    own_crc_errors++;
    // LOG_INF("CRC ERROR, %d\n", own_crc_errors);
  }
  else
  {
    // LOG_INF("CRC OK\n");
  }

  // received_ok++;

  // mac_header.mlcf_a = _rxData[3];
  memcpy(&mac_header.mlcf_b_1[0], &_rxData[3], 2);
  get_mlcf_b_1_rev(&mac_header);

  


  if(previous_received>0 && rx_counter != previous_received+1){
    //multiple errors if multiple messages lost
    if(rx_counter != 0)
    {
      missing_errors=missing_errors+(rx_counter-previous_received)-1;
    }
  }
  previous_received=rx_counter;
  // first listen done, somethig received, continue listen
  if(INIT_DONE==0) {
    LOG_INF("INITIAL listenting, received data");
    LOG_INF("RECEIVED DATA, %d, rssi_2, %d, missed errors, %d, crc errors, %d\n", rx_counter, rx_rssi, missing_errors, own_crc_errors);
    INIT_DONE=1;
    RECEIVER_ROLE=1;
  }
  else if(rx_counter%100==0) {
    LOG_INF("RECEIVED DATA, %d, rssi_2, %d, missed errors, %d, crc errors, %d\n", rx_counter, rx_rssi, missing_errors, own_crc_errors);
    
    //Prints for MAC Headers testings
    LOG_INF("Version: %d\n", mac_header.Reserved);
    LOG_INF("Security: %d\n", mac_header.Reset);
    LOG_INF("HeaderType: %d\n", mac_header.SequenceNumber);

    //Prints to check that the package is filled.
    // LOG_INF("Mid byte: %u\n", _rxData[DATA_LEN/2]);
  }

  // Default code ---------------------------------------------------------------
  // int rx_counter=0;
  // int32_t rx_rssi=calcRSSI(status->rssi_2, 1);
  // memcpy(&_rxData, data, 4);
  // rx_counter = (_rxData[0]<<24) + (_rxData[1]<<16) + (_rxData[2]<<8) + (_rxData[3]);
  // received_ok++;
  // if(previous_received>0 && rx_counter != previous_received+1){
  //   //multiple errors if multiple messages lost
  //   missing_errors=missing_errors+(rx_counter-previous_received)-1;
  // }
  // previous_received=rx_counter;
  // // first listen done, somethig received, continue listen
  // if(INIT_DONE==0) {
  //   LOG_INF("INITIAL listenting, received data");
  //   LOG_INF("RECEIVED DATA, %d, rssi_2, %d, missed/crc errors, %d", rx_counter, rx_rssi, missing_errors);
  //   INIT_DONE=1;
  //   RECEIVER_ROLE=1;
  // }
  // //if(rx_counter%10==0) 
  // printk("RECEIVED DATA, %d, rssi_2, %d, missed/crc errors, %d\n", rx_counter, rx_rssi, missing_errors);
  // restart the timer
  k_timer_start(&my_timer, K_SECONDS(10), K_SECONDS(10));
}

void pdc_crc_err(
  const uint64_t *time, const struct nrf_modem_dect_phy_rx_pdc_crc_failure *crc_failure)
{
  crc_errors++;
  int16_t resp=calcRSSI(crc_failure->rssi_2, 0);
  LOG_INF("PDC CRC ERROR, rssi_2, %d, crc error count, %d, continuing", resp, crc_errors);
}

void link_config(const uint64_t *time, enum nrf_modem_dect_phy_err err)
{
  return;
}

void time_get(const uint64_t *time, enum nrf_modem_dect_phy_err err)
{
  LOG_DBG("Time query response time %"PRIu64" Status %d", *time, err); 
}

void capability_get(const uint64_t *time, enum nrf_modem_dect_phy_err err,const struct nrf_modem_dect_phy_capability *capability)
{
  LOG_DBG("Capability query response FIXME %"PRIu64" Status %d", *time, err); 
}

void deinit(const uint64_t *time, enum nrf_modem_dect_phy_err err)
{
  LOG_DBG("DEINIT response time %"PRIu64" Status %d", *time, err); 
}

//set function callbacks to structure, registered to  modem API  in main
struct nrf_modem_dect_phy_callbacks dect_cb_config = {
    .init = init,
    .op_complete = op_complete,
    .rssi = rssi,
    .rx_stop = rx_stop,
    .pcc = pcc,
    .pcc_crc_err = pcc_crc_err,
    .pdc = pdc,
    .pdc_crc_err = pdc_crc_err,
    .link_config = link_config,
    .time_get = time_get,
    .capability_get = capability_get,
    .deinit=deinit
};

//listen, start immediately and listen for time_s duration
void modem_rx(uint32_t rxMode, int time_s)
{
  // Setup the nrf_modem_dect_phy_operation_rx
  struct nrf_modem_dect_phy_rx_params rxOpsParams={0};
  rxOpsParams.start_time = 0; //start immediately
  rxOpsParams.handle = rxHandle;
  rxOpsParams.network_id=0;
  rxOpsParams.mode = rxMode;
  rxOpsParams.link_id = NRF_MODEM_DECT_PHY_LINK_UNSPECIFIED;
  rxOpsParams.rssi_level = -60;
  rxOpsParams.carrier = CARRIER;
  // modem clock ticks NRF_MODEM_DECT_MODEM_TIME_TICK_RATE_KHZ --> 69120*1000* TIME_S
  rxOpsParams.duration = time_s*69120*1000; 
  //filter on the short network id, last 8 bits of the network identifier in dect nr
  rxOpsParams.filter.short_network_id = (uint8_t)(0x0a);
  rxOpsParams.filter.is_short_network_id_used = 1;
  //listen for everything (broadcast mode usedd)
  rxOpsParams.filter.receiver_identity = 0;
  k_sem_take(&modem, K_FOREVER);
  int err=nrf_modem_dect_phy_rx(&rxOpsParams);
  if(err!=0) LOG_ERR("RX FAIL %d", err);
  if(rxHandle==65000)rxHandle=31400;
  else rxHandle++;
}

//send counter value immediately, start_time=0
void modem_tx(uint8_t* data, size_t datasize, uint8_t* header, size_t header_size)
{
  // with the MCS 1 , data must be 37 bytes
  // copy the increasing counter value to message to be sent, fill rest with 'A'
  // uint8_t tmp[DATA_LEN];
  // tmp[0] = (i >> 24) & 0xff;
  // tmp[1] = (i >> 16) & 0xff;
  // tmp[2] = (i >> 8) & 0xff;
  // tmp[3] = (i) & 0xff;


  // Pretende hacer lo mismo que arriba pero adaptandose al tamaño de DATA_LEN
  // uint8_t tmp[DATA_LEN];
  // for(int j=0;j<DATA_LEN/8;j++)
  // {
  //   tmp[j] = (i >> 8*(DATA_LEN/8)) & 0xff;  //Desplaza 
  // }
 
  //copy to send buffer
  memcpy(_txData, data, datasize);
  // struct phy_ctrl_field_common header = {
  //     //short header format, broadcast
  //     .header_format = (uint8_t)0x0,
  //     //length given in subslots
  //     .packet_length_type =  (uint8_t)0x0,
  //     //lenght in type
  //     .packet_length = (uint8_t) 0x07,
  //     .short_network_id = (uint8_t)(0x0a & 0xff),
  //     //made up transmitter ID
  //     .transmitter_id_hi = (uint8_t) (0x0101 >> 8),
  //     .transmitter_id_lo = (uint8_t) (0x0101 & 0xff),
  //     .transmit_power = POWER,
  //     .reserved = 0,
  //     //lowest MCS, maximum range aimed
  //     .df_mcs = MCS
  // };
  memcpy(&phyHeader.type_1, &header, header_size);

  // Setup the nrf_modem_dect_phy_operation_tx
  struct nrf_modem_dect_phy_tx_params txOpsParams;
  //immediate operation
  txOpsParams.start_time = 0; 
  txOpsParams.handle = txHandle;
  //netework id value, used in rx filtering
  txOpsParams.network_id = 0x0a;
  txOpsParams.phy_type = 0;
  txOpsParams.lbt_rssi_threshold_max = 0;
  //  EU carrier, see ETSI TS 103 636-2 5.4.2 for the calculation 
  txOpsParams.carrier = CARRIER;
  //NOTE !!! no LBT done
  txOpsParams.lbt_period = 0;
  txOpsParams.phy_header = &phyHeader;
  txOpsParams.data = _txData;
  txOpsParams.data_size = datasize;

  // and call nrf_modem_dect_phy_schedule_tx_operation_add()
  k_sem_take(&modem, K_FOREVER);
  int err=nrf_modem_dect_phy_tx(&txOpsParams);
  if(err!=0) LOG_ERR("TX FAIL %d", err);
  if(txHandle==30000)txHandle=1;
  else txHandle++;
}


int main(void)
  {
    printk("START DECT SAMPLE\n");
    dk_buttons_init(button_handler);
    printk("Buttons init\t");
    dk_leds_init();
    printk("leds init\t");

    nrf_modem_lib_init();
  
    k_sem_take(&modem, K_FOREVER);
    int err=0;
    err=nrf_modem_dect_phy_callback_set(&dect_cb_config);
    if(err!=0) {
      printk("ERROR settings callbacks %d\n",err);
    }
    err=nrf_modem_dect_phy_init(&init_params);
    if(err!=0) {
      printk("ERROR initializing modem PHY %d\n",err);
      return -1;
    }
    printk("DECT init started\n");
    
    // //printk("Listening on channel %d for 10 secs to see if there is transmissions\n", CARRIER);
    // k_timer_start(&my_timer, K_SECONDS(10), K_SECONDS(10));
    // modem_rx(NRF_MODEM_DECT_PHY_RX_MODE_SINGLE_SHOT, 10);

    int i = 0;
    while(1){
      k_msleep(930);
      if(i%2==0) {
        dk_set_led_on(DK_LED1);
      }
      else {
        dk_set_led_off(DK_LED1);
      }
        i++;
        if(i==10) i=0;
        if(i == INT_MAX) return 0;
    }
  return 0;
}

int simple_rx(void)
{
  
  
  dk_set_led_on(DK_LED1);
  dk_set_led_on(DK_LED2);
  printk("RX ROLE\n");
  //timer is used, if there is 10 seconds without any reception, exits
  //timer is refreshed/restarted in reception
  k_timer_start(&my_timer, K_SECONDS(10), K_SECONDS(10));
  //loop until above timer sets the EXIT flag when nothing received in 10 secs
  while(EXIT==0){
    //loop RX mode   
    modem_rx(NRF_MODEM_DECT_PHY_RX_MODE_SINGLE_SHOT, 2);
  }
  return 0;
}

static int slot = 0;
static uint8_t txData[DATA_LEN];
int simple_tx(void)
{
  EXIT = 0;
  // uint32_t i =1;
  int package = 0;
  int i = 0;  
  int j = 65;

  //Example increasing counter
  dk_set_led_on(1);
  printk("TX ROLE\n");
  while(EXIT == 0) {
    k_msleep(5);

    // if(slot < 2){
    //   k_usleep(500);
    //   slot++;
    // }
    // else if(slot < 4){
    //   k_usleep(1000);
    //   slot = 0;
    // }
    // k_usleep(500);
    // if(i%2==0) {
    //   dk_set_led_on(DK_LED1);
    //   dk_set_led_on(DK_LED2);
    // }
    // else {
    //   dk_set_led_off(DK_LED1);
    //   dk_set_led_off(DK_LED2);
    // }
    // modem_tx(i);
    // if(i%10==0) printk("TX %d\n",i);
    // i++;
    // if(i==INT32_MAX) {
    //   i=0;
    //   LOG_INF("Sent integer value looped");
    // }

    // //Own code
    // while (i < DATA_LEN) {
    //   alphabet |= (uint32_t)((j + i) & 0xFF) << (i * 8);
    //   if(j == 122) j = 65;
    //   i++;
    // }
    // i = 0;
    // printk("Alphabet: %d\n", alphabet);
    // //printk("Alphabet: %c\n", alphabet); Sólo imprime el último byte
    // for (int i = 3; i >= 0; i--) {
    //   char c = (alphabet >> (i * 8)) & 0xFF;
    //   printk("%c", c);
    // }
    // printk("\n");
    // printk("Alphabet: %x\n", alphabet);

    // modem_tx(alphabet);
    // LOG_INF("Sended package nº %d", package);
    // package++;


    //Own code
    i = 0;
    txData[0] = package;      // Añadimos el número de paquete

    
    mac_header.Reserved = 6;
    mac_header.Reset = 1;
    mac_header.SequenceNumber = 800;
    get_mlcf_b_1(&mac_header);

    // txData[3] = mac_header.mlcf_a;
    memcpy(&txData[3], &mac_header.mlcf_b_1[0], 2);



    while (i < DATA_LEN) {
      txData[i+5] = j;
      i++;
      j++;
      if (j > 122) {
        j = 65;
      }
    }

    txData[1] = 0;
    
    // Uncomment to add CRC
    txData[1] = calcularChecksum(txData, DATA_LEN);
    
    // printk("\n");
    // printk("Alphabet: %x\n", alphabet);

    //modem_tx(txData, DATA_LEN);
    if(package%100 == 0)LOG_INF("Sended package nº %d", package);
    package++;

  }
  return 0;
}


