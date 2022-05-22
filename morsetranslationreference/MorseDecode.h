/****************************************************************************

  Header file for Morse Decode service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef MorseDecode_H
#define MorseDecode_H

#include "ES_Types.h"
#include "DisplayFSM.h"
// Public Function Prototypes

bool InitMorseDecode(uint8_t Priority);
bool PostMorseDecode(ES_Event_t ThisEvent);
ES_Event_t RunMorseDecode(ES_Event_t ThisEvent);

#endif /* MorseDecode_H */

