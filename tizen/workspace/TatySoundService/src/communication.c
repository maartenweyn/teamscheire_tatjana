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
		//curl_easy_setopt(curlHandler, CURLOPT_URL, "http://demo.thingsboard.io/api/v1/ObCtJ5ttQ8U9tToxcQvD/telemetry");
		//curl_easy_setopt(curlHandler, CURLOPT_URL, "https://demo.thingsboard.io/api/v1/w4ntKFw4M1eK0MEmMHvt/telemetry");
		curl_easy_setopt(curlHandler, CURLOPT_URL, thingsboard_url);
		//curl_easy_setopt(curlHandler, CURLOPT_URL, FIREBASE_HOST);
		//json [{"ts":1560594942583, "values":{"leq":79, "cleq":71, "resp":0}}]
		struct curl_slist *list = NULL;
		list = curl_slist_append(list, "Content-Type: application/json");
		curl_easy_setopt(curlHandler, CURLOPT_HTTPHEADER, list);
		/* Now specify the POST data */
		char json[100 * lenght];
		int current_length = 1;

		json[0] = '[';

		for (int i = 0; i < lenght; i++) {
			char temp[100];
			dlog_print(DLOG_INFO, LOG_TAG, "ts %f  %.0f", data[i].ts, data[i].ts * 1000);
			int temp_length = snprintf(temp, sizeof(temp), "{\"ts\":%.0f, \"values\":{\"leq\":%d, \"cleq\":%d, \"resp\":%d}}", data[i].ts * 1000, data[i].avg_leq, data[i].corr_avg_leq, data[i].response);
			//int temp_length = snprintf(temp, sizeof(temp), "{\"ts\":%.0f, \"leq\":%d, \"cleq\":%d, \"resp\":%d}", data[i].ts * 1000, data[i].avg_leq, data[i].corr_avg_leq, data[i].response);
			memcpy(&json[current_length], temp, temp_length);
			current_length += temp_length;
			if (i + 1 < lenght) {
				json[current_length++] = ',';
			} else {
				json[current_length++] = ']';
				json[current_length++] = '\0';
			}
		}
		dlog_print(DLOG_INFO, LOG_TAG, "json %s", json);

		curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, json);
		curl_easy_setopt(curlHandler, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curlHandler, CURLOPT_READDATA, (void *)&chunk);
		//curl_easy_setopt(curlHandler, CURLOPT_VERBOSE, 1L);

		/* Perform the request */
		CURLcode res = curl_easy_perform(curlHandler);

		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "CURL failed: %s\n", curl_easy_strerror(res));
		/* Clean up */
		curl_easy_cleanup(curlHandler);
		free(chunk.memory);
		connection_destroy(connection);

		if (res == CURLE_OK)
			return 0;
		else
			return 3;
		//free(jObj);
	}

	return 2;
}


