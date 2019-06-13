/********************************************************************

   File Name:     warn_dlg.cpp
   Author:        kfd & nlc
   Date:          11/14/94 and 05/01/97

   Contents:      Warn
                  WarnWin()
                  WarnWin::DoCommand

********************************************************************/

#include "appdef.hpp"

#include NButton_i
#include NText_i
#include NScrollText_i
#include CApplication_i
#include CDesktop_i
#include CRect_i
#include CNavigator_i

#include "spglobal.hpp"
#include "warn_dlg.hpp"
#include "spfunc2.hpp"


void Warn(const char* theText)
{
               // This function puts up a "dialog box" and returns when the
               // user presses remove

               // The calling routine supplies the text to be display
               // Count the number of line feeds in theText.  This will be
               // used to determine type of text box (scrolling or NText)

   if (!theText) return;

   int aLineCnt = numberLines(theText) - 1;

               // Create the window:
   WarnWin * WarnWindow = new WarnWin(theText, aLineCnt);
   WarnWindow->DoModal();

   return;
}





WarnWin::WarnWin(const char* theText, int aLineCnt)
   :CWindow(theSpGlobals->theAppDocument,
            CenterWinPlacement(CRect(16,16,512,216)), "Warning",
            WSF_DEFER_MODAL , W_MODAL)
{
 
   // If many text lines passed, use NText to display

   if (aLineCnt <= 6)
   {
      new NText(this, CRect(28,12,468,148), CStringRW(theText));
   }

   else
   {

      NScrollText* itsText =
            new NScrollText(this, CRect(12,12,484,148),
                            TRUE, TRUE,
                            TX_BORDER | TX_NOMENU | TX_READONLY,
                            1000, 1000, TRUE);

      itsText->AddParagraph(CStringRW(theText), USHRT_MAX);
   }

   NButton *theCloseButton = new NButton(this, CRect(200,160,300,192),
                                         "Close");
   theCloseButton->SetCommand(1);
   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();
   aNav->AppendTabStop(new CTabStop(theCloseButton));
   aNav->InitFocus();
}




void WarnWin::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:  // Close button was pressed.
         Close();
         break;
   }
}
