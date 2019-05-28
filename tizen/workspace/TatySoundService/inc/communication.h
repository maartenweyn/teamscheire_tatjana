/*
 * communication.h
 *
 *  Created on: May 27, 2019
 *      Author: maartenweyn
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <stdbool.h>


bool post_to_thingsboard(double ts, int avg_leq, int corr_avg_leq);


#endif /* COMMUNICATION_H_ */
