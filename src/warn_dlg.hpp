/********************************************************************

   File Name:     warn_dlg.hpp
   Author:        kdf & nlc
   Date:          11/29/94 and 05/01/97

   Class:         WarnWin
   Inheritance:   CWindow

   Purpose:       Put up a warning dialog that can contain many many
                  lines.
 ********************************************************************/

#ifndef Warn_dlg_H
#define Warn_dlg_H

#include "PwrDef.h"
#include CWindow_i

class WarnWin : public CWindow
{
   public:

      friend void Warn(const char* theText);

   private:

      WarnWin(const char*  theText, int aLineCnt);
      virtual void DoCommand(long theCommand,void* theData=NULL);

};

#endif 






