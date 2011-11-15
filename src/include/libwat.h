/*
 * libwat: Wireless AT commands library
 *
 * David Yat Sin <dyatsin@sangoma.com>
 * Copyright (C) 2011, Sangoma Technologies.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *
 */
#ifndef _LIBWAT_H
#define _LIBWAT_H


#include <stdlib.h>
#include <stdint.h>

#include "wat_declare.h"

#if 1
#define WAT_FUNC_DEBUG 1
#endif
/* Debugging */
#define WAT_DEBUG_UART_RAW		(1 << 0) /* Show raw uart reads */
#define WAT_DEBUG_UART_DUMP		(1 << 1) /* Show uart commands */
#define WAT_DEBUG_CALL_STATE	(1 << 2) /* Debug call states */
#define WAT_DEBUG_AT_PARSE		(1 << 3) /* Debug how AT commands are parsed */
#define WAT_DEBUG_AT_HANDLE		(1 << 4) /* Debug how AT commands are scheduled/processed */

/*ENUMS & Defines ******************************************************************/

#define WAT_MAX_SPANS		32
#define WAT_MAX_NUMBER_SZ	32 /* DAVIDY TODO: Find real max sizes based on specs */
#define WAT_MAX_NAME_SZ		24 /* DAVIDY TODO: Find real max sizes based on specs */
#define WAT_MAX_SMS_SZ		1024 /* DAVIDY TODO: Find real max sizes based on specs */
#define WAT_MAX_CMD_SZ		2048 /* DAVIDY TODO: Find real max sizes based on specs */

#define WAT_MAX_CALLS_PER_SPAN			16
#define WAT_MAX_SMSS_PER_SPAN			16
#define WAT_MAX_ERROR_SZ				40

typedef size_t wat_size_t;

typedef enum {
	WAT_SIGSTATUS_DOWN,
	WAT_SIGSTATUS_UP,
} wat_sigstatus_t;

typedef enum {
	WAT_ALARM_NO_SIGNAL,
	WAT_ALARM_NO_SIM,
} wat_alarm_t;

typedef enum {
	WAT_SMS_PDU,
	WAT_SMS_TXT,
} wat_sms_type_t;

typedef enum {
	WAT_SMS_CAUSE_QUEUE_FULL = 1,
	WAT_SMS_CAUSE_MODE_NOT_SUPPORTED,
	WAT_SMS_CAUSE_NO_RESPONSE,
	WAT_SMS_CAUSE_NO_NETWORK,
	WAT_SMS_CAUSE_NETWORK_REJECT,
	WAT_SMS_CAUSE_UNKNOWN,
} wat_sms_cause_t;

typedef enum {
	WAT_MODULE_TELIT,
	WAT_MODULE_INVALID,
} wat_moduletype_t;

#define WAT_MODULETYPE_STRINGS "telit", "invalid"
WAT_STR2ENUM_P(wat_str2wat_moduletype, wat_moduletype2str, wat_moduletype_t);

typedef enum {
	WAT_NUMBER_TYPE_UNKNOWN,
	WAT_NUMBER_TYPE_INTERNATIONAL,
	WAT_NUMBER_TYPE_INVALID,
} wat_number_type_t;

#define WAT_NUMBER_TYPE_STRINGS "unknown", "international" ,"invalid"

WAT_STR2ENUM_P(wat_str2wat_number_type, wat_number_type2str, wat_number_type_t);

typedef enum {
	WAT_NUMBER_PLAN_UNKNOWN,
	WAT_NUMBER_PLAN_ISDN,
	WAT_NUMBER_PLAN_INVALID,
} wat_number_plan_t;

#define WAT_NUMBER_PLAN_STRINGS "unknown", "ISDN" ,"invalid"

WAT_STR2ENUM_P(wat_str2wat_number_plan, wat_number_plan2str, wat_number_plan_t);

typedef enum {
	WAT_NUMBER_VALIDITY_VALID,			/* CLI Number is valid */
	WAT_NUMBER_VALIDITY_WITHELD,		/* CLI has been withheld by originator */
	WAT_NUMBER_VALIDITY_UNAVAILABLE,	/* CLI unavailable due to interworking problems or limitation of originating network */
	WAT_NUMBER_VALIDITY_INVALID,
} wat_number_validity_t;

#define WAT_NUMBER_VALIDITY_STRINGS "valid", "witheld" ,"unavailable", "invalid"

