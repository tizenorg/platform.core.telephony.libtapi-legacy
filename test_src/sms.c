/*
 * Telephony test application
 *
 * Copyright (c) 2013 Samsung Electronics Co. Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>

#include <tapi_common.h>
#include <TelSms.h>
#include <TapiUtility.h>
#include <ITapiNetText.h>

#include "menu.h"
#include "sms.h"
#include "sms_util.h"

int					dcs_pos =0;
unsigned char			tp_mr = 0x00;

static char data_sms_index[MENU_DATA_SIZE + 1] = {};
static char data_sms_msg_status[MENU_DATA_SIZE + 1] = {};
static char data_sms_memory_status[MENU_DATA_SIZE + 1] = {};
static char data_sms_sca_ton[MENU_DATA_SIZE + 1] = {};
static char data_sms_sca_npi[MENU_DATA_SIZE + 1] = {1, };
static char data_sms_sca_number[MENU_DATA_SIZE + 1] = {};

static char data_sms_set_params_index[MENU_DATA_SIZE + 1] = {};
static char data_sms_set_params_vp[MENU_DATA_SIZE + 1] = {};

static char data_sms_set_cb_status[MENU_DATA_SIZE + 1] = {};
static char data_sms_set_cb_msg_id_rng_cnt[MENU_DATA_SIZE + 1] = {0, };
static char data_sms_set_cb_from_msg_id[MENU_DATA_SIZE + 1] = {};
static char data_sms_set_cb_to_msg_id[MENU_DATA_SIZE + 1] = {};
static char data_sms_set_cb_msg_id_selected[MENU_DATA_SIZE + 1] = {};

static char data_sms_send_more_msgs[MENU_DATA_SIZE + 1] = {};
static char data_sms_send_msg[MENU_DATA_SIZE + 1] = {};
static char data_sms_send_msg_dialling_num[MENU_DATA_SIZE + 1] = {};

static void on_resp_write_sms_in_sim(TapiHandle *handle, int result, void *data, void *user_data);
static void on_resp_sms_delivery_report(TapiHandle *handle, int result, void *data, void *user_data);
static void on_send_sms(TapiHandle *handle, int result, void *data, void *user_data);

/* Utilities */
static gboolean __encode_and_send_sms_submit_tpdu (MManager *mm, char *diallingNum, unsigned int dialNumLen,
		char *msg, unsigned int msg_len)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);

	TPDU_SMS_SUBMIT tpdu_submit;

	unsigned char  packet[TAPI_NETTEXT_MSG_SIZE_MAX];

	unsigned char  SCA[TAPI_NETTEXT_SCADDRESS_LEN_MAX] = {0,}; //send SMS SCA

	//	unsigned char tp_mr = 0x03;
	tp_mr++;
	unsigned char tp_pid = 0; //reaplace type (65)
	unsigned char tp_dcs; //=17; //class_1(17),class_2(18)
	unsigned char tp_vp;


	int index = 0, ScIndex = 0;
	int i;
	unsigned char             sca_len =0;
	TapiResult_t returnStatus ;

	TelSmsDatapackageInfo_t *pDataPackage= malloc(sizeof(TelSmsDatapackageInfo_t));

	memset(pDataPackage, 0, sizeof(TelSmsDatapackageInfo_t));

	memset(packet, 0, TAPI_NETTEXT_MSG_SIZE_MAX);

	/************SCA ADDRESS GET START**************/
	/*
	   returnStatus = TelTapiSmsGetSCA(&sc_addr,0);
	   msg("Service center addr returned from TAPI SCA address %s returnStatus %d\n",sc_addr.szDiallingNum,returnStatus);
	   SmsUtilDecodeAddrField(decodeScaAddr, &sc_addr.szDiallingNum[0],&sc_addr.Ton, &sc_addr.Npi);
	   sca_len = strlen( decodeScaAddr );

	   memcpy( SCA, decodeScaAddr, sca_len);
	   SCA[sca_len] = '\0';
	   */

	g_strlcpy((gchar *)SCA, data_sms_sca_number, TAPI_NETTEXT_SCADDRESS_LEN_MAX + 1);

	sca_len=strlen((char *)SCA);

	msg("Decode SCA Address =========");
	msg("SCA Number :%s, SCA Length :%d ",SCA , sca_len);
	msg("========================");

	//SCA ADDRESS GET END

	//SCA

	//TODO : SCA

	//	index +=SmsUtilEncodeAddrField(packet,SCA, strlen(SCA)  ,0x01,0x01);
	ScIndex  =SmsUtilEncodeAddrField((unsigned char*)pDataPackage->Sca ,(char *)SCA, sca_len  ,0x01,0x01);


	msg("ScIndex is %d", ScIndex);

	// TP-MTI, TP-RD, TP-VPF, TP-SRR, TP_UDHI, TP-RP
	tpdu_submit.msg_type = SMS_TPDU_SUBMIT;
	tpdu_submit.rd   =FALSE; //false :accept duplicate msg , true: reject duplicate msg
	tpdu_submit.vpf = SMS_VP_RELATIVE;
	tpdu_submit.srr  = FALSE;   //false :A status report is not requested , true: A status report is requested
	tpdu_submit.udhi = FALSE ; //false: contains only the short message , true :a Header in addition to the short message
	tpdu_submit.rp = FALSE  ; //false :TP Reply Path parameter is not set in this SMS SUBMIT, true : TP Reply Path parameter is set in this SMS SUBMIT

	packet[index] =  tpdu_submit.msg_type;							// SUBMIT: bits 0, 1
	packet[index] |= tpdu_submit.rd? 0x04 : 0;						// bit 2
	packet[index] |= tpdu_submit.vpf  << 3;	// bits 3, 4
	packet[index] |= tpdu_submit.srr ? 0x20 : 0;						// bit 5
	packet[index] |= tpdu_submit.udhi? 0x40 : 0;						// bit 6
	packet[index] |= tpdu_submit.rp ? 0x80 : 0;						// bit 7

	index++;
	msg("index of message ref is %d, msgref is %d",index,tp_mr);
	/* TP-MR */
	packet[index++] = tp_mr;

	// TP-DA
	index +=  SmsUtilEncodeAddrField( packet+index, diallingNum, dialNumLen, 0x02, 0x01);

	//msg("DESTINATTION  ADDR Encoded =========");

	//TP-PID
	packet[index++] = tp_pid;

	// TP_DCS (Data Coding Scheme)

	tpdu_submit.dcs.bCompressed = FALSE;
	tpdu_submit.dcs.bmsg_class_set = FALSE;
	tpdu_submit.dcs.coding_group_type = SMS_CODGRP_SM_GENERAL_DCS;
	tpdu_submit.dcs.alphabet_type = SMS_ALPHABET_DEFAULT;
	tpdu_submit.dcs.class_type = SMS_CLASS_2;

	SmsUtilEncodeDCS(&tp_dcs, &tpdu_submit.dcs);

	packet[index++] = tp_dcs;

	//msg("DCS Encoded:tp_dcs =%d =========",tp_dcs);

	//TP-VP

	tpdu_submit.vp.vp_rel_type = SMS_VP_REL_6H ;
	tpdu_submit.vp.vp_type = SMS_VP_RELATIVE;
	tpdu_submit.vp.vpValue = 0x01;

	SmsUtilEncodeValidity(&tp_vp,&tpdu_submit.vp);

	//tp_vp =65;
	packet[index++] = tp_vp;
	//msg("Validity Encoded :tp_vp =%d =========",tp_vp);

	// UDL
	packet[index++] = msg_len;

	// TP-UD

	if (tpdu_submit.dcs.alphabet_type == SMS_ALPHABET_DEFAULT )
	{
		int pos;
		pos =SmsUtilPackGSMCode( packet + index, msg, msg_len);
		index += pos;
	}

	msg("Send packet to phonseSever[%d] -----", index);

	for(i=0; i< index ; i++ )
	{
		msg("[%02x]", packet[i]);
		if( i%10 == 9)
			msg("\n");
	}
	msg("\n");

	memcpy( pDataPackage->szData, packet, index);
	pDataPackage->MsgLength = index;

	msg("***Sending the Message (Api:SendMsg)***");

	msg("Length : %d",pDataPackage->MsgLength);

	returnStatus = tel_send_sms(handle,  pDataPackage, 0, on_send_sms, NULL);

	CHECK_RESULT(returnStatus);

	g_free(pDataPackage);

	return TRUE;
}

