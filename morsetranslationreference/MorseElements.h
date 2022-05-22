/****************************************************************************

  Header file for Morse Elements Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef MorseElements_H
#define MorseElements_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ButtonEventChecker.h"
#include "DisplayFSM.h"
#include "MorseDecode.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    InitMorseElementsState = 0, CalibrationCompleted, 
    CalWaitForRise, CalWaitForFall,
    EOC_WaitRise, EOC_WaitFall,
    DecodeWaitRise, DecodeWaitFall,
    DebugState
} MorseElementsState_t;

// Public Function Prototypes

bool InitMorseElements(uint8_t Priority);
bool PostMorseElements(ES_Event_t ThisEvent);
ES_Event_t RunMorseElementsSM(ES_Event_t ThisEvent);
MorseElementsState_t QueryMorseElements(void);

bool CheckMorseEvents(void); // Event Checker function

#endif /* MorseElements_H */

