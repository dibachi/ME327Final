/****************************************************************************
 Module
   MorseDecode.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MorseDecode.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
#define MORSE_LEN 8
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
char DecodeMorseString(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static char MorseString[MORSE_LEN];
static uint8_t MorseIdx;

static char legalChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890?.,:'-/()\"= !$&+;@_";
static char morseCode[][MORSE_LEN] ={ ".-","-...","-.-.","-..",".","..-.","--.",
                      "....","..",".---","-.-",".-..","--","-.","---",
                      ".--.","--.-",".-.","...","-","..-","...-",
                      ".--","-..-","-.--","--..",".----","..---",
                      "...--","....-",".....","-....","--...","---..",
                      "----.","-----","..--..",".-.-.-","--..--",
                      "---...",".----.","-....-","-..-.","-.--.-",
                      "-.--.-",".-..-.","-...-","-.-.--","...-..-",
                      ".-...",".-.-.","-.-.-.",".--.-.","..--.-"
                     };

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMorseDecode

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
 
InitMorseDecode
Takes a priority number, returns True. 

Initialize the MyPriority variable with the passed in parameter.
Clear (empty) the MorseString variable
End of InitMorseDecode

****************************************************************************/
bool InitMorseDecode(uint8_t Priority)
{
    //Set MyPriority
    MyPriority = Priority;
    //clear MorseString
    memset(MorseString,0, MORSE_LEN); // empty MorseString
    MorseIdx = 0; // reset index to 1st position
    
    return true;
}

/****************************************************************************
 Function
     PostMorseDecode

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMorseDecode(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMorseDecode

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunMorseDecode(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ES_Event_t PostEvent;
    char DecodedChar;
    
    switch (ThisEvent.EventType)
    {
        case DotDetected:
        {
            //puts("MorseDecode: Dot detected\r\n");
            // if there is room in the MorseString
            if (MorseIdx < (MORSE_LEN-1))
            {
                // Insert a dot and increment idx
                MorseString[MorseIdx] = '.';
                MorseIdx++;
            }
            else // Error occurred
            {
                ReturnEvent.EventType = ES_ERROR;
                ReturnEvent.EventParam = DotDetected;
            }
        }
        break;
        
        case DashDetected:
        {
            //puts("MorseDecode: Dash detected\r\n");
            // if there is room in the MorseString
            if (MorseIdx < (MORSE_LEN-1))
            {
                // Insert a dash and increment idx
                MorseString[MorseIdx] = '-';
                MorseIdx++;
            }
            else // Error occurred
            {
                ReturnEvent.EventType = ES_ERROR;
                ReturnEvent.EventParam = DashDetected;
            }
        }
        break;
        case EOCDetected:
        {
            DecodedChar = DecodeMorseString(); // decode the morse string
            
            //write decoded char to display
            PostEvent.EventType = ES_ADD_CHAR;
            PostEvent.EventParam = (uint16_t) DecodedChar; 
            PostDisplayFSM(PostEvent);
            DB_printf("MorseDecode: EOC detected with char %c\r\n", DecodedChar);
            
            //Clear MorseString
            memset(MorseString,0, MORSE_LEN); // empty MorseString
            MorseIdx = 0;
        }
        break;
        case EOWDetected:
        {
            DecodedChar = DecodeMorseString(); // decode the morse string
            
            //write decoded char to display
            PostEvent.EventType = ES_ADD_CHAR;
            PostEvent.EventParam = (uint16_t) DecodedChar; 
            PostDisplayFSM(PostEvent);
            DB_printf("MorseDecode: EOW detected with char %c\r\n", DecodedChar);
            
            //write a space to display
            PostEvent.EventType = ES_ADD_CHAR;
            PostEvent.EventParam = (uint16_t) ' '; 
            PostDisplayFSM(PostEvent);
            
            //Clear MorseString
            memset(MorseString,0, MORSE_LEN); // empty MorseString
            MorseIdx = 0;
        }
        break;
        case BadSpace:
        {
            puts("MorseDecode: BadSpace detected\r\n");
            //Clear MorseString
            memset(MorseString,0, MORSE_LEN); // empty MorseString
            MorseIdx = 0;
        }
        break;
        case BadPulse:
        {
            puts("MorseDecode: BadPulse detected\r\n");
            //Clear MorseString
            memset(MorseString,0, MORSE_LEN); // empty MorseString
            MorseIdx = 0;
        }
        break;
        case ButtonDown: // want to clear MorseString when calibrating
        {
            puts("MorseDecode: ButtonDown detected\r\n");
            //Clear MorseString
            memset(MorseString,0, MORSE_LEN); // empty MorseString
            MorseIdx = 0;
        }
        break;
        default:
        {}
        break;
        
    }
    
    
    return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
Function
 * DecodeMorseString
Parameters
 * None
Returns
 * Char that string corresponds to or '~' if it is a failure
Description
 * Compares MorseString to legal morse codes to determine its meaning
Notes
 * 
Author
 * Andrew Sack
****************************************************************************/
char DecodeMorseString(void)
{
    DB_printf("DecodeMorseString: MorseString = %s\r\n", MorseString);
    char MorseChar = '~';// ~ is the failure character
    
    uint8_t idx;
    // iterate through all the legal chars
    for (idx = 0; idx < ARRAY_SIZE(legalChars); idx++)
    {
        // if strings are identical
        if (0 == strcmp(MorseString, morseCode[idx]))
        {
            //set MorseChar to corresponding char and break
            MorseChar = legalChars[idx];
            break;
        }
    }
    return MorseChar;  
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

