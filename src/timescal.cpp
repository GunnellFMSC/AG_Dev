/********************************************************************

   File Name:     timescal.cpp
   Author:        kfd & nlc (major overhaul)
   Date:          01/09/95 02/06/96


   Contents:      TimeScale()
                  TimeScale::SetTimeScale()
                  TimeScale::HScroll
                  TimeScale::DoCommand
                  TimeScale::SetStartYear

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

#include "timescal.hpp"
#include "appdoc.hpp"
#include "suppdefs.hpp"
#include "cmngwin.hpp"        /* for CheckRange  */
#include "yearobj.hpp"        /* for SelectedYearObj*/
#include "warn_dlg.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"

#include <rw/cstring.h>
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;


void SetTimeScale()
{
   // Create the window:

   TimeScale *YearWindow = new TimeScale();
   YearWindow->DoModal();

   // Window is gone, return the selected year
   return;
}



//TimeScale():CWindow(theSpGlobals->theAppDocument,CRect(118,115,637,268), "Set Time Scale",WSF_DEFER_MODAL, W_MODAL)
TimeScale::TimeScale()
          :CWindow(theSpGlobals->theAppDocument,
                   CRect(110,107,637,268), "Set Time Scale",
                   WSF_DEFER_MODAL, W_MODAL)
{

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

                  // build buttons
   itsOKButton = new NButton(this, CRect(8,112,88,144), "OK");
   itsOKButton->SetCommand(1);

   itsResetButton = new NButton(this, CRect(219,112,299,144), "Reset");
   itsResetButton->SetCommand(2);

   itsCancelButton = new NButton(this, CRect(430,112,510,144), "Cancel");
   itsCancelButton->SetCommand(3);

                  // get saved values from their object

   int startYear = theSpGlobals->yearObj->GetStartYear();
   int endYear   = theSpGlobals->yearObj->GetEndYear();
   int length    = theSpGlobals->yearObj->GetCycleLength();
   int interval  = length*(theSpGlobals->yearObj->GetMaxCycles());

                  // build start year controls

   startYearBar = new NScrollBar(this, CRect(256,8,276,40), VERTICAL);
   startYearBar ->IScrollBar(-999, 9999, startYear, 1, 1);


   NText * sYrTxt = new NText(this, CRect(8,12,180,36),
                              "Common starting year:", 0L);
   startYearEdit = new NEditControl(this,
                                    CRect(184,8,248,40),
                                    ConvertToString(startYear),
                                    CTL_FLAG_RIGHT_JUST);
   startYearEdit->SetFocusCommands(NULLcmd,4);


                   // build common cycle length edit control

   NText * cycleLenTxt = new NText(this, CRect(286,12,458,36),
                                   "Common cycle length:", 0L);
   cycleLengthEdit = new NEditControl(this,
                                    CRect(458,8,510,40),
                                    ConvertToString(length),
                                    CTL_FLAG_RIGHT_JUST);
   cycleLengthEdit->SetFocusCommands(NULLcmd,4);


                 // build end year controls -- if necessary, adjust max to keep in bounds

   endYearBar = new NScrollBar(this, CRect(8,88,510,104), HORIZONTAL);   
   endYearBar->IScrollBar(startYear,
                         (endYear > startYear+interval) ? endYear : startYear+interval,
                          endYear,1,theSpGlobals->yearObj->GetCycleLength());

   NText * eYrTxt = new NText(this, CRect(8,56,180,80),
                              "Common ending year:",0);
   endYearEdit = new NEditControl(this,
                                  CRect(184,48,248,80),
                                  ConvertToString(endYear),
                                  CTL_FLAG_RIGHT_JUST);
   endYearEdit->SetFocusCommands(NULLcmd,4);

   aNav->AppendTabStop(new CTabStop(itsOKButton));
   aNav->AppendTabStop(new CTabStop(itsResetButton));
   aNav->AppendTabStop(new CTabStop(itsCancelButton));
   aNav->AppendTabStop(new CTabStop(startYearEdit));
   aNav->AppendTabStop(new CTabStop(startYearBar));
   aNav->AppendTabStop(new CTabStop(cycleLengthEdit));
   aNav->AppendTabStop(new CTabStop(endYearEdit));
   aNav->AppendTabStop(new CTabStop(endYearBar));
   aNav->InitFocus();

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), TIMESCALWindow, 0L);
}




