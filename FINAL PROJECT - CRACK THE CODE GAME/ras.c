/*
 * ras.c
 *
 *  Created on: Mar 5, 2019
 *      Author: Stefan Mijalkov
 */

#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include "ras.h"
#include "seg7digit.h"
#include "seg7(1).h"
#include <driverlib/adc.h>

void rasInit(){

   SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); //enables the peripheral on analog to digital converter ADC0

   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)){}

   ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0); //configures the sequence number 0, with
                                                                 //trigger type ADC_TRIGGER_PROCESSOR and highest priority

   ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH7); //configures the step 0, channel 7 (knob 1)

   //configures the step 1, channel 6 (knob 2), as a last step and causes interrupt when the step is completed
   ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH6);

   ADCSequenceEnable(ADC0_BASE, 0); // enables the sequence 0


}





