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
#include <ITapiGps.h>

//#include <TelGps.h>
#include <TapiUtility.h>

#include "menu.h"
#include "gps.h"

/*                                    GPS Chipset on AP - START                                      */
//GPS CP MO Location

/* MOLR Type */
typedef enum {
	TAPI_GPS_MOLR_INVALID,
	TAPI_GPS_MOLR_LOCATION_ESTIMATE,
	TAPI_GPS_MOLR_ASSISTANCE_DATA,
	TAPI_GPS_MOLR_DECIPHERING_KEY
} tapi_gps_molr_e_type;

/* Response Time Type */
typedef enum {
	TAPI_GPS_RESPONSE_TIME_INVALID,
	TAPI_GPS_RESPONSE_TIME_LOW_DELAY,
	TAPI_GPS_RESPONSE_TIME_DELAY_TOLERANT
} tapi_gps_response_time;

/* Location Method Type */
typedef enum {
	TAPI_GPS_LOCATION_METHOD_INVALID,
	TAPI_GPS_LOCATION_METHOD_MSBASED_EOTD,
	TAPI_GPS_LOCATION_METHOD_MSASSISTED_EOTD,
	TAPI_GPS_LOCATION_METHOD_ASSISTED_GPS
} tapi_gps_location_method_e_type;

/* GAD Shape Type */
typedef enum {
	TAPI_GPS_SHAPE_ELLIPSOIDPOINT,
	TAPI_GPS_SHAPE_ELLIPSOIDPOINT_WITH_UNCERTAINTY_CIRCLE,
	TAPI_GPS_SHAPE_ELLIPSOIDPOINT_WITH_UNCERTAINTY_ELLIPSE,
	TAPI_GPS_SHAPE_POLYGON,
	TAPI_GPS_SHAPE_ELLIPSOIDPOINT_WITH_ALTITUDE,
	TAPI_GPS_SHAPE_ELLIPSOIDPOINT_WITH_ALTITUDE_AND_UNCERTAINTYELI,
	TAPI_GPS_SHAPE_ELLIPSOIDPOINT_ARC
} tapi_gps_gad_shape_e_type;

/* GPS Assistance Data Flags */
#define GPS_ASSISTANCE_NONE                     0x00000000
#define GPS_ASSISTANCE_ALMANAC                  0x00000001
#define GPS_ASSISTANCE_UTC_MODEL                0x00000002
#define GPS_ASSISTANCE_IONO_MODEL               0x00000004
#define GPS_ASSISTANCE_NAVI_MODEL               0x00000008
#define GPS_ASSISTANCE_DGPS_CORRECTION          0x00000010
#define GPS_ASSISTANCE_REF_LOCATION             0x00000020
#define GPS_ASSISTANCE_REF_TIME                 0x00000040
#define GPS_ASSISTANCE_ACQUISITION_ASSISTANCE   0x00000080
#define GPS_ASSISTANCE_REAL_TIME_INTEGRITY      0x00000100

//GPS Assist Data Message

/* GPS Crrier Type */
typedef enum {
	TAPI_GPS_CARRIER_INVALID,
	TAPI_GPS_CARRIER_GSM,
	TAPI_GPS_CARRIER_UMTS
} tapi_gps_carrier_e_type;

/* Utran Sfn Uncertainty */
typedef enum {
	TAPI_GPS_UTRAN_SFN_UNCERTAINTY_INVALID,
	TAPI_GPS_UTRAN_SFN_UNCERTAINTY_LESSTHAN_10,
	TAPI_GPS_UTRAN_SFN_UNCERTAINTY_MORETHAN_10
} tapi_gps_sfn_unc_e_type;

/* UTRAN Choice mode Type */
typedef enum {
	TAPI_GPS_UTRAN_CHOICE_INVALID,
	TAPI_GPS_UTRAN_CHOICE_FDD,
	TAPI_GPS_UTRAN_CHOICE_TDD
} tapi_gps_utrn_choice_mode_e_type;

/* DGPS Status Type */
typedef enum {
	TAPI_GPS_DGPS_INVALID,
	TAPI_GPS_DGPS_UDRE_SCALE_1_0,
	TAPI_GPS_DGPS_UDRE_SCALE_0_75,
	TAPI_GPS_DGPS_UDRE_SCALE_0_5,
	TAPI_GPS_DGPS_UDRE_SCALE_0_3,
	TAPI_GPS_DGPS_UDRE_SCALE_0_2,
	TAPI_GPS_DGPS_UDRE_SCALE_0_1,
	TAPI_GPS_DGPS_NO_DATA
} tapi_gps_dgps_status_e_type;

