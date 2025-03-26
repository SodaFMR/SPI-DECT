/*
 * Author: Iván Tomas García
 * Property of the Universidad Politécnica de Cartagena
*/


#include <string.h>

#include "mac_message.h"
#include "debug.h"


	//PDU MAC Layer Message Type Construction

	int get_mlmf_1(mlmf_1_t* mlmf_1) {    //Network Beacon Message (6.4.2.2)

		mlmf_1->mlmf_1[0] = (mlmf_1->mlmf_1[0] & 0x1F) | (mlmf_1->Reserved << 5);
		mlmf_1->mlmf_1[0] = (mlmf_1->mlmf_1[0] & 0xEF) | (mlmf_1->TX_Power << 4);
		mlmf_1->mlmf_1[0] = (mlmf_1->mlmf_1[0] & 0xF7) | (mlmf_1->Power_Const << 3);
		mlmf_1->mlmf_1[0] = (mlmf_1->mlmf_1[0] & 0xFB) | (mlmf_1->Current << 2);
		mlmf_1->mlmf_1[0] = (mlmf_1->mlmf_1[0] & 0xFC) | mlmf_1->Network_Beacon_Channels;
		mlmf_1->mlmf_1[1] = (mlmf_1->mlmf_1[1] & 0x0F) | (mlmf_1->Network_Beacon_Period << 4);
		mlmf_1->mlmf_1[1] = (mlmf_1->mlmf_1[1] & 0xF0) | mlmf_1->Cluster_Beacon_Period;
		mlmf_1->mlmf_1[2] = (mlmf_1->mlmf_1[2] & 0x1F) | (mlmf_1->Reserved_1 << 5);
		mlmf_1->mlmf_1[2] = (uint8_t) (mlmf_1->mlmf_1[2] & 0xE0) | (mlmf_1->Next_Cluster_Channel >> 8);
		mlmf_1->mlmf_1[3] = (uint8_t) (mlmf_1->Next_Cluster_Channel & 0xFF);
		mlmf_1->mlmf_1[4] = (uint8_t) 0xFF & (mlmf_1->TTn >> 24);
		mlmf_1->mlmf_1[5] = (uint8_t) 0xFF & (mlmf_1->TTn >> 16);
		mlmf_1->mlmf_1[6] = (uint8_t) 0xFF & (mlmf_1->TTn >> 8);
		mlmf_1->mlmf_1[7] = (uint8_t) (0xFF & mlmf_1->TTn);
		mlmf_1->mlmf_1[8] = (mlmf_1->mlmf_1[8] & 0x0F) | (mlmf_1->Reserved_2 << 4);
		mlmf_1->mlmf_1[8] = (mlmf_1->mlmf_1[8] & 0xF0) | mlmf_1->Clusters_Max_TX_Power;
		mlmf_1->mlmf_1[9] = (mlmf_1->mlmf_1[9] & 0x1F) | (mlmf_1->Reserved_3 << 5);
		mlmf_1->mlmf_1[9] = (uint8_t) (mlmf_1->mlmf_1[9] & 0xE0) | (mlmf_1->Current_Cluster_Channel >> 8);
		mlmf_1->mlmf_1[10] = (uint8_t) (0xFF & mlmf_1->Current_Cluster_Channel);
		mlmf_1->mlmf_1[11] = (mlmf_1->mlmf_1[8] & 0x1F) | (mlmf_1->Reserved_4);
		mlmf_1->mlmf_1[11] = (mlmf_1->mlmf_1[2] & 0xE0) | (mlmf_1->ANBC << 5);
		mlmf_1->mlmf_1[12] = (uint8_t) (0xFF & mlmf_1->ANBC);

		return DECT2020_SUCCESS;
	}

	int get_mlmf_2(mlmf_2_t* mlmf_2) {    //Cluster Beacon Message (6.4.2.3)

		mlmf_2->mlmf_2[0] = mlmf_2->SFN;
		mlmf_2->mlmf_2[1] = (mlmf_2->mlmf_2[0] & 0x1F) | (mlmf_2->Reserved << 5);
		mlmf_2->mlmf_2[1] = (mlmf_2->mlmf_2[1] & 0xEF) | (mlmf_2->TX_Power << 4);
		mlmf_2->mlmf_2[1] = (mlmf_2->mlmf_2[1] & 0xF7) | (mlmf_2->Power_Const << 3);
		mlmf_2->mlmf_2[1] = (mlmf_2->mlmf_2[1] & 0xFB) | (mlmf_2->FO << 2);
		mlmf_2->mlmf_2[1] = (mlmf_2->mlmf_2[1] & 0xFD) | (mlmf_2->Next_Channel << 1);
		mlmf_2->mlmf_2[1] = (mlmf_2->mlmf_2[1] & 0xFE) | mlmf_2->TTN;
		mlmf_2->mlmf_2[2] = (mlmf_2->mlmf_2[2] & 0x0F) | (mlmf_2->Network_Beacon_Period << 4);
		mlmf_2->mlmf_2[2] = (mlmf_2->mlmf_2[2] & 0xF0) | mlmf_2->Cluster_Beacon_Period;
		mlmf_2->mlmf_2[3] = (mlmf_2->mlmf_2[3] & 0x0F) | (mlmf_2->countToTrigger << 4);
		mlmf_2->mlmf_2[3] = (mlmf_2->mlmf_2[3] & 0xF3) | (mlmf_2->ReQuality << 2);
		mlmf_2->mlmf_2[3] = (mlmf_2->mlmf_2[3] & 0xFC) | mlmf_2->MinQuality;
		mlmf_2->mlmf_2[4] = (mlmf_2->mlmf_2[4] & 0x0F) | (mlmf_2->Reserved_1 << 4);
		mlmf_2->mlmf_2[4] = (mlmf_2->mlmf_2[4] & 0xF0) | mlmf_2->Clusters_Max_TX_Power;
		mlmf_2->mlmf_2[5] = mlmf_2->Frame_Offset;
		mlmf_2->mlmf_2[6] = (mlmf_2->mlmf_2[6] & 0x0F) | (mlmf_2->Reserved_2 << 4);
		mlmf_2->mlmf_2[6] = (uint8_t) (mlmf_2->mlmf_2[6] & 0xF0) | (mlmf_2->Next_Cluster_Channel >> 8);
		mlmf_2->mlmf_2[7] = (uint8_t) (0xFF & mlmf_2->Next_Cluster_Channel);
		mlmf_2->mlmf_2[8] = (uint8_t)0xFF & (mlmf_2->TTn >> 24);
		mlmf_2->mlmf_2[9] = (uint8_t)0xFF & (mlmf_2->TTn >> 16);
		mlmf_2->mlmf_2[10] = (uint8_t) (0xFF & (mlmf_2->TTn >> 8));
		mlmf_2->mlmf_2[11] = (uint8_t) (0xFF & mlmf_2->TTn);

		return DECT2020_SUCCESS;
	}

	int get_mlmf_3(mlmf_3_t* mlmf_3) {	//Association Request Message IE(6.4.2.4)

		mlmf_3->mlmf_3[0] = (mlmf_3->mlmf_3[0] & 0x1F) | (mlmf_3->Setup_Cause << 5);
		mlmf_3->mlmf_3[0] = (mlmf_3->mlmf_3[0] & 0xE3) | (mlmf_3->N_Flows << 2);
		mlmf_3->mlmf_3[0] = (mlmf_3->mlmf_3[0] & 0xFD) | (mlmf_3->Power_Const << 1);
		mlmf_3->mlmf_3[0] = (mlmf_3->mlmf_3[0] & 0xFE) | mlmf_3->FT_Mode;
		mlmf_3->mlmf_3[1] = (mlmf_3->mlmf_3[1] & 0x7F) | (mlmf_3->Current << 7);
		mlmf_3->mlmf_3[1] = (mlmf_3->mlmf_3[1] & 0x80) | mlmf_3->Reserved;
		mlmf_3->mlmf_3[2] = (mlmf_3->mlmf_3[2] & 0x1F) | (mlmf_3->HARQ_P_TX << 5);
		mlmf_3->mlmf_3[2] = (mlmf_3->mlmf_3[2] & 0x70) | mlmf_3->MAX_HARQ_Re_TX;
		mlmf_3->mlmf_3[3] = (mlmf_3->mlmf_3[3] & 0x1F) | (mlmf_3->HARQ_P_RX << 5);
		mlmf_3->mlmf_3[3] = (mlmf_3->mlmf_3[3] & 0x70) | mlmf_3->MAX_HARQ_Re_RX;
		mlmf_3->mlmf_3[4] = (mlmf_3->mlmf_3[4] & 0x3F) | (mlmf_3->Reserved_1 << 6);
		mlmf_3->mlmf_3[4] = (mlmf_3->mlmf_3[4] & 0xC0) | mlmf_3->FlowID;
		mlmf_3->mlmf_3[5] = (mlmf_3->mlmf_3[5] & 0x0F) | (mlmf_3->Network_Beacon_Period << 4);
		mlmf_3->mlmf_3[5] = (mlmf_3->mlmf_3[5] & 0xF0) | mlmf_3->Cluster_Beacon_Period;
		mlmf_3->mlmf_3[6] = (mlmf_3->mlmf_3[6] & 0x1F) | (mlmf_3->Reserved_2 << 5);
		mlmf_3->mlmf_3[6] = (uint8_t) (mlmf_3->mlmf_3[6] & 0xE0) | (mlmf_3->Next_Cluster_Channel >> 8);
		mlmf_3->mlmf_3[7] = (uint8_t) (0xFF & mlmf_3->Next_Cluster_Channel);
		mlmf_3->mlmf_3[8] = (uint8_t) 0xFF & (mlmf_3->TTn >> 24);
		mlmf_3->mlmf_3[9] = (uint8_t) 0xFF & (mlmf_3->TTn >> 16);
		mlmf_3->mlmf_3[10] = (uint8_t) 0xFF & (mlmf_3->TTn >> 8);
		mlmf_3->mlmf_3[11] = (uint8_t) (0xFF & mlmf_3->TTn);
		mlmf_3->mlmf_3[12] = (mlmf_3->Reserved_3 << 5);
		mlmf_3->mlmf_3[12] = (uint8_t) (mlmf_3->mlmf_3[12] & 0xE0) | (mlmf_3->Current_Cluster_Channel >> 8);
		mlmf_3->mlmf_3[13] = (uint8_t) (0xFF & mlmf_3->Current_Cluster_Channel);


		return DECT2020_SUCCESS;
	}

	int get_mlmf_4(mlmf_4_t* mlmf_4) {	//Association Response Message (6.4.2.5)

		mlmf_4->mlmf_4[0] = (mlmf_4->mlmf_4[0] & 0x7F) | (mlmf_4->ACK_NACK << 7);
		mlmf_4->mlmf_4[0] = (mlmf_4->mlmf_4[0] & 0xBF) | (mlmf_4->Reserved << 6);
		mlmf_4->mlmf_4[0] = (mlmf_4->mlmf_4[0] & 0xDF) | (mlmf_4->HARQ_mod << 5);
		mlmf_4->mlmf_4[0] = (mlmf_4->mlmf_4[0] & 0xE3) | (mlmf_4->NFlows << 2);
		mlmf_4->mlmf_4[0] = (mlmf_4->mlmf_4[0] & 0xFD) | (mlmf_4->Group << 1);
		mlmf_4->mlmf_4[0] = (mlmf_4->mlmf_4[0] & 0xFE) | mlmf_4->TX_Power;
		mlmf_4->mlmf_4[1] = (mlmf_4->mlmf_4[1] & 0x0F) | (mlmf_4->Reject_Cause << 4);
		mlmf_4->mlmf_4[1] = (mlmf_4->mlmf_4[1] & 0xF0) | mlmf_4->Reject_Time;
		mlmf_4->mlmf_4[2] = (mlmf_4->mlmf_4[2] & 0x1F) | (mlmf_4->HARQ_P_RX << 5);
		mlmf_4->mlmf_4[2] = (mlmf_4->mlmf_4[2] & 0x70) | mlmf_4->MAX_HARQ_Re_RX;
		mlmf_4->mlmf_4[3] = (mlmf_4->mlmf_4[3] & 0x1F) | (mlmf_4->HARQ_P_TX << 5);
		mlmf_4->mlmf_4[3] = (mlmf_4->mlmf_4[3] & 0x70) | mlmf_4->MAX_HARQ_Re_TX;
		mlmf_4->mlmf_4[4] = (mlmf_4->mlmf_4[4] & 0x3F) | (mlmf_4->Reserved_1 << 6);
		mlmf_4->mlmf_4[4] = (mlmf_4->mlmf_4[4] & 0xC0) | mlmf_4->Flow_ID;
		mlmf_4->mlmf_4[5] = (mlmf_4->mlmf_4[5] & 0x7F) | (mlmf_4->Reserved_2 << 7);
		mlmf_4->mlmf_4[5] = (mlmf_4->mlmf_4[5] & 0x80) | mlmf_4->Group_ID;
		mlmf_4->mlmf_4[6] = (mlmf_4->mlmf_4[6] & 0x7F) | (mlmf_4->Reserved_3 << 7);
		mlmf_4->mlmf_4[6] = (mlmf_4->mlmf_4[6] & 0x80) | mlmf_4->Resource_TAG;


		return DECT2020_SUCCESS;
	}


	int get_mlmf_5(mlmf_5_t* mlmf_5) {    //Association Release Message (6.4.2.6)
		mlmf_5->mlmf_5 = (mlmf_5->mlmf_5 & 0x0F) | (mlmf_5->Release_Cause << 4);
		mlmf_5->mlmf_5 = (mlmf_5->mlmf_5 & 0xF0) | mlmf_5->Reserved;

		return DECT2020_SUCCESS;
	}


	int get_mlmf_6(mlmf_6_t* mlmf_6) {	//Reconfiguration Request Message (6.4.2.7)

		mlmf_6->mlmf_6[0] = (mlmf_6->mlmf_6[0] & 0x7F) | (mlmf_6->TX_HARQ << 7);
		mlmf_6->mlmf_6[0] = (mlmf_6->mlmf_6[0] & 0xBF) | (mlmf_6->RX_HARQ << 6);
		mlmf_6->mlmf_6[0] = (mlmf_6->mlmf_6[0] & 0xDF) | (mlmf_6->RD_Capability << 5);
		mlmf_6->mlmf_6[0] = (mlmf_6->mlmf_6[0] & 0xE3) | (mlmf_6->N_Flows << 2);
		mlmf_6->mlmf_6[0] = (mlmf_6->mlmf_6[0] & 0xFD) | mlmf_6->Radio_Resource;
		mlmf_6->mlmf_6[1] = (mlmf_6->mlmf_6[1] & 0x1F) | (mlmf_6->HARQ_P_TX << 5);
		mlmf_6->mlmf_6[1] = (mlmf_6->mlmf_6[1] & 0x70) | mlmf_6->MAX_HARQ_Re_TX;
		mlmf_6->mlmf_6[2] = (mlmf_6->mlmf_6[2] & 0x1F) | (mlmf_6->HARQ_P_RX << 5);
		mlmf_6->mlmf_6[2] = (mlmf_6->mlmf_6[2] & 0x70) | mlmf_6->MAX_HARQ_Re_RX;
		mlmf_6->mlmf_6[3] = (mlmf_6->mlmf_6[3] & 0x7F) | (mlmf_6->Setup_Release << 7);
		mlmf_6->mlmf_6[3] = (mlmf_6->mlmf_6[3] & 0xBF) | (mlmf_6->Reserved << 6);
		mlmf_6->mlmf_6[3] = (mlmf_6->mlmf_6[3] & 0xC0) | mlmf_6->Flow_ID;


		return DECT2020_SUCCESS;
	}


	int get_mlmf_7(mlmf_7_t* mlmf_7) {	//Reconfiguration Response Message (6.4.2.8)

		mlmf_7->mlmf_7[0] = (mlmf_7->mlmf_7[0] & 0x7F) | (mlmf_7->TX_HARQ << 7);
		mlmf_7->mlmf_7[0] = (mlmf_7->mlmf_7[0] & 0xBF) | (mlmf_7->RX_HARQ << 6);
		mlmf_7->mlmf_7[0] = (mlmf_7->mlmf_7[0] & 0xDF) | (mlmf_7->RD_Capability << 5);
		mlmf_7->mlmf_7[0] = (mlmf_7->mlmf_7[0] & 0xE3) | (mlmf_7->N_Flows << 2);
		mlmf_7->mlmf_7[0] = (mlmf_7->mlmf_7[0] & 0xFD) | mlmf_7->Radio_Resource;
		mlmf_7->mlmf_7[1] = (mlmf_7->mlmf_7[1] & 0x1F) | (mlmf_7->HARQ_P_TX << 5);
		mlmf_7->mlmf_7[1] = (mlmf_7->mlmf_7[1] & 0x70) | mlmf_7->MAX_HARQ_Re_TX;
		mlmf_7->mlmf_7[2] = (mlmf_7->mlmf_7[2] & 0x1F) | (mlmf_7->HARQ_P_RX << 5);
		mlmf_7->mlmf_7[2] = (mlmf_7->mlmf_7[2] & 0x70) | mlmf_7->MAX_HARQ_Re_RX;
		mlmf_7->mlmf_7[3] = (mlmf_7->mlmf_7[3] & 0x7F) | (mlmf_7->Setup_Release << 7);
		mlmf_7->mlmf_7[3] = (mlmf_7->mlmf_7[3] & 0xBF) | (mlmf_7->Reserved << 6);
		mlmf_7->mlmf_7[3] = (mlmf_7->mlmf_7[3] & 0xC0) | mlmf_7->Flow_ID;


		return DECT2020_SUCCESS;
	}

	//To be revised

	int get_mlmf_1_rev(mlmf_1_t* mlmf_1) {

		mlmf_1->Reserved					= (mlmf_1->mlmf_1[0] & 0xE0) >> 5;
		mlmf_1->TX_Power 				= (mlmf_1->mlmf_1[0] & 0x10) >> 4;
		mlmf_1->Power_Const 				= (mlmf_1->mlmf_1[0] & 0x08) >> 3;
		mlmf_1->Current 					= (mlmf_1->mlmf_1[0] & 0x04) >> 2;
		mlmf_1->Network_Beacon_Channels	= mlmf_1->mlmf_1[0] & 0x03;
		mlmf_1->Network_Beacon_Period 	= (mlmf_1->mlmf_1[1] & 0xF0) >> 4;
		mlmf_1->Cluster_Beacon_Period 	= mlmf_1->mlmf_1[1] & 0x0F;
		mlmf_1->Reserved_1 				= (mlmf_1->mlmf_1[2] & 0xE0) >> 5;
		mlmf_1->Next_Cluster_Channel 	= (mlmf_1->mlmf_1[2] & 0x1F) << 8 | mlmf_1->mlmf_1[3];
		mlmf_1->TTn 						= (mlmf_1->mlmf_1[4] << 24) | (mlmf_1->mlmf_1[5] << 16) | (mlmf_1->mlmf_1[6] << 8) | mlmf_1->mlmf_1[7];
		mlmf_1->Reserved_2 				= (mlmf_1->mlmf_1[8] & 0xF0) >> 4;
		mlmf_1->Clusters_Max_TX_Power 	= mlmf_1->mlmf_1[8] & 0x0F;
		mlmf_1->Reserved_3 				= (mlmf_1->mlmf_1[9] & 0xE0) >> 5;
		mlmf_1->Current_Cluster_Channel 	= (mlmf_1->mlmf_1[9] & 0x1F) << 8 | mlmf_1->mlmf_1[10];
		mlmf_1->Reserved_4 				= (mlmf_1->mlmf_1[11] & 0x1F);
		mlmf_1->ANBC 					= (mlmf_1->mlmf_1[11] & 0xE0) & mlmf_1->mlmf_1[12];

		return DECT2020_SUCCESS;
	}

	int get_mlmf_2_rev(mlmf_2_t* mlmf_2) {

		mlmf_2->SFN 					= mlmf_2->mlmf_2[0];
		mlmf_2->Reserved 			= (mlmf_2->mlmf_2[1] & 0x20) >> 5;
		mlmf_2->TX_Power 			= (mlmf_2->mlmf_2[1] & 0x10) >> 4;
		mlmf_2->Power_Const 			= (mlmf_2->mlmf_2[1] & 0x08) >> 3;
		mlmf_2->FO 					= (mlmf_2->mlmf_2[1] & 0x04) >> 2;
		mlmf_2->Next_Channel 		= (mlmf_2->mlmf_2[1] & 0x02) >> 1;
		mlmf_2->TTN 					= mlmf_2->mlmf_2[1] & 0x01;
		mlmf_2->Network_Beacon_Period = (mlmf_2->mlmf_2[2] & 0xF0) >> 4;
		mlmf_2->Cluster_Beacon_Period = mlmf_2->mlmf_2[2] & 0x0F;
		mlmf_2->countToTrigger 		= (mlmf_2->mlmf_2[3] & 0xF0) >> 4;
		mlmf_2->ReQuality 			= (mlmf_2->mlmf_2[3] & 0x0C) >> 2;
		mlmf_2->MinQuality 			= mlmf_2->mlmf_2[3] & 0x03;
		mlmf_2->Reserved_1 			= (mlmf_2->mlmf_2[4] & 0x40) >> 6;
		mlmf_2->Clusters_Max_TX_Power = mlmf_2->mlmf_2[4] & 0x3F;
		mlmf_2->Frame_Offset 		= mlmf_2->mlmf_2[5];
		mlmf_2->Reserved_2 			= (mlmf_2->mlmf_2[6] & 0x10) >> 4;
		mlmf_2->Next_Cluster_Channel = (mlmf_2->mlmf_2[6] & 0x0F) << 8 | mlmf_2->mlmf_2[7];
		mlmf_2->TTn 					= (mlmf_2->mlmf_2[8] << 24) | (mlmf_2->mlmf_2[9] << 16) | (mlmf_2->mlmf_2[10] << 8) | mlmf_2->mlmf_2[11];

		return DECT2020_SUCCESS;
	}

	int get_mlmf_3_rev(mlmf_3_t* mlmf_3) {
		
		mlmf_3->Setup_Cause 			= (mlmf_3->mlmf_3[0] & 0xE0) >> 5;
		mlmf_3->N_Flows 				= (mlmf_3->mlmf_3[0] & 0x1C) >> 2;
		mlmf_3->Power_Const 			= (mlmf_3->mlmf_3[0] & 0x02) >> 1;
		mlmf_3->FT_Mode 				= mlmf_3->mlmf_3[0] & 0x01;
		mlmf_3->Current 				= (mlmf_3->mlmf_3[1] & 0x80) >> 7;
		mlmf_3->Reserved 			= (mlmf_3->mlmf_3[1] & 0x40) >> 6;
		mlmf_3->HARQ_P_TX 			= (mlmf_3->mlmf_3[2] & 0xE0) >> 5;
		mlmf_3->MAX_HARQ_Re_TX 		= (mlmf_3->mlmf_3[2] & 0x70) >> 4;
		mlmf_3->HARQ_P_RX 			= (mlmf_3->mlmf_3[3] & 0xE0) >> 5;
		mlmf_3->MAX_HARQ_Re_RX 		= (mlmf_3->mlmf_3[3] & 0x70) >> 4;
		mlmf_3->Reserved_1 			= (mlmf_3->mlmf_3[4] & 0xC0) >> 6;
		mlmf_3->FlowID 				= mlmf_3->mlmf_3[4] & 0x3F;
		mlmf_3->Network_Beacon_Period = (mlmf_3->mlmf_3[5] & 0xF0) >> 4;
		mlmf_3->Cluster_Beacon_Period = mlmf_3->mlmf_3[5] & 0x0F;
		mlmf_3->Reserved_2 			= (mlmf_3->mlmf_3[6] & 0xE0) >> 5;
		mlmf_3->Next_Cluster_Channel = (mlmf_3->mlmf_3[6] & 0x1F) << 8 | mlmf_3->mlmf_3[7];
		mlmf_3->TTn 					= (mlmf_3->mlmf_3[8] << 24) | (mlmf_3->mlmf_3[9] << 16) | (mlmf_3->mlmf_3[10] << 8) | mlmf_3->mlmf_3[11];
		mlmf_3->Reserved_3 			= (mlmf_3->mlmf_3[12] & 0xE0) >> 5;
		mlmf_3->Current_Cluster_Channel = (mlmf_3->mlmf_3[12] & 0x1F) << 8 | mlmf_3->mlmf_3[13];

		return DECT2020_SUCCESS;
	}

	int get_mlmf_4_rev(mlmf_4_t* mlmf_4) {

		mlmf_4->ACK_NACK = (mlmf_4->mlmf_4[0] & 0x80) >> 7;
		mlmf_4->Reserved = (mlmf_4->mlmf_4[0] & 0x40) >> 6;
		mlmf_4->HARQ_mod = (mlmf_4->mlmf_4[0] & 0x20) >> 5;
		mlmf_4->NFlows = (mlmf_4->mlmf_4[0] & 0x0C) >> 2;
		mlmf_4->Group = (mlmf_4->mlmf_4[0] & 0x02) >> 1;
		mlmf_4->TX_Power = mlmf_4->mlmf_4[0] & 0x01;
		mlmf_4->Reject_Cause = (mlmf_4->mlmf_4[1] & 0xF0) >> 4;
		mlmf_4->Reject_Time = mlmf_4->mlmf_4[1] & 0x0F;
		mlmf_4->HARQ_P_RX = (mlmf_4->mlmf_4[2] & 0xE0) >> 5;
		mlmf_4->MAX_HARQ_Re_RX = (mlmf_4->mlmf_4[2] & 0x70) >> 4;
		mlmf_4->HARQ_P_TX = (mlmf_4->mlmf_4[3] & 0xE0) >> 5;
		mlmf_4->MAX_HARQ_Re_TX = (mlmf_4->mlmf_4[3] & 0x70) >> 4;
		mlmf_4->Reserved_1 = (mlmf_4->mlmf_4[4] & 0xC0) >> 6;
		mlmf_4->Flow_ID = mlmf_4->mlmf_4[4] & 0x3F;
		mlmf_4->Reserved_2 = (mlmf_4->mlmf_4[5] & 0x80) >> 7;
		mlmf_4->Group_ID = mlmf_4->mlmf_4[5] & 0x7F;
		mlmf_4->Reserved_3 = (mlmf_4->mlmf_4[6] & 0x80) >> 7;
		mlmf_4->Resource_TAG = mlmf_4->mlmf_4[6] & 0x7F;

		return DECT2020_SUCCESS;
	}

	int get_mlmf_5_rev(mlmf_5_t* mlmf_5) {

		mlmf_5->Release_Cause = (mlmf_5->mlmf_5 & 0xF0) >> 4;
		mlmf_5->Reserved = mlmf_5->mlmf_5 & 0x0F;

		return DECT2020_SUCCESS;
	}


	int get_mlmf_6_rev(mlmf_6_t* mlmf_6) {
		mlmf_6->TX_HARQ = (mlmf_6->mlmf_6[0] & 0x80) >> 7;
		mlmf_6->RX_HARQ = (mlmf_6->mlmf_6[0] & 0x40) >> 6;
		mlmf_6->RD_Capability = (mlmf_6->mlmf_6[0] & 0x20) >> 5;
		mlmf_6->N_Flows = (mlmf_6->mlmf_6[0] & 0x1C) >> 2;
		mlmf_6->Radio_Resource = mlmf_6->mlmf_6[0] & 0x03;
		mlmf_6->HARQ_P_TX = (mlmf_6->mlmf_6[1] & 0xE0) >> 5;
		mlmf_6->MAX_HARQ_Re_TX = (mlmf_6->mlmf_6[1] & 0x70) >> 4;
		mlmf_6->HARQ_P_RX = (mlmf_6->mlmf_6[2] & 0xE0) >> 5;
		mlmf_6->MAX_HARQ_Re_RX = (mlmf_6->mlmf_6[2] & 0x70) >> 4;
		mlmf_6->Setup_Release = (mlmf_6->mlmf_6[3] & 0x80) >> 7;
		mlmf_6->Reserved = (mlmf_6->mlmf_6[3] & 0x40) >> 6;
		mlmf_6->Flow_ID = mlmf_6->mlmf_6[3] & 0x3F;

		return DECT2020_SUCCESS;
	}

	int get_mlmf_7_rev(mlmf_7_t* mlmf_7) {

		mlmf_7->TX_HARQ = (mlmf_7->mlmf_7[0] & 0x80) >> 7;
		mlmf_7->RX_HARQ = (mlmf_7->mlmf_7[0] & 0x40) >> 6;
		mlmf_7->RD_Capability = (mlmf_7->mlmf_7[0] & 0x20) >> 5;
		mlmf_7->N_Flows = (mlmf_7->mlmf_7[0] & 0x1C) >> 2;
		mlmf_7->Radio_Resource = mlmf_7->mlmf_7[0] & 0x03;
		mlmf_7->HARQ_P_TX = (mlmf_7->mlmf_7[1] & 0xE0) >> 5;
		mlmf_7->MAX_HARQ_Re_TX = (mlmf_7->mlmf_7[1] & 0x70) >> 4;
		mlmf_7->HARQ_P_RX = (mlmf_7->mlmf_7[2] & 0xE0) >> 5;
		mlmf_7->MAX_HARQ_Re_RX = (mlmf_7->mlmf_7[2] & 0x70) >> 4;
		mlmf_7->Setup_Release = (mlmf_7->mlmf_7[3] & 0x80) >> 7;
		mlmf_7->Reserved = (mlmf_7->mlmf_7[3] & 0x40) >> 6;
		mlmf_7->Flow_ID = mlmf_7->mlmf_7[3] & 0x3F;

		return DECT2020_SUCCESS;
	}



