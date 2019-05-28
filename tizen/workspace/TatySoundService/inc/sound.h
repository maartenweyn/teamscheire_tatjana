/*
 * sound.h
 *
 *  Created on: May 27, 2019
 *      Author: maartenweyn
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <stdbool.h>

#define CORR_X0		55.0 // watch
#define CORR_X1		82.0 // watch
#define CORR_REF0	20.0 // ref
#define	CORR_REF1	76.0 // ref

double a_filter(double input);
int correctdB(double input);

bool audio_device_init(void);
void measure_sound();

#endif /* SOUND_H_ */
