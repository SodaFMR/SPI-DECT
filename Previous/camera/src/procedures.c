#include <stdint.h>
#include <string.h>
#include "physical_header_field.h"
#include "mac_header_field.h"
#include "mac_message.h"
#include "tx_rx.h"
#include "procedures.h"



int sendAssocBeacon(struct AssocBeaconMessage *abm){
    
    // uint8_t phyheader[5];
    // uint8_t message[22];
    // plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  
    // mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
    // mlcf_b_2t mlcf_b_2;     //MAC Beacon Header - 7 bytes
    // mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type c - 1 byte
    // mlmf_1_t mlmf_1;        //MAC Beacon Message - 13 bytes


    //Montamos la cabecera física
    abm -> plcf_10.HeaderFormat = 0;
    abm->plcf_10.PacketLengthType = 0;
    abm->plcf_10.PacketLength = 7;
    abm->plcf_10.ShortNetworkID = (uint8_t)0x0a;
    abm->plcf_10.TransmitterIdentity = (uint16_t)0x01010101;
    abm->plcf_10.TransmitPower = (uint8_t)0xb;
    abm->plcf_10.Reserved = 0;
    abm->plcf_10.DFMCS = 4;

    //Montamos la cabecera MAC
    abm->mlcf_a.Version = 0;
    abm->mlcf_a.Security = 0;
    abm->mlcf_a.HeaderType = 1;

    //Montamos la cabecera MAC del tipo elegido
    abm->mlcf_b_2.NetworkID = 0x0a;
    abm->mlcf_b_2.TransmitterAddress = 0x01010101;

    //Montamos la cabecera MAC MUX correspondiente
    abm->mlcf_c_2.MAC_Ext = 3;
    abm->mlcf_c_2.IE_Type = 0b001000;

    //Montamos el mensaje del beacon
    abm->mlmf_1.Reserved = 0;
    abm->mlmf_1.TX_Power = 1;
    abm->mlmf_1.Power_Const = 0;
    abm->mlmf_1.Current = 1;
    abm->mlmf_1.Network_Beacon_Channels = 1;
    abm->mlmf_1.Network_Beacon_Period = 7;
    abm->mlmf_1.Cluster_Beacon_Period = 0;
    abm->mlmf_1.Reserved_1 = 0;
    abm->mlmf_1.Next_Cluster_Channel = 1;
    abm->mlmf_1.TTn = 0;
    abm->mlmf_1.Reserved_2 = 0;
    abm->mlmf_1.Clusters_Max_TX_Power = 1;
    abm->mlmf_1.Reserved_3 = 0;
    abm->mlmf_1.Current_Cluster_Channel = 0;
    abm->mlmf_1.Reserved_4 = 0;
    abm->mlmf_1.ANBC = 0;


    //Obtenemos la cabecera física montada
    get_plcf_1(&abm->plcf_10);
    //Obtenemos la cabecera MAC montada
    get_mlcf_a(&abm->mlcf_a);
    //Obtenemos la cabecera MAC del tipo elegido
    get_mlcf_b_2(&abm->mlcf_b_2);
    //Obtenemos la cabecera MAC MUX correspondiente
    get_mlcf_c_2(&abm->mlcf_c_2);
    //Obtenemos el mensaje del beacon montado
    get_mlmf_1(&abm->mlmf_1);
    //Copiamos cabecera en buffer
    memcpy(abm->phyheader, &abm->plcf_10.plcf, 5);
    //Copiamos en el contenido la cabecera MAC
    memcpy(abm->message, &abm->mlcf_a.mlcf_a, 1);
    //Copiamos en el contenido la cabecera MAC del tipo elegido
    memcpy(abm->message + 1, &abm->mlcf_b_2.mlcf_b_2, 7);
    //Copiamos en el contenido la cabecera MAC MUX correspondiente
    memcpy(abm->message + 8, &abm->mlcf_c_2.mlcf_c_2, 1);
    //Copiamos en el contenido del mensaje beacon
    memcpy(abm->message + 9, &abm->mlmf_1.mlmf_1, 13);

    return 1;
}