/* Navigation Satellite Status Type*/
typedef enum {
	TAPI_GPS_NAVIGATION_MODEL_NEW_SATELLITE_NEW_NAVIGATION,
	TAPI_GPS_NAVIGATION_MODEL_EXIST_SATELLITE_EXIST_NAVIGATION,
	TAPI_GPS_NAVIGATION_MODEL_EXIST_SATELLITE_NEW_NAVIGATION,
	TAPI_GPS_NAVIGATION_MODEL_RESERVED
} tapi_gps_navigation_sat_status_e_type;

//GPS Measure Position Message
/* Method Type */
typedef enum {
	TAPI_GPS_METHODTYPE_INVALID,
	TAPI_GPS_METHODTYPE_MS_ASSISTED,
	TAPI_GPS_METHODTYPE_MS_BASED,
	TAPI_GPS_METHODTYPE_MS_BASED_PREF,
	TAPI_GPS_METHODTYPE_MS_ASSISTED_PREF
} tapi_gps_method_e_type;

/* Use Multiple Sets Type */
typedef enum {
	TAPI_GPS_MULTIPLESETS_INVALID,
	TAPI_GPS_MULTIPLESETS_MULTIPLESETS,
	TAPI_GPS_MULTIPLESETS_ONESET
} tapi_gps_use_multi_sets_e_type;

/* Environment Char Type */
typedef enum {
	TAPI_GPS_ENVIRONMENT_INVALID,
	TAPI_GPS_ENVIRONMENT_BAD_AREA,
	TAPI_GPS_ENVIRONMENT_NOT_BAD_AREA,
	TAPI_GPS_ENVIRONMENT_MIXED_AREA
} tapi_gps_env_char_e_type;

/* Cell Timing Wanted Type */
typedef enum {
	TAPI_GPS_CELLTIMING_INVALID,
	TAPI_GPS_CELLTIMING_WANTED,
	TAPI_GPS_CELLTIMING_NOT_WANTED
} tapi_gps_cell_timing_wnt_e_type;

/* Additional Assist Request Type */
typedef enum {
	TAPI_GPS_ADDITIONAL_ASSISREQ_INVALID,
	TAPI_GPS_ADDITIONAL_ASSISREQ_REQ,
	TAPI_GPS_ADDITIONAL_ASSISREQ_NOT_REQ
} tapi_gps_add_assit_req_e_type;

/* Measure Position Response Type */
typedef enum {
	TAPI_GPS_MSR_POS_RES_LOCATION,
	TAPI_GPS_MSR_POS_RES_GPS_MEASUREMENTS,
	TAPI_GPS_MSR_POS_RES_AID_REQ,
	TAPI_GPS_MSR_POS_RES_ERROR
} tapi_gps_msr_pos_res_e_type;

/* GPS element multi path Type */
typedef enum {
	TAPI_GPS_MULTIPATH_NOT_MEASURED,
	TAPI_GPS_MULTIPATH_LOW,
	TAPI_GPS_MULTIPATH_MEDIUM,
	TAPI_GPS_MULTIPATH_HIGH
}tapi_gps_multi_path_e_type;

/* Accuracy Flag Value */
#define QOS_PRESENT_HORIZONTAL_ACCURACY 0x01
#define QOS_PRESENT_VERTICAL_ACCURACY   0x02

//GPS MTLR Notification Message
/* MTLR Notification Type */
typedef enum {
	TAPI_GPS_NO_NOTIFY_NO_VERIFY,
	TAPI_GPS_USER_NOTIFY_ONLY,     /* Location Notification Allowed */
	TAPI_GPS_USER_NOTIFY_VERIFY_ALLOW_NO_RESP, /* notify and verify, if no response, Location Notification Allowed */
	TAPI_GPS_USER_NOTIFY_VERIFY_NOT_ALLOW_NO_RESP, /* notify and verify, if no response, Location Notification Not Allowed */
	TAPI_GPS_PRIVACY_NEEDED,
	TAPI_GPS_PRIVACY_OVERRIDE
} tapi_gps_mtlr_notify_e_type;

