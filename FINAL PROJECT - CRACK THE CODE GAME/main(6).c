/*
 * main.c for ECE 266 Lab 6, Knob Control
 *
 *      Author: Stefan_Mijalkov & Joshua_Zychal
 */
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "launchpad.h"
#include "seg7(1).h"
#include "seg7(2).h"
#include "seg7digit.h"
#include "seg7digit_2.h"
#include "ras.h"
#include <inc/hw_memmap.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/adc.h>
#include "pwmbuzzer.h"

//*Tiva C values***************************

uint32_t tivaCFreq = 50000000; //Cortex_M4 clock frequency

uint32_t dutyCycle = 50000000*0.25; //max dutyCycle ratio

//**************************************

//Sound values*****************************************************************************

enum {
    C4, D4, E4, F4, G4, A4, B4, C5  // enumerated type to keep track of the tone positions
};

float tone[8]={261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25}; //frequency of each tone from C4 to C5

bool playEndMusic=true;  // allow music to be played once the game is over

//*****************************************************************************************

//Game statistics********************************************************************

//Structure to keep track of all data during the game
typedef struct{
    uint32_t number;
    uint16_t timeLeftSeconds;
    uint16_t attempts;
    bool gameOver;
}gameStatistics;

gameStatistics game = {0,120,0,false}; //initialized to 2 minutes, 6 attempts left

bool confirm=false; // confirmation that SW2 is pressed

uint32_t savedNumber; //saved number = the random number generated

//Structure for each digit of the randomly generated number
typedef struct {
    uint8_t digit0;                 // digit 1, rightmost
    uint8_t digit1;                 // digit 2
    uint8_t digit2;                 // digit 3
    uint8_t digit3;                 // digit 4, leftmost              // the status of the colon, 0 -- off, 1 -- on
}digitsRandNum;

//initialized to 0000
digitsRandNum randomNum = {
    0, 0, 0, 0
};

//**********************************************************************************

//System Run values********************************************************

enum {
    Pause, Run, Reset //system state - Pause/Run/Reset
}sysState=Pause;

uint32_t buzzDelay; // buzzer delay value in milliseconds

typedef enum {On, Off} OnOff_t; //On/Off state for the buzzer

static OnOff_t buzzerState = Off; //initialized to off

int angle1, angle2; //to keep track of the returned knob1 and knob2 values

uint32_t dataArray[2]={0,0}; //array (dataArray[0] the value of knob 1, dataArray[1] the value of knob 2)

uint8_t oldValues[4]={0,0,0,0}; //array to keep the old values of the digits of the randomly generated number

const int valuePerAngle=4096/99; // change of 1 digit on the display occurs when there is a change in data[] of 4096/99

// The initial state of the 7-segment display 1: "0000" with colon off
seg7Display_t seg7Display = {
    0, 0, 0, 0, 0
};

//The initial state of the 7-segment display 2: "02:00"
seg7Display_t timer_Display ={
   0, 0, 2, 0, 1
};
//*************************************************************************


//Sound Play functions*******************************************************

static void wrongSound(){ //sound when the number guessed is wrong

    if(buzzerState==On){
        buzzerState=Off;
    }

    buzzerPwmSet(0,0);

    buzzerState=On;

    buzzerPwmSet(tivaCFreq/180, dutyCycle/180);
    waitMs(130);

    buzzerPwmSet(0,0);
    waitMs(100);

    buzzerState=Off;
}

static void correctSound(){ //sound when the number guessed is correct
    if(buzzerState==On){
        buzzerState=Off;
    }

    buzzerPwmSet(0,0);

    waitMs(50);

    buzzerPwmSet(tivaCFreq/100, dutyCycle/300);
    waitMs(60);


    buzzerPwmSet(tivaCFreq/500, dutyCycle/500);
    waitMs(60);

    buzzerPwmSet(0,0);
    waitMs(700);

    buzzerState=Off;

}

static void melodyPlayBegin(){ //sound played every time the game starts over

    uint8_t i;

        buzzerState=On;

        for(i=0; i<8; i++){
            buzzerPwmSet(tivaCFreq/tone[i], dutyCycle/tone[i]);
            waitMs(200);

            buzzerPwmSet(0,0);
            waitMs(45);
        }

        for(i=0; i<8; i++){
            buzzerPwmSet(tivaCFreq/tone[i], dutyCycle/tone[i]);
            waitMs(30);
        }

        buzzerPwmSet(0,0);

        buzzerState=Off;

}

static void melodyPlayEnd(){ //sound played every time the game is over

    int i;

        buzzerState=On;

        for(i=8; i>=0; i--){
            buzzerPwmSet(tivaCFreq/tone[i], dutyCycle/tone[i]);
            waitMs(200);

            buzzerPwmSet(0,0);
            waitMs(45);
        }

        for(i=8; i>=0; i--){
            buzzerPwmSet(tivaCFreq/tone[i], dutyCycle/tone[i]);
            waitMs(30);
        }

        buzzerPwmSet(0,0);

        buzzerState=Off;

}


