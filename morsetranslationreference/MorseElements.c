/****************************************************************************
 Module
   MorseElements.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MorseElements.h"
#include "HALS/PIC32PortHAL.h"
/*----------------------------- Module Defines ----------------------------*/
#define INPUT_PORT _Port_B
#define INPUT_PIN _Pin_2
#define INPUT_VAL PORTBbits.RB2

#define DASH_MULT 3 // Dash length is 3x LengthofDot
#define EOC_MULT 3 // Character space is 3x LengthOfDot
#define EOW_MULT 7 // Word space is 7x LengthOfDot

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void TestCalibration(void);
static void CharacterizeSpace(void);
static void CharacterizePulse(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static MorseElementsState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint16_t TimeOfLastRise;
static uint16_t TimeOfLastFall;
static uint16_t LengthOfDot;
static uint16_t FirstDelta;

static bool LastInputState;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMorseElements

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
***************************************/
bool InitMorseElements(uint8_t Priority)
{
    puts("Initializing MorseElements SM\n\r");
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    
    //Initialize the input line to receive Morse code
    //Return false if configure fails
    if(false == PortSetup_ConfigureDigitalInputs(INPUT_PORT, INPUT_PIN))
    {
        puts("Error: MorseElements failed to Configure Input Pin\n\r");
        return false; 
    }
    //Sample port line and use it to initialize the LastInputState variable
    LastInputState = (bool) INPUT_VAL;
    //Set CurrentState to be InitMorseElementsState
    CurrentState = InitMorseElementsState;
    //Set FirstDelta to 0
    FirstDelta = 0;
    //Call InitButtonStatus to initialize local var in button event checking module
    InitButtonStatus();
    
    // Post Event ES_Init to MorseElements queue (this service)
    ThisEvent.EventType = ES_INIT;
    if (PostMorseElements(ThisEvent) == true)
    {
        return true;
    }
    else
    {
        puts("Error: MorseElements failed to Initialize correctly\n\r");
        return false;
    }

}

/****************************************************************************
 Function
     PostMorseElements

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMorseElements(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMorseElements

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunMorseElementsSM(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    // If nextstate isn't explicitly set in function, want it to stay currstate  
    MorseElementsState_t NextState = CurrentState;
    
    //DB_printf("MorseElements: Event = %u \t State = %u \r\n", ThisEvent.EventType, CurrentState);
    
    switch (CurrentState)
    {
        case InitMorseElementsState:// If current state is initial Psedudo State
        {
            if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
            {
                //Change to DebugState for testing CalWaitForRise
                NextState = CalWaitForRise;
                puts("MorseElements: ES_INIT\r\n");
            }
        }
        break;
        
        case CalWaitForRise: 
        {
            if (ThisEvent.EventType == RisingEdge)
            {
                // Set TimeOfLastRise to Time from event parameter
                TimeOfLastRise = ThisEvent.EventParam;
                NextState = CalWaitForFall;
            }
            else if (ThisEvent.EventType == CalCompleted)
            {
                DB_printf("MorseElements: Calibration Completed. DotLength = %u \r\n", LengthOfDot);
                DM_ClearDisplayBuffer(); // clear display buffer when calcomplete
                NextState = EOC_WaitRise;
            }
        }
        break;

        case CalWaitForFall:
        {
            if (ThisEvent.EventType == FallingEdge)
            {
                // Set TimeOfLastFall to Time from event parameter
                TimeOfLastFall = ThisEvent.EventParam;
                NextState = CalWaitForRise;
                TestCalibration(); // test for length of dot     
            }
        }
        break;
 
        case EOC_WaitRise:
        {
            if (ThisEvent.EventType == RisingEdge)
            {
                // Set TimeOfLastRise to Time from event parameter
                TimeOfLastRise = ThisEvent.EventParam;
                NextState = EOC_WaitFall;
                puts("EOC_WaitRise: RisingEdge \r\n");
                CharacterizeSpace();
            }
            else if (ThisEvent.EventType == ButtonDown) // re-calibrate
            {
                NextState = CalWaitForRise;
                FirstDelta = 0;
            }
        }
        break;
        
        case EOC_WaitFall:
        {
            if (ThisEvent.EventType == FallingEdge)
            {
                // Set TimeOfLastFall to Time from event parameter
                TimeOfLastFall = ThisEvent.EventParam;
                NextState = EOC_WaitRise;
            }
            else if (ThisEvent.EventType == ButtonDown) // re-calibrate
            {
                NextState = CalWaitForRise;
                FirstDelta = 0;
            }
            else if (ThisEvent.EventType == EOCDetected)
            {
                //Set NextState to DecodeWaitFall
                NextState = DecodeWaitFall;
            }
        }
        break;
        
        case DecodeWaitRise:
        {
            if (ThisEvent.EventType == RisingEdge)
            {
                // Set TimeOfLastRise to Time from event parameter
                TimeOfLastRise = ThisEvent.EventParam;
                NextState = DecodeWaitFall;
                CharacterizeSpace();
            }
            else if (ThisEvent.EventType == ButtonDown) // re-calibrate
            {
                NextState = CalWaitForRise;
                FirstDelta = 0;
            }
        }
        break;
        
        case DecodeWaitFall:
        {
            if (ThisEvent.EventType == FallingEdge)
            {
                // Set TimeOfLastFall to Time from event parameter
                TimeOfLastFall = ThisEvent.EventParam;
                NextState = DecodeWaitRise;
                CharacterizePulse(); // check the pulse type
            }
            else if (ThisEvent.EventType == ButtonDown) // re-calibrate
            {
                NextState = CalWaitForRise;
                FirstDelta = 0;
            }
        }
        break;
        
        case DebugState:
        {
            if (RisingEdge == ThisEvent.EventType) // If rising edge detected
            {
                DB_printf("MorseElements: Received RisingEdge at Time=%u. Writing ->R<- to display\n\r", ThisEvent.EventParam);
                //Add an R to the display for now
                PostEvent.EventType = ES_ADD_CHAR;
                PostEvent.EventParam = (uint16_t) 'R';
                PostDisplayFSM(PostEvent);
            }
            else if (FallingEdge == ThisEvent.EventType) // If falling edge detected
            {
                DB_printf("MorseElements: Received FallingEdge at Time=%u. Writing ->F<- to display\n\r", ThisEvent.EventParam);
                //Add an F to the display for now
                PostEvent.EventType = ES_ADD_CHAR;
                PostEvent.EventParam = (uint16_t) 'F';
                PostDisplayFSM(PostEvent);
            }
            else if (ButtonDown == ThisEvent.EventType)
            {
                puts("MorseElements: Received ButtonDown.\n\r");
            } 
            else if (ButtonUp == ThisEvent.EventType)
            {
                puts("MorseElements: Received ButtonUp.\n\r");
            }   
        }
        break;
        // repeat state pattern as required for other states
        default:
            ;
    }                                   // end switch on Current State
    
    CurrentState = NextState; // update current state
    return ReturnEvent;
}

/****************************************************************************
 Function
     QueryTemplateSM

 Parameters
     None

 Returns
     TemplateState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
MorseElementsState_t QueryMorseElements(void)
{
    return CurrentState;
}

/****************************************************************************
Function
 * CheckMorseEvents
Parameters
 * None
Returns
 * True if an event was posted
Description
 * Checks if the input line changes state and, if so,
 * posts [Rising/Falling]Edge event with the current time as parameter 
Notes
 * 
Author
 * Andrew Sack
****************************************************************************/
bool CheckMorseEvents(void)
{
    bool ReturnVal = false;
    bool CurrentInputState;
    ES_Event_t PostEvent;
    uint16_t CurrentTime;
    
    // Get the CurrentInputState from the input line
    CurrentInputState = (bool) INPUT_VAL;
    // If the state of the Morse input line has changed
    if (CurrentInputState != LastInputState)
    {
        ReturnVal = true;
        CurrentTime = ES_Timer_GetTime(); // read current time
        if (1 == CurrentInputState) //Current state hi
        {
            //puts("MorseElements: RisingEdge detected\n\r");
            // Set post event type to RisingEdge
            PostEvent.EventType = RisingEdge;
        }
        else // current state low
        {
            //puts("MorseElements: FallingEdge detected\n\r");
            // Set post event type to FallingEdge
            PostEvent.EventType = FallingEdge; 
        }
        // PostEvent to this FSM with param of Current Time
        PostEvent.EventParam = CurrentTime;
        PostMorseElements(PostEvent);
    }
    LastInputState = CurrentInputState;
         
    return ReturnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
Function
 * TestCalibration
Parameters
 * None
Returns
 * None
Description
 * Compares 2 pulse lengths to find length of dot
Notes
 * 
Author
 * Andrew Sack
****************************************************************************/
static void TestCalibration(void)
{
    uint16_t SecondDelta;
    ES_Event_t PostEvent;
    // If calibration is just starting
    if (0 == FirstDelta)
    {
        // Set FirstDelta to most recent pulse width
        FirstDelta = TimeOfLastFall - TimeOfLastRise;
        
        
        // Set display to indicate calibration happening
        DM_ClearDisplayBuffer(); // clear display buffer when calibrating
        //Write an ! so we know it's calibrating
        PostEvent.EventType = ES_ADD_CHAR;
        PostEvent.EventParam = (uint16_t) '!';
        PostDisplayFSM(PostEvent);
        
    } else {
        // Set SecondDelta to most recent pulse width
        SecondDelta = TimeOfLastFall - TimeOfLastRise;
        // If (100.0 * FirstDelta / SecondDelta) less than or equal to 33.33
        if (33.33 >= (100.0 * FirstDelta / SecondDelta))
        {
            // Save FirstDelta as LengthOfDot
            LengthOfDot = FirstDelta;
            // PostEvent CalCompleted to MorseElementsSM
            PostEvent.EventType = CalCompleted;
            PostMorseElements(PostEvent);
        }
        // ElseIf (100.0 * FirstDelta / Second Delta) greater than 300.0
        else if (300.0 < (100.0 * FirstDelta / SecondDelta))
        {
            // Save SecondDelta as LengthOfDot
            LengthOfDot = SecondDelta;
            // PostEvent CalCompleted to MorseElementsSM
            PostEvent.EventType = CalCompleted;
            PostMorseElements(PostEvent);
        }
        else // Prepare for next pulse
        {
            FirstDelta = SecondDelta;
        }
    }
    return;
}

/****************************************************************************
Function
 * CharacterizeSpace
Parameters
 * None
Returns
 * None
Description
 * Characterizes space in signal as EOC, EOW, or BadSpace 
 * and posts to MorseDecode
Notes
 * 
Author
 * Andrew Sack
****************************************************************************/
static void CharacterizeSpace(void)
{
    ES_Event_t PostEvent;
    uint16_t LastInterval;
    
    LastInterval = TimeOfLastRise - TimeOfLastFall;
    
    // if Last Interval not a valid dot space
    if ((LastInterval < LengthOfDot) || (LastInterval > (LengthOfDot+1)))
    {
        // if LastInterval is a valid CharacterSpace
        if ((LastInterval >= (LengthOfDot * EOC_MULT)) && 
                (LastInterval <= ((LengthOfDot+1) * EOC_MULT)))
        {
            PostEvent.EventType = EOCDetected;
            // Post to both MorseElements and MorseDecode
            ES_PostList00(PostEvent); 
            
        }
        else // could be EOW or Invalid
        {
            // if LastInterval is a valid WordSpace
            if ((LastInterval >= (LengthOfDot * EOW_MULT)) && 
                (LastInterval <= ((LengthOfDot+1) * EOW_MULT)))
            {
                PostEvent.EventType = EOWDetected;
                PostMorseDecode(PostEvent);
            }
            else // invalid space
            {
                PostEvent.EventType = BadSpace;
                PostMorseDecode(PostEvent);
            }
        }
    }
            
    
    return;
}

/****************************************************************************
Function
 * CharacterizePulse
Parameters
 * None
Returns
 * None
Description
 * Characterizes pulse in signal as dot, dash, or BadPulse 
 * and posts to MorseDecode 
Notes
 * 
Author
 * Andrew Sack
***************************************************/
static void CharacterizePulse(void)
{
    uint16_t LastPulseWidth;
    ES_Event_t PostEvent;
    
    LastPulseWidth = TimeOfLastFall - TimeOfLastRise;
    // if LastPulseWidth ok for a dot
    if ((LastPulseWidth >= LengthOfDot) && (LastPulseWidth <= (LengthOfDot+1)))
    {
        //Post DotDetected to MorseDecode Service
        PostEvent.EventType = DotDetected;
        PostMorseDecode(PostEvent);
    }
    //elseif LastPulseWidth ok for a Dash
    else if ((LastPulseWidth >= (LengthOfDot * DASH_MULT)) && 
            (LastPulseWidth <= ((LengthOfDot+1) * DASH_MULT)))
    {
        //Post DashDetected to MorseDecode Service
        PostEvent.EventType = DashDetected;
        PostMorseDecode(PostEvent);
    }
    else //Bad pulse length
    {
        //Post BadPulse to MorseDecode Service
        PostEvent.EventType = BadPulse;
        PostMorseDecode(PostEvent);
    }
    
    return;
}