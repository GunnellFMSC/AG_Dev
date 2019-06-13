/********************************************************************

   File Name:     runwizw.hpp
   Author:        nlc
   Date:          07/18/96

   Class:         RunWizardWin
   Inheritance:   CWindow -> RunWizardWin

   Purpose:       Provide a set of steps in accomplishing general
                  tasks.

*******************************************************************/

#ifndef runwizw_h
#define runwizw_h

#include "PwrDef.h"
#include "appdef.hpp"
#include CWindow_i
#include <rw/ordcltn.h>

class NButton;
class NListButton;
class RWOrdered;
class NScrollText;
class MSText;
class MyCScroller;
class CNavigator;

#include "cwwindow.hpp"


//------------------------------------------------------------------

class RunWizardWin : public CWwindow
{
  public:

      RunWizardWin (CDocument *theDocument);
      virtual ~RunWizardWin(void);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual void SizeWindow(int theWidth, int theHeigth);

  private:

      void LoadGeneralTaskList( void );
      void LoadScroller( void );

      CNavigator        * itsNavigator;
      MSText            * itsMSText;
      NButton           * itsCloseButton;
      NListButton       * itsGeneralTaskListBox;
      RWOrdered           itsGeneralTaskList;
      MyCScroller       * itsScrollingWindow;
};

#endif