WAT_STR2ENUM_P(wat_str2wat_number_validity, wat_number_validity2str, wat_number_validity_t);

typedef struct {
	char digits [WAT_MAX_NUMBER_SZ];
	wat_number_type_t type;
	wat_number_plan_t plan;
	uint8_t validity;
} wat_number_t;

typedef enum {
	WAT_CALL_TYPE_VOICE,
	WAT_CALL_TYPE_DATA,
	WAT_CALL_TYPE_FAX,
	WAT_CALL_TYPE_INVALID,
} wat_call_type_t;

#define WAT_CALL_TYPE_STRINGS "voice", "data", "fax", "invalid"
WAT_STR2ENUM_P(wat_str2wat_call_type, wat_call_type2str, wat_call_type_t);

typedef enum {
	WAT_CALL_SUB_REAL,		/* Regular call */
	WAT_CALL_SUB_CALLWAIT,	/* Call Waiting */
	WAT_CALL_SUB_THREEWAY,	/* Three-way call */
	WAT_CALL_SUB_INVALID, 
} wat_call_sub_t;

#define WAT_CALL_SUB_STRINGS "real", "call waiting", "three-way", "invalid"
WAT_STR2ENUM_P(wat_str2wat_call_sub, wat_call_sub2str, wat_call_sub_t);

typedef enum {
	WAT_LOG_CRIT,
	WAT_LOG_ERROR,
	WAT_LOG_WARNING,
	WAT_LOG_INFO,
	WAT_LOG_NOTICE,
	WAT_LOG_DEBUG,
} wat_loglevel_t;

/* Structures  *********************************************************************/
typedef struct _wat_con_event {
	wat_call_type_t type;
	wat_call_sub_t	sub;
	wat_number_t called_num;
	wat_number_t calling_num;
	char calling_name[WAT_MAX_NAME_SZ];
} wat_con_event_t;

typedef struct _wat_sms_pdu_number {
	uint8_t len;
	uint8_t toa; /* Type of Address */
	char number[WAT_MAX_NUMBER_SZ];
} wat_sms_pdu_number_t;

typedef struct _wat_sms_pdu_deliver {
	/* From  www.dreamfabric.com/sms/deliver_fo.html */
	uint8_t rp:1; /* Reply Path */
	uint8_t udhi:1; /* User data header indicator. 1 => User Data field starts with a header */
	uint8_t sri:1; /* Status report indication. 1 => Status report is going to be returned to the SME */
	uint8_t mms:1; /* More messages to send. 0 => There are more messages to send  */
	uint8_t mti:2; /* Message type indicator. 0 => this PDU is an SMS-DELIVER */
} wat_sms_pdu_deliver_t;

typedef struct _wat_sms_pdu_timestamp {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	int8_t timezone;
} wat_sms_pdu_timestamp_t;

typedef struct _wat_sms_event_pdu {
	wat_sms_pdu_number_t smsc;	
	wat_sms_pdu_deliver_t deliver;
	wat_sms_pdu_number_t sender;
	
	uint16_t pid;		/* Protocol Identifier */
	uint16_t dcs;		/* Daca coding scheme */

	wat_sms_pdu_timestamp_t timestamp;
	
} wat_sms_event_pdu_t;

typedef struct _wat_sms_event {
	wat_number_t calling_num;
	wat_number_t called_num;
	wat_sms_type_t type;				/* PDU or Plain Text */
	uint32_t len;						/* Length of message */
	char message[WAT_MAX_SMS_SZ];		/* Message */
	wat_sms_event_pdu_t pdu;
} wat_sms_event_t;

typedef struct _wat_rel_event {
	uint32_t cause;
	const char *error;
} wat_rel_event_t;

typedef enum _wat_con_status_type {
	WAT_CON_STATUS_TYPE_RINGING = 1,
	WAT_CON_STATUS_TYPE_ANSWER,
} wat_con_status_type_t;

typedef struct _wat_con_status {
	wat_con_status_type_t type;
} wat_con_status_t;

typedef struct _wat_sms_status {
	wat_bool_t success;
	wat_sms_cause_t cause;
	const char *error;
} wat_sms_status_t;

typedef struct _wat_cmd_status {
	wat_bool_t success;
	const char *error;
} wat_cmd_status_t;