void callbackBuzzerPlay(uint32_t time)    // the scheduled time
{
        buzzDelay = 10;

    if(sysState==Run){
        switch(buzzerState){

        case On:
                {
                    buzzerPwmSet(50000000/261 , 0); // Keeps the width = 0, to keep the volume to 0 when off
                    buzzerState = Off;
                    buzzDelay = abs((angle2*100+angle1)-savedNumber); //formula to calculate and set the buzzer delay based on how far
                                                                      // the displayed value is from the correct value

                    uprintf("%d\r\n", buzzDelay);

                    break;
                }


        case Off: {
                    buzzerPwmSet(50000000/261.63 , 50000000/261.63*0.25);
                    buzzerState = On;
                    buzzDelay = 30;
                } } }

     else if (sysState==Pause || game.gameOver){ //stop beeping when the game is over
         buzzerPwmSet(50000000/261, 0);
     }

        schdCallback(callbackBuzzerPlay, time+buzzDelay);
}


//********************************************************************

//Score calculation functions************************************************************

//score calculation function based on number of attempts and the time left
static void gameOverFunc(){

    uint32_t score;
    uint32_t anotherNum;

    //The score is based on the time left and different multiply factors applied based on the number of attempts.
    // If the correct number is guessed with only 1 attempt, the time left is multiplied by 50, 2 attempts by 40 ... more than 5, by 0
    switch(game.attempts){

    case 1:
                score=game.timeLeftSeconds*50;
                break;

    case 2:
                score=game.timeLeftSeconds*40;
                break;

    case 3:
                score=game.timeLeftSeconds*30;
                break;

    case 4:

                score=game.timeLeftSeconds*20;
                break;
    case 5:
                score=game.timeLeftSeconds*10;
                break;

    default:
                score=game.timeLeftSeconds*0;
                break;
    }

    //uses the display 2 to display the calculated score
    timer_Display.d4=score/1000;
    anotherNum=score-timer_Display.d4*1000;

    timer_Display.d3=(anotherNum)/100;
    anotherNum=anotherNum-timer_Display.d3*100;

    timer_Display.d2=(anotherNum)/10;
    anotherNum=anotherNum-timer_Display.d2*10;

    timer_Display.d1=anotherNum;

    //Allows the end music to be played only 1 time, once the game is over
    if(playEndMusic){
    melodyPlayEnd();
    playEndMusic=false;
    }

    seg7_2_DigitUpdate(&timer_Display);

    ledTurnOnOff(false, false , false);

}

//**********************************************************************************************

//System Control functions*********************************************************************************************************

//Simulates time, goes backwards from 02:00 to 00:00
void timerDecrement(uint32_t time){

      if(game.gameOver==false){
      uint32_t minutes;
      uint32_t seconds;

             if(sysState==Run){

                      game.timeLeftSeconds--;

                      minutes=game.timeLeftSeconds/60;

                      seconds=game.timeLeftSeconds-minutes*60;

                      timer_Display.d3=minutes;

                          timer_Display.d2=seconds/10;
                          timer_Display.d1=seconds%10;

                          // if 0 seconds left, game is over
                          if(game.timeLeftSeconds==0){
                              game.gameOver=true;
                              sysState=Pause;
                          }


                      seg7_2_DigitUpdate(&timer_Display);
                }


           }

      else if(game.gameOver==true){ //if the game is over, does stop displaying time and displays the final score
          timer_Display.colon=0;
          gameOverFunc();
      }

      schdCallback(timerDecrement, time+1000);

}

// random 4-digit number generator function
static void randGenerator(){

    game.number=rand()%9999;

    uint32_t anotherNum;

    savedNumber=game.number;

    uprintf("%d\n\r", game.number);

      randomNum.digit3=game.number/1000;
     anotherNum=game.number-randomNum.digit3*1000;

      randomNum.digit2=(anotherNum)/100;
      anotherNum=anotherNum-randomNum.digit2*100;

      randomNum.digit1=(anotherNum)/10;
      anotherNum=anotherNum-randomNum.digit1*10;

      randomNum.digit0=anotherNum;



    uprintf("%d\n\r", randomNum.digit3);

    uprintf("%d\n\r", randomNum.digit2);

    uprintf("%d\n\r", randomNum.digit1);

    uprintf("%d\n\n\r", randomNum.digit0);

}

