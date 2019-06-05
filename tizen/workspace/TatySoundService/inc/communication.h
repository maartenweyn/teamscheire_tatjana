/*
 * communication.h
 *
 *  Created on: May 27, 2019
 *      Author: maartenweyn
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <stdbool.h>
#include "tatysoundservice.h"


int post_to_thingsboard(post_data_s data[], int lenght);

#endif /* COMMUNICATION_H_ */