typedef struct _wat_span_config_t {
	wat_moduletype_t moduletype;	

	/* Timeouts */
	uint32_t timeout_cid_num; /* Timeout to wait for a CLIP */
	uint32_t progress_poll_interval; /* How often to check for call status on outbound call */
	uint32_t signal_poll_interval;	/* How often to check for signal quality */
} wat_span_config_t;

typedef struct _wat_interface {
	/* Call-backs */
	void (*wat_sigstatus_change)(uint8_t span_id, wat_sigstatus_t sigstatus);
	void (*wat_alarm)(uint8_t span_id, wat_alarm_t alarm);

	/* Memory management */
	void *(*wat_malloc)(size_t size);
	void *(*wat_calloc)(size_t nmemb, size_t size);	
	void (*wat_free)(void *ptr);

	/* Logging */
	void (*wat_log)(uint8_t level, char *fmt, ...);
	void (*wat_log_span)(uint8_t span_id, uint8_t level, char *fmt, ...);

	/* Assert */
	void (*wat_assert)(char *message);

	/* Events */
	void (*wat_con_ind)(uint8_t span_id, uint8_t call_id, wat_con_event_t *con_event);
 	void (*wat_con_sts)(uint8_t span_id, uint8_t call_id, wat_con_status_t *con_status);
	void (*wat_rel_ind)(uint8_t span_id, uint8_t call_id, wat_rel_event_t *rel_event);
	void (*wat_rel_cfm)(uint8_t span_id, uint8_t call_id);
	void (*wat_sms_ind)(uint8_t span_id, wat_sms_event_t *sms_event);
	void (*wat_sms_sts)(uint8_t span_id, uint8_t sms_id, wat_sms_status_t *sms_status);
	void (*wat_cmd_sts)(uint8_t span_id, wat_cmd_status_t *status);
	int (*wat_span_write)(uint8_t span_id, void *data, uint32_t len);
} wat_interface_t;

/* Functions  *********************************************************************/
/* DAVIDY: TODO: add Doxygen headers */
WAT_DECLARE(void) wat_version(uint8_t *current, uint8_t *revision, uint8_t *age);
WAT_DECLARE(wat_status_t) wat_register(wat_interface_t *interface);
WAT_DECLARE(wat_status_t) wat_span_config(uint8_t span_id, wat_span_config_t *span_config);
WAT_DECLARE(wat_status_t) wat_span_unconfig(unsigned char span_id);
WAT_DECLARE(wat_status_t) wat_span_start(uint8_t span_id);
WAT_DECLARE(wat_status_t) wat_span_stop(uint8_t span_id);
WAT_DECLARE(void) wat_span_process_read(uint8_t span_id, void *data, uint32_t len);
WAT_DECLARE(uint32_t) wat_span_schedule_next(uint8_t span_id);
WAT_DECLARE(void) wat_span_run(uint8_t span_id);

WAT_DECLARE(wat_status_t) wat_span_get_chip_info(uint8_t span_id,
													char *manufacturer_name, wat_size_t len_manufacturer_name,
													char *manufacturer_id, wat_size_t len_manufacturer_id,
													char *revision_id, wat_size_t len_revision_id,
													char *serial_number, wat_size_t len_serial_number,
													char *imsi, wat_size_t len_imsi,
													char *subscriber_number, wat_size_t len_subscriber_number);

WAT_DECLARE(wat_status_t) wat_span_get_netinfo(uint8_t span_id, char *net_info, wat_size_t len);
WAT_DECLARE(wat_status_t) wat_span_get_signal_quality(uint8_t span_id, char *strength, wat_size_t len_strength, char *ber, wat_size_t len_ber);


WAT_DECLARE(wat_status_t) wat_con_cfm(uint8_t span_id, uint8_t call_id);
WAT_DECLARE(wat_status_t) wat_con_req(uint8_t span_id, uint8_t call_id, wat_con_event_t *con_event);
WAT_DECLARE(wat_status_t) wat_rel_req(uint8_t span_id, uint8_t call_id);
WAT_DECLARE(wat_status_t) wat_rel_cfm(uint8_t span_id, uint8_t call_id);
WAT_DECLARE(wat_status_t) wat_sms_req(uint8_t span_id, uint8_t sms_id, wat_sms_event_t *sms_event);

#endif /* _LIBWAT_H */

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */

/******************************************************************************/


