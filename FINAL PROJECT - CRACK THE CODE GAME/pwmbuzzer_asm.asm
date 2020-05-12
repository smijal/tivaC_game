;/*
 ;* pwmbuzzer_asm.asm
 ;*
 ;*  Created on: Mar 18, 2019
 ;*      Author: Stefan Mijalkov
 ;*/


                .cdecls "stdint.h","stdbool.h","inc/hw_memmap.h","inc/hw_timer.h","inc/hw_gpio.h","driverlib/pin_map.h","driverlib/timer.h", "ras.h", "pwmbuzzer.h"

                .text
;
WTIMER0         .field  WTIMER0_BASE ;defines variable WTIMER0 with value WTIMER0_BASE
SUB_TIMER_B     .field  TIMER_B ; defines variable SUB_TIMER_B with value TIMER_B

				.asmfunc

buzzerPwmSet    PUSH{r0,r1,lr} ; pushes the arguments passed to the function and the link register to the stack
				LDR r0, WTIMER0 ; loads the value from the WTIMER0 port
				LDR r1, SUB_TIMER_B ; loads the value from the sub-timer B port
				POP{r2}             ; pops the buzzerPeriod to the register 2
				BL  TimerLoadSet    ; This function configures the timer load value; if the timer is running then the value is
									; immediately loaded into the timer. Sets the period of the buzzer signal


				LDR r0, WTIMER0     ; loads the value from the WTIMER) port
				LDR r1, SUB_TIMER_B ; loads the value from the sub-timer B port
				POP{r2}             ; Pops the pulseWidth to register 2
				BL TimerMatchSet    ; used in PWM mode to determine the duty cycle of the output signal.
				                    ; the higher the pulseWidth, the longer the duty cycle
				POP{PC}             ; returns, exits the function

			    .endasmfunc
