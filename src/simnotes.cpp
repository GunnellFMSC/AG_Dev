/*********************************************************************

   File Name:    simnotes.cpp
   Author:       nlc
   Date:         02/27/97

   see simnotes.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include NButton_i
#include NScrollText_i
#include NScrollBar_i
#include CStringRW_i
#include CApplication_i
#include CDesktop_i
#include NListButton_i
#include NGroupBox_i
#include NRadioButton_i
#include CStringCollection_i
#include CNavigator_i
#include NRadioNavigator_i

#include "appdoc.hpp"
#include "spglobal.hpp"
#include "suppdefs.hpp"
#include "warn_dlg.hpp"
#include "compon.hpp"
#include "runstr.hpp"

#include "simnotes.hpp"
#include "spfunc2.hpp"

#include "suppdefs.hpp"
#include "supphelp.hpp"

extern XVT_HELP_INFO helpInfo;

/////////////////////////////////////////////////////////////////////////
void PWR_CALLCONV1 hookTextEditCallback_SimNotesWindow(
	TXEDIT theTxtHandle,
	T_LNUM theOrigin,
	T_LNUM theLines,
	T_CPOS theOrgOffset )
{
   // Call base method first
	NScrollText::ScrollTextCallback(theTxtHandle, theOrigin, theLines, theOrgOffset);

   if (theOrgOffset != USHRT_MAX)
   {
      NScrollText *aNScrollText = (NScrollText*)xvt_tx_get_data(theTxtHandle);
      SimNotesWindow *aSimNotesWindow = (SimNotesWindow *)aNScrollText->GetCWindow();
      aSimNotesWindow->ScrollColumnRuler(theOrgOffset);
   }
}
/////////////////////////////////////////////////////////////////////////

SimNotesWindow::SimNotesWindow(CDocument * theDocument)
               :CWwindow(theDocument,
                         CRect(20,50,850,400),
                         "Edit Simulation Notes",
                         WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                         W_DOC, TASK_MENUBAR),
                itsComponent(NULL)
{

   // create and clear the navitagor

   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   // location or group radio buttons

   itsNotesRadioGroup = new CRadioGroup(this,CPoint(4,4), FALSE);

   itsNotesTop = itsNotesRadioGroup->AddButton(CPoint(0,0),
                                               "Simultion Notes ",1);

   itsNotesComponent = itsNotesRadioGroup->AddButton(CPoint(150,0),
                                                     "Component Notes ",2);

   itsNotesTopButton = (NRadioButton*) itsNotesRadioGroup->
      FindSubview(itsNotesTop);

   itsNotesComponentButton = (NRadioButton*) itsNotesRadioGroup->
      FindSubview(itsNotesComponent);

   itsNotesRadioGroup->SetSelectedButton(itsNotesTop);

   // the components list button is created below...to avoid a resize.

   CStringRW column = "....+....1....+....2....+....3....+....4"
                      "....+....5....+....6....+....7....+....8"
                      "....+....9....+....0....+....1....+....2"
                      "....+....3....+....4....+....5....+....6";

   itsColumnRuler = new NScrollText(this, CRect(4,4,12,36), FALSE, FALSE,
                                    TX_BORDER | TX_NOMENU | TX_READONLY);

#ifdef AIXV4
   itsColumnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
#else
   itsColumnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
#endif
   itsColumnRuler->SetText(column);

   // margin for all characters in columnRuler
   int maxmargin = xvt_dwin_get_text_width(itsColumnRuler->GetCWindow()->GetXVTWindow(),
                                           (char *) column.data(), -1);
   // margin for first 80 characters in columnRuler
   int margin = xvt_dwin_get_text_width(itsColumnRuler->GetCWindow()->GetXVTWindow(),
                                        (char *) CStringRW(column(0, 80)).data(), -1);

   int leadingp;
   int ascentp;
   int descent;

   xvt_dwin_get_font_metrics(itsColumnRuler->GetCWindow()->GetXVTWindow(),
               &leadingp, &ascentp, &descent);

#ifdef AIXV4
   int top = 40;
#else
   int top = 32;
#endif
   int width = margin+16;
   int bottom = top+8+leadingp+ascentp+descent;
   itsColumnRuler->Size(CRect(4,top,width,bottom));
   itsColumnRuler->Disable();

   top = bottom+4;
   int nlines = 10;

   bottom  = 12+top+(nlines*(leadingp+ascentp+descent));

   width = width+NScrollBar::NativeWidth()+2;
   itsSimNotesEdit = new NScrollText(this, CRect(4,top,width,bottom),
                     TRUE, TRUE, TX_BORDER| TX_AUTOHSCROLL | TX_WRAP | TX_ENABLECLEAR,
                     (UNITS) maxmargin, 10000, TRUE);

  	xvt_tx_set_scroll_callback(itsSimNotesEdit->GetXVTTextEdit(), hookTextEditCallback_SimNotesWindow);

#ifdef AIXV4
   itsSimNotesEdit->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
#else
   itsSimNotesEdit->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
#endif

   DoCommand (6,NULL);

   width += 4;
   int left = 4;
   top = bottom+4;
   bottom = top+32;

   itsOkButton =
      new NButton(this, CRect(left,top,left+96,bottom), "Ok", 0L);
   itsOkButton->SetCommand(4);

   left = (width-96)/2;
   itsSaveButton =
      new NButton(this, CRect(left,top,left+96,bottom), "Save", 0L);
   itsSaveButton->SetCommand(5);

   left = width-100;
   itsResetButton =
      new NButton(this, CRect(left,top,left+96,bottom), "Reset", 0L);
   itsResetButton->SetCommand(6);

   RCT newrct = RCT (GetClippedFrame());
   newrct.bottom=newrct.top+bottom+4;
   if (width < 604) width = 604;

   itsComponentsListButton =
      new NListButton(this, CRect(308,4,width-4,320),
                      *theSpGlobals->runStream->allComponents, 0, 0L);

   itsComponentsListButton->SetCommand(3);
   itsComponentsListButton->DeselectAll();
   if (theSpGlobals->runStream->allComponents->entries() == 0)
   {
      itsComponentsListButton->Disable();
      itsNotesComponentButton->Disable();
   }

   newrct.right =newrct.left+width;

#ifdef AIXV4
   xvt_app_process_pending_events();
#endif

   xvt_vobj_translate_points (GetXVTWindow(), TASK_WIN,
                             (PNT *) &newrct, 2);
   Size(CRect(newrct));

   Show();

#if XVTWS==WIN32WS

   // needed on windows, for sure.

   G->GetDesktop()->SetFrontWindow(this);

#endif

   // set the glue...setting the glue on AIX causes the window to be
   // incorrectly drawn to begin with.  So, it is not being done.

#ifdef AIXV4
   xvt_app_process_pending_events();
#endif

   itsColumnRuler->SetGlue(RIGHTSTICKY  | LEFTSTICKY);
   itsSimNotesEdit->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                            RIGHTSTICKY | LEFTSTICKY);

   itsOkButton->SetGlue(BOTTOMSTICKY);
   itsSaveButton->SetGlue(BOTTOMSTICKY);
   itsResetButton->SetGlue(BOTTOMSTICKY);

   aNavigator->AppendTabStop(new CTabStop
                               (new NRadioNavigator(itsNotesRadioGroup)));
   aNavigator->AppendTabStop(new CTabStop(itsComponentsListButton));
   aNavigator->AppendTabStop(new CTabStop(itsSimNotesEdit));
   aNavigator->AppendTabStop(new CTabStop(itsOkButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveButton));
   aNavigator->AppendTabStop(new CTabStop(itsResetButton));
   aNavigator->InitFocus();

   itsSimNotesEdit->Activate();

   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), SIMNOTESWindow, 0L);
}



void SimNotesWindow::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                         // Simultion Notes
      {
         DoCommand (5,NULL);    // Save
         itsComponent = NULL;
         itsComponentsListButton->DeselectAll();
         DoCommand (6,NULL);        // Reset
         break;
      }
      case 2:                         // Component Notes
      {
         itsComponentsListButton->SelectItem(0);
         DoCommand (3,NULL);    // Component List Button
         break;
      }
      case 3:                         // Component List Button
      {
         int aPos = itsComponentsListButton->GetSelectPosition();
         if (aPos > -1)
         {
            DoCommand (5,NULL);    // Save
            itsNotesRadioGroup->SetSelectedButton(itsNotesComponent);
            itsComponent = (Component *) theSpGlobals->runStream->
                  allComponents->at((size_t) aPos);
            DoCommand (6,NULL);          // Reset
         }
         break;
      }
      case 4:                         // Ok
      {
         Close();
         break;
      }
      case 5:                         // Save
      {
         int isChanged;
         if (itsComponent)
         {
            itsComponent->comment = GetSimNotes(itsComponent->comment,
                                                &isChanged);
            if (isChanged) itsComponent->userEditedComment = 1;
         }

         else theSpGlobals->runStream->topSimNotes =
                 GetSimNotes(theSpGlobals->runStream->topSimNotes,&isChanged);

         if (isChanged) theSpGlobals->theAppDocument->SetSave(TRUE);
         break;
      }

      case 6:                         // Reset
      {
         if (itsComponent)
         {
            if (itsComponent->comment)
               itsSimNotesEdit->SetText(*itsComponent->comment);
            else itsSimNotesEdit->Clear();
         }
         else
         {
            if (theSpGlobals->runStream->topSimNotes)
               itsSimNotesEdit->SetText(*theSpGlobals->runStream->topSimNotes);
            else itsSimNotesEdit->Clear();
         }
         break;
      }
      case COMPONENTSListChanged:
      {
         if (theSpGlobals->runStream->allComponents->entries() == 0)
         {
            itsComponentsListButton->Disable();
            itsComponentsListButton->Clear();
            itsNotesComponentButton->Disable();
            itsNotesRadioGroup->SetSelectedButton(itsNotesTop);
            if (itsComponent)
            {
               itsComponent = NULL;
               DoCommand (6,NULL);
            }
         }
         else
         {
            itsComponentsListButton->Enable();
            itsNotesComponentButton->Enable();
            itsComponentsListButton->
               IListButton(*theSpGlobals->runStream->allComponents);
            itsComponentsListButton->DeselectAll();

            if (itsComponent)
            {
               size_t thePosition =
                  theSpGlobals->runStream->allComponents->index(itsComponent);
               if (thePosition == RW_NPOS)
               {
                  itsNotesRadioGroup->SetSelectedButton(itsNotesTop);
                  itsComponent = NULL;
                  DoCommand (6,NULL);
               }
               else itsComponentsListButton->SelectItem((int) thePosition);
            }
         }

         break;
      }
      default:
         CWwindow::DoCommand(theCommand,theData);

   }
}



BOOLEAN SimNotesWindow::Close( void )
{
         DoCommand (5,NULL);
         return CWwindow::Close();
}



CStringRW * SimNotesWindow::GetSimNotes (CStringRW * theNotes, int * isChanged)
{
   *isChanged = 0;
   CStringRW * returnString = theNotes;
   CStringRW newString;

   if (itsSimNotesEdit->GetNCharInText() == 0)
   {
      if (returnString)
      {
         delete returnString;
         *isChanged = 1;
         return NULL;
      }
      else return NULL;
   }
   else
   {

      int first = 1;
      for (T_PNUM ipar = 0; ipar < itsSimNotesEdit->GetNParInText(); ipar++)
      {
         for (T_LNUM ilin = 0;
              ilin < itsSimNotesEdit->GetNLineInPar(ipar); ilin++)
         {
            CStringRW line = itsSimNotesEdit->GetLine(ipar,ilin);

            // watch out for "End" as the first 3 chars of a line.

            if (line.index("end",0,RWCString::ignoreCase) == 0)
               line = line.insert(0," ");

            // strip off trailing newline and spaces.

            line = line.strip(RWCString::trailing,'\n');
            line = line.strip(RWCString::trailing,' ');

            if (first)
            {
               newString = line;
               first = 0;
            }
            else
            {
               newString += "\n";
               newString += line;
            }
         }
      }
   }

   if (returnString)
   {
      if (*returnString != newString)
      {
         *returnString = newString;
         *isChanged = 1;
      }
   }
   else
   {
      returnString = new CStringRW(newString);
      *isChanged = 1;
   }
   return returnString;
}