//callback function for knob
void
knobCallback(uint32_t time)
{

    uint32_t delay=100;

    if(sysState==Run){

            rasRead(dataArray); //reads from the knob and changes the data

            angle1=(dataArray[0]/valuePerAngle); // calculates the value that should appear on the screen, right digits
            angle2=(dataArray[1]/valuePerAngle); // calculates the value that should appear on the screen , left digits
            //takes the calculations in angle1 and angle2 and separates the values of each digit
            // assigns it to the appropriate 7seg display digits

            //Stores the current values in arrays to be used later
            if(angle1/10!=oldValues[1]){
                seg7Display.d2=angle1/10;
            }

            if(angle1%10!=oldValues[0]){
                seg7Display.d1=angle1%10;
            }

            if(angle2/10!=oldValues[3])
            {
                seg7Display.d4=angle2/10;
            }

            if(angle2%10!=oldValues[2]){
                seg7Display.d3=angle2%10;
            }


            //Removes the flickering
            oldValues[1]=seg7Display.d2;
            oldValues[0]=seg7Display.d1;

            oldValues[3]=seg7Display.d4;
            oldValues[2]=seg7Display.d3;

            // if all digits match and the user pressed SW2 then the number guessed is correct
            if(oldValues[0]==randomNum.digit0 && oldValues[1]==randomNum.digit1 && oldValues[2]==randomNum.digit2 && oldValues[3]==randomNum.digit3 && confirm)
            {
                uprintf("%s\r\n", "THE VALUE MATCHED!");
                confirm=false;
                game.gameOver=true;
                ledTurnOnOff(false /* red */, false /* blue */, true /* green */);
                waitMs(400);
                correctSound();
            }

            //if the values don't match but the user pressed SW2, the number guessed is wrong
            else if(confirm){
                ledTurnOnOff(true /* red */, false /* blue */, false /* green */);
                waitMs(150);
                wrongSound();

                ledTurnOnOff(false /* red */, true /* blue */, false /* green */);
                confirm=false;
            }

            //checks if the game is over, stops the system
            if(game.gameOver==true){
                sysState=Pause;
                ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
            }

            seg7DigitUpdate(&seg7Display); //updates the display

    }



    schdCallback(knobCallback, time + delay); //schedule callback every 250ms
}

void
checkPushButton(uint32_t time)
{
    uint32_t delay;

            int code = pbRead();

            int sw1PressTime=0;



            switch (code) {
            case 1:

                //allows hold key press option to reset
                while(pbRead()==1){

                    sw1PressTime++;

                    if(sw1PressTime>1000000){
                        sysState=Reset;

                        uprintf("%s\r\n", "System entered a reset mode");
                        delay=250;
                        break;
                    }

                }

         // if it is still not game over, when SW1 is pressed switches to Pause/Run
         if(game.gameOver==false){

                if(sysState==Run){
                    sysState=Pause;

                    ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
                }

                else if(sysState==Pause){
                    sysState=Run;

                    ledTurnOnOff(false /* red */, true /* blue */, false /* green */);
                }

           }

                 //If SW1 is held for few seconds, enters a Reset mode and resets everything
                if(sysState==Reset){

                uprintf("%s\r\n", "SYSTEM RESET");

                sysState=Pause;

                game.gameOver=false;

                game.attempts=0;

                game.timeLeftSeconds=120;

                confirm=false;

                buzzDelay=10;

                randGenerator();

                seg7Display.d1=0;
                seg7Display.d2=0;
                seg7Display.d3=0;
                seg7Display.d4=0;

                seg7DigitUpdate(&seg7Display);



                timer_Display.colon=1;
                timer_Display.d1=0;
                timer_Display.d2=0;
                timer_Display.d3=2;
                timer_Display.d4=0;


                seg7_2_DigitUpdate(&timer_Display);

                ledTurnOnOff(false /* red */, false /* blue */, false /* green */);

                melodyPlayBegin();

                playEndMusic=true;

            }

                delay = 250;                // software debouncing
                break;

            case 2:
                    if(game.gameOver==false){// while the game is still not over, every SW2 pressed means attempt++

                         if(sysState==Run){
                         confirm=true;
                         game.attempts++;

                         //if number of attempts is greater than 5 the game is over
                         if(game.attempts>=6){
                            game.gameOver=true;

                            ledTurnOnOff(false /* red */, false /* blue */, false /* green */);
                         }
                     }
                 }

                     delay=1000;
                     break;

            default:
                delay = 10;
    }

    schdCallback(checkPushButton, time + delay);
}

//*************************************************************************************************

// Main function******************************************************
int main(void)
{
    lpInit();
    seg7Init();
    seg7_2_Init();
    rasInit(); //calls the initializing function for ras
    buzzerPwmInit();

    schdCallback(callbackBuzzerPlay, 1002);

    uprintf("%s\r\n", "GAME CRACK THE CODE");

    srand((int)time(0));

    randGenerator();

    ledTurnOnOff(false /* red */, false /* blue */, false /* green */);

    // Update the clock display
    seg7DigitUpdate(&seg7Display);

    seg7_2_DigitUpdate(&timer_Display);

    melodyPlayBegin();

    schdCallback(knobCallback, 1000);  // call the schedule call back function

    schdCallback(checkPushButton, 1004);

    schdCallback(timerDecrement, 1006);

    // Run the event scheduler to process callback events
    while (true) {
        schdExecute();
    }
}
//**************************************************************************
