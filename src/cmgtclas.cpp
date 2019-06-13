/********************************************************************

   File Name:     cmgtclas.cpp
   Author:        kfd    & nlc
   Date:          02/95  12/95

   Contents:     (see .hpp for description)

********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "runstr.hpp"
#include "compon.hpp"

#include CDesktop_i
#include CDocument_i
#include NEditControl_i
#include NButton_i
#include CText_i
#include CMenuBar_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "cmgtclas.hpp"
#include "cwwindow.hpp"
#include "cretcomp.hpp"
#include "compon.hpp"
#include "runstr.hpp"
#include "spglobal.hpp"
#include "schedobj.hpp"
#include "fvslocs.hpp"
#include "schcond.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "express.hpp"
#include "sendupdt.hpp"
#include <rw/ordcltn.h>
#include "suppdefs.hpp"
#include "viewkey.hpp"
#include "extens.hpp"

CMgtClassWindow::CMgtClassWindow(CDocument *theDocument,
                                 const CRect& theRegion,
                                 Component * theComponent,
                                 Component * theConditionComponent,
                                 const CStringRW& theTitle,
                                 CStringRW * theExtension,
                                 long theAttributes,
                                 WIN_TYPE theWindowType,
                                 int theMenuBar)

                 :CWwindow(theDocument, CenterTopWinPlacement(theRegion), theTitle,
                           theAttributes, theWindowType, theMenuBar),
                  itsComponent(theComponent),
                  itsConditionComponent(theConditionComponent),
                  scheduleBy(NULL),
                  itsSchByCondWindow(NULL),
                  itsComponentName(NULL),
                  itsOKButton(NULL),
                  itsResetButton(NULL),
                  itsFreeFormCommand(NULL),
                  itsCancelButton(NULL),
                  itsScrollingFrame(NULL),
                  closeCalledByCloseCancel(TRUE)
{
   if (itsComponent) itsComponent->openedWindow = this;

   if (itsConditionComponent) noExistingCondition = 1;
   else                       noExistingCondition = 0;

   // wake up the view component keywords menu item.

   GetMenuBar()->SetEnabled (M_SIM_PREP_VIEWKEYWORDS, TRUE);

   if (theExtension) itsExtension = *theExtension;
   else if (itsComponent &&
            itsComponent->extension) itsExtension = *itsComponent->extension;
   else itsExtension = NULLString;

   // create and clear the navigator

   itsNavigator = GetNavigator();
   itsNavigator->ClearTabStops();
}



SchByCond * CMgtClassWindow::BuildMgtConditionWindow(
              SchByCond  * itsSchByCondWindow,  CWindow * itsParent,
              const char * nameString, const char * defaultString,
              CStringRW& conditionTitle, Component * conditionComponent)
{
   const char * theDefaultCondition = "";
   if (conditionComponent && conditionComponent->openedWindow)
      itsSchByCondWindow = (SchByCond *) conditionComponent->openedWindow;

                           // display window, if it exits
   if (itsSchByCondWindow && G->GetDesktop()->FindWindow(itsSchByCondWindow))
   {

      G->GetDesktop()->SetFrontWindow(itsSchByCondWindow);
      return(itsSchByCondWindow);
   }

   if (conditionComponent  &&
       (conditionComponent->sourceWindow &&
        strcmp("BuildExpression",
               conditionComponent->sourceWindow->data()) == 0))
   {
      itsSchByCondWindow = (SchByCond *) new
         BuildExpression(conditionTitle,
                         conditionComponent,
                         NULL,
                         conditionComponent->componentType);
      return itsSchByCondWindow;
   }

   // find out if there is a default condition for
   // this keyword.

   PKeyData * tmp = GetPKeyData(nameString, defaultString);

   if (tmp) theDefaultCondition = tmp->pString.data();

   // *** #101 is the DoCommand number ****
   // *** it will call CheckForNULLMgtCondition

   itsSchByCondWindow = new SchByCond (itsParent, 101,
                                       theDefaultCondition,
                                       conditionTitle,
                                       conditionComponent,
                                       itsComponent);
   return(itsSchByCondWindow);
}





void CMgtClassWindow::SaveTheComponent(int usingFreeForm,
                                       const char * screenName,
                                       CStringRW  & keywordLine,
                                       CStringRW  & settingsLine,
                                       const char * extension)
{
   int componentEdit = itsComponent ? 1 : 0;

   // update if this is existing component

   if (itsComponent)
   {
      CStringRW cName = itsComponentName->GetTitle();
      (CStringRW &) *itsComponent = cName;
      *itsComponent->keywords = keywordLine;
      *itsComponent->windowParms = settingsLine;
      if  (itsComponent->extension) *itsComponent->extension=extension;
      else itsComponent->extension = new CStringRW (extension);
   }
   else
   {
      // else, new one -- add to data structure in the code below.

      CStringRW cName = itsComponentName->GetTitle();
      itsComponent = CreateComponent(cName.data(),
         screenName, keywordLine, settingsLine, NULL, extension, 2);
   }

   // if we have a condition component, make sure that we also
   // do not have year selected.  If we do have year selected, we
   // perhaps need to delete itsConditionComponent.

   if (itsConditionComponent)
   {
      if (scheduleBy->YearIsSelected())
      {
         if (!theSpGlobals->runStream->
             allComponents->find(itsConditionComponent))
         {
            delete itsConditionComponent;
            itsConditionComponent = NULL;
         }
         itsComponent->componentType=2;
         itsComponent->conditionSerialNum = -1;
      }
      else
      {
         itsComponent->componentType=3;
         itsComponent->conditionSerialNum=
            itsConditionComponent->componentNumber;
      }
   }

   if (usingFreeForm)
   {
      Hide();
      xvt_scr_set_busy_cursor();
      new BuildExpression((CStringRW &) *itsComponent, itsComponent,
                           itsConditionComponent, 0);
   }
   else
   {
      theSpGlobals->runStream->AddComponent(itsComponent);
      if (itsConditionComponent) theSpGlobals->runStream->
         AddComponent(itsConditionComponent, itsComponent);
      SendUpdateMessage(COMPONENTSListChanged,NULL,this);
      itsDocument->SetSave(TRUE);
   }
}




void CMgtClassWindow::CheckForNULLMgtCondition( void )
{
   itsSchByCondWindow = NULL;

   // if itsConditionComponent is NULL, then no selection was made
   // or the window was closed with a CANCEL...switch to using YEAR.

   if (!itsConditionComponent) scheduleBy->SetYearButton( TRUE );
}





Component * CMgtClassWindow::FindConditionComponent( void )
{

                              // check for existing condition; if there is one
                              // change button to "for editing only"

   Component * theFoundConditionComponent = (Component *) 0;

   if (itsComponent->conditionSerialNum >= 0)
   {
      RWOrderedIterator nextComp(*theSpGlobals->runStream->allComponents);
      Component * component;
      while ((component = (Component *) nextComp()) != NULL)
      {
         if (itsComponent->conditionSerialNum ==
                component->componentNumber)
         {
            theFoundConditionComponent = component;
            break;
         }
      }
   }                          // condition component has been found.
   if (theFoundConditionComponent)
   {
      noExistingCondition = 0;

                              // since this component has an associated
                              // condition, set the do command in case the
                              // condition button is pressed when in edit
      scheduleBy->ResetConditionButtonCommand(50);

   }
   return theFoundConditionComponent;
}




BOOLEAN CMgtClassWindow::Close( void )
{
   if (closeCalledByCloseCancel)
   {
      if (noExistingCondition && itsConditionComponent &&
          !theSpGlobals->runStream->
          allComponents->find(itsConditionComponent))
      {
         delete itsConditionComponent;
         itsConditionComponent = NULL;
      }
   }

   if (itsComponent) itsComponent->openedWindow = NULL;

   if (itsConditionComponent && itsConditionComponent->openedWindow)
   {
      itsConditionComponent->openedWindow->Close();
   }
   else
   {
      if (itsSchByCondWindow &&
          G->GetDesktop()->FindWindow(itsSchByCondWindow))
          itsSchByCondWindow->Close();
   }

   return CWwindow::Close();
}




void CMgtClassWindow::MakeStandardControls(const CStringRW & name,
                                           long okCommand,
                                           long resetCommand,
                                           long cancelCommand,
                                           long freeFormCommand)

{

   itsOKCommand=okCommand;
   itsResetCommand=resetCommand;
   itsCancelCommand=cancelCommand;
   itsFreeFormCommand=freeFormCommand;

   new CText (this, CPoint(8,12), CStringRW("Name:"));
   itsComponentName = new NEditControl(this, CRect(72,4,332,36), name, 0L);

   CRect aRect = GetFrame();
   aRect.Bottom(aRect.Bottom()-4);
   aRect.Top(aRect.Bottom()-32);
   UNITS space = (aRect.Right()-360)/3;  // 360 = 3*88+4+4

   aRect.Left(4);
   aRect.Right(aRect.Left()+88);

   itsOKButton= new NButton(this, aRect, "OK");
   itsOKButton->SetCommand(itsOKCommand);
   itsOKButton->SetGlue(BOTTOMSTICKY);

   aRect.Left(aRect.Right()+space);
   aRect.Right(aRect.Left()+88);

   itsFreeFormButton= new NButton(this, aRect, "Use Editor");
   itsFreeFormButton->SetCommand(itsFreeFormCommand);
   itsFreeFormButton->SetGlue(BOTTOMSTICKY);

   aRect.Left(aRect.Right()+space);
   aRect.Right(aRect.Left()+88);

   itsResetButton= new NButton(this, aRect, "Reset");
   itsResetButton->SetCommand(itsResetCommand);
   itsResetButton->SetGlue(BOTTOMSTICKY);

   aRect.Left(aRect.Right()+space);
   aRect.Right(aRect.Left()+88);

   itsCancelButton= new NButton(this, aRect, "Cancel");
   itsCancelButton->SetCommand(itsCancelCommand);
   itsCancelButton->SetGlue(BOTTOMSTICKY);

   aRect.Left(4);
   aRect.Bottom(aRect.Top()-4);
   aRect.Top(40);

   itsScrollingFrame = new MyCScroller(this, aRect,
                                       aRect.Right() -aRect.Left(),
                                       aRect.Bottom()-aRect.Top());
   itsScrollingFrame ->IScroller(FALSE, TRUE, TRUE, 32, 240);

   itsNavigator->AppendTabStop(new CTabStop(itsOKButton));
   itsNavigator->AppendTabStop(new CTabStop(itsResetButton));
   itsNavigator->AppendTabStop(new CTabStop(itsFreeFormButton));
   itsNavigator->AppendTabStop(new CTabStop(itsCancelButton));
   itsNavigator->AppendTabStop(new CTabStop(itsComponentName));
   itsScrollingFrame->AppendToNavigator(itsNavigator);
}




void CMgtClassWindow::SizeWindow(int theWidth, int theHeigth)
{

   // propogate the resize message first, then set then size of
   // the scroller.

   CWwindow::SizeWindow(theWidth, theHeigth);

   if (theWidth < 500 || theHeigth < 200)
   {
      if (theWidth  < 500) theWidth  = 500;
      if (theHeigth < 200) theHeigth = 200;

      // this call will cause a recursive call back to this routine.

      Size (CRect(80,100,80+theWidth,100+theHeigth));
   }
   if (itsScrollingFrame)
   {
      itsScrollingFrame->Size(CRect(8,40,theWidth-8,theHeigth-48));

      // the following line is needed to cause the contents of the
      // scroller to be drawn correctly.

      itsScrollingFrame->DoSetOrigin(CPoint(0,0));
   }
   DoDraw();
}



void CMgtClassWindow::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case EXTENSIONListChanged:
      {
         if (itsExtension == NULLString) break;

         if (theSpGlobals->theExtensions->
             FindLegalExtension(itsExtension.data()) == -1)
         {
            CWindow * curFront = G->GetDesktop()->GetFrontWindow();
            if (this != curFront) G->GetDesktop()->SetFrontWindow(this);

            xvt_dm_post_warning("%s uses an extension that is "
                                "no longer being used in this simulation.",
                                GetTitle().data());
            if (this != curFront) G->GetDesktop()->SetFrontWindow(curFront);
         }
         break;
      }

      case VARIANTSelectionChanged:
      {
         CWindow * curFront = G->GetDesktop()->GetFrontWindow();
         if (this != curFront) G->GetDesktop()->SetFrontWindow(this);
         xvt_dm_post_warning("%s is set up for variant that is "
                             "no longer being used in this simulation.",
                             GetTitle().data());
         if (this != curFront) G->GetDesktop()->SetFrontWindow(curFront);
      }

      default:
         CWwindow::DoCommand(theCommand, theData);
   }
}



void CMgtClassWindow::DoMenuCommand(MENU_TAG theMenuItem,
                                    BOOLEAN isShiftkey,
                                    BOOLEAN isControlKey)
{
   switch(theMenuItem)
   {
      case M_SIM_PREP_VIEWKEYWORDS:
      {
         CStringRW viewTitle = "Keywords - ";
         viewTitle += itsComponentName->GetTitle();

         CStringRW keywordLine(BuildKeywordString());

         new ViewKeywords(keywordLine.data(),
                          (const char *) viewTitle);

         break;
      }

      default:
      CWindow::DoMenuCommand(theMenuItem,isShiftkey, isControlKey);
      break;
   }
}




CStringRW CMgtClassWindow::BuildKeywordString(void)
{
   CStringRW keyLine = "No keyword building function available.";
   return keyLine;
}