/* location estimate Type */
typedef enum {
	TAPI_GPS_ESTIMATE_TYPE_CURRENT_LOCATION,	///< Current Location
	TAPI_GPS_ESTIMATE_TYPE_CURRENT_OR_LAST_KNOWN_LOCATION,	///< Current or Last Known Location
	TAPI_GPS_ESTIMATE_TYPE_INITIAL_LOCATION,		///< Initial Location
	TAPI_GPS_ESTIMATE_TYPE_ACTIVATE_DEFERRED_LOC,	///< Activate Deferred Location
	TAPI_GPS_ESTIMATE_TYPE_CANCEL_DEFERRED_LOC,	///< Cancel Deferred Location
	TAPI_GPS_ESTIMATE_TYPE_NOTIFY_VERIFY_ONLY,		///< Notification and Verification Only
	TAPI_GPS_ESTIMATE_TYPE_INVALID
} tapi_gps_loc_estimate_e_type;

/* ID Format Type */
typedef enum {
	TAPI_GPS_FORMAT_IND_LOGICAL_NAME,
	TAPI_GPS_FORMAT_IND_EMAIL_ADDRESS,
	TAPI_GPS_FORMAT_IND_MSISDN,
	TAPI_GPS_FORMAT_IND_URL,
	TAPI_GPS_FORMAT_IND_SIPURL,
	TAPI_GPS_FORMAT_IND_IMS_PUBLIC_ID
} tapi_gps_format_ind_e_type;

/* Verify Response Type */
typedef enum {
	TAPI_GPS_VERIFY_RSP_TYPE_DENIED,
	TAPI_GPS_VERIFY_RSP_TYPE_GRANTED,
	TAPI_GPS_VERIFY_RSP_TYPE_INVALID
} tapi_gps_verify_rsp_e_type;

typedef enum {
	TAPI_GPS_FREQ_AIDING_DISABLE = 0,
	TAPI_GPS_FREQ_AIDING_ENABLE,
} tapi_gps_freq_aiding_e_type;

/*                                     GPS Chipset on AP - END                                       */


/*                                    GPS Chipset on AP - START                                      */
typedef struct {
	unsigned long int qosFlag;
	unsigned char horizontalAccuracy;
	unsigned char verticalCoordinateRequest;
	unsigned char verticalAccuracy;
	unsigned char responseTime; //0x01:low delay, 0x02: delay tolerant
}__attribute__ ((packed)) tapi_gps_qos_t;

typedef struct {
	unsigned char sat_id;
	unsigned char iode;
}__attribute__ ((packed)) tapi_gps_sat_info_t;

typedef struct {
	unsigned char beginWeek;
	unsigned char endWeek;
	unsigned char beginTow;
	unsigned char endTow;
}__attribute__ ((packed)) tapi_gps_ext_ephe_chk_t;

typedef struct {
	unsigned long int assistanceFlag;
	unsigned short gpsWeek;
	unsigned char gpsToe;
	unsigned char nSat;
	unsigned char toeLimit;
	tapi_gps_sat_info_t satInfo[15];
	unsigned char gpsExtendedEphemeris;
	tapi_gps_ext_ephe_chk_t extEphemerisChk;
}__attribute__ ((packed)) tapi_gps_assistance_data_t;

typedef struct {
	tapi_gps_molr_e_type molr_type;
	tapi_gps_location_method_e_type location_method;
	tapi_gps_qos_t qos;
	unsigned char client_id[82];
	unsigned char mlc_num[82];
	tapi_gps_assistance_data_t assistance_data;
	tapi_gps_gad_shape_e_type gad_shape;
	unsigned char serviceTypeID;
	unsigned char pseudonymIndicator;
}__attribute__ ((packed)) tapi_gps_cp_mo_loc_set_t; //APGPS - Control Plane Mo Location - Set

//Measure Position message
typedef struct {
	unsigned char satId; //Satellite ID
	unsigned char cno; // 0~63, unit of dB-Hz
	signed short doppler; // -32768~32767, Hz and scale factor 0.2
	unsigned short wholeChips; // 0~1022
	unsigned short fracChips; // 0~1024
	unsigned char lcsMultiPath;
	unsigned char pseuRangeRmsErr; // 0~63
}__attribute__ ((packed)) tapi_gps_measuremet_element_t;

typedef struct {
	unsigned long int gpsTow; ///< GPS time of week [msec]
	unsigned short gpsWeek; ///< GPS week [0 .. 1023]
	unsigned char nrOfSats; ///< number of satellites [1 .. 16]
	tapi_gps_measuremet_element_t GpsMeasure[16];
}__attribute__ ((packed)) tapi_gps_measure_t;

typedef struct {
	signed long int latitude;
	signed long int longitude;
}__attribute__ ((packed)) tapi_gps_ellipsoid_po_t;

