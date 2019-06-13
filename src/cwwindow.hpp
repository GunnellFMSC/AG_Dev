/********************************************************************

   File Name:    cwwindow.hpp
   Author:       kfd & nlc
   Date:         1994-1996

 ********************************************************************/

#ifndef CWwindow_H
#define CWwindow_H

#include "PwrDef.h"
#include "suppdefs.hpp"

#include CTypeInfo_i
#include CWindow_i

class CWwindow : public CWindow
{
   public:

      CWwindow(CDocument *theDocument,
               const CRect& theRegion,
               const CStringRW&         = NULLString,
               long theWindowAttributes = WSF_NONE,
               WIN_TYPE theWindowType   = W_DOC,
               int theMenuBarId         = TASK_MENUBAR);

     virtual BOOLEAN Close (void);

     virtual void DoCommand(long theCommand,void* theData);
     virtual void DoKey(const CKey& theKey );

     PWRClassInfo
};
#endif //CWwindow_H
