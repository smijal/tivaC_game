;/*
 ;* ras_asm.asm
 ;*
 ;*  Created on: Mar 5, 2019
 ;*      Author: Joshua Zychal
 ;*/

   			.cdecls "stdint.h","stdbool.h","inc/hw_memmap.h","driverlib/pin_map.h","driverlib/sysctl.h","driverlib/gpio.h","ras.h", "driverlib/adc.h","seg7digit.h", "seg7(1).h"

               .text
                .global rasRead

ADC_BASE     .field  ADC0_BASE

;Implements the rasRead function in assembly
					.asmfunc

rasRead				PUSH{lr, r0} ; pushes the lr and the r0 to the stack
					LDR r0, ADC_BASE ;loads the ADC_BASE to r0
 			        MOV r1, #0 ;moves 0 to r1
                    BL ADCProcessorTrigger ;calls the processor trigger function


loop
					LDR r0,ADC_BASE
					MOV r1, #0
					MOV r2, #0
					BL ADCIntStatus ; calls the ADCIntStatus function
					CMP r0, #0 ; compares if the returned value is 0, raw data
					BEQ loop ; if yes, continues the loop

                    LDR r0, ADC_BASE ;loads ADC_BASE again to r0
                    MOV r1, #0 ; moves 0 to r1
                    POP{r2} ; pops the adress value stored on the stack into r2
                    BL ADCSequenceDataGet ; calls the ADCSequencerDataGet function

                    LDR r0, ADC_BASE ;loads
                    MOV r1, #0 ;moves
                    BL ADCIntClear ; clears the interupt on sequencer 0

                    POP{PC} ; returns

					.endasmfunc







