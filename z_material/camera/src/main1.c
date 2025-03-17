/******************************************************************************

DECT NR+ implementation in progress
Property of Universidad Politécnica de Cartagena, QARTIA Department.
Autor: Iván Tomás García (ivan.tomas@edu.upct.es)
Start project date: 4/3/2023

****************************************************************************
TODO:
- A lot
****************************************************************************/

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_dect_phy.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include "mac_header_field.h"
#include "mac_message.h"
#include "procedures.h"
#include "physical_header_field.h"


LOG_MODULE_REGISTER(app);

// PROTOCOL OWN VARIABLES
const int SCAN_MEAS_DURATION = 24;   // time listening the channel in slots 
int8_t RSSI_THRESHOLD_MIN = -85;      // Lower limit for RSSI
int8_t RSSI_THRESHOLD_MAX = -33;      // Upper limit for RSSI
int SCAN_SUITABLE = 0;                // Number of subslots required to consider free or possible

//state flags
//switch between RX or TX role
int RECEIVER_ROLE = 1;
int INIT_DONE = 0;
int EXIT = 0;
int STATE = 0;
uint16_t lastSequenceNumber = 0;
bool DECT_PROTECTED = true;           // Defines if specific channel selection rules apply
bool isBroadcast = true;             // Flag that indicates if the next message is a broadcast
bool associated = false;            // Changes flowchart to transmission one.

// Timers
bool scanStatusValid = false;         // Flag that indicates both if timer is on and if actual value is valid
bool dectProTime = false;

int16_t measured_rssi = NULL;         // RSSI measured in the last scan
bool package_received = false;     // Flag that indicates if a package has been received

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

//SPIM Definitions
#define SPI_CS_PIN  20  // Pin GPIO para CS
#define SPI_DEV     "SPI_3"
#define GPIO_DEV    "GPIO_0"

/* Definir tiempo de delay en milisegundos (2000 ms = 2 segundos) */
#define DELAY_MS 1000

/* Configuración del buffer */
#define BUFFER_SIZE 700-7

static const struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .frequency = 200000, // Frecuencia SPI 1 MHz
    .slave = 0, // Dispositivo esclavo
    .cs = NULL, // La manejaremos manualmente
};

struct spi_cs_control spi_cs_ctrl;


static const struct device *spi_dev;
static const struct device *gpio_dev;

uint8_t rx_buffer[BUFFER_SIZE] = {0}; // Buffer de recepción
struct spi_buf rx_buf = {
    .buf = rx_buffer,
    .len = sizeof(rx_buffer),
};
struct spi_buf_set rx = {
    .buffers = &rx_buf,
    .count = 1,
};

uint8_t tx_buffer[BUFFER_SIZE] = {0}; // Buffer de recepción

// uint8_t tx_buffer[BUFFER_SIZE] = {0}; // Buffer de recepción
struct spi_buf tx_buf = {
    .buf = tx_buffer,
    .len = sizeof(tx_buffer),
};
struct spi_buf_set tx = {
    .buffers = &tx_buf,
    .count = 1,
};



//semaphore for API calls, only 1 async operation at time in this sample 
K_SEM_DEFINE(modem, 1, 1);

mlcf_a_t mac_header;


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

void request_rssi_measurement(void) {

    struct nrf_modem_dect_phy_rssi_params rssiParams = {
      .start_time = 0,
      .handle = 4,
      .carrier = CARRIER,
      .duration = SCAN_MEAS_DURATION,
      .reporting_interval = NRF_MODEM_DECT_PHY_RSSI_INTERVAL_24_SLOTS,
    };

    // printk("Requesting RSSI measurement...\n");
    int err = nrf_modem_dect_phy_rssi(&rssiParams);
    if (err != 0) {
        LOG_INF("RSSI measurement request failed with error %d\n", err);
    } else {
        // printk("RSSI measurement request succeeded.\n");
    }
}

struct k_timer my_timer;
struct k_timer tdectProTime;
struct k_timer tscanStatusValid;

static struct TXParams tp;

//timer is called if there is nothing received in receiver role in 10 seconds 
//if in init mode, decides then to start sending
//if init done, in receiver role, will exit
void timer_expiry_function(struct k_timer *dummy)
{
    EXIT = 1;
    if (dummy == &my_timer) {
        // LOG_INF("Timer expired");
    }
    if (dummy == &tdectProTime) {
        dectProTime = true;
    }
    if (dummy == &tscanStatusValid) {
        scanStatusValid = true;
    }
}
K_TIMER_DEFINE(my_timer, timer_expiry_function, NULL);
K_TIMER_DEFINE(tdectProTime, timer_expiry_function, NULL);
K_TIMER_DEFINE(tscanStatusValid, timer_expiry_function, NULL);