static int __decode_sms_deliver_tpdu (unsigned int tpdu_len, char *tpdu)
{
	TPDU_SMS_DELIVER   tpdu_deliver;
	TmDateTime		   SCTimeStamp;;
	int		    orgAddr_len = 0;
	char	    diallingNum[SMS_ADDRESS_LEN_MAX];
	char		    orgAddr[SMS_ADDRESS_LEN_MAX]; //originating number
	int		    org_ton,org_npi;
	int		    position=0;

	/* TP-MTI, TP-MMS, TP-SRI, TP_UDHI, TP-RP */
	tpdu_deliver.msg_type = SMS_TPDU_DELIVER;
	tpdu_deliver.mms = ( tpdu[position] & 0x04 ) ? TRUE : FALSE;// bit 2 (Message Type Indicator)
	tpdu_deliver.sri =( tpdu[position] & 0x20 ) ? TRUE : FALSE;	// bit 5 (Status Report Indication)
	tpdu_deliver.udhi=( tpdu[position] & 0x40 ) ? TRUE : FALSE;	// bit 6 (User Data Header Indicator)
	tpdu_deliver.rp = ( tpdu[position] & 0x80 ) ? TRUE : FALSE;	// bit 7 (Reply Path)

	position++ ;

	/* TP-OA */

	memset(diallingNum, 0, sizeof(diallingNum));

	orgAddr_len = SmsUtilDecodeAddrField(diallingNum, &tpdu[position],&org_ton, &org_npi);

	position+=2;

	if ( orgAddr_len % 2 )
		position += orgAddr_len / 2 + 1;
	else
		position += orgAddr_len / 2;

	msg("pos = %d, length = %d",position, orgAddr_len);

	if ( org_ton == SMS_TON_INTERNATIONAL )
	{
		orgAddr[0] = '+';
		memcpy( &orgAddr[1], diallingNum, orgAddr_len );
		orgAddr[orgAddr_len+1] = '\0';
	}
	else
	{
		memcpy( orgAddr, diallingNum, orgAddr_len );
		orgAddr[orgAddr_len] = '\0';
	}

	/* TP-PID */
	tpdu_deliver.pId= tpdu[position];

	position ++;

	/* TP-DCS */
	SmsUtilDecodeDCS( &tpdu_deliver.dcs, tpdu[position] );
	dcs_pos = position;
	position++;

	/* TP-SCTS */
	SmsUtilDecodeTimeStamp( &tpdu[position], & SCTimeStamp );

	position+=7;

	/* TP-UDL */
	tpdu_deliver.udl = tpdu[position];

	if ( tpdu_deliver.udl> SMS_SMDATA_SIZE_MAX )
		tpdu_deliver.udl = SMS_SMDATA_SIZE_MAX;

	position++;

	/* TP-UD */
	tpdu_deliver.userData = malloc( sizeof ( unsigned char ) * ( SMS_SMDATA_SIZE_MAX + 1 ) );

	memset(tpdu_deliver.userData, 0, sizeof ( unsigned char ) * ( SMS_SMDATA_SIZE_MAX + 1 ) );

	if (tpdu_deliver.dcs.alphabet_type == SMS_ALPHABET_DEFAULT )
	{
		char	inData[SMS_SMDATA_SIZE_MAX+1];

		memset( inData, 0x00, SMS_SMDATA_SIZE_MAX + 1 );

		memcpy( (void*) inData, (void*) &tpdu[position], tpdu_deliver.udl );

		SmsUtilUnpackGSMCode(tpdu_deliver.userData, inData, (unsigned char) tpdu_deliver.udl);
	}

	msg("*************************************");
	msg("Msg Type: TPDU_DELIVER");

	if(tpdu_deliver.rp )
	{msg("RP: Set Reply Path")}
	else if(!tpdu_deliver.rp )
	{msg("RP: Not set Reply Path")}

	if(tpdu_deliver.sri)
	{msg("SRI: A status report shall be returned to the SME")}
	else if(!tpdu_deliver.sri)
	{msg("SRI: A status report shall be not returned to the SME")}

	if(tpdu_deliver.mms)
	{msg("MMS: No more messages are waiting for the MS in this SC")}
	else if(!tpdu_deliver.mms)
	{msg("MMS: More messages are waiting for the MS in this SC")}

	if(tpdu_deliver.udhi)
	{msg("UDHI: Contains a Header in addition to the short message.")}
	else	if(!tpdu_deliver.udhi)
	{msg("UDHI: Only short message.")}

	msg("DcsClass : %x (4 means Class None) ",tpdu_deliver.dcs.class_type);

	msg("From : %10s" ,orgAddr );
	msg("Time : %d-%d-%d  , %d:%d:%d",
			SCTimeStamp.year, SCTimeStamp.month, SCTimeStamp.day, SCTimeStamp.hour, SCTimeStamp.minute ,SCTimeStamp.second);
	msg("Message : %s " , tpdu_deliver.userData);
	msg("*************************************");


	return dcs_pos;
}

