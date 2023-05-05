/*
 * @Author:Rajesh Srirangam,Rajesh.Srirangam@colorado.edu
 * @File Name:timer.c
 * @File Description:Used for LETIMER0 Initialisation
 * @Reference:David Sluiter Lecture 4,Lecture 5 Slides
 *
 */

/*********HEADERS************/
#include <stdio.h>
#include <stdbool.h>
#include "em_letimer.h"
#include "em_cmu.h"
#include "timer.h"
#include "oscillators.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
//#include "app.h"
/*****************************/

/******************************
 * @Function void init_LETIMER0()
 * @Param NULL
 * @Return NULL
 * @Function Description Used for LETIMER0 Timer Initialisation
 */
void init_LETIMER0() {
  uint32_t temp=0x00;
  // this data structure is passed to LETIMER_Init (), used to set LETIMER0_CTRL reg bits and other values
  const LETIMER_Init_TypeDef letimerInitData = {
      .enable=false, // enable; don't enable when init completes, we'll enable last
      .debugRun=true, // debugRun; useful to have the timer running when single-stepping in the debugger
      .comp0Top=true, // comp0Top; load COMP0 into CNT on underflow
      .bufTop=false, // bufTop; don't load COMP1 into COMP0 when REP0==0
      .out0Pol=0, // out0Pol; 0 default output pin value
      .out1Pol=0, // out1Pol; 0 default output pin value
      .ufoa0=letimerUFOANone, // ufoa0; no underflow output action
      .ufoa1=letimerUFOANone, // ufoa1; no underflow output action
      .repMode=letimerRepeatFree, // repMode; free running mode i.e. load & go forever
      .topValue=COMP0_VALUE_TO_LOAD // COMP0(top) Value, I calculate this below
  };

  // init the timer
  LETIMER_Init (LETIMER0, &letimerInitData);
  // calculate and load COMP0 (top)
  LETIMER_CompareSet(LETIMER0, 0, COMP0_VALUE_TO_LOAD);    // COMP0
  // calculate and load COMP1
  //LETIMER_CompareSet(LETIMER0, 1,COMP1_VALUE_TO_LOAD); // COMP1
  // Clear all IRQ flags in the LETIMER0 IF status register
  LETIMER_IntClear (LETIMER0, 0xFFFFFFFF); // punch them all down
  // Set UF and COMP1 in LETIMER0_IEN, so that the timer will generate IRQs to the NVIC.
  temp = LETIMER_IEN_UF ;
  //| LETIMER_IEN_COMP1;
  LETIMER_IntEnable (LETIMER0, temp); // Make sure you have defined the ISR routine LETIMER0_IRQHandler()
  // Enable the timer to starting counting down, set LETIMER0_CMD[START] bit, see LETIMER0_STATUS[RUNNING] bit
  LETIMER_Enable (LETIMER0, true);
  // Test code:
  // read it a few times to make sure it's running within the range of values we expect
  //temp = LETIMER_CounterGet(LETIMER0);
  //temp = LETIMER_CounterGet(LETIMER0);
  //temp = LETIMER_CounterGet(LETIMER0);
} // initLETIMER0 ()