//exit on ANY button on the devkit
void button_handler(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & button_state & DK_BTN1_MSK) {
                LOG_INF("Button 1 pressed");
                LOG_INF("Setting device as FT");
                STATE = 7;
        }
        if (has_changed & button_state & DK_BTN2_MSK) {
                LOG_INF("Button 2 pressed");
                LOG_INF("Setting device as PT");
                STATE = 8;
                // simple_rx();
        }
        if (has_changed & button_state & DK_BTN3_MSK) {
                LOG_INF("Button 3 pressed");
                LOG_INF("Calculating RSSI");
                request_rssi_measurement();
        }
        if (has_changed & button_state & DK_BTN4_MSK) {
                LOG_INF("Button 4 pressed");
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
  LOG_INF("RSSI Measurement: %+d dBm\n", (int8_t)status->meas);
  measured_rssi = status->meas;
  // LOG_INF("RSSI meas length: %d\n", status->meas_len);
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
  int rx_counter=0;
  int32_t rx_rssi=calcRSSI(status->rssi_2, 1);
  memcpy(&_rxData, data, DATA_LEN);
  rx_counter = _rxData[0];
  //LOG_INF("%d", rx_counter);
  uint8_t calculatedChecksum = 0;
  // uint8_t checksum = _rxData[1];

  // Uncomment to check CRC
  // calculatedChecksum = calcularChecksum(_rxData, DATA_LEN);

  // if(checksum != calculatedChecksum)
  // {
  //   own_crc_errors++;
  //   // LOG_INF("CRC ERROR, %d\n", own_crc_errors);
  // }
  // else
  // {
  //   // LOG_INF("CRC OK\n");
  // }

  package_received = true;

  LOG_INF("__________________________________________________________________________________________");
  LOG_INF("Packet recieved");
  memcpy(&mac_header.mlcf_a, &_rxData[0], 1);
  get_mlcf_a_rev(&mac_header);
  LOG_INF("Header Type: %d\n", mac_header.HeaderType);

  if(mac_header.HeaderType == 1)
  {
    if(STATE =! 2){
      LOG_INF("No Beacon PDU received, waiting for Beacon...");
      return;
    }
    // Getting the MAC Beacon Header
    mlcf_b_2t mac_beacon_header;
    memcpy(&mac_beacon_header.mlcf_b_2, &_rxData[1], 7);
    get_mlcf_b_2_rev(&mac_beacon_header);
    LOG_INF("MAC Beacon received");

    // Content checking
    // LOG_INF("Network ID: %x", mac_beacon_header.NetworkID);
    // LOG_INF("Transmitter Address: %x\n", mac_beacon_header.TransmitterAddress);

    // Getting the MUX Header
    mlcf_c_2t mac_mux_header;
    memcpy(&mac_mux_header.mlcf_c_2, &_rxData[8], 1);
    get_mlcf_c_2_rev(&mac_mux_header);
    // LOG_INF("MAC MUX Header: %x", mac_mux_header.MAC_Ext);
    // LOG_INF("MAC MUX Header: %x", mac_mux_header.IE_Type);

    // Getting the MAC Beacon message
    mlmf_1_t mac_beacon_message;
    memcpy(&mac_beacon_message.mlmf_1, &_rxData[9], 13);
    get_mlmf_1_rev(&mac_beacon_message);

    //  Checkings
    // LOG_INF("mac_beacon_message %x, %x, %x, %x", mac_beacon_message.mlmf_1[0],
    //                                               mac_beacon_message.mlmf_1[1], 
    //                                               mac_beacon_message.mlmf_1[2],
    //                                               mac_beacon_message.mlmf_1[3]);
    // LOG_INF("Reserved: %d", mac_beacon_message.Reserved);
    // LOG_INF("TX Power: %d", mac_beacon_message.TX_Power);
    // LOG_INF("Power Const: %d", mac_beacon_message.Power_Const);
    // LOG_INF("Current: %d", mac_beacon_message.Current);
    // LOG_INF("Reserved 1: %d", mac_beacon_message.Reserved_1);
    STATE = 3;

  }
  else if(mac_header.HeaderType == 2)
  {
    LOG_INF("MAC Unicast Header received");

    if((STATE < 4) || (STATE > 6)){ //Receiving either Assoc Req or Res
      LOG_INF("No Unicast PDU received, waiting for Assoc PDU...");
      return;
    }
    if(STATE == 4){
      LOG_INF("Assoc Req recieved");
      mlcf_b_3t mac_unicast;
      memcpy(&mac_unicast.mlcf_b_3, &_rxData[1], 10);

      // LOG_INF("CHECKINGS:");
      // LOG_INF("%x", mac_unicast.mlcf_b_3[6]);
      // LOG_INF("%x", mac_unicast.mlcf_b_3[7]);
      // LOG_INF("%x", mac_unicast.mlcf_b_3[8]);
      // LOG_INF("%x", mac_unicast.mlcf_b_3[9]);

      get_mlcf_b_3_rev(&mac_unicast);
      // LOG_INF("Reciever Address: %x", mac_unicast.ReceiverAddress);

      // Getting the MUX Header
      mlcf_c_2t mac_mux_header;
      memcpy(&mac_mux_header.mlcf_c_2, &_rxData[11], 1);

      
      // LOG_INF("CHECKINGS 2:");
      // LOG_INF("%x", mac_mux_header.mlcf_c_2);

      get_mlcf_c_2_rev(&mac_mux_header);
      // LOG_INF("MAC MUX Header: %x", mac_mux_header.MAC_Ext);
      // LOG_INF("MAC MUX Header: %x", mac_mux_header.IE_Type);

      // Getting the message
      mlmf_3_t mac_assoc_message;
      memcpy(&mac_assoc_message.mlmf_3, &_rxData[12], 14);
      get_mlmf_3_rev(&mac_assoc_message);
      // LOG_INF("Content to be checked on RX: %x", mac_assoc_message.mlmf_3[0]);
      // LOG_INF("Content to be checked on RX: %x", mac_assoc_message.mlmf_3[1]);
      // LOG_INF("Content to be checked on RX: %x", mac_assoc_message.mlmf_3[2]);
      // LOG_INF("Setup Cause: %x", mac_assoc_message.Setup_Cause);
      // LOG_INF("N Flows: %x\n", mac_assoc_message.N_Flows);
      STATE = 5;
    }
    else if(STATE == 6){

      LOG_INF("Assoc Resp received");
      // Getting the MAC Header (Unicast Header)
      mlcf_b_3t mac_unicast;
      memcpy(&mac_unicast.mlcf_b_3, &_rxData[1], 10);
      get_mlcf_b_3_rev(&mac_unicast);
      // LOG_INF("Receiver Address: %x", mac_unicast.ReceiverAddress);

      // Getting the MUX Header
      mlcf_c_2t mac_mux_header; 
      memcpy(&mac_mux_header.mlcf_c_2, &_rxData[11], 1);
      get_mlcf_c_2_rev(&mac_mux_header);
      // LOG_INF("MAC MUX Header: %x", mac_mux_header.MAC_Ext);
      // LOG_INF("MAC MUX Header: %x", mac_mux_header.IE_Type);

      // Getting the message
      mlmf_4_t mac_assoc_resp_message;
      memcpy(&mac_assoc_resp_message.mlmf_4, &_rxData[12], 7);
      get_mlmf_4_rev(&mac_assoc_resp_message);
      // LOG_INF("Accepted: %d", mac_assoc_resp_message.ACK_NACK);
      // LOG_INF("Flow ID: %d\n", mac_assoc_resp_message.Flow_ID);
      LOG_INF("ASSOCIATION SUCCESS");
      STATE = 8;

    }
    
  }
  else if(mac_header.HeaderType == 0)
  {
    if(STATE == 8){

      LOG_INF("Receiving MAC DATA PDU");

      // Checking the MAC Header (DATA MAC PDU Header)
      mlcf_b_1t mac_pdu_header;
      memcpy(&mac_pdu_header.mlcf_b_1, &_rxData[1], 2);
      get_mlcf_b_1_rev(&mac_pdu_header);
      LOG_INF("MAC DATA PDU received");

      // Content checking
      if(mac_pdu_header.Reset == 1){
        // LOG_INF("Reset bit set, resetting...");
      }
      else{
        // LOG_INF("Reset bit not set, continuing...");
      }

      if(mac_pdu_header.SequenceNumber < lastSequenceNumber){
        // LOG_INF("Missed package");
        STATE = 8;
      }


      // Getting the MUX Header
      mlcf_c_5t mac_mux_header;
      memcpy(&mac_mux_header.mlcf_c_5, &_rxData[3], 4);
      get_mlcf_c_5_rev(&mac_mux_header);
      // LOG_INF("MAC MUX Header: %x", mac_mux_header.MAC_Ext);
      // LOG_INF("MAC MUX Header: %x", mac_mux_header.IE_Type);

      // Getting the MAC PDU Data
      memcpy(&rx_buffer, &_rxData[7], BUFFER_SIZE);
      // for(int i = 0; i < BUFFER_SIZE; i++){
      //   LOG_INF("Data: %x", rx_buffer[i]);
      // }
      sendSPIData(rx_buffer, BUFFER_SIZE);
      
      


    }
    else{
      LOG_INF("No data packet received, waiting for data packet...");
    }
  }
  else if(mac_header.HeaderType == 3)
  {
    LOG_INF("RD Bradcasting Header received");
  }

  if(previous_received>0 && rx_counter != previous_received+1){
    //multiple errors if multiple messages lost
    if(rx_counter != 0)
    {
      missing_errors=missing_errors+(rx_counter-previous_received)-1;
    }
  }
  previous_received=rx_counter;

  EXIT = 1;
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
  //listen for everything (broadcast mode used)
  rxOpsParams.filter.receiver_identity = 0;
  k_sem_take(&modem, K_FOREVER);
  int err=nrf_modem_dect_phy_rx(&rxOpsParams);
  if(err!=0) LOG_ERR("RX FAIL %d", err);
  if(rxHandle==65000)rxHandle=31400;
  else rxHandle++;
}