typedef struct {
	tapi_gps_ellipsoid_po_t point;
	unsigned char uncertainRadius;
}__attribute__ ((packed)) tapi_gps_po_unc_circle_t;

typedef struct {
	tapi_gps_ellipsoid_po_t point;
	unsigned char semiMajorAxis;
	unsigned char semiMinorAxis;
	unsigned char orientationAngle;
	unsigned char confidence;
}__attribute__ ((packed)) tapi_gps_po_unc_ellipse_t;

typedef struct {
	tapi_gps_ellipsoid_po_t point;
	signed short altitude;
	unsigned char semiMajorAxis;
	unsigned char semiMinorAxis;
	unsigned char orientationAngle;
	unsigned char uncertainAltitude;
	unsigned char confidence;
}__attribute__ ((packed)) tapi_gps_po_alt_unc_ellipse_t;

typedef struct {
	tapi_gps_ellipsoid_po_t point;
	unsigned short innerRadius;
	unsigned char uncertainRadius;
	unsigned char offsetAngle;
	unsigned char includedAngle;
	unsigned char confidence;
}__attribute__ ((packed)) tapi_gps_ellipsoid_arc_t;

typedef struct {
	tapi_gps_ellipsoid_po_t point;
	signed short altitude;
}__attribute__ ((packed)) tapi_gps_ellipsoid_alt_t;

typedef struct {
	unsigned char noOfPoints;
	tapi_gps_ellipsoid_po_t points[15];
}__attribute__ ((packed)) tapi_gps_polygon_t;

typedef struct {
	unsigned char shape_type;
	tapi_gps_po_unc_circle_t p_unc_clrcle;
	tapi_gps_po_unc_ellipse_t p_unc_ellipse;
	tapi_gps_po_alt_unc_ellipse_t p_alt_unc_ellipse;
	tapi_gps_ellipsoid_arc_t ellipsoid_arc;
	tapi_gps_ellipsoid_po_t ellipsoid_po;
	tapi_gps_ellipsoid_alt_t ellipsoid_alt;
	tapi_gps_polygon_t polygon;
}__attribute__ ((packed)) tapi_gps_loc_info_t;

typedef struct {
	unsigned long int gpsTow; ///< GPS time of week [msec]
	unsigned short gpsWeek; ///< GPS week [0 .. 1023]
	unsigned char fixType; ///< Fix type. 2D(0x01) or 3D(0x02)
	tapi_gps_loc_info_t measured_loc_info;
}__attribute__ ((packed)) tapi_gps_measure_loc_info_t;

typedef struct {
	unsigned char valid;
	unsigned long int cellFrames;
	unsigned char choice_mode;
	unsigned long int UtranFdd; //FDD Primary Scrambling Code
	unsigned long int UtranTdd; // TDD Cell Parameter ID
	unsigned long int sfn; //SFN
}__attribute__ ((packed)) tapi_gps_utran_gps_ref_time_t;

typedef struct {
	unsigned char result; // 0x00 : SUCCESS, 0x01 : Fail
	tapi_gps_msr_pos_res_e_type response_type; //should be 4 byte
	tapi_gps_measure_t gps_measure;
	tapi_gps_measure_loc_info_t loc_info;
	tapi_gps_assistance_data_t measured_assit_data;
	tapi_gps_utran_gps_ref_time_t UtranGpsRefTime; // only for 3G
}__attribute__ ((packed)) tapi_gps_measure_position_confirm_t; //APGPS - Measure Position message - confirm

typedef struct {
	unsigned char reqId;
	tapi_gps_verify_rsp_e_type response;
}__attribute__ ((packed)) tapi_gps_mtlr_notification_confirm_t; //AGPS - MTLR Notification message - confirm


//RX part
typedef struct {
	unsigned short arfcn; //ARFCN of GSM network
	unsigned char bsic; //Base Station Identity Code
	unsigned char rx_lev; // RX signal level
	unsigned char ta; //Timing Advance
}__attribute__ ((packed)) tapi_gps_gsm_extended_radio_signal_info_t; //APGPS - GSM Extended Radio Signal Info - Resp

typedef struct {
	unsigned long int flag;
	unsigned char horizontalAccuracy;
	unsigned char vertcalAccuracy;
}__attribute__ ((packed)) tapi_gps_accuracy_t;

