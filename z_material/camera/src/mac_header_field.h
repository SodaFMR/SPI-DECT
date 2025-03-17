/*
 * Author: Iván Tomas García
 * Property of the Universidad Politécnica de Cartagena
*/

#include <stdint.h>

#ifndef MAC_HEADER_FIELD_H
#define MAC_HEADER_FIELD_H



typedef struct{
	//Header type (6.3.2)
	uint8_t Version		: 2;
	uint8_t Security	: 2;
	uint8_t HeaderType	: 4;

	uint8_t mlcf_a;
} mlcf_a_t;

typedef struct {
	//Common header - DATA MAC PDU Header (6.3.3.1)
	uint8_t Reserved		: 3;
	uint8_t Reset			: 1;
	uint16_t SequenceNumber	: 12;

	uint8_t mlcf_b_1[2];
} mlcf_b_1t;

typedef struct {
	//Common header - Beacon Header (6.3.3.2)
	uint32_t NetworkID				: 24;
	uint32_t TransmitterAddress		: 32;

	uint8_t mlcf_b_2[7];
} mlcf_b_2t;

typedef struct {
	//Common header - Unicast Header (6.3.3.3)
	uint8_t Reserved				: 3;
	uint8_t Reset					: 1;
	uint8_t MACSequence				: 4;
	uint8_t SequenceNumber			: 8;
	uint32_t ReceiverAddress		: 32;
	uint32_t TransmitterAddress		: 32;

	uint8_t mlcf_b_3[10];
} mlcf_b_3t;

typedef struct {
	//Common header - RD Bradcasting Header (6.3.3.4)
	uint8_t Reserved			: 3;
	uint8_t Reset				: 1;
	uint16_t SequenceNumber		: 12;
	uint32_t TransmitterAddress	: 32;

	uint8_t mlcf_b_4[6];
} mlcf_b_4t;

typedef struct {
	//MAC MUX Header - Type 1    (Option a & b)			Padding IE (6.4.3.8), Configuration Request IE (6.4.3.9), Keep alive IE (6.4.3.14)
	uint8_t MAC_Ext		: 2;						//	MAC Security Info IE (6.4.3.14)
	uint8_t Length		: 1;
	uint8_t IE_Type		: 5;

	uint8_t mlcf_c_1;
} mlcf_c_1t;

typedef struct {
	//MAC MUX Header - Type 2    (Option c)				Padding IE (6.4.3.8), Radio Device Status IE (6.4.3.13)
	uint8_t MAC_Ext		: 2;
	uint8_t IE_Type		: 6;

	uint8_t mlcf_c_2;
} mlcf_c_2t;

typedef struct {
	//MAC MUX Header - Type 3    (Option d)
	uint8_t MAC_Ext		: 2;
	uint8_t IE_Type		: 6;
	uint8_t Length		: 8;

	uint8_t mlcf_c_3[2];
} mlcf_c_3t;

typedef struct {
	//MAC MUX Header - Type 4    (Option e)
	uint8_t MAC_Ext		: 2;
	uint8_t IE_Type		: 6;
	uint16_t Length		: 16;

	uint8_t mlcf_c_4[3];
} mlcf_c_4t;

typedef struct {
	//MAC MUX Header - Type 5    (Option f)
	uint8_t MAC_Ext				: 2;
	uint8_t IE_Type				: 6;
	uint16_t Length				: 16;
	uint8_t IE_Type_Extension	: 8;
	
	uint8_t mlcf_c_5[4];
} mlcf_c_5t;


int get_mlcf_a(mlcf_a_t* mlcf_a);
int get_mlcf_b_1(mlcf_b_1t* mlcf_b_1);
int get_mlcf_b_2(mlcf_b_2t* mlcf_b_2);
int get_mlcf_b_3(mlcf_b_3t* mlcf_b_3);
int get_mlcf_b_4(mlcf_b_4t* mlcf_b_4);
int get_mlcf_c_1(mlcf_c_1t* mlcf_c_1);
int get_mlcf_c_2(mlcf_c_2t* mlcf_c_2);
int get_mlcf_c_3(mlcf_c_3t* mlcf_c_3);
int get_mlcf_c_4(mlcf_c_4t* mlcf_c_4);
int get_mlcf_c_5(mlcf_c_5t* mlcf_c_5);

int get_mlcf_a_rev(mlcf_a_t* mlcf_a);
int get_mlcf_b_1_rev(mlcf_b_1t* mlcf_b_1);
int get_mlcf_b_2_rev(mlcf_b_2t* mlcf_b_2);
int get_mlcf_b_3_rev(mlcf_b_3t* mlcf_b_3);
int get_mlcf_b_4_rev(mlcf_b_4t* mlcf_b_4);
int get_mlcf_c_1_rev(mlcf_c_1t* mlcf_c_1);
int get_mlcf_c_2_rev(mlcf_c_2t* mlcf_c_2);
int get_mlcf_c_3_rev(mlcf_c_3t* mlcf_c_3);
int get_mlcf_c_4_rev(mlcf_c_4t* mlcf_c_4);
int get_mlcf_c_5_rev(mlcf_c_5t* mlcf_c_5);

#endif