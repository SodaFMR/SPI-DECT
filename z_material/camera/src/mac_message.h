/*
	Author: Iván Tomás García
	Property of the Universidad Politécnica de Cartagena
*/

#include <stdint.h>
#ifndef MAC_MESSAGE_H
#define MAC_MESSAGE_H

	typedef struct {

		//Network Beacon Message(6.4.2.2)
		uint8_t	Reserved					: 3;
		uint8_t	TX_Power					: 1;
		uint8_t	Power_Const					: 1;
		uint8_t	Current						: 1;
		uint8_t	Network_Beacon_Channels		: 2;
		uint8_t	Network_Beacon_Period		: 4;
		uint8_t	Cluster_Beacon_Period		: 4;
		uint8_t	Reserved_1					: 3;
		uint16_t	Next_Cluster_Channel	: 13;
		uint32_t	TTn						: 32;	//Time To next
		uint8_t	Reserved_2					: 4;
		uint8_t	Clusters_Max_TX_Power		: 4;
		uint8_t	Reserved_3					: 3;
		uint16_t	Current_Cluster_Channel	: 13;
		uint8_t	Reserved_4					: 3;
		uint16_t	ANBC					: 13;	//Additional_Network_Beacon_Channels
		
		uint8_t	mlmf_1[13];
	}mlmf_1_t;

	typedef struct {

		//Cluster Beacon Message(6.4.2.3)
		uint8_t	SFN							: 8;
		uint8_t	Reserved					: 3;
		uint8_t	TX_Power					: 1;
		uint8_t	Power_Const					: 1;
		uint8_t	FO							: 1;
		uint8_t Next_Channel				: 1;
		uint8_t	TTN							: 1;
		uint8_t	Network_Beacon_Period		: 4;
		uint8_t	Cluster_Beacon_Period		: 4;
		uint8_t	countToTrigger				: 4;
		uint8_t	ReQuality					: 2;
		uint8_t	MinQuality					: 2;
		uint8_t	Reserved_1					: 4;
		uint8_t	Clusters_Max_TX_Power		: 4;
		uint8_t	Frame_Offset				: 8;
		uint8_t	Reserved_2					: 3;
		uint16_t	Next_Cluster_Channel	: 13;
		uint32_t	TTn						: 32;	//Time To next
		
		uint8_t	mlmf_2[12];
	}mlmf_2_t;

	typedef struct {

		//Association Request Message(6.4.2.4)
		uint8_t	Setup_Cause					: 3;
		uint8_t	N_Flows						: 3;
		uint8_t	Power_Const					: 1;
		uint8_t	FT_Mode						: 1;
		uint8_t Current						: 1;
		uint8_t	Reserved					: 7;
		uint8_t	HARQ_P_TX					: 3;
		uint8_t	MAX_HARQ_Re_TX				: 5;
		uint8_t	HARQ_P_RX					: 3;
		uint8_t	MAX_HARQ_Re_RX				: 5;
		uint8_t	Reserved_1					: 2;
		uint8_t	FlowID						: 6;
		uint8_t	Network_Beacon_Period		: 4;
		uint8_t	Cluster_Beacon_Period		: 4;
		uint8_t	Reserved_2					: 3;
		uint16_t	Next_Cluster_Channel	: 13;
		uint32_t	TTn						: 32;	//Time To next
		uint8_t	Reserved_3					: 3;
		uint16_t	Current_Cluster_Channel	: 13;

		uint8_t	mlmf_3[14];
	}mlmf_3_t;

	typedef struct {

		//Association Response Message(6.4.2.5)
		uint8_t	ACK_NACK		: 1;
		uint8_t	Reserved		: 1;
		uint8_t	HARQ_mod		: 1;
		uint8_t	NFlows			: 3;
		uint8_t Group			: 1;
		uint8_t	TX_Power		: 1;
		uint8_t	Reject_Cause	: 4;
		uint8_t	Reject_Time		: 4;
		uint8_t	HARQ_P_RX		: 3;
		uint8_t	MAX_HARQ_Re_RX	: 5;
		uint8_t	HARQ_P_TX		: 3;
		uint8_t	MAX_HARQ_Re_TX	: 5;
		uint8_t	Reserved_1		: 2;
		uint8_t	Flow_ID			: 6;
		uint8_t	Reserved_2		: 1;
		uint8_t	Group_ID		: 7;
		uint8_t	Reserved_3		: 1;	//Time To next
		uint8_t	Resource_TAG	: 7;

		uint8_t	mlmf_4[7];
	}mlmf_4_t;


	typedef struct {

		//Association Request Message(6.4.2.6)
		uint8_t	Release_Cause	: 4;
		uint8_t	Reserved		: 4;

		uint8_t	mlmf_5;
	}mlmf_5_t;


	typedef struct {

		//Reconfiguration Request Message(6.4.2.7)
		uint8_t	TX_HARQ			: 1;
		uint8_t	RX_HARQ			: 1;
		uint8_t	RD_Capability	: 1;
		uint8_t	N_Flows			: 3;
		uint8_t Radio_Resource	: 2;
		uint8_t HARQ_P_TX		: 3;
		uint8_t	MAX_HARQ_Re_TX	: 5;
		uint8_t	HARQ_P_RX		: 3;
		uint8_t	MAX_HARQ_Re_RX	: 5;
		uint8_t	Setup_Release	: 1;
		uint8_t	Reserved		: 1;
		uint8_t	Flow_ID			: 6;

		uint8_t	mlmf_6[4];
	}mlmf_6_t;


	typedef struct {

		//Reconfiguration Response Message(6.4.2.8)
		uint8_t	TX_HARQ			: 1;
		uint8_t	RX_HARQ			: 1;
		uint8_t	RD_Capability	: 1;
		uint8_t	N_Flows			: 3;
		uint8_t Radio_Resource	: 2;
		uint8_t HARQ_P_TX		: 3;
		uint8_t	MAX_HARQ_Re_TX	: 5;
		uint8_t	HARQ_P_RX		: 3;
		uint8_t	MAX_HARQ_Re_RX	: 5;
		uint8_t	Setup_Release	: 1;
		uint8_t	Reserved		: 1;
		uint8_t	Flow_ID			: 6;

		uint8_t	mlmf_7[4];
	}mlmf_7_t;


	typedef struct {

		//Additional MAC message (6.4.2.9)
	
	}mlmf_8_t;

	int get_mlmf_1(mlmf_1_t* mlmf_1);
	int get_mlmf_2(mlmf_2_t* mlmf_2);
	int get_mlmf_3(mlmf_3_t* mlmf_3);
	int get_mlmf_4(mlmf_4_t* mlmf_4);
	int get_mlmf_5(mlmf_5_t* mlmf_5);
	int get_mlmf_6(mlmf_6_t* mlmf_6);
	int get_mlmf_7(mlmf_7_t* mlmf_7);

	int get_mlmf_1_rev(mlmf_1_t* mlmf_1);
	int get_mlmf_2_rev(mlmf_2_t* mlmf_2);
	int get_mlmf_3_rev(mlmf_3_t* mlmf_3);
	int get_mlmf_4_rev(mlmf_4_t* mlmf_4);
	int get_mlmf_5_rev(mlmf_5_t* mlmf_5);
	int get_mlmf_6_rev(mlmf_6_t* mlmf_6);
	int get_mlmf_7_rev(mlmf_7_t* mlmf_7);



#endif 