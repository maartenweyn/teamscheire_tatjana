#ifndef __tatysoundservice_H__
#define __tatysoundservice_H__

#include <dlog.h>
#include <stdint.h>
#include "settings.h"


#define SAMPLE_RATE 44100
#define SAMPLE_TYPE AUDIO_SAMPLE_TYPE_S16_LE
#define RECORDING_SEC 1
#define RECORDING_INTERVAL 10
#define MIN_RECORDING_INTERVAL 60

#define POSTDATA_BUFFER_SIZE	60 * 24

//#define MYSERVICELAUNCHER_APP_ID "be.wesdec.tatysoundviewer" // an ID of the UI application of our package
#define MYSERVICELAUNCHER_APP_ID "arq901aCcl.TatySoundWebViewer" // an ID of the UI application of our package

#define HOST_URL	 "http://teamscheire.wesdec.be:8080/api/v1/%s/telemetry"

#define STRNCMP_LIMIT 256 // the limit of characters to be compared using strncmp function


typedef struct
{
	double ts;
	int id;
	double sound_level;
	double leq_min;
	double leq_hour;
	double leq_8hours;
	double leq_day;
	int response;
} post_data_s;

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "tatysoundservice"


#define _PRINT_MSG_LOG_BUFFER_SIZE_ 1024
#define PRINT_MSG(fmt, args...) do { char _log_[_PRINT_MSG_LOG_BUFFER_SIZE_]; \
    snprintf(_log_, _PRINT_MSG_LOG_BUFFER_SIZE_, fmt, ##args); } while (0)

#define _DEBUG_MSG_LOG_BUFFER_SIZE_ 1024
#define DLOG_PRINT_DEBUG_MSG(fmt, args...) do { char _log_[_DEBUG_MSG_LOG_BUFFER_SIZE_]; \
    snprintf(_log_, _PRINT_MSG_LOG_BUFFER_SIZE_, fmt, ##args); \
    dlog_print(DLOG_DEBUG, LOG_TAG, _log_); } while (0)

#define DLOG_PRINT_ERROR(fun_name, error_code) dlog_print(DLOG_ERROR, LOG_TAG, \
        "%s() failed! Error: %s [code: %d]", \
        fun_name, get_error_message(error_code), error_code)

#define CHECK_ERROR(fun_name, error_code) if (error_code != 0) { \
    DLOG_PRINT_ERROR(fun_name, error_code); \
    }

#define CHECK_ERROR_AND_RETURN(fun_name, error_code) if (error_code != 0) { \
    DLOG_PRINT_ERROR(fun_name, error_code); \
    return; \
    }

#define CHECK_ERROR_AND_RETURN(fun_name, error_code) if (error_code != 0) { \
    DLOG_PRINT_ERROR(fun_name, error_code); \
    return; \
    }

void push_current_values(double ts, int leq, int corrected);
uint8_t push_average_values(double ts, int id, double sound_level, double leq_min, double leq_hour, double leq_8hours, double leq_day);
char thingsboard_url[100];

#endif /* __tatysoundservice_H__ */
