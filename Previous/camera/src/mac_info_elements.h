/*
 * Author: Iván Tomas García
 * Property of the Universidad Politécnica de Cartagena
*/

#include <stdint.h>

#ifndef MAC_INFO_ELEMENTS_H
#define MAC_INFO_ELEMENTS_H


typedef struct {

	//MAC Security Info IE (6.4.3.1)

	uint8_t	Version  		: 2;
	uint8_t	Key_Index		: 2;
	uint8_t	Sec_IV_Type		: 4;
	uint32_t	HPC			: 32;

	uint8_t	mlie_1[5];
}mlie_1_t;

typedef struct {

	//Route Info IE (6.4.3.2)

	uint32_t	Sink_Address	: 32;
	uint8_t	Route_Cost			: 8;
	uint8_t	App_Sec_N			: 8;

	uint8_t	mlie_2[6];
}mlie_2_t;

typedef struct {

	//Resource Allocation IE (6.4.3.3)

	uint8_t	Allocation_Type		: 2;
	uint8_t	Add					: 1;
	uint8_t	ID					: 1;
	uint8_t	Repeat				: 3;
	uint8_t SFN					: 1;
	uint8_t Channel				: 1;
	uint8_t RLF					: 1;
	uint8_t Reserved			: 7;
	uint16_t Start_Subslot		: 9;
	uint8_t Length_Type			: 1;
	uint8_t Length				: 7;
	uint8_t Reserved_1			: 7;	
	uint16_t Start_Subslot_1	: 9;
	uint8_t Length_Type_1		: 1;
	uint8_t Length_1			: 7;
	uint16_t Short_RD_ID		: 16;
	uint8_t Repetition			: 8;
	uint8_t Validity			: 8;
	uint8_t SFN_Value			: 8;
	uint8_t Reserved_2			: 3;
	uint16_t Channel_1			: 13;
	uint8_t Reserved_3			: 4;
	uint8_t dect_Scheduled_Resource_Failure	: 4;

	uint8_t	mlie_3[16];

}mlie_3_t;


typedef struct {

	//Random Access Resource IE (6.4.3.4)

	uint8_t	Reserved		: 3;
	uint8_t	Repeat			: 2;
	uint8_t	SFN				: 1;
	uint8_t	Channel			: 1;
	uint8_t Channel2		: 1;
	uint8_t Reserved_1		: 7;
	uint16_t Start_Subslot	: 1;
	uint8_t Length_Type		: 1;
	uint8_t Length			: 7;
	uint8_t MAX_Length_Type	: 1;
	uint8_t MAX_RACH_Length	: 4;
	uint8_t CW_Min_Sig		: 3;
	uint8_t DECT_Delay		: 1;
	uint8_t Response_Window	: 4;
	uint8_t CW_Max_Sig		: 3;
	uint8_t Repetition		: 8;
	uint8_t Validity		: 8;
	uint8_t SNF_Value		: 8;
	uint8_t Reserved_2		: 3;
	uint16_t Channel_1		: 13;
	uint8_t Reserved_3		: 3;
	uint16_t Channel_2		: 13;

	uint8_t	mlie_4[13];
}mlie_4_t;

typedef struct {

	//RD Capability IE field definitions (6.4.3.5)

	uint8_t	N_PHY_Cap			: 3;
	uint8_t	Release				: 5;
	uint8_t	Reserved			: 2;
	uint8_t Group_As			: 1;
	uint8_t Paging				: 1;
	uint8_t	Op_Modes			: 2;
	uint8_t Mesh				: 1;	
	uint8_t Schedul				: 1;
	uint8_t MAC_Security		: 3;
	uint8_t DLC_Service_Type	: 3;
	uint8_t Reserved_1			: 2;
	uint8_t RD_Power_Class		: 3;
	uint8_t MAX_NSS_RX			: 2;
	uint8_t RX_TX_Diversity		: 2;
	uint8_t RX_Gain				: 4;
	uint8_t Max_MCS				: 4;
	uint8_t Softbuffer_Size		: 4;
	uint8_t N_HARQ_Process		: 2;
	uint8_t Reserved_2			: 2;
	uint8_t HARQ_Feedback_delay	: 4;
	uint8_t D_Delay				: 1;
	uint8_t HalfDup				: 1;
	uint8_t Reserved_3			: 2;
	uint8_t mu					: 3;
	uint8_t beta				: 4;
	uint8_t Reserved_4			: 2;
	uint8_t RD_Power_Class_1	: 3;
	uint8_t MAX_NSS_RX_1		: 2;
	uint8_t RX_TX_Diversity_1	: 2;
	uint8_t RX_Gain_1			: 4;
	uint8_t MAX_MCS_1			: 4;
	uint8_t Softbuffer_Size_1	: 4;
	uint8_t N_HARQ_Process_1	: 2;
	uint8_t Reserved_5			: 2;
	uint8_t HARQ_Feedback_delay_1	: 4;
	uint8_t Reserved_6			:4;

	uint8_t	mlie_5[12];
}mlie_5_t;


