#include "mac_header_field.h"
#include "physical_header_field.h"
#include "stdbool.h"
#ifndef PROCEDURES_H
#define PROCEDURES_H

//Struct of Assoc Beacon Message
struct AssocBeaconMessage
{
    uint8_t phyheader[5];
    uint8_t message[22];

    plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  

    mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
    mlcf_b_2t mlcf_b_2;     //MAC Beacon Header - 7 bytes
    mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type c - 1 byte

    mlmf_1_t mlmf_1;        //MAC Beacon Message - 13 bytes
};

struct AssocReqMessage
{
    uint8_t phyheader[5];
    uint8_t message[26];

    plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes 

    mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
    mlcf_b_3t mlcf_b_3;     //MAC Unicast Header - 10 bytes
    mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type c - 1 byte

    mlmf_3_t mlmf_3;        //MAC Association Request Message - 14 bytes
};

struct AssocRespMessage
{
    uint8_t phyheader[5];
    uint8_t message[19];

    plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  

    mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
    mlcf_b_3t mlcf_b_3;     //MAC Unicast Header - 10 bytes
    mlcf_c_2t mlcf_c_2;     //MAC MUX Header - Type c - 1 byte

    mlmf_4_t mlmf_4;        //MAC Association Response Message - 7 bytes
};

struct DataMessage
{
    uint8_t phyheader[5];
    uint8_t message[7];
    uint8_t payload[700-7]; //Size available left by the header. Max data length

    plcf_10_t plcf_10;      //Physical Layer Common Field - 5 bytes  

    mlcf_a_t mlcf_a;        //MAC Header Type - 1 byte
    mlcf_b_1t mlcf_b_1;     //MAC Beacon Header - 2 bytes
    mlcf_c_5t mlcf_c_5;     //MAC MUX Header - Type f - 4 bytes

    //Payload               //MAC Data Message - XXXXX bytes
};



struct TXParams
{
    bool reset;
    uint16_t sequenceNumber;

};

int sendAssocBeacon(struct AssocBeaconMessage *abm);
int sendAssocReq(struct AssocReqMessage *aRm);
int sendAssocResp(struct AssocRespMessage *arm);
int sendData(struct DataMessage *dm, struct TXParams *tp);
int nSubslots(int bytes, int MCS);


#endif