static int __decode_sms_status_report_tpdu(int tpdu_len, char* tpdu)
{
	TPDU_SMS_STATUS_REPORT status_report;


	TmDateTime   SCTimeStamp;
	TmDateTime   DischargeTime;

	int rcpAddr_len = 0;
	char diallingNum[SMS_ADDRESS_LEN_MAX];

	char rcpAddr[SMS_ADDRESS_LEN_MAX]; //recipient number
	int rcp_ton,rcp_npi;
	int position=0;
	unsigned int mask;

	/* TP-MTI, TP MMS, TP SRQ, TP_UDHI */

	status_report.msg_type = tpdu[position] & 0x03; // bit 0,1
	status_report.mms = tpdu[position] & 0x04 ? TRUE : FALSE ; // bit 2
	status_report.srq = tpdu[position] &  0x20 ? TRUE : FALSE ; //bit 5
	status_report.udhi = tpdu[position] & 0x40 ? TRUE : FALSE;	 //bit 6

	position ++;

	/* TP-MR */
	status_report.mr = tpdu[position];
	position ++;

	/* TP-RA */

	memset(diallingNum, 0, sizeof(diallingNum));

	SmsUtilDecodeAddrField(diallingNum, &tpdu[position],&rcp_ton, &rcp_npi);

	position+=2;

	rcpAddr_len = strlen( (char*) diallingNum );
	if ( rcpAddr_len % 2 )
		position += rcpAddr_len / 2 + 1;
	else
		position += rcpAddr_len / 2;


	if ( rcp_ton == SMS_TON_INTERNATIONAL )
	{
		rcpAddr[0] = '+';
		memcpy( &rcpAddr[1], diallingNum, rcpAddr_len );
		rcpAddr[rcpAddr_len+1] = '\0';
	}
	else
	{
		memcpy( rcpAddr, diallingNum, rcpAddr_len );
		rcpAddr[rcpAddr_len] = '\0';
	}

	status_report.rcpAddr.DialNumLen= rcpAddr_len;
	memcpy(status_report.rcpAddr.szDiallingNum, rcpAddr , rcpAddr_len);
	status_report.rcpAddr.Npi = rcp_npi;
	status_report.rcpAddr.Ton = rcp_ton;

	/* TP-SCTS */
	SmsUtilDecodeTimeStamp( &tpdu[position], & SCTimeStamp );
	position+=7;

	/* TP-DT */
	SmsUtilDecodeTimeStamp( &tpdu[position], & DischargeTime );
	position+=7;

	/* TP-ST */
	status_report.status = tpdu[position];
	position ++;

	/* TP-PI */
	status_report.paraInd = tpdu[position];

	mask = status_report.paraInd  ;

	if ( ( mask != 0xFF ) && ( mask != 0 ) )
	{
		/* Increment only if mask is valid */
		position ++;

		/* TP-PID */
		if ( mask & 0x01)
		{
			status_report.pId= tpdu[position];
			position ++;
		}
		/* TP-DCS */
		if ( mask & 0x02)
		{
			SmsUtilDecodeDCS( &status_report.dcs, tpdu[position] );
			position ++;
		}
		if ( mask & 0x04)
		{
			/* TP-UDL */
			status_report.udl = tpdu[position];
			position++;

			/* TP-UD */

			status_report.userData = malloc( sizeof ( unsigned char ) * ( SMS_SMDATA_SIZE_MAX + 1 ) );

			memset(status_report.userData, 0, sizeof ( unsigned char ) * ( SMS_SMDATA_SIZE_MAX + 1 ) );

			if (status_report.dcs.alphabet_type == SMS_ALPHABET_DEFAULT )
			{
				char	inData[SMS_SMDATA_SIZE_MAX+1];

				memset( inData, 0x00, SMS_SMDATA_SIZE_MAX + 1 );

				memcpy( (void*) inData, (void*) &tpdu[position], status_report.udl );

				SmsUtilUnpackGSMCode(status_report.userData, inData, (unsigned char) status_report.udl);
			}

		}
	}

	msg("*************************************");
	msg("Msg Type: STATUS_REPORT");
	if(status_report.mms)
	{msg("MMS: No more messages are waiting for the MS in this SC")}
	else if(!status_report.mms)
	{msg("MMS: More messages are waiting for the MS in this SC")}

	if(status_report.srq)
	{msg("SRQ: the result of an SMS COMMAND ")}
	else if(!status_report.srq)
	{msg("SRQ: the result of a SMS SUBMIT.")}

	if(status_report.udhi)
	{msg("UDHI: Contains a Header in addition to the short message.")}
	else if(!status_report.udhi)
	{msg("UDHI: Only Short Message")}

	msg("STATUS:%x",status_report.status);
	if(status_report.status==0x00)
		{msg("STATUS : Short message received by the SME")}
	else if(status_report.status==0x01)
	{msg("STATUS : Short message forwarded by the SC to the SMEbut the SC is unable to confirm delivery")}
	else if(status_report.status==0x02)
	{msg("STATUS : Short message replaced by the SC")}

	msg("Recipient Number : %s" ,rcpAddr );
	msg("SC Time Stamp : %d-%d-%d  , %d:%d:%d",
			SCTimeStamp.year, SCTimeStamp.month, SCTimeStamp.day, SCTimeStamp.hour, SCTimeStamp.minute ,SCTimeStamp.second);
	msg("Discharge Time : %d-%d-%d  , %d:%d:%d",
			DischargeTime.year, DischargeTime.month, DischargeTime.day, DischargeTime.hour, DischargeTime.minute ,DischargeTime.second);

	if(mask & 0x04)
		msg("Message : %s " , status_report.userData);

	msg("*************************************");

	return 1;
}

