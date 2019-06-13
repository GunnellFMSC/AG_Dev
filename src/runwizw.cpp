/********************************************************************

   File Name:     runwizw.cpp   (The Run Wizard).
   Author:        nlc
   Date:          07/19/96


********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "spglobal.hpp"
#include "runwizw.hpp"
#include "suppdefs.hpp"
#include "mycscrol.hpp"

#include CStringCollection_i
#include CStringRWC_i
#include NButton_i
#include NScrollBar_i
#include NListButton_i
#include NScrollText_i
#include CStringRW_i
#include CText_i
#include NCheckBox_i
#include CNavigator_i

#include <rw/ordcltn.h>
#include <rw/collstr.h>
#include "msgwin.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"

#include "supphelp.hpp"  // for help #defines

extern XVT_HELP_INFO helpInfo;


RunWizardWin::RunWizardWin(CDocument *theDocument)
            :CWwindow(theDocument, RightWinPlacement(CRect(8,25,446,433)),
                     "Run Wizard",
                     WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE,
                     W_DOC,TASK_MENUBAR)
{

   itsNavigator = GetNavigator();
   itsNavigator->ClearTabStops();

   itsMSText = NULL;

   CText * tmp = new CText(this,CPoint(4,4),"General Tasks:");
   tmp->SetGlue(TOPSTICKY | LEFTSTICKY);

   itsGeneralTaskListBox = new NListButton(this, CRect(4,28,434,368),
                                        itsGeneralTaskList, NULL);
   itsGeneralTaskListBox->SetCommand(2);
   itsGeneralTaskListBox->SetGlue(TOPSTICKY | RIGHTSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsGeneralTaskListBox));

   LoadGeneralTaskList();

   tmp = new CText(this,CPoint(4,72),
                   "Steps (check them off as you work):");
   tmp->SetGlue(TOPSTICKY | LEFTSTICKY);

   itsScrollingWindow = new MyCScroller(this, CRect(4,96,434,368),
                                        434-4-NScrollBar::NativeWidth(),
                                        272);
   itsScrollingWindow->AppendToNavigator(itsNavigator);

   itsCloseButton = new NButton(this, CRect(4,372,136,404), "Close");
   itsCloseButton->SetCommand(1);
   itsCloseButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsCloseButton));

#if XVTWS==WIN32WS

   // On Motif, SizeWindow is called automatically...Here is the call
   // for Windows...necessary because LoadScroller is called after the
   // size is known.

   CRect tmpRect = GetFrame();
   SizeWindow(tmpRect.Right()-tmpRect.Left(),
              tmpRect.Bottom()-tmpRect.Top());


#endif

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), RUNWizard, 0L);
   itsNavigator->InitFocus();

}




RunWizardWin::~RunWizardWin( void )
{
   itsMSText->FreeThePKeys();
}






void RunWizardWin::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:                           // Close
         Close();
         break;

      case 2:                           // Task List Selection
         LoadScroller();
         break;

      case SUPPOSEResetCommand:         // Reset Notification Command.
      {
         itsMSText = GetMSTextPtr("runWizard");
         break;
      }
      default: CWwindow::DoCommand(theCommand,theData); break;
   }
}




void RunWizardWin::LoadGeneralTaskList( void )
{
   itsMSText = GetMSTextPtr("runWizard");

   if (itsMSText)
   {
      PKeyData * taskList = itsMSText->FindPKeyData("taskList");
      if (taskList)
      {
         int defaultTask =  LoadRWOrdered(taskList->pString.data(),
                                          &itsGeneralTaskList);

         itsGeneralTaskListBox->IListButton(itsGeneralTaskList);
         itsGeneralTaskListBox->SelectItem(defaultTask);

         return;
      }
   }
   xvt_dm_post_error("Suppose can not find necessary data "
                     "in the Parameters File.");
}



void RunWizardWin::LoadScroller( void )
{

   // first clear the scroller.

   while (!itsScrollingWindow->GetSubviews()->isEmpty())
   {
      CSubview * aSubview;
      RWOrderedIterator subviewIterator
            (*itsScrollingWindow->GetSubviews());

      while ((aSubview = (CSubview *) subviewIterator()) != NULL)
      {
         itsScrollingWindow->RemoveSubview(aSubview);
         delete aSubview;
      }
   }

   PKeyData * fieldPKeyData = FALSE;
   int thePosition = itsGeneralTaskListBox->GetSelectPosition()+1;
   int theStep = 1;
   char search[7];

   int leadingp, ascentp, descent, startPoint;

   startPoint = 4;
   xvt_dwin_get_font_metrics(GetCWindow()->GetXVTWindow(),
                             &leadingp, &ascentp, &descent);
   leadingp = leadingp+ascentp+descent;

   CRect tmpRect;
   NTextEdit * nTextEdit = NULL;

   do
   {
      sprintf (search,"t%ds%d", thePosition, theStep++);
      fieldPKeyData = itsMSText->FindPKeyData(search);

      if (fieldPKeyData)
      {
         tmpRect.Top   (startPoint);
         tmpRect.Bottom(startPoint+leadingp);
         tmpRect.Left  (4);
         tmpRect.Right (6+leadingp);

         new NCheckBox((CSubview*) itsScrollingWindow, tmpRect);

         if (!nTextEdit)
         {
            tmpRect.Bottom(startPoint+500*leadingp);
            tmpRect.Left  (10+leadingp);
            tmpRect.Right (itsScrollingWindow->GetFrame().Right()-
                           NScrollBar::NativeWidth()-4);

            nTextEdit = new NTextEdit((CSubview*) itsScrollingWindow,
                                      tmpRect,
                                      TX_NOMENU | TX_WRAP | TX_READONLY,
                                      tmpRect.Right()-tmpRect.Left()-2);
         }
         else nTextEdit->Clear();

         CStringRW aString(fieldPKeyData->pString);
         StripSingleNewLines(&aString);
         nTextEdit->AddParagraph(aString, USHRT_MAX);

         int totLine = 0;
         for (T_PNUM ipar = 0; ipar < nTextEdit->GetNParInText(); ipar++)
         {
            for (T_LNUM ilin = 0;
                 ilin < nTextEdit->GetNLineInPar(ipar); ilin++)
            {
               if (nTextEdit->GetLine(ipar,ilin).length() > 0)
               {
                  new CText ((CSubview*) itsScrollingWindow,
                             CPoint(10+leadingp,
                                    startPoint+(totLine*leadingp)),
                             nTextEdit->GetLine(ipar,ilin));
               }
               totLine++;
            }
         }
         startPoint+=(totLine+1)*leadingp;
      }
   } while (fieldPKeyData);

   itsScrollingWindow->RemoveSubview(nTextEdit);
   delete nTextEdit;

   itsScrollingWindow->IScroller(FALSE, TRUE, TRUE, leadingp,
                                 itsScrollingWindow->GetFrame().Bottom());
   itsScrollingWindow->
      SetVirtualFrame(itsScrollingWindow->GetFrame().Right()-
                      itsScrollingWindow->GetFrame().Left(),
                      startPoint);
   itsScrollingWindow->SetScrollingOrigin(CPoint(0,0));
   itsNavigator->InitFocus();

}




void RunWizardWin::SizeWindow(int theWidth, int theHeigth)
{

   // propogate the resize message first, then set the size of
   // the scroller.

   CWindow::SizeWindow(theWidth, theHeigth);

   if (theWidth < 200 || theHeigth < 200)
   {
      if (theWidth  < 200) theWidth  = 200;
      if (theHeigth < 200) theHeigth = 200;

      // this call will cause a recursive call back to this routine.

      Size (CRect(80,100,80+theWidth,100+theHeigth));
   }
   if (itsScrollingWindow)
   {
      itsScrollingWindow->Hide();
      itsScrollingWindow->Size(CRect(4,96,theWidth-4,theHeigth-40));
      itsScrollingWindow->DoSetOrigin(CPoint(0,0));
      LoadScroller();
      itsScrollingWindow->DoDraw();
      itsScrollingWindow->Show();
   }
}