int sendAssocReq(struct AssocReqMessage *aRm){

  // uint8_t phyheader[5];
  // uint8_t message[26];
  // plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  
  // mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
  // mlcf_b_3t mlcf_b_3;     //MAC Unicast Header - 10 bytes
  // mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type c - 1 byte
  // mlmf_3_t mlmf_3;        //MAC Association Request Message - 14 bytes

  //Montamos la cabecera física
  aRm->plcf_10.HeaderFormat = 0;
  aRm->plcf_10.PacketLengthType = 0;
  aRm->plcf_10.PacketLength = 7;
  aRm->plcf_10.ShortNetworkID = (uint8_t)0x0a;
  aRm->plcf_10.TransmitterIdentity = (uint16_t)0x01010101;
  aRm->plcf_10.TransmitPower = (uint8_t)0xb;
  aRm->plcf_10.Reserved = 0;
  aRm->plcf_10.DFMCS = 4;

  //Montamos la cabecera MAC
  aRm->mlcf_a.Version = 0;
  aRm->mlcf_a.Security = 0;
  aRm->mlcf_a.HeaderType = 2;

  //Montamos la cabecera MAC del tipo elegido
  aRm->mlcf_b_3.Reserved = 0;
  aRm->mlcf_b_3.Reset = 1;
  aRm->mlcf_b_3.MACSequence = 1;
  aRm->mlcf_b_3.SequenceNumber = 0;
  aRm->mlcf_b_3.ReceiverAddress = (uint32_t)0x01010101;
  aRm->mlcf_b_3.TransmitterAddress = (uint32_t)0x01010101;

  //Montamos la cabecera MAC MUX correspondiente
  aRm->mlcf_c_2.MAC_Ext = 3;
  aRm->mlcf_c_2.IE_Type = 0b001010;

  //Montamos el mensaje de asociación
  aRm->mlmf_3.Setup_Cause = 0;
  aRm->mlmf_3.N_Flows = 1;
  aRm->mlmf_3.Power_Const = 1;
  aRm->mlmf_3.FT_Mode = 0;
  aRm->mlmf_3.Current = 0;
  aRm->mlmf_3.Reserved = 0;
  aRm->mlmf_3.HARQ_P_TX = 0;
  aRm->mlmf_3.MAX_HARQ_Re_TX = 0;
  aRm->mlmf_3.HARQ_P_RX = 0;
  aRm->mlmf_3.MAX_HARQ_Re_RX = 0;
  aRm->mlmf_3.Reserved_1 = 0;
  aRm->mlmf_3.FlowID = 0;
  aRm->mlmf_3.Network_Beacon_Period = 0;
  aRm->mlmf_3.Cluster_Beacon_Period = 0;
  aRm->mlmf_3.Reserved_2 = 0;
  aRm->mlmf_3.Next_Cluster_Channel = 0;
  aRm->mlmf_3.TTn = 0;
  aRm->mlmf_3.Reserved_3 = 0;
  aRm->mlmf_3.Current_Cluster_Channel = 0;


  //Obtenemos la cabecera física montada
  get_plcf_1(&aRm->plcf_10);
  //Obtenemos la cabecera MAC montada
  get_mlcf_a(&aRm->mlcf_a);
  //Obtenemos la cabecera MAC del tipo elegido
  get_mlcf_b_3(&aRm->mlcf_b_3);
  //Obtenemos la cabecera MAC MUX correspondiente
  get_mlcf_c_2(&aRm->mlcf_c_2);
  //Obtenemos el mensaje del beacon montado
  get_mlmf_3(&aRm->mlmf_3);
  //Copiamos cabecera en buffer
  memcpy(aRm->phyheader, &aRm->plcf_10.plcf, 5);
  //Copiamos en el contenido la cabecera MAC
  memcpy(aRm->message, &aRm->mlcf_a.mlcf_a, 1);
  //Copiamos en el contenido la cabecera MAC del tipo elegido
  memcpy(aRm->message + 1, &aRm->mlcf_b_3.mlcf_b_3, 10);
  //Copiamos en el contenido la cabecera MAC MUX correspondiente
  memcpy(aRm->message + 11, &aRm->mlcf_c_2.mlcf_c_2, 1);
  //Copiamos en el contenido del mensaje beacon
  memcpy(aRm->message + 12, &aRm->mlmf_3.mlmf_3, 14);
  // //Mandamos a enviar todo
  // modem_tx(aRm->message, 25, &aRm->phyheader, 5);

  return 1;

}