void TimeScale::DoCommand(long theCommand,void* theData)
{
   int proposedYear;
   int goodYear;
   int proposedCycle;
   int goodCycle;

   switch (theCommand)
   {
      case 1:                    // OK Button  was pressed.
      {
         int oops=0;
         // check the starting year.
         proposedYear = (int) ConvertToLong(startYearEdit->GetTitle());
         goodYear = theSpGlobals->yearObj->SetStartYear(proposedYear);
         if (goodYear != proposedYear)
         {
            DoCommand(2);
            oops=1;
            xvt_dm_post_warning("Common starting year was out of range.");
            break;
         }

         // check the cycle length.

         proposedCycle = (int) ConvertToLong(cycleLengthEdit->GetTitle());
         goodCycle = theSpGlobals->yearObj->SetCycleLength(proposedCycle);
         if (proposedCycle != goodCycle)
         {
            DoCommand(2);
            oops=1;
            xvt_dm_post_warning("Cycle length was out of range.");
            break;
         }

         // check the ending year.
         proposedYear = (int) ConvertToLong(endYearEdit->GetTitle());
         goodYear = theSpGlobals->yearObj->SetEndYear(proposedYear);
         if (goodYear != proposedYear )
         {
            DoCommand(2);
            oops=1;
            xvt_dm_post_warning("Ending year was out of range.");
            break;
         }

         if (oops) break;
         theSpGlobals->theAppDocument->SetSave(TRUE);
         Close();
         break;
      }

      case 2:                    // Reset Button  was pressed.
      {
         int startYear = theSpGlobals->yearObj->GetStartYear();
         int endYear   = theSpGlobals->yearObj->GetEndYear();
         int length    = theSpGlobals->yearObj->GetCycleLength();
         int interval  = length*(theSpGlobals->yearObj->GetMaxCycles());
         startYearEdit->SetTitle(ConvertToString(startYear));
         cycleLengthEdit ->SetTitle(ConvertToString(length));
         endYearBar->IScrollBar(startYear,startYear+interval,
                                endYear,1,
                                theSpGlobals->yearObj->GetCycleLength());
         endYearEdit->SetTitle(ConvertToString(endYear));

         break;
      }

      case 3:                    // Cancel Button  was pressed.
         Close();
         break;

      case 4:                    // text lost focus and some others.
      {
         int startYear = (int) ConvertToLong(startYearEdit->GetTitle());
         int endYear   = (int) ConvertToLong(endYearEdit->GetTitle());
         int length    = (int) ConvertToLong(cycleLengthEdit->GetTitle());
         int interval  = length*(theSpGlobals->yearObj->GetMaxCycles());

         if (endYear < startYear) endYear=startYear;
         if (endYear > startYear+interval) endYear=startYear+interval;
         endYearBar->IScrollBar(startYear,startYear+interval,
                                endYear,1,length);
         endYearEdit->SetTitle(ConvertToString(endYear));
         break;
      }

      default:
         CWindow::DoCommand(theCommand,theData);
   }
}




void TimeScale::HScroll(SCROLL_CONTROL theEventType, UNITS thePosition)
{
   int year = (int) endYearBar->GetPosition();
   endYearEdit->SetTitle(ConvertToString(year));
}


void TimeScale::VScroll(SCROLL_CONTROL theEventType, UNITS thePosition)
{
   int year = (int) ConvertToLong(startYearEdit->GetTitle());
   if (theEventType == SC_LINE_DOWN) year--;
   if (theEventType == SC_LINE_UP)   year++;
   startYearEdit->SetTitle(ConvertToString(year));
   DoCommand(4);
}

