/********************************************************************

   File Name:     timescal.hpp
   Author:        kfd
   Date:          01/09/95

   Class:         TimeScale
   Inheritance:   TimeScale->CWindow

   Purpose:       In this class, a CWindow dialog window contains three
                  LineBoxes and two NScrollBars.  One of the member functions
                  passes a current year string and constructs the window.
                  The value returned is whatever the user chooses as the
                  selected year.  The initial values in the LineBoxes are
                  obtained from a SelectedYearObj.

********************************************************************/

#ifndef TimeScale_H
#define TimeScale_H

#include "PwrDef.h"
#include CWindow_i

class NButton;
class NScrollBar;
class NEditControl;
class LineBox;

class TimeScale : public CWindow
{
   public:

      TimeScale(void);
      friend void SetTimeScale(void);
      virtual void HScroll(SCROLL_CONTROL theEventType, UNITS thePosition);
      virtual void VScroll(SCROLL_CONTROL theEventType, UNITS thePosition);
      virtual void DoCommand(long theCommand,void* theData=NULL);

   private:

      NButton       * itsCancelButton;
      NButton       * itsOKButton;
      NButton       * itsResetButton;
      NEditControl  * cycleLengthEdit;
      NEditControl  * startYearEdit;
      NScrollBar    * startYearBar;
      NEditControl  * endYearEdit;
      NScrollBar    * endYearBar;

};

#endif //TimeScale_H