//send counter value immediately, start_time=0
void modem_tx(uint8_t* data, size_t datasize, uint8_t* phyheader, size_t phyheadersize)
{
  LOG_INF("Modem tx function");
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
  // Default code
  // memcpy(&phyHeader.type_1, &header, 5);

  // LOG_INF("PHY Header: %x %x %x %x %x", phyHeader.type_1[0], phyHeader.type_1[1], phyHeader.type_1[2], phyHeader.type_1[3], phyHeader.type_1[4]);
  // LOG_INF("PHY Header: %x %x %x %x %x", phyheader[0], phyheader[1], phyheader[2], phyheader[3], phyheader[4]);

  // LOG_INF("Sending %x, %x", _txData[datasize/2], _txData[0]);
  
  memcpy(&phyHeader.type_1, phyheader, phyheadersize); //Falta añadir checkings para saber de que tipo es la cabecera
  // LOG_INF("PHY Header: %x %x %x %x %x", phyHeader.type_1[0], phyHeader.type_1[1], phyHeader.type_1[2], phyHeader.type_1[3], phyHeader.type_1[4]);


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

// Si se va a transmitir broadcast
void broadcastTransmission(void){
  if(isBroadcast == true){
    if((DECT_PROTECTED == true) && (dectProTime == false)){

      // No explicit measurement
      // Case 1: The RD was transmitting inmediately before
      // Case 2: The RD was receiving inmediately before and was able
      // to determine the status of the subslot

      
      // Perform clause 5.1.3 - Last Minute Scan
      request_rssi_measurement();
      if (measured_rssi < RSSI_THRESHOLD_MIN)
      {
        LOG_INF("RSSI level is very low, negligible signals, setting device as FT");
      }
      else if(measured_rssi > RSSI_THRESHOLD_MAX)
      {
        LOG_INF("RSSI level is too high, setting device as PT");
      }
      else
      {
        LOG_INF("RSSI level is suitable, setting device as PT");
      }

      // Start the timer dectProTime
      dectProTime = true;
      k_timer_start(&tdectProTime, K_MSEC(800), K_MSEC(800));
      

    }
  }
}

// Recepción de datos útiles
int getSPIData(struct DataMessage *dp, struct TXParams *tp){

  LOG_INF("Receiving data from SPI");

  /* Inicializar el dispositivo SPIM */
  spi_dev = device_get_binding(SPI_DEV);
  if (!spi_dev) {
      printk("Error: SPIM device not found.\n");
      return 0;
  }

  /* Obtener el dispositivo GPIO */
  gpio_dev = device_get_binding(GPIO_DEV);
  if (!gpio_dev) {
      printk("Error: GPIO device not found.\n");
      return 0;
  }

  /* Configurar el pin de CS como salida */
  gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);


  // while (1) {
    /* Activar la señal CS */
    // LOG_INF("CS is low\n");
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);

    /* Iniciar la transacción SPI */
    int ret = spi_transceive(spi_dev, &spi_cfg, NULL, &rx);
    if (ret == 0) {
        // LOG_INF("Data received: ");
        for (int i = 0; i < BUFFER_SIZE; i++) {
          LOG_INF("%02X ", rx_buffer[i]);
          dp->payload[i] = rx_buffer[i];
        }
        // LOG_INF("\n");
    } else {
        printk("SPIM transaction failed: %d\n", ret);
    }

    // k_msleep(500);

    /* Desactivar la señal CS */
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);
    // printk("CS is high again\n");
    /* Esperar 2 segundos antes de la próxima lectura */
    k_msleep(DELAY_MS);
  // } 
    return 1;

}