typedef struct {

	//Neighbouring IE definitions (6.4.3.6)

	uint8_t	Reserved				: 1;
	uint8_t	ID						: 1;
	uint8_t	mu						: 1;
	uint8_t	SNR						: 1;
	uint8_t RSSI_2					: 1;
	uint8_t Power_Const				: 1;
	uint8_t Next_Channel			: 1;
	uint8_t TTN						: 1;
	uint8_t	Network_Beacon_Period	: 4;
	uint8_t	Cluster_Beacon_Period	: 4;
	uint32_t Long_RD_ID				: 32;
	uint8_t Reserved_1				: 4;
	uint16_t	Next_Cluster_Channel	: 12;
	uint32_t	TTN_1				: 32;
	uint8_t RSSI_2_1				: 8;
	uint8_t SNR_1					: 8;
	uint8_t Radio_Device_Class_mu	: 3;
	uint8_t	Radio_Device_Class_beta	: 4;
	uint8_t	Reserved_2				: 1;

	uint8_t	mlie_6[18];
}mlie_6_t;


typedef struct {

	//Broadcast Indication IE definitions (6.4.3.7)

	uint8_t	Indication_Type		: 3;
	uint8_t	IDType				: 1;
	uint8_t	ACK_NACK			: 1;
	uint8_t	Feedback			: 2;
	uint8_t Resource_Allocation	: 1;
	uint16_t LongShort_RDID		: 16;
	uint8_t MCS_MIMO_Feedback	: 8;

	uint8_t	mlie_7[4];
}mlie_7_t;


typedef struct {

	//Padding IE definitions (6.4.3.8)

	//Better to create a function due to its variable length.

}mlie_8_t;


typedef struct {

	//Group Assignment IE definitions (6.4.3.9)

	uint8_t Single			: 1;
	uint8_t Group_ID		: 7;
	uint8_t Direct			: 1;
	uint8_t ResourceTag		: 7;
	uint8_t Direct1			: 1;
	uint8_t ResourceTag1	: 7;
	uint8_t Direct2			: 1;
	uint8_t ResourceTag2	: 7;

	uint8_t	mlie_9[4];


}mlie_9_t;


typedef struct {

	//Load Info IE (6.4.3.10)

	uint8_t Reserved				: 4;
	uint8_t MAX_Assoc				: 1;
	uint8_t RD_PT_Load				: 1;
	uint8_t RACH_Load				: 1;
	uint8_t Channel_Load			: 1;
	uint8_t Traffic_Load_Percentage	: 8;	
	uint16_t MAX_n_Assoc_RDs		: 16;
	uint8_t Associated_RDs_FT		: 8;
	uint8_t Associated_RDs_PT		: 8;
	uint8_t RACH_Load_percentage	: 8;
	uint8_t Free_subslots_detected	: 8;
	uint8_t Busy_subslots_detected	: 8;

	uint8_t	mlie_10[9];

}mlie_10_t;



typedef struct {

	//Configuration Request IE (6.4.3.11)

	//Zero length

}mlie_11_t;


typedef struct {

	//Measurement Report IE (6.4.3.12)

	uint8_t	Reserved			: 3;
	uint8_t	SNR					: 1;
	uint8_t RSSI_2				: 1;
	uint8_t RSSI_1				: 1;
	uint8_t Tx_Count			: 1;
	uint8_t RACH				: 1;
	uint8_t SNR_Result			: 8;
	uint8_t RSSI_2_Result		: 8;
	uint8_t RSSI_1_Result		: 8;
	uint8_t Tx_Count_Result		: 8;

	uint8_t	mlie_12[5];

}mlie_12_t;


typedef struct {

	//Radio Device Status (6.4.3.13)

	uint8_t	Reserved	: 2;
	uint8_t	Status_Flag	: 2;
	uint8_t	Duration	: 4;

	uint8_t	mlie_13[1];

}mlie_13_t;


typedef struct {

	//Keep Alive IE (6.4.3.14)

	//Zero length

}mlie_14_t;





int get_mlie_1(mlie_1_t* mlie_1);
int get_mlie_2(mlie_2_t* mlie_2);
int get_mlie_3(mlie_3_t* mlie_3);
int get_mlie_4(mlie_4_t* mlie_4);
int get_mlie_5(mlie_5_t* mlie_5);
int get_mlie_6(mlie_6_t* mlie_6);
int get_mlie_7(mlie_7_t* mlie_7);
int get_mlie_8(mlie_8_t* mlie_8);
int get_mlie_9(mlie_9_t* mlie_9);
int get_mlie_10(mlie_10_t* mlie_10);
int get_mlie_11(mlie_11_t* mlie_11);
int get_mlie_12(mlie_12_t* mlie_12);
int get_mlie_13(mlie_13_t* mlie_13);




#endif