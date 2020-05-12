/*
 * pwmbuzzer.c
 *
 *  Created on: Mar 16, 2019
 *      Author: Stefan Mijalkov
 */


#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/timer.h>
#include "pwmbuzzer.h"

#define WTIMER0          WTIMER0_BASE

#define PORTC           GPIO_PORTC_BASE


// BUZZER: PC5, WT0CCP1 Wide Timer 0, sub-timer B

void buzzerPwmInit()
{
    // Enables peripheral Wide Timer 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);

   // Enables the buzzer on port C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    // Sets the buzzer as an output device on PORT C, Pin 5
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5);

    // Connect Port C Pin 5 to the wide timer 0
    GPIOPinTypeTimer(PORTC, GPIO_PIN_5);


    // This function configures the pin that selects the peripheral function associated with a
    // particular GPIO pin.
    GPIOPinConfigure(GPIO_PC5_WT0CCP1); //PIN C5


    // Select PWM for Wide Timer 0 sub-Timer B
    // TIMER_CFG_SPLIT_PAIR splits it to two half width timers, we are using only sub-Timer B
    TimerConfigure(WTIMER0, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM));

    // Invert the PWM waveform, so that the Match register value is the pulse width.
    // Otherwise, the pulse width will be (Load value) - (Match value).
    // The inversion is done by enabling output inversion on the PWM pins.
    TimerControlLevel(WTIMER0, TIMER_B, true /* output inversion */);

    // Enables the wide timer 0's sub timer B
    TimerEnable(WTIMER0, TIMER_B);
}