int sendSPIData(uint8_t *data, size_t datasize){

  LOG_INF("Sending data to SPI device");

  /* Inicializar el dispositivo SPIM */
  spi_dev = device_get_binding(SPI_DEV);
  if (!spi_dev) {
      printk("Error: SPIM device not found.\n");
      return 0;
  }

  /* Obtener el dispositivo GPIO */
  gpio_dev = device_get_binding(GPIO_DEV);
  if (!gpio_dev) {
      printk("Error: GPIO device not found.\n");
      return 0;
  }

  /* Configurar el pin de CS como salida */
  gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
  memcpy(tx_buffer, data, datasize);


  // while (1) {
    /* Activar la señal CS */
    // LOG_INF("CS is low\n");
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);

    /* Iniciar la transacción SPI */
    int ret = spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
    if (ret == 0) {
        LOG_INF("Data sent \n");
        // LOG_INF("\n");
    } else {
        printk("SPIM transaction failed: %d\n", ret);
    }

    // k_msleep(500);

    /* Desactivar la señal CS */
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);
    // printk("CS is high again\n");
    /* Esperar 2 segundos antes de la próxima lectura */
    k_msleep(DELAY_MS);
  // } 
    return 1;
}



int main(void)
  {
    printk("START DECT NR+\n");
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
    printk("Environment checked and working\n");
    printk("----------------------------------------------------------\n");
    
    // //printk("Listening on channel %d for 10 secs to see if there is transmissions\n", CARRIER);
    // k_timer_start(&my_timer, K_SECONDS(10), K_SECONDS(10));
    // modem_rx(NRF_MODEM_DECT_PHY_RX_MODE_SINGLE_SHOT, 10);


    // INITIAL MODE SELECTION

    // while(measured_rssi == NULL){
    //   request_rssi_measurement();
    //   k_msleep(1000);
    // }

    // Timer scanStatusValid
    // k_timer_start(&tscanStatusValid, K_SECONDS(300), K_SECONDS(300));


    // Insert checkings regarding RSSI levels

    // if(measured_rssi < RSSI_THRESHOLD_MIN){
    //   LOG_INF("RSSI level is very low, negligible signals, setting device as FT");
    // }
    // else if(measured_rssi > RSSI_THRESHOLD_MAX){
    //   LOG_INF("RSSI level is too high, setting device as PT");
    // }
    // else{
    //   LOG_INF("RSSI level is suitable, setting device as PT");
    // }


  // package_received = false;
  // modem_rx(NRF_MODEM_DECT_PHY_RX_MODE_SINGLE_SHOT, 10);

  // if(package_received == false){
  //   LOG_INF("No beacons detected, setting device as FT");
  //   STATE = 1;
  // }

  // k_timer_init(&my_timer, timer_expiry_function, NULL);


    int i = 0;
    while(1){
      k_msleep(930);

      //LED Blinking
      if(i%2==0) {
        dk_set_led_on(DK_LED1);
      }
      else {
        dk_set_led_off(DK_LED1);
      }
      i++;
      if(i==10) i=0;
      if(i == INT_MAX) return 0;

      // request_rssi_measurement();

      // k_msleep(7000);


      if(STATE%2 != 0){

        switch (STATE)
        {
          case 1:
            LOG_INF("Sending Assoc Beacon");
            static struct AssocBeaconMessage abm;
            sendAssocBeacon(&abm);

            //Checkings
            // LOG_INF("MAC Beacon Header: %x", abm.message[1]);
            // LOG_INF("MAC Beacon Header: %x", abm.message[2]);
            // LOG_INF("MAC Beacon Header: %x", abm.message[3]);
            // LOG_INF("MAC Beacon Header: %x", abm.message[4]);
            // LOG_INF("MAC Beacon Header: %x", abm.message[5]);
            // LOG_INF("MAC Beacon Header: %x", abm.message[6]);
            // LOG_INF("MAC Beacon Header: %x", abm.message[7]);
            // LOG_INF("Network ID: %x", abm.mlcf_b_2.NetworkID);
            // LOG_INF("Transmitter Address: %x", abm.mlcf_b_2.TransmitterAddress);

            //Transmitting beacon
            modem_tx(&abm.message, 22, &abm.phyheader, 5);
            STATE = 4;

          break;

          case 3:
            LOG_INF("Sending Assoc Req");
            static struct AssocReqMessage aRm;
            sendAssocReq(&aRm);

            // LOG_INF("CHECKINGS TX:");
            // LOG_INF("MAC_Ext %x", aRm.mlcf_c_2.MAC_Ext);
            // LOG_INF("IE_Type %x", aRm.mlcf_c_2.IE_Type);
            // LOG_INF("Content to be checked %x", aRm.mlcf_c_2.mlcf_c_2);
            // LOG_INF("%x", aRm.message[11]);

            // LOG_INF("CHECKINGS TX:");
            // // LOG_INF("Content to be checked %x", aRm.mlmf_3.mlmf_3[0]);
            // LOG_INF("Setup Clause %x", aRm.mlmf_3.Setup_Cause);
            // LOG_INF("N_Flows %x", aRm.mlmf_3.N_Flows);
            // LOG_INF("Content to be checked %x", aRm.message[12]);
            // LOG_INF("Content to be checked %x", aRm.message[13]);
            // LOG_INF("Content to be checked %x", aRm.message[14]);
          
            //Transmitting beacon
            modem_tx(&aRm.message, 26, &aRm.phyheader, 5);
            STATE = 6;

          break;

          case 5:
            LOG_INF("Sending Assoc Resp");
            static struct AssocRespMessage arm;
            sendAssocResp(&arm);

            // LOG_INF("CHECKINGS TX:");
            // LOG_INF("ACK_NACK %x", arm.mlmf_4.ACK_NACK);
            // LOG_INF("Flow_ID %x", arm.mlmf_4.Flow_ID);

            //Checkings
            // LOG_INF("MAC Beacon Header: %x", arm.message[1]);
            // LOG_INF("MAC Beacon Header: %x", arm.message[2]);
            // LOG_INF("MAC Beacon Header: %x", arm.message[3]);
            // LOG_INF("MAC Beacon Header: %x", arm.message[4]);
            // LOG_INF("MAC Beacon Header: %x", arm.message[5]);
            // LOG_INF("MAC Beacon Header: %x", arm.message[6]);
            // LOG_INF("MAC Beacon Header: %x", arm.message[7]);
            // LOG_INF("Network ID: %x", arm.mlcf_b_3.ReceiverAddress);
            // LOG_INF("Transmitter Address: %x", arm.mlmf_4.ACK_NACK);

            modem_tx(&arm.message, 19, &arm.phyheader, 5);
            LOG_INF("Transmitter associated successfully");
            STATE = 7;
          break;

          case 7:
            LOG_INF("Sending Data Packet");
            static struct DataMessage dp;
            getSPIData(&dp, &tp);
            sendData(&dp, &tp);
            tp.sequenceNumber = tp.sequenceNumber + 1;
            // LOG_INF("CHECKINGS TX:");
            // LOG_INF("Flow_ID %x", dp.mlmf_5.Flow_ID);
            // LOG_INF("Data %x", dp.mlmf_5.Data);

            //Checkings
            // LOG_INF("MAC Beacon Header: %x", dp.message[1]);
            // LOG_INF("MAC Beacon Header: %x", dp.message[2]);
            // LOG_INF("MAC Beacon Header: %x", dp.message[3]);
            // LOG_INF("MAC Beacon Header: %x", dp.message[4]);
            // LOG_INF("MAC Beacon Header: %x", dp.message[5]);
            // LOG_INF("MAC Beacon Header: %x", dp.message[6]);
            // LOG_INF("MAC Beacon Header: %x", dp.message[7]);
            // LOG_INF("Network ID: %x", dp.mlcf_b_3.ReceiverAddress);
            // LOG_INF("Transmitter Address: %x", dp.mlmf_5.Flow_ID);

            modem_tx(&dp.message, 700, &dp.phyheader, 5);
            k_msleep(5000);

          break;

          default:
          break;
        }
      }
      else if(STATE%2 == 0)
      {
        simple_rx();
      }
    }
  }

int simple_rx(void)
{
  dk_set_led_on(DK_LED1);
  dk_set_led_on(DK_LED2);
  printk("RX\n");
  EXIT = 0;
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