static gboolean __decode_cb_msg(int length, char * pPDU)
{
	int			msgLength;
	int			sn = 0; //serial number
	int			msgID; //message identifier
	Sms_coding_scheme  dcs;

	unsigned char	page[2];
	int			contents_length;
	char	 contents[SMS_CB_SIZE_MAX];

	unsigned char	gs;
	unsigned char	msgCode;
	unsigned char	updateNumber;

	if(pPDU == NULL){
		msg("pData is Null")
		return FALSE;
	}

	msg("cb msg type is %x",pPDU[0]);
	msg("cb msg len is %x",pPDU[1]);


	msg("$$$$$$$$$$$$$$$length of cb msg is %d",length);
	sn = (pPDU[2] <<8 ) |pPDU[1] ;

	gs = (pPDU[2] & 0xC0) >> 6;  //bit 7, bit 6

	msgCode = (( (pPDU[2] & 0x3F) <<4 ) | pPDU[3] ) & 0xF0 ;

	updateNumber = pPDU[3] & 0x0F;  //bit3, bit2, bit1, bit0

	msgID = pPDU[4] << 4 |pPDU[3];

	SmsUtilDecodeDCS( &dcs, pPDU[6] );

	page[0] = pPDU[7] & 0x0F; //total page
	page[1] = (pPDU[7] & 0xF0)>>4 ; //current page


	msgLength = length -2;

	contents_length = msgLength -6;

	if (dcs.alphabet_type == SMS_ALPHABET_DEFAULT )
	{
		unsigned char	inData[SMS_CB_SIZE_MAX+1];

		memset( inData, 0x00, SMS_CB_SIZE_MAX + 1 );

		memcpy( (void*) inData, (void*) &pPDU[8], contents_length);
		SmsUtilUnpackGSMCode(contents,  &pPDU[8], msgLength );
	}
	else
	{
		memcpy( contents, &pPDU[8], msgLength);
		contents[msgLength] = 0;
	}

	msg("*************************************\n");
	msg("serical number : 0x%04x \n" , sn);
	msg("Geographical Scope: 0x%x\n" , gs);
	msg("message code : 0x%x\n", msgCode);
	msg("update Number : 0x%x\n",updateNumber);
	msg("message identifier :  0x%x\n" , msgID);
	msg("DCS-Commpressed?: %d (1: true)\n" , dcs.bCompressed);
	msg("DCS-coding_group_type:  %x\n" , dcs.coding_group_type);
	msg("contents bytes size : %d\n", contents_length);
	msg("page : (%d /%d) (page /total ) \n", page[1], page[0] );
	msg("contents: %s\n", contents);
	msg("*************************************\n");

	return TRUE;
}

static void on_noti_sms_incom_msg(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	int                scaAddr_len =0;
	char		    * pTPDU;
	int		    tpdu_len =0;
	char		    diallingNum[TAPI_NETTEXT_ADDRESS_LEN_MAX];
	char             scaAddr[TAPI_NETTEXT_SCADDRESS_LEN_MAX]; //service center address
	int		    sca_ton, sca_npi;
	int		    position;

	unsigned char MTI = 0;
	unsigned char	MMS = 0;
	unsigned char SRI = 0;
	unsigned char UDHI = 0;
	unsigned char RP = 0;

	Sms_coding_scheme dcs;
	TelSmsDatapackageInfo_t * datapackage;
	TapiResult_t returnStatus;
	position=0;

	msg("");
	msgb("event(%s) receive !!", TAPI_NOTI_SMS_INCOM_MSG);

	if(data == NULL){
		msg("data is Null")
		return;
	}
	/*	for debug
		for( i=0;i<pdu_len ; i++)
		msg("[%02x]", pPDU[i]);
		*/

	datapackage = (TelSmsDatapackageInfo_t *)data;

	memset(diallingNum, 0, sizeof(diallingNum));

	SmsUtilDecodeAddrField(diallingNum, (char *)datapackage->Sca, &sca_ton, &sca_npi);

	scaAddr_len = strlen( diallingNum );

	if ( sca_ton == SMS_TON_INTERNATIONAL )
	{
		scaAddr[0] = '+';
		memcpy( &scaAddr[1], diallingNum, scaAddr_len );
		scaAddr[scaAddr_len+1] = '\0';
	}
	else
	{
		memcpy( scaAddr, diallingNum, scaAddr_len );
		scaAddr[scaAddr_len] = '\0';
	}

	msg("Sc address in test app is %s",scaAddr);

	RP = datapackage->szData[position] & 0x80;
	UDHI = datapackage->szData[position] & 0x40;
	SRI = datapackage->szData[position] & 0x20;
	MMS = datapackage->szData[position] & 0x04;
	MTI = datapackage->szData[position] & 0x03;

	msg("RP [%x]", RP);
	msg("UDHI [%x]", UDHI);
	msg("SRI [%x]", SRI);
	msg("MMS [%x]", MMS);
	msg("MTI [%02x]", MTI);

	tpdu_len = datapackage->MsgLength;

	pTPDU = malloc(sizeof(unsigned char) * tpdu_len);

	memcpy(pTPDU, &datapackage->szData[position], tpdu_len);

	if(MTI ==SMS_TPDU_DELIVER)
	{
		__decode_sms_deliver_tpdu(tpdu_len, pTPDU);

		msg("dcs_pos : %d", dcs_pos);

		SmsUtilDecodeDCS(&dcs, datapackage->szData[position+dcs_pos]);

		//if(dcs.class_type == TAPI_NETTEXT_CLASS_2)
		{
			msg("dcs type is of class2");
			TelSmsData_t WriteData;

			memset(&WriteData, 0, sizeof(TelSmsData_t));

			memcpy(WriteData.SmsData.Sca , datapackage->Sca, TAPI_SIM_SMSP_ADDRESS_LEN);

			WriteData.SmsData.MsgLength = datapackage->MsgLength;

			memcpy(WriteData.SmsData.szData, datapackage->szData, datapackage->MsgLength);

			WriteData.MsgStatus = TAPI_NETTEXT_STATUS_READ;

			returnStatus = tel_write_sms_in_sim(handle, &WriteData, on_resp_write_sms_in_sim, NULL);

			msg("returnStatus for tel_write_sms_in_sim(): 0x%x",returnStatus);
		}


	}
	else if(MTI==SMS_TPDU_STATUS_REPORT)
	{
		__decode_sms_status_report_tpdu(tpdu_len, pTPDU);
	}

	TelSmsDatapackageInfo_t * del_report= NULL;

	del_report= malloc(sizeof(TelSmsDatapackageInfo_t));

	memset(del_report, 0, sizeof(TelSmsDatapackageInfo_t));

	memcpy(del_report->Sca, datapackage->Sca, TAPI_SIM_SMSP_ADDRESS_LEN);

	del_report->szData[0] = SMS_TPDU_DELIVER_REPORT;	//TP-UDHI[bit6] : 0
	del_report->szData[1] = 0;	// TP-PI = 0; bit2: TP-UDL bit1: TP-DCS bit0: TP-PID (No option field)
	del_report->MsgLength = 2;

	msg("***receive message (Api:SendDeliverreport)****");


	returnStatus =tel_send_sms_deliver_report(handle, del_report, TAPI_NETTEXT_SENDSMS_SUCCESS, on_resp_sms_delivery_report, NULL);

	msg("returnStatus for tel_send_sms_deliver_report(): 0x%x",returnStatus);

	g_free(del_report);

	g_free(pTPDU);
}

static void on_noti_sms_cb_incom_msg(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelSmsCbMsg_t *cbMsg = data;
	int length = 0;
	char pPDU[SMS_CB_SIZE_MAX + 3] = {0,};

	msg("");
	msgb("event(%s) receive !!", TAPI_NOTI_SMS_CB_INCOM_MSG);

	length = cbMsg->Length;
	pPDU[0] = cbMsg->CbMsgType;
	pPDU[1] = length;
	memcpy(&(pPDU[2]), cbMsg->szMsgData, SMS_CB_SIZE_MAX + 1);

	__decode_cb_msg(length, pPDU);
}

static void on_noti_sms_etws_incom_msg(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	TelSmsEtwsMsg_t *etwsMsg = data;
	int length = 0;
	char pPDU[TAPI_NETTEXT_ETWS_SIZE_MAX + 3] = {0,};

	msg("");
	msgb("event(%s) receive !!", TAPI_NOTI_SMS_ETWS_INCOM_MSG);

	length = etwsMsg->Length;
	pPDU[0] = etwsMsg->EtwsMsgType;
	pPDU[1] = length;
	memcpy(&(pPDU[2]), etwsMsg->szMsgData, TAPI_NETTEXT_ETWS_SIZE_MAX + 1);
}

static void on_noti_sms_memory_status (TapiHandle *handle, const char *noti_id,
		void *data, void *user_data)
{
	gboolean *mem_status;
	msg("event [%s] received !!!", TAPI_NOTI_SMS_MEMORY_STATUS);

	if (!data) {
		msg("TAPI_NOTI_SMS_MEMORY_STATUS data is null");
		return;
	}

	mem_status = (gboolean *)data;
	msg("sms memory status[%s]", (*mem_status == TRUE) ? "Available" : "Not Available");
}

static void on_noti_sms_ready_status (TapiHandle *handle, const char *noti_id,
		void *data, void *user_data)
{
	gboolean *ready_status;

	msg("event [%s] received !!!", TAPI_NOTI_SMS_DEVICE_READY);

	if (!data) {
		msg("TAPI_NOTI_SMS_DEVICE_READY data is null");
		return;
	}

	ready_status = (gboolean *)data;
	msg("[%s]", (*ready_status == TRUE) ? "Device Ready" : "Device Not Ready");
}

/* Requests and Responses */
static void on_resp_write_sms_in_sim(TapiHandle *handle, int result, void *data, void *user_data)
{
	unsigned int *index;
	msg("tel_write_sms_in_sim() response received: [%d]", result);

	if (!data) {
		msg("data is null");
		return;
	}

	index = (unsigned int *)data;
	msg("index[%d]", *index);
}

static void on_resp_sms_delivery_report(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_send_deliver_report() response received: [%d]", result);
}

static int run_get_init_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	gboolean init_status;
	int rt = 0;

	msg("call tel_get_init_status()");

	rt = tel_check_sms_device_status(handle, &init_status);

	CHECK_RESULT(rt);

	msg("init status: [%s]", ((init_status == TRUE) ? "INITIALIZED" : "NOT INITIALIZED"));

	return 0;
}

static void on_set_msg_status(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_set_message_status() response received: [%d]", result);
}

static int run_set_msg_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelSmsMsgStatus_t sms_status_info;
	int index;
	int rt = 0;

	msg("call tel_set_message_status()");

	memset(&sms_status_info, 0, sizeof(TelSmsMsgStatus_t));

	index = atoi(data_sms_index);
	sms_status_info = atoi(data_sms_msg_status);

	msg("index[%d]", index);
	msg("msg status[%d]", sms_status_info);

	rt = tel_set_sms_message_status(handle, index, sms_status_info, on_set_msg_status, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_set_memory_status(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_set_memory_status() response received: [%d]", result);
}

static int run_set_memory_status(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int mem_status;
	int rt = 0;

	msg("call tel_set_memory_status()");

	mem_status = atoi(data_sms_memory_status);
	msg("mem_status: [%s]", (mem_status == 1) ? "AVAILABLE" : "NOT AVAILABLE");

	rt = tel_set_sms_memory_status(handle, mem_status, on_set_memory_status, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_get_sca(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSmsAddressInfo_t *sca;

	msg("tel_get_sca() response received: [%d]", result);

	if(!data) {
		msg("get sca data is null");
		return;
	}

	sca = (TelSmsAddressInfo_t *)data;

	msg("sca_ton[%d]", sca->Ton);
	msg("sca_npi[%d]", sca->Npi);
}

static int run_get_sca(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	int index = 1;

	msg("call tel_get_sca()");

	rt = tel_get_sms_sca(handle, index, on_get_sca, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_set_sca(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_set_sca() response received: [%d]", result);
}

static int run_set_sca(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelSmsAddressInfo_t sca;
	int rt = 0;
	int index = 1;

	msg("call tel_set_sca()");

	memset(&sca, 0, sizeof(TelSmsAddressInfo_t));

	sca.Ton= atoi(data_sms_sca_ton);
	sca.Npi= atoi(data_sms_sca_npi);
	g_strlcpy((gchar *)sca.szDiallingNum, data_sms_sca_number, TAPI_NETTEXT_SCADDRESS_LEN_MAX + 1);

	msg("sca_ton[%d]", sca.Ton);
	msg("sca_npi[%d]", sca.Npi);
	msg("sca_number[%s]", sca.szDiallingNum);

	rt = tel_set_sms_sca(handle, &sca, index, on_set_sca, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_get_parameters(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSmsParams_t *param_list;

	msg("tel_get_parameters() response received: [%d]", result);

	if (!data) {
		msg("get parameters data is null");
		return;
	}

	param_list = (TelSmsParams_t *)data;

	msg("index[%d]", param_list->RecordIndex);
	msg("sca_ton[%d]", param_list->TpSvcCntrAddr.Ton);
	msg("sca_npi[%d]", param_list->TpSvcCntrAddr.Npi);
	msg("sca_number[%s]", param_list->TpSvcCntrAddr.szDiallingNum);
	msg("validity period[%d]", param_list->TpValidityPeriod);
}

static int run_get_parameters(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;
	int index = 1;

	msg("call tel_get_parameters()");

	rt = tel_get_sms_parameters(handle, index, on_get_parameters, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_set_parameters(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_set_parameters() response received: [%d]", result);
}

static int run_set_parameters(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelSmsParams_t params;
	int rt = 0;

	msg("call tel_set_parameters()");

	memset(&params, 0, sizeof(TelSmsParams_t));

	params.RecordIndex= atoi(data_sms_set_params_index);
	params.TpSvcCntrAddr.Ton = atoi(data_sms_sca_ton);
	params.TpSvcCntrAddr.Npi = atoi(data_sms_sca_npi);
	g_strlcpy((gchar *)params.TpSvcCntrAddr.szDiallingNum, data_sms_sca_number, TAPI_NETTEXT_SCADDRESS_LEN_MAX + 1);
	params.TpValidityPeriod = atoi(data_sms_set_params_vp);

	msg("index[%d]", params.RecordIndex);
	msg("sca_ton[%d]", params.TpSvcCntrAddr.Ton);
	msg("sca_npi[%d]", params.TpSvcCntrAddr.Npi);
	msg("sca_num[%s]", params.TpSvcCntrAddr.szDiallingNum);
	msg("validity period[%d]", params.TpValidityPeriod);

	rt = tel_set_sms_parameters(handle, &params, on_set_parameters, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_get_cb_config(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSmsCbConfig_t *config;
	int i = 0;

	msg("tel_get_cb_config() response received: [%d]", result);

	if (!data) {
		msg("get cb config data is null");
		return;
	}

	config = (TelSmsCbConfig_t *)data;

	msg("ebabled[%s]", (config->CBEnabled) ? "cb enabled" : "cb disabled");
	msg("msg_id range count[%d]", config->MsgIdRangeCount);

	for(i = 0; i < config->MsgIdRangeCount; i++)
		msg("[%d].from msg_id[%d] to msg_id[%d] selected[%s]", i+1,
		config->MsgIDs[i].Net3gpp.FromMsgId, config->MsgIDs[i].Net3gpp.ToMsgId,
		(config->MsgIDs[i].Net3gpp.Selected) ? "YES" : "NO");
}

static int run_get_cb_config(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call tel_get_cb_config()");

	rt = tel_get_sms_cb_config(handle, on_get_cb_config, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_set_cb_config(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_set_cb_config() response received: [%d]", result);
}

static int run_set_cb_config(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	TelSmsCbConfig_t cb_config;
	int rt = 0;
	int ret = 0;
	int i = 0;

	msg("call tel_set_cb_config()");

	memset(&cb_config, 0, sizeof(TelSmsCbConfig_t));

	cb_config.CBEnabled= atoi(data_sms_set_cb_status);
	cb_config.MsgIdRangeCount= atoi(data_sms_set_cb_msg_id_rng_cnt);

	for(i = 0; i < cb_config.MsgIdRangeCount; i++) {
		msg("[%d].Enter from_msg_id", i+1);
		ret = read(0, data_sms_set_cb_from_msg_id, sizeof(data_sms_set_cb_from_msg_id));
		if (ret <= 0) {
			msg("read 'from_msg_id' returned error: [%d]", ret);
			return -1;
		}
		cb_config.MsgIDs[i].Net3gpp.FromMsgId= atoi(data_sms_set_cb_from_msg_id);

		msg("[%d].Enter to_msg_id", i+1);
		ret = read(0, data_sms_set_cb_to_msg_id, sizeof(data_sms_set_cb_to_msg_id));
		if (ret <= 0) {
			msg("read 'to_msg_id' returned error: [%d]", ret);
			return -1;
		}
		cb_config.MsgIDs[i].Net3gpp.ToMsgId= atoi(data_sms_set_cb_to_msg_id);

		msg("[%d].Enter msg_id selected", i+1);
		ret = read(0, data_sms_set_cb_msg_id_selected, sizeof(data_sms_set_cb_msg_id_selected));
		if (ret <= 0) {
			msg("read 'selected' returned error: [%d]", ret);
			return -1;
		}
		cb_config.MsgIDs[i].Net3gpp.Selected= atoi(data_sms_set_cb_msg_id_selected);
	}

	msg("cb_status[%s]", (cb_config.CBEnabled) ? "cb enabled" : "cb disabled");
	msg("msg_id range count[%d]", cb_config.MsgIdRangeCount);

	for(i = 0; i < cb_config.MsgIdRangeCount; i++)
		msg("[%d].from msg_id[%d] to msg_id[%d] selected[%s]", i+1,
		cb_config.MsgIDs[i].Net3gpp.FromMsgId, cb_config.MsgIDs[i].Net3gpp.ToMsgId,
		(cb_config.MsgIDs[i].Net3gpp.Selected) ? "YES" : "NO");

	rt = tel_set_sms_cb_config(handle, &cb_config, on_set_cb_config, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_get_msg_count(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSmsStoredMsgCountInfo_t *sms_count_info;
	int i = 0;

	msg("tel_get_count() response received: [%d]", result);

	if (!data) {
		msg("get msg count data is null");
		return;
	}

	sms_count_info = (TelSmsStoredMsgCountInfo_t *)data;

	msg("total count[%d]", sms_count_info->TotalCount);
	msg("used count [%d]", sms_count_info->UsedCount);

	msg("Index List :");
	for(i = 0; i < sms_count_info->UsedCount; i++)
		msg("[%d]", sms_count_info->IndexList[i]);

}

static int run_get_msg_count(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int rt = 0;

	msg("call tel_get_count()");

	rt = tel_get_sms_count(handle, on_get_msg_count, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_delete_sms(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_delete_in_sim() response received: [%d]", result);
}

static int run_delete_sms(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	unsigned int index;
	int rt = 0;

	msg("call tel_delete_sms_in_sim()");

	index = atoi(data_sms_index);
	msg("index[%d]", index);

	rt = tel_delete_sms_in_sim(handle, index, on_delete_sms, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_read_msg_in_sim(TapiHandle *handle, int result, void *data, void *user_data)
{
	TelSmsData_t *read_sms_data;

	msg("tel_read_sms_in_sim() response received: [%d]", result);

	if (!data) {
		msg("read sms in sim data is null");
		return;
	}

	read_sms_data = (TelSmsData_t *)data;

	msg("index[%d]", read_sms_data->SimIndex);
	msg("status[%d]", read_sms_data->MsgStatus);
	msg("sca[%s]", read_sms_data->SmsData.Sca);

	__decode_sms_deliver_tpdu(read_sms_data->SmsData.MsgLength, (char *)read_sms_data->SmsData.szData);
}

static int run_read_msg_in_sim(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	unsigned int index;
	int rt = 0;

	msg("call tel_read_sms_in_sim()");

	index = atoi(data_sms_index);
	msg("index[%d]", index);

	rt = tel_read_sms_in_sim(handle, index, on_read_msg_in_sim, NULL);

	CHECK_RESULT(rt);

	return 0;
}

static void on_send_sms(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("tel_send() response received: [%d]", result);
}

static int run_send_sms(MManager *mm, struct menu_data *menu)
{
	char dialling_num[SMS_ADDRESS_LEN_MAX + 1];
	char message[512];
	unsigned int dial_num_len = 0, msg_len = 0;

	msg("call tel_send()");

	/* dialling number */
	if (g_strlcpy(dialling_num, data_sms_send_msg_dialling_num,
			SMS_ADDRESS_LEN_MAX + 1) > SMS_ADDRESS_LEN_MAX) {
		msg("Invalid dialling num length entered");
		return 0;
	}
	dial_num_len = strlen(dialling_num);

	/* message */
	g_strlcpy(message, data_sms_send_msg, strlen(data_sms_send_msg)+1);
	msg_len = strlen(message);

	msg("To: [%s]", dialling_num);
	msg("Message: [%s]", message);
	msg("dial num length[%d] msg_len[%d]", dial_num_len, msg_len);

	__encode_and_send_sms_submit_tpdu (mm, dialling_num, dial_num_len, message, msg_len);

	return 0;
}

/* SubMenu */
static struct menu_data menu_sms_send_sms[] = {
	{ "1", "more messages (0=No 1=Yes)", NULL, NULL, data_sms_send_more_msgs },
	{ "2", "sca ton (0=Unknown 1=International 2=National)", NULL, NULL, data_sms_sca_ton },
	{ "3", "sca npi (0=Unknown 1=Telephony)", NULL, NULL, data_sms_sca_npi },
	{ "4", "sca number", NULL, NULL, data_sms_sca_number },
	{ "5", "message", NULL, NULL, data_sms_send_msg},
	{ "6", "dialling number (<= 20)", NULL, NULL, data_sms_send_msg_dialling_num},
	{ "7", "run", NULL, run_send_sms, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_read_msg_in_sim[] = {
	{ "1", "index", NULL, NULL, data_sms_index },
	{ "2", "run", NULL, run_read_msg_in_sim, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_delete_msg_in_sim[] = {
	{ "1", "index", NULL, NULL, data_sms_index },
	{ "2", "run", NULL, run_delete_sms, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_get_msg_count[] = {
	{ "1", "run", NULL, run_get_msg_count, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_set_cb_config[] = {
	{ "1", "cb status (0=disabled 1=enabled)", NULL, NULL, data_sms_set_cb_status },
	{ "2", "msg id range count ( < 50)", NULL, NULL, data_sms_set_cb_msg_id_rng_cnt },
	{ "3", "run", NULL, run_set_cb_config, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_get_cb_config[] = {
	{ "1", "run", NULL, run_get_cb_config, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_set_parameters[] = {
	{ "1", "index", NULL, NULL, data_sms_set_params_index },
	{ "2", "sca ton(0=Unknown 1=International 2=National)", NULL, NULL, data_sms_sca_ton },
	{ "3", "sca npi (0=Unknown 1=Telephony)", NULL, NULL, data_sms_sca_npi },
	{ "4", "sca number", NULL, NULL, data_sms_sca_number },
	{ "5", "validity period", NULL, NULL, data_sms_set_params_vp },
	{ "6", "run", NULL, run_set_parameters, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_get_parameters[] = {
	{ "1", "run", NULL, run_get_parameters, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_set_sca[] = {
	{ "1", "sca ton(0=Unknown 1=International 2=National)", NULL, NULL, data_sms_sca_ton },
	{ "2", "sca npi (0=Unknown 1=Telephony)", NULL, NULL, data_sms_sca_npi },
	{ "3", "sca number", NULL, NULL, data_sms_sca_number },
	{ "4", "run", NULL, run_set_sca, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_get_sca[] = {
	{ "1", "run", NULL, run_get_sca, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_set_memory_status[] = {
	{ "1", "status (1:Memory Available, 2:Memory Full)", NULL, NULL, data_sms_memory_status },
	{ "2", "run", NULL, run_set_memory_status, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_set_msg_status[] = {
	{ "1", "index", NULL, NULL, data_sms_index },
	{ "2", "status (0=MT_READ 1=MT_UNREAD 2=MO_SENT 3=MO_NOT_SENT 4=MO_DELIVERED" \
		"5=MO_DELIVERY_NOT_CONFIRMED 6=REPLACED )", NULL, NULL, data_sms_msg_status },
	{ "3", "run", NULL, run_set_msg_status, NULL },
	{ NULL, NULL, },
};

static struct menu_data menu_sms_get_init_status[] = {
	{ "1", "run", NULL, run_get_init_status, NULL },
	{ NULL, NULL, },
};

/* Menu */
struct menu_data menu_sms[] = {
	{ "1", "Send SMS", menu_sms_send_sms, NULL, NULL},
	{ "2", "Read SMS in SIM", menu_sms_read_msg_in_sim, NULL, NULL},
	{ "3", "Delete SMS in SIM", menu_sms_delete_msg_in_sim, NULL, NULL},
	{ "4", "Get SMS count", menu_sms_get_msg_count, NULL, NULL},
	{ "5", "Set SMS cell broadcast configuration", menu_sms_set_cb_config, NULL, NULL},
	{ "6", "Get SMS cell broadcast configuration", menu_sms_get_cb_config, NULL, NULL},
	{ "7", "Set SMS parameters", menu_sms_set_parameters, NULL, NULL},
	{ "8", "Get SMS parameters", menu_sms_get_parameters, NULL, NULL},
	{ "9", "Set SCA", menu_sms_set_sca, NULL, NULL},
	{ "10", "Get SCA", menu_sms_get_sca, NULL, NULL},
	{ "11", "Set SMS memory status", menu_sms_set_memory_status, NULL, NULL},
	{ "12", "Set SMS msg status", menu_sms_set_msg_status, NULL, NULL},
	{ "13", "Get SMS init status", menu_sms_get_init_status, NULL, NULL},
	{ NULL, NULL, },
};

void register_sms_event(TapiHandle *handle)
{
	int ret;

	ret = tel_register_noti_event(handle, TAPI_NOTI_SMS_INCOM_MSG, on_noti_sms_incom_msg, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SMS_INCOM_MSG - Event register failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_SMS_CB_INCOM_MSG, on_noti_sms_cb_incom_msg, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SMS_CB_INCOM_MSG - Event register failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_SMS_ETWS_INCOM_MSG, on_noti_sms_etws_incom_msg, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SMS_ETWS_INCOM_MSG - Event register failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_SMS_MEMORY_STATUS, on_noti_sms_memory_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SMS_MEMORY_STATUS - Event register failed: [%d]", ret);

	ret = tel_register_noti_event(handle, TAPI_NOTI_SMS_DEVICE_READY, on_noti_sms_ready_status, NULL);
	if (ret != TAPI_API_SUCCESS)
		msg("TAPI_NOTI_SMS_DEVICE_READY - Event register failed: [%d]", ret);
}