typedef struct {
	unsigned char method_type;
	tapi_gps_accuracy_t accuracy;
	unsigned char rsp_time;
	unsigned char use_multi_sets;
	unsigned char environment_char;
	unsigned char cell_timing_wnt;
	unsigned char add_assist_req;
}__attribute__ ((packed)) tapi_gps_measure_position_indi_t; //APGPS - Measure Position message -indication

typedef struct {
	unsigned short deferredLocEventType;
	tapi_gps_mtlr_notify_e_type locEstimateType;
}__attribute__ ((packed)) tapi_gps_mtlr_loc_t;

typedef struct {
	unsigned long int length; //Length of APDU
	unsigned char val[63]; //APDU data
}__attribute__ ((packed)) tapi_gps_string_t;

typedef struct {
	unsigned char dcs;
	tapi_gps_string_t str;
	tapi_gps_format_ind_e_type format_indicator;
}__attribute__ ((packed)) tapi_gps_dcs_string_t;

typedef struct {
	unsigned char dcs;
	tapi_gps_string_t str;
}__attribute__ ((packed)) tapi_gps_code_word_t;

typedef struct {
	unsigned char req_id;
	tapi_gps_mtlr_notify_e_type notify_type;
	tapi_gps_mtlr_loc_t loc;
	unsigned char client_id[82];
	tapi_gps_dcs_string_t client_name;
	tapi_gps_dcs_string_t requestor_id;
	tapi_gps_code_word_t code_word;
	unsigned char svc_type_id;
}__attribute__ ((packed)) tapi_gps_mtlr_notification_indi_t; //AGPS - MTLR Notification message - Indication

typedef struct {
	unsigned char cipherKeyFlag;
	unsigned char currentDecipherKey[7];
	unsigned char nextDecipherKey[7];
}__attribute__ ((packed)) tapi_gps_deciphering_keys_t;

typedef struct {
	tapi_gps_loc_info_t loc_info;
	unsigned char no_loc;
	tapi_gps_deciphering_keys_t decper_keys;
}__attribute__ ((packed)) tapi_gps_cp_mo_loc_noti_t; //APGPS - Control Plane Mo Location - Notification

typedef struct {
	unsigned char valid;
	unsigned short bcchCarrier;
	unsigned short bsic;
	unsigned long int frameNumber;
	unsigned short timeSlot;
	unsigned short bitNumber;
}__attribute__ ((packed)) tapi_gps_gsm_time_t;

typedef struct {
	unsigned char valid;
	unsigned long int gpsTimeUncertainty;
}__attribute__ ((packed)) tapi_gps_utran_gps_unc_t;

typedef struct {
	unsigned char valid;
	signed long int driftRate;
}__attribute__ ((packed)) tapi_gps_drift_rate_t;

typedef struct {
	tapi_gps_utran_gps_ref_time_t UtranGpsRefTime;
	tapi_gps_utran_gps_unc_t UtranGpsUncertainty;
	unsigned char UtranSfnUncertainty;
	tapi_gps_drift_rate_t UtranDriftRate;
}__attribute__ ((packed)) tapi_gps_utran_time_t;

typedef struct {
	unsigned short satID;
	unsigned short tlmWord;
	unsigned char antiSpoofFlag;
	unsigned char alertFlag;
	unsigned char tmlReservedBits;
}__attribute__ ((packed)) tapi_gps_gps_tow_assist_t;

typedef struct {
	unsigned long int gpsTow;
	unsigned long int gpsWeek;
	unsigned char nrOfSats;
	union {
		tapi_gps_gsm_time_t gsm_time;
		tapi_gps_utran_time_t UtranTime;
	} networkTimeInfo;
	tapi_gps_gps_tow_assist_t GpsTowAssist[12];
}__attribute__ ((packed)) tapi_gps_ref_time_t;

typedef struct {
	unsigned char shapeType;
	unsigned char hemisphere;
	unsigned short altitude;
	unsigned long int latitude;
	signed long int longitude;
	unsigned char directionOfAlt;
	unsigned char semiMajorUncert;
	unsigned char semiMinorUncert;
	unsigned char majorAxis;
	unsigned char altUncert;
	unsigned char confidence;
}__attribute__ ((packed)) tapi_gps_ref_loc_t;

typedef struct {
	unsigned char satId; //Satellite ID
	unsigned short iode;
	unsigned char udre;
	signed short pseudoRangeCor;
	signed short rangeRateCor;
}__attribute__ ((packed)) tapi_gps_dgps_sat_list_t;

typedef struct {
	unsigned long int gpsTow;
	unsigned char status;
	unsigned long int numberOfSat;
	tapi_gps_dgps_sat_list_t seqOfSatElement[16];
}__attribute__ ((packed)) tapi_gps_dgps_correction_t;

