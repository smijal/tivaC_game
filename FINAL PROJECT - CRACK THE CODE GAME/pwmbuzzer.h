/*
 * pwmbuzzer.h
 *
 *  Created on: Mar 16, 2019
 *      Author: Joshua Zychal
 */

#ifndef PWMBUZZER_H_
#define PWMBUZZER_H_

#include <stdint.h>

/*
 * Initialize the timer PWM functions connected to the three sub-LEDs.
 */
void buzzerPwmInit();

/*
 * Set the LED parameters for the three sub-LEDs
 */
void buzzerPwmSet(int pulsePeriod, int pulseWidth);

#endif /* PWMBUZZER_H_ */
