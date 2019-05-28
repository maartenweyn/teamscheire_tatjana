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

bool post_to_thingsboard(double ts, int avg_leq, int corr_avg_leq) {
	/* Initialize CURL */
	CURL *curlHandler = curl_easy_init();

	connection_h connection;

	int conn_err;
	conn_err = connection_create(&connection);
	if (conn_err != CONNECTION_ERROR_NONE) {
		/* Error handling */
		return false;
	}

	char *proxy_address;
	conn_err = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);

	if(curlHandler) {
			if (conn_err == CONNECTION_ERROR_NONE && proxy_address)
			curl_easy_setopt(curlHandler, CURLOPT_PROXY, proxy_address);

	  /* Set CURL parameters */
	  //curl_easy_setopt(curlHandler, CURLOPT_URL, "http://apidev.accuweather.com/currentconditions/v1/28143.json?language=en&apikey=hoArfRosT1215");
	  curl_easy_setopt(curlHandler, CURLOPT_URL, "http://demo.thingsboard.io/api/v1/ObCtJ5ttQ8U9tToxcQvD/telemetry");

	  struct curl_slist *list = NULL;
	  list = curl_slist_append(list, "Content-Type: application/json");
	  curl_easy_setopt(curlHandler, CURLOPT_HTTPHEADER, list);
	  /* Now specify the POST data */
	  char json[100];
	  dlog_print(DLOG_INFO, LOG_TAG, "ts %f  %.0f", ts, ts * 1000);
	  snprintf(json, sizeof(json), "{\"ts\":%.0f, \"values\":{\"leq\":%d, \"cleq\":%d}}", ts * 1000, avg_leq, corr_avg_leq);
	  dlog_print(DLOG_INFO, LOG_TAG, "json %s", json);
	  curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, json);
	  curl_easy_setopt(curlHandler, CURLOPT_READFUNCTION, read_callback);
	  curl_easy_setopt(curlHandler, CURLOPT_READDATA, (void *)&chunk);
	  curl_easy_setopt(curlHandler, CURLOPT_VERBOSE, 1L);

	  /* Perform the request */
	  CURLcode res = curl_easy_perform(curlHandler);

	  /* Check for errors */
	  if(res != CURLE_OK)
		fprintf(stderr, "CURL failed: %s\n", curl_easy_strerror(res));

	  /* Clean up */
	  curl_easy_cleanup(curlHandler);
	  free(chunk.memory);
	  connection_destroy(connection);
	  //free(jObj);
	}

	return true;
}