typedef struct {
	unsigned long int rsv1; // 0~838860
	unsigned long int rsv2; // 0~16777215
	unsigned long int rsv3; // 0~16777215
	unsigned long int rsv4; // 0~65535
}__attribute__ ((packed)) tapi_gps_navi_subframe_rsv_t;

typedef struct {
	unsigned char ephemCodeOnL2; // 0~3
	unsigned char ephemUra; // 0~15
	unsigned char ephemSvHealth; // 0~63
	unsigned short ephemIodc; // 0~1023
	unsigned char ephemL2PFlag; // 0~1
	tapi_gps_navi_subframe_rsv_t NavigationSubFrameRsv;
	signed char ephemTgd; // -128~127
	unsigned short ephemToc; // 0~37799
	signed char ephemAf2; // -128~12
	signed short ephemAf1; // -32768~32767
	signed long int ephemAf0; // -2097152~2097151
	signed short ephemCrs; // -32768~32767
	signed short ephemDeltaN; // -32768~32767
	signed long int ephemM0; // -2147483648~2147483647
	signed short ephemCuc; // -32768~32767
	unsigned long int ephemE; // 0~4294967295
	signed short ephemCus; // -32768~32767
	unsigned long int ephemAPowrHalf; // 0~4294967295
	unsigned short ephemToe; // 0~37799
	signed char ephemFitFlag; // 0~1
	unsigned char ephemAoda; // 0~31
	signed short ephemCic; // -32768~32767
	signed long int ephemOmegaA0; // -2147483648~2147483647
	signed short ephemCis; // -32768~32767
	signed long int ephemI0; // -2147483648~2147483647
	signed short ephemCrc; // -32768~32767
	signed long int ephemW; // -2147483648~2147483647
	signed long int ephemOmegaADot; // -8388608~8388607
	signed short ephemIDot; // -8192~8191
}__attribute__ ((packed)) tapi_gps_navi_ephe_t;

typedef struct {
	unsigned char satId;
	unsigned char NavigationSatStatus;
	tapi_gps_navi_ephe_t NavigationEphemeris;
}__attribute__ ((packed)) tapi_gps_navi_sat_info_t;

typedef struct {
	unsigned long int numberOfSat;
	tapi_gps_navi_sat_info_t NavigationSatInfo[16];
}__attribute__ ((packed)) tapi_gps_navi_model_t;

typedef struct {
	signed char alfa0; // -128~127
	signed char alfa1; // -128~127
	signed char alfa2; // -128~127
	signed char alfa3; // -128~127
	signed char beta0; // -128~127
	signed char beta1; // -128~127
	signed char beta2; // -128~127
	signed char beta3; // -128~127
}__attribute__ ((packed)) tapi_gps_iono_model_t;

typedef struct {
	signed long int utcA1; // -8388608~8388607
	signed long int utcA0; // -2147483648~2147483647
	unsigned char utcTot; // 0~255
	unsigned char utcWNt; // 0~255
	signed char utcDeltaTls; // -128~127
	unsigned char utcWNlsf; // 0~255
	signed char utcDN; // -128~127
	signed char utcDeltaTlsf; // -128~127
}__attribute__ ((packed)) tapi_gps_utc_model_t;

typedef struct {
	signed char dataId; // only for 3G, 0~3, if this value is -1, it means this value is invalid
	unsigned char satId;
	unsigned short almanacE; // 0~65536
	unsigned char almanacToa; // 0~255
	signed short almanacKsii; // -32768~3276
	signed short almanacOmegaDot; // -32768~3276
	unsigned char almanacSvHealth; // 0~255
	unsigned long int almanacAPowerHalf; // 0~16777215
	signed long int almanacOmega0; // -8388608~8388607
	signed long int almanacW; // -8388608~8388607
	signed long int almanacM0; // -8388608~8388607
	signed short almanacAf0; // -1024~1023
	signed short almanacAf1; // -1024~1023
}__attribute__ ((packed)) tapi_gps_almanac_sat_info_t;

typedef struct {
	unsigned char almanacWNa; // 0~255
	unsigned long int numberOfSat;
	tapi_gps_almanac_sat_info_t AlmanacSatInfo[64];
}__attribute__ ((packed)) tapi_gps_almanac_model_t;

typedef struct {
	tapi_gps_utran_gps_ref_time_t AcqUtranGpsRefTime;
	tapi_gps_utran_gps_unc_t AcqUtranGpsUncertainty;
}__attribute__ ((packed)) tapi_gps_acq_utran_time_t;

