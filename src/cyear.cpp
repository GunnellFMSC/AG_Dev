/********************************************************************

   File Name:     cyear.cpp
   Author:        kfd
   Date:          09/07/94


   Contents:      CYear()
                  CYear::Year()
                  CYear::HScroll
                  CYear::VScroll
                  CYear::DoCommand
                  CYear::SpSetScrollControls

********************************************************************/
#include "appdef.hpp"

#include NButton_i
#include CText_i
#include CApplication_i
#include CDesktop_i
#include CStringRW_i
#include CPoint_i
#include NText_i
#include NScrollBar_i
#include NEditControl_i
#include CUnits_i
#include CWindow_i
#include CNavigator_i

#include "cyear.hpp"
#include "spfunc2.hpp"
#include "suppdefs.hpp"
#include "cmngwin.hpp"
#include "yearobj.hpp"
#include "msgwin.hpp"
#include "warn_dlg.hpp"
#include "spglobal.hpp"
#include "supphelp.hpp"
#include "spfuncs.hpp"
#include <rw/vstream.h>
#include <rw/cstring.h>

extern XVT_HELP_INFO helpInfo;

// This function puts up a "dialog box" and returns when the user
// presses ok.  Input is a string which contains the current year on
// the calling screen.  Year returns the string which is entered in
// itsSelectedYearBox box.

CStringRW Year(const CStringRW& callersYear)
{
                                 // Create the window:
   CStringRW theYear;
   CYear* YearWindow = new CYear(&theYear, callersYear);
   YearWindow->DoModal();

                                 // Window is gone, return the selected year
   return theYear;
}


CYear::CYear(CStringRW* theYear, const CStringRW& callersYear)
      :CWindow(theSpGlobals->theAppDocument,
               CRect(138,115,627,244), "Select Year",
               WSF_DEFER_MODAL, W_MODAL),
       itsResponse(theYear)
{

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

                  // save current year in case user presses "cancel"
   itsCallersYear = callersYear;


                  // get saved values from their object
   startYear    = theSpGlobals->yearObj->GetInvYear(1);
   if (startYear < 0) startYear = theSpGlobals->yearObj->GetStartYear();
   selectedYear = theSpGlobals->yearObj->GetSelectedYear();
   interval     = 450;

                  // build year scrollbar
   itsYearBar = new NScrollBar(this, CRect(8,16,480,32), HORIZONTAL);
   itsYearBar->IScrollBar(startYear,startYear+interval,selectedYear,1,10);


                  // build selectedYear box
   new NText(this, CRect(304,48,424,72), "Selected year:",0);
   itsSelectedYear = new NEditControl(this,
                                      CRect(416,40,480,72),
                                      ConvertToString(theSpGlobals->yearObj->
                                                      GetSelectedYear()),
                                      CTL_FLAG_RIGHT_JUST);
   itsSelectedYear->SetFocusCommands(6,7);
   itsSelectedYear->SetTextCommand(8);

                  // build buttons
   itsOKButton = new NButton(this, CRect(8,88,88,120), "OK");
   itsOKButton->SetCommand(1);

   itsCancel = new NButton(this, CRect(400,88,480,120), "Cancel");
   itsCancel->SetCommand(2);

   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), SCHYEARDialog, 0L);

   aNav->AppendTabStop(new CTabStop(itsOKButton));
   aNav->AppendTabStop(new CTabStop(itsCancel));
   aNav->AppendTabStop(new CTabStop(itsSelectedYear));
   aNav->AppendTabStop(new CTabStop(itsYearBar));
   aNav->InitFocus();
}




void CYear::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:                    // OK Button  was pressed.
      {
                                 // see if this is a valid year
         int selectedYr = CheckRange(itsSelectedYear->GetTitle(),
                              startYear,startYear+interval);
         if (selectedYr == INT_MAX)
            return;
                                 // save selected year
         theSpGlobals->yearObj->SetSelectedYear((int)
             ConvertToLong(itsSelectedYear->GetTitle()));

                                 // if selected year >= endYear
                                 // revise endYear and save it
         if (theSpGlobals->yearObj->GetSelectedYear() >=
             theSpGlobals->yearObj->GetEndYear())
             theSpGlobals->yearObj->SetEndYear(
               selectedYr + min(startYear+interval-selectedYear, 10));

         *itsResponse = itsSelectedYear->GetTitle();
         Close();
         break;
      }

      case 2:                    //  Cancel Button  was pressed.
                                 // return the old selectedYear
         *itsResponse = itsCallersYear;
         Close();
         break;


      case 6:                    // itsSelectedYear has focus
         break;

      case 7:                    // itsSelectedYear lost focus
                                 // get the year and update scrollers
         sscanf(itsSelectedYear->GetTitle(),"%d", &selectedYear);
         SpSetScrollControls();
         break;

      case 8:                    // itsSelectedYear got a keystroke
         ValidateCharacters(itsSelectedYear);
         break;

      default:
         CWindow::DoCommand(theCommand,theData);
   }
}




void CYear::HScroll(SCROLL_CONTROL theEventType, UNITS thePosition)
{
//  HScroll overrides NScrollBar::HScroll.  itsYearBar is the only scroller
//  which uses this method.  HScroll converts the thumb position to integer
//  and UNITS. It writes the year to the text control; resets the other
//  scrollbar; and updates the private variable selectedYear.

                              // get the thumb position on the scroll bar
      int tmp = (int) itsYearBar->GetPosition();

                              // copy thumb position into text area
      itsSelectedYear->SetTitle(ConvertToString(tmp));
      selectedYear = tmp;
}




void CYear::SpSetScrollControls()
{
//  SpSetScrollControls sets the range and position for the object's
//  scroll bars.  It uses private data: startYear, interval, selectedYear

                                 // set control ranges
   itsYearBar->SetRange(startYear,startYear+interval,selectedYear);

                                 // position the thumb bars
   itsYearBar->SetPosition(selectedYear);
}



int CYear::ValidateCharacters(NEditControl* itsEntry)
{
                // Validate the text entered.  Accept 0-9, arrow keys + others

   int theLength = (itsEntry->GetTitle()).length();

                // Validate the text entered.  Accept 0-9
   int theChar;
   for(size_t i = 0; i < theLength; i++)
   {
      theChar = (itsEntry->GetTitle())[i];
      if( theChar < 48 || theChar > 57)     // found a non numbers
      {
         xvt_scr_beep();
         CStringRW tmp(itsEntry->GetTitle());
         itsEntry->SetTitle(tmp.remove(i,1));
         return 0;
      }
   }

                              // accept only 4 digit years
   if (theLength  > 4)
   {
      xvt_dm_post_warning("A valid year must be 4 digits.");
      CStringRW tmp(itsEntry->GetTitle());
      itsEntry->SetTitle(tmp.remove(4));      // remove chars (pos 4 - end)
      return 0;
   }

                              // update the scroll bar if a valid year
   if (theLength == 4)
   {
      int aYear = CheckRange(itsEntry->GetTitle(),
              startYear,startYear+interval);
      if (aYear != INT_MAX)
         SpSetScrollControls();
   }

   return 0;
}
