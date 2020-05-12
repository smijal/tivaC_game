/*
 * seg7(2).h
 *
 *  Created on: Apr 15, 2019
 *      Author: hazar
 */

#ifndef SEG7_2__H_
#define SEG7_2__H_

// Initialize the port connection to TiM1637 and the 7-segment display
void seg7_2_Init();

// Update all 4 digits of the 7-segment displays
void seg7_2_Update(uint8_t code[]);


#endif /* SEG7_2__H_ */