typedef struct {
	unsigned char satId;
	signed short doppler0; // -2048~2047 (real value is from -5120 to 5117.5 by step of 2.5)
	unsigned char doppler1; // 0~63 (real value is from -0.966 to 0.483 by step of 0.023)
	unsigned char dopplerUncertainty; // 0~7 (12.5, 25, 50, 100, 200)
	unsigned short codePhase; // 0~1022
	unsigned char intCodePhase; // 0~19
	unsigned char gpsBitNumber; // 0~3
	unsigned char codePhaseSearchWindow; // 0~15 (1023, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192)
	unsigned char azimuth; // 0~31, 11.25 degree resolution
	unsigned char elevation; // 0~7, 11.25 degree resolution
}__attribute__ ((packed)) tapi_gps_acq_sat_info_t;

typedef struct {
	unsigned long int gpsTow;
	union {
		tapi_gps_gsm_time_t gsm_time;
		tapi_gps_acq_utran_time_t AcqUtranTime;
	} acquisitionTimeInfo;
	unsigned long int numberOfSat;
	tapi_gps_acq_sat_info_t lcsAcquisitionSatInfo[16];
}__attribute__ ((packed)) tapi_gps_acq_assist_t;

typedef struct {
	unsigned char satId[16];
	unsigned char numOfSat;
}__attribute__ ((packed)) tapi_gps_r_time_int_t;

typedef struct {
	unsigned long int flag;
//	tapi_gps_carrier_e_type cr_type;
	tapi_gps_ref_time_t ref_time;
	tapi_gps_ref_loc_t ref_loc;
	tapi_gps_dgps_correction_t dgps_corrections;
	tapi_gps_navi_model_t navi_model;
	tapi_gps_iono_model_t iono_model;
	tapi_gps_utc_model_t utc_model;
	tapi_gps_almanac_model_t almanac;
	tapi_gps_acq_assist_t acq_assist;
	tapi_gps_r_time_int_t r_time_int;
}__attribute__ ((packed)) tapi_gps_assist_data_noti_t; //APGPS -  GPS Assist Data Message - Notification

typedef struct {
	unsigned char lock_status;
	unsigned char afc_update;
}__attribute__ ((packed)) tapi_gps_frequency_aiding_noti_t;

/*                                     GPS Chipset on AP - END                                       */

static char data_gps_set_frequency_aiding[MENU_DATA_SIZE + 1] = "1";


static void _gps_hex_dump(char *pad, int size, const void *data)
{
	char buf[255] = {0, };
	char hex[4] = {0, };
	int i;
	unsigned char *p;

	if (size <= 0) {
		msg("%sno data", pad);
		return;
	}
	p = (unsigned char *)data;

	snprintf(buf, 255, "%s%04X: ", pad, 0);
	for (i = 0; i<size; i++) {
		snprintf(hex, 4, "%02X ", p[i]);
		strcat(buf, hex);

		if ((i + 1) % 8 == 0) {
			if ((i + 1) % 16 == 0) {
				msg("%s", buf);
				memset(buf, 0, 255);
				snprintf(buf, 255, "%s%04X: ", pad, i + 1);
			}
			else {
				strcat(buf, "  ");
			}
		}
	}

	msg("%s", buf);
}

static void _gps_print_dump(int data_len, void *data)
{
	if(!data)
		return;

	msg("");
	msg("  \tlen=%d", data_len);
	_gps_hex_dump("        ", data_len, data);

	msg("");
}


/* Noti */
static void on_noti_assist_data(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	guchar *decoded_data = NULL;
	gsize length;
	tapi_gps_assist_data_noti_t *noti = NULL;

	msg("");
	msgb("event(%s) receive !!", TAPI_NOTI_GPS_ASSIST_DATA);
	msg("data=%p", data);
	if(!data)
		return;

	msg(" ## Before decoding ##");
	_gps_print_dump(strlen(data), data);

	decoded_data = g_base64_decode((const gchar *)data, &length);
	noti = (tapi_gps_assist_data_noti_t *)decoded_data;

	msg(" ## After decoding ##");
	_gps_print_dump(length, decoded_data);

	msg(" - length = %d", length);
	msg(" - flag = %ld", noti->flag);
	g_free(decoded_data);
}