int sendAssocResp(struct AssocRespMessage *arm){

  // uint8_t phyheader[5];
  // uint8_t message[19];
  // plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  
  // mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
  // mlcf_b_3t mlcf_b_3;     //MAC Unicast Header - 10 bytes
  // mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type c - 1 byte
  // mlmf_4_t mlmf_4;        //MAC Association Response Message - 7 bytes

  //Montamos la cabecera física
  arm->plcf_10.HeaderFormat = 0;
  arm->plcf_10.PacketLengthType = 0;
  arm->plcf_10.PacketLength = 0;
  arm->plcf_10.ShortNetworkID = (uint8_t)0x0a;
  arm->plcf_10.TransmitterIdentity = (uint16_t)0x01010101;
  arm->plcf_10.TransmitPower = (uint8_t)0xb;
  arm->plcf_10.Reserved = 0;
  arm->plcf_10.DFMCS = 4;

  //Montamos la cabecera MAC
  arm->mlcf_a.Version = 0;
  arm->mlcf_a.Security = 0;
  arm->mlcf_a.HeaderType = 2;

  //Montamos la cabecera MAC del tipo elegido
  arm->mlcf_b_3.Reserved = 0;
  arm->mlcf_b_3.Reset = 0;
  arm->mlcf_b_3.MACSequence = 0;
  arm->mlcf_b_3.SequenceNumber = 0;
  arm->mlcf_b_3.ReceiverAddress = 0x01010101;
  arm->mlcf_b_3.TransmitterAddress = 0x01010101;

  //Montamos la cabecera MAC MUX correspondiente
  arm->mlcf_c_2.MAC_Ext = 3;
  arm->mlcf_c_2.IE_Type = 0b001011;

  //Montamos el mensaje de respuesta a la asociación
  arm->mlmf_4.ACK_NACK = 1;
  arm->mlmf_4.Reserved = 0;
  arm->mlmf_4.HARQ_mod = 0;
  arm->mlmf_4.NFlows = 0;
  arm->mlmf_4.Group = 0;
  arm->mlmf_4.TX_Power = 0;
  arm->mlmf_4.Reject_Cause = 0;
  arm->mlmf_4.Reject_Time = 0;
  arm->mlmf_4.HARQ_P_RX = 0;
  arm->mlmf_4.MAX_HARQ_Re_RX = 0;
  arm->mlmf_4.HARQ_P_TX = 0;
  arm->mlmf_4.MAX_HARQ_Re_TX = 0;
  arm->mlmf_4.Reserved_1 = 0;
  arm->mlmf_4.Flow_ID = 8;
  arm->mlmf_4.Reserved_2 = 0;
  arm->mlmf_4.Group_ID = 0;
  arm->mlmf_4.Reserved_3 = 0;
  arm->mlmf_4.Resource_TAG = 0;


  //Obtenemos la cabecera física montada
  get_plcf_1(&arm->plcf_10);
  //Obtenemos la cabecera MAC montada
  get_mlcf_a(&arm->mlcf_a);
  //Obtenemos la cabecera MAC del tipo elegido
  get_mlcf_b_3(&arm->mlcf_b_3);
  //Obtenemos la cabecera MAC MUX correspondiente
  get_mlcf_c_2(&arm->mlcf_c_2);
  //Obtenemos el mensaje del beacon montado
  get_mlmf_4(&arm->mlmf_4);
  //Copiamos cabecera en buffer
  memcpy(arm->phyheader, &arm->plcf_10.plcf, 5);
  //Copiamos en el contenido la cabecera MAC
  memcpy(arm->message, &arm->mlcf_a.mlcf_a, 1);
  //Copiamos en el contenido la cabecera MAC del tipo elegido
  memcpy(arm->message + 1, &arm->mlcf_b_3.mlcf_b_3, 10);
  //Copiamos en el contenido la cabecera MAC MUX correspondiente
  memcpy(arm->message + 11, &arm->mlcf_c_2.mlcf_c_2, 1);
  //Copiamos en el contenido del mensaje beacon
  memcpy(arm->message + 12, &arm->mlmf_4.mlmf_4, 7);
  //Mandamos a enviar todo
  // modem_tx(arm->message, 18, &arm->phyheader, 5);

  return 0;

}

