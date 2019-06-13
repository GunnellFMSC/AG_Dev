/********************************************************************

   File Name:     sendupdt.hpp
   Author:        nlc
   Date:          09/27/96

   SendUpdateMessage

   SendUpdateMessage is used to let all windows know when
   some event has occurred that needs to be broadcast.
   The broadcast is done by creating an instance of this
   a local class and calling the document's DoCommand method
   with SENDUpdateCommand.  Note that the documents do command never
   sends the message to theOriginatingWindow, if it is non-null.

********************************************************************/

#ifndef SendUpdateMessage_H
#define SendUpdateMessage_H

#include "suppdefs.hpp"

void SendUpdateMessage (long      theMessageNumber     = NULLUpdateMessage,
                        void    * theData              = NULL,
                        CWindow * theOriginatingWindow = NULL);

class UpdateMessage
{
   public:

      UpdateMessage (long theMessageNumber,
                     void    * theData,
                     CWindow * theOriginatingWindow);
      long      itsMessageNumber;
      void    * itsData;
      CWindow * itsOriginatingWindow;
};


#endif

