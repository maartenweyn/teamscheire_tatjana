/*
 * communication.c
 *
 *  Created on: May 27, 2019
 *      Author: maartenweyn
 */

#include "communication.h"
#include "tatysoundservice.h"

#include <curl/curl.h>
#include <net_connection.h>
#include <Ecore.h>

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct MemoryStruct chunk;
static size_t read_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;

	dlog_print(DLOG_INFO, LOG_TAG, "read_callback size: %d", realsize);
	dlog_print(DLOG_INFO, LOG_TAG, "read_callback %s", (char*) contents);

	return realsize;
}

int post_to_thingsboard(post_data_s data[], int lenght) {
	/* Initialize CURL */
	CURL *curlHandler = curl_easy_init();

	connection_h connection;

	int conn_err;
	conn_err = connection_create(&connection);
	if (conn_err != CONNECTION_ERROR_NONE) {
		/* Error handling */
		return 1;
	}

	char *proxy_address;
	conn_err = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);

	if(curlHandler) {
		if (conn_err == CONNECTION_ERROR_NONE && proxy_address)
			curl_easy_setopt(curlHandler, CURLOPT_PROXY, proxy_address);

		/* Set CURL parameters */

		curl_easy_setopt(curlHandler, CURLOPT_URL, thingsboard_url);
		struct curl_slist *list = NULL;
		list = curl_slist_append(list, "Content-Type: application/json");
		curl_easy_setopt(curlHandler, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curlHandler, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curlHandler, CURLOPT_READDATA, (void *)&chunk);
		curl_easy_setopt(curlHandler, CURLOPT_TIMEOUT, 5);
		/* Now specify the POST data */
		char json[300];


		while (lenght > 0)
		{
			//int temp_length =
			snprintf(json, sizeof(json), "{\"ts\":%.0f,\"values\":{\"sound_level\":%.4f,\"min\":%.1f, \"hour\":%.1f,\"8hours\":%.1f,\"day\":%.1f,\"id\":%d,\"length\":%d,\"resp\":%d}}", data[lenght-1].ts * 1000, data[lenght-1].sound_level, data[lenght-1].leq_min, data[lenght-1].leq_hour, data[lenght-1].leq_8hours, data[lenght-1].leq_day, data[lenght-1].id, lenght, data[lenght-1].response);
			dlog_print(DLOG_INFO, LOG_TAG, "json %s", json);
			curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, json);

			/* Perform the request */
			CURLcode res = curl_easy_perform(curlHandler);

			if(res != CURLE_OK) {
				fprintf(stderr, "CURL failed: %s\n", curl_easy_strerror(res));
				break;
			}

			lenght--;

			//todo delay needed?
		}

//		for (int i = 0; i < lenght; i++) {
//			char temp[200];
//			//dlog_print(DLOG_INFO, LOG_TAG, "ts %f  %.0f", data[i].ts, data[i].ts * 1000);
//			int temp_length = snprintf(temp, sizeof(temp), "{\"ts\":%.0f,\"values\":{\"sound_level\":%.4f,\"min\":%.1f, \"hour\":%.1f,\"8hours\":%.1f,\"day\":%.1f,\"id\":%d,\"length\":%d,\"resp\":%d}}", data[i].ts * 1000, data[i].sound_level, data[i].leq_min, data[i].leq_hour, data[i].leq_8hours, data[i].leq_day, data[i].id, lenght, data[i].response);
//			memcpy(&json[current_length], temp, temp_length);
//			current_length += temp_length;
//			if (i + 1 < lenght) {
//				json[current_length++] = ',';
//			} else {
//				json[current_length++] = ']';
//				json[current_length++] = '\0';
//			}
//		}

//		dlog_print(DLOG_INFO, LOG_TAG, "json %s", json);
//
//		curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, json);
//		//curl_easy_setopt(curlHandler, CURLOPT_VERBOSE, 1L);
//
//		/* Perform the request */
//		CURLcode res = curl_easy_perform(curlHandler);
//
//		/* Check for errors */
//		if(res != CURLE_OK)
//			fprintf(stderr, "CURL failed: %s\n", curl_easy_strerror(res));


		/* Clean up */
		curl_easy_cleanup(curlHandler);
		free(chunk.memory);
		connection_destroy(connection);

		if (lenght == 0)
			return 0;
		else
			return 1;
		//free(jObj);
	}

	return 2;
}