int sendData(struct DataMessage *dm, struct TXParams *tp){
    
  // uint8_t phyheader[5];
  // uint8_t message[19];
  // plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  
  // mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
  // mlcf_b_2t mlcf_b_2;     //MAC DATA PDU Header - 2 bytes
  // mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type f - 4 bytes
  // mlmf_5_t mlmf_5;        //MAC Data Message - XXXXX bytes

  //Montamos la cabecera física
    
  dm->plcf_10.HeaderFormat = 0;
  dm->plcf_10.PacketLengthType = 0;
  dm->plcf_10.PacketLength = 7;
  dm->plcf_10.ShortNetworkID = (uint8_t)0x0a;
  dm->plcf_10.TransmitterIdentity = (uint16_t)0x01010101;
  dm->plcf_10.TransmitPower = (uint8_t)0xb;
  dm->plcf_10.Reserved = 0;
  dm->plcf_10.DFMCS = 4;

  //Montamos la cabecera MAC
  dm->mlcf_a.Version = 0;
  dm->mlcf_a.Security = 0;
  dm->mlcf_a.HeaderType = 0;

  //Montamos la cabecera MAC del tipo elegido
  dm->mlcf_b_1.Reserved = 0;
  dm->mlcf_b_1.Reset = tp->reset;
  dm->mlcf_b_1.SequenceNumber = tp->sequenceNumber;

  //Montamos la cabecera MAC MUX correspondiente
  dm->mlcf_c_5.MAC_Ext = 2;
  dm->mlcf_c_5.IE_Type = 0b000011;

  //Datos de aplicacion
  // dm->payload;




  //Obtenemos la cabecera física montada
  get_plcf_1(&dm->plcf_10);
  //Obtenemos la cabecera MAC montada
  get_mlcf_a(&dm->mlcf_a);
  //Obtenemos la cabecera MAC del tipo elegido
  get_mlcf_b_1(&dm->mlcf_b_1);
  //Obtenemos la cabecera MAC MUX correspondiente
  get_mlcf_c_5(&dm->mlcf_c_5);
  //Obtenemos el mensaje del ...
  // get_mlmf_1(&dm->mlmf_5);
  //Copiamos cabecera en buffer
  memcpy(dm->phyheader, &dm->plcf_10.plcf, 5);
  //Copiamos en el contenido la cabecera MAC
  memcpy(dm->message, &dm->mlcf_a.mlcf_a, 1);
  //Copiamos en el contenido la cabecera MAC del tipo elegido
  memcpy(dm->message + 1, &dm->mlcf_b_1.mlcf_b_1, 2);
  //Copiamos en el contenido la cabecera MAC MUX correspondiente
  memcpy(dm->message + 3, &dm->mlcf_c_5.mlcf_c_5, 4);
  //Copiamos en el contenido del mensaje beacon
  memcpy(dm->message + 7, &dm->payload, 700-7);

  return 1;



}

int nSubslots(int bytes, int MCS){
  
  int nSubslots = 0;

  int data[5][15] = {
    {0, 136, 264, 400, 536, 664, 792, 920, 1064, 1192, 1320, 1448, 1576, 1704, 1864, 1992},
    {32, 296, 552, 824, 1096, 1352, 1608, 1864, 2104, 2360, 2616, 2872, 3128, 3384, 3704, 3960},
    {56, 456, 856, 1256, 1640, 2024, 2360, 2744, 3192, 3576, 3960, 4320, 4768, 5152, 5536, -1},
    {88, 616, 1128, 1672, 2168, 2680, 3192, 3704, 4256, 4768, 5280, -1, -1, -1, -1, -1},
    {144, 936, 1736, 2488, 3256, 4024, 4832, 5600, -1, -1, -1, -1, -1, -1, -1, -1}
  };
  
  for (int i = 0; i < 15; i++) {
    if (data[MCS][i] == bytes * 8) {
      nSubslots = i+1;
      break;
    }
  }


  nSubslots = data[MCS][bytes*8];
  

  return nSubslots;
}

