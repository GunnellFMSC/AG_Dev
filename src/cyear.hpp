/********************************************************************

  File Name:     cyear.hpp
   Author:        kfd
   Date:          09/07/94

   Class:         CYear
   Inheritance:   CWindow-> CYear

   Purpose:       In this class, a CWindow dialog window contains three
                  LineBoxes and two NScrollBars.  One of the member functions
                  passes a current year string and constructs the window.
                  The value returned is whatever the user chooses as the
                  selected year.  The initial values in the LineBoxes are
                  obtained from a SelectedYearObj.

   Contents:      CYear()
                  CYear::Year()
                  CYear::HScroll
                  CYear::VScroll
                  CYear::DoCommand
                  CYear::SpSetScrollControls

********************************************************************/

#ifndef CYear_H
#define CYear_H

#include "PwrDef.h"
#include CWindow_i

class NButton;
class NScrollBar;
class NEditControl;
class LineBox;

class CYear : public CWindow
{
   public:

      CYear(CStringRW* theYear, const CStringRW& callersYear);
      friend CStringRW Year(const CStringRW& callersYear);
      virtual void HScroll(SCROLL_CONTROL theEventType, UNITS thePosition);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      int ValidateCharacters(NEditControl* itsString);
      virtual void SpSetScrollControls(void);

   private:

      NButton* itsCancel;
      NButton* itsOKButton;
      int startYear;
      int selectedYear;
      int interval;
      CStringRW* itsResponse;
      CStringRW itsCallersYear;
      NEditControl* itsSelectedYear;
      NScrollBar* itsYearBar;

};

#endif //CYear_H