static void on_ind_measure_position(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	guchar *decoded_data = NULL;
	gsize length;
	tapi_gps_measure_position_indi_t *ind = NULL;

	msg("");
	msgb("event(%s) receive !!", TAPI_IND_GPS_MEASURE_POSITION);
	msg("data=%p", data);
	if(!data)
		return;

	msg(" ## Before decoding ##");
	_gps_print_dump(strlen(data), data);

	decoded_data = g_base64_decode((const gchar *)data, &length);
	ind = (tapi_gps_measure_position_indi_t *)decoded_data;

	msg(" ## After decoding ##");
	_gps_print_dump(length, decoded_data);

	msg(" - length = %d", length);
	msg(" - method_type = %d", ind->method_type);
	msg(" - rsp_time = %d", ind->rsp_time);
	msg(" - use_multi_sets = %d", ind->use_multi_sets);
	msg(" - environment_char = %d", ind->environment_char);
	msg(" - cell_timing_wnt = %d", ind->cell_timing_wnt);
	msg(" - add_assist_req = %d", ind->add_assist_req);
	g_free(decoded_data);
}

static void on_noti_reset_assist_data(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	msg("");
	msgb("event(%s) receive !!", TAPI_NOTI_GPS_RESET_ASSIST_DATA);
	msg("data=%p", data?data:0);
}

static void on_noti_frequency_aiding(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	tapi_gps_frequency_aiding_noti_t *noti = (tapi_gps_frequency_aiding_noti_t *)data;

	msg("");
	msgb("event(%s) receive !!", TAPI_NOTI_GPS_FREQUENCY_AIDING);
	msg("data=%p", data);
	if(!data)
		return;

	msg(" - lock = %d", noti->lock_status);
	msg(" - afc_update = %d", noti->afc_update);
}

/* Response */
static void on_gps_set_frequency_aiding(TapiHandle *handle, int result, void *data, void *user_data)
{
	msg("");
	msgb("tel_set_gps_frequency_aiding() response receive");
	msg(" - result = 0x%x", result);
}

/* Request */
static int run_gps_set_frequency_aiding(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	unsigned char state;
	msg("call tel_set_gps_frequency_aiding()");

	state = atoi(data_gps_set_frequency_aiding);

	result = tel_set_gps_frequency_aiding(handle, state, on_gps_set_frequency_aiding, NULL);
	if (result != TAPI_API_SUCCESS) {
		msg("failed. (result = %d)", result);
	}
	msg("exit");

	return 0;
}

static int run_gps_confirm_measure_position(MManager *mm, struct menu_data *menu)
{
	TapiHandle *handle = menu_manager_ref_user_data(mm);
	int result;
	tapi_gps_measure_position_confirm_t data;

	msg("call tel_confirm_gps_measure_pos().");
	_gps_print_dump(sizeof(tapi_gps_measure_position_confirm_t), &data);

	result = tel_confirm_gps_measure_pos(handle, (unsigned char *)&data, sizeof(tapi_gps_measure_position_confirm_t));

	if (result != TAPI_API_SUCCESS) {
		msg("failed. (result = %d)", result);
	}
	msg("exit");
	return 0;
}

/* Menu */
static struct menu_data menu_gps_set_frequency_aiding[] = {
	{ "1", "state (0=Disable, 1=Enable)", NULL, NULL, data_gps_set_frequency_aiding},
	{ "2", "run", NULL, run_gps_set_frequency_aiding, NULL},
	{ NULL, NULL, },
};

static struct menu_data menu_gps_confirm_measure_position[] = {
	{ "1", "run", NULL, run_gps_confirm_measure_position, NULL},
	{ NULL, NULL, },
};


struct menu_data menu_gps[] = {
	{ "1", "SET Frequency Aiding", menu_gps_set_frequency_aiding, NULL, NULL},
	{ "2", "CONFIRM Measure Position", menu_gps_confirm_measure_position, NULL, NULL},
	{ NULL, NULL, },
};

void register_gps_event(TapiHandle *handle)
{
	int ret;

	/* Gps */
	ret = tel_register_noti_event(handle, TAPI_NOTI_GPS_ASSIST_DATA, on_noti_assist_data, NULL);
	ret = tel_register_noti_event(handle, TAPI_IND_GPS_MEASURE_POSITION, on_ind_measure_position, NULL);
	ret = tel_register_noti_event(handle, TAPI_NOTI_GPS_RESET_ASSIST_DATA, on_noti_reset_assist_data, NULL);
	ret = tel_register_noti_event(handle, TAPI_NOTI_GPS_FREQUENCY_AIDING, on_noti_frequency_aiding, NULL);

}
