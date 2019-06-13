/********************************************************************

   File Name:     sendupdt.cpp
   Author:        nlc
   Date:          09/27/96


   See sendupdt.hpp for information.

********************************************************************/


#include "suppdefs.hpp"
#include "appdoc.hpp"
#include "spglobal.hpp"
#include "sendupdt.hpp"

inline UpdateMessage::UpdateMessage (long      theMessageNumber,
                                     void    * theData,
                                     CWindow * theOriginatingWindow)
                     :itsMessageNumber    (theMessageNumber),
                      itsData             (theData),
                      itsOriginatingWindow(theOriginatingWindow)
{}

void SendUpdateMessage (long      theMessageNumber,
                        void    * theData,
                        CWindow * theOriginatingWindow)
{
   if (theMessageNumber != NULLUpdateMessage)
   {
      UpdateMessage aMessage = UpdateMessage(theMessageNumber,
                                             theData,
                                             theOriginatingWindow);

      // note that the app doc's DoCommand will not send the message to
      // the originating window if it is a non null pointer.  Leave it
      // null if you want the message to be sent to theOriginatingWindow.

      theSpGlobals->theAppDocument->DoCommand(SENDUpdateCommand,
                                              &aMessage);
   }
}

