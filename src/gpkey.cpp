/*********************************************************************

   File Name:    gpkey.cpp
   Author:       nlc
   Date:         12/18/94 & 01/18/95

   see gpkey.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include "compon.hpp"
#include "cretcomp.hpp"

#include NText_i
#include NButton_i
#include NEditControl_i
#include NScrollText_i
#include NListButton_i
#include CMenuBar_i
#include CNavigator_i
#include CStringCollection_i
#include CUnits_i
#include CView_i
#include CWindow_i
#include CDocument_i
#include CStringRWC_i
#include CBoss_i
#include CApplication_i
#include CDesktop_i
#include CWindow_i

#include "spparms.hpp"
#include "suppdefs.hpp"
#include "fvslocs.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "viewkey.hpp"
#include "cparmrtn.hpp"
#include "extens.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include "mycscrol.hpp"
#include "gpkey.hpp"
#include "boxsldr.hpp"
#include "schedobj.hpp"
#include "schcond.hpp"
#include "express.hpp"

#include "sendupdt.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

GPKeyword::GPKeyword(const CStringRW & theTitle,
                     MSText          * keywordMSTextPointer,
                     const CStringRW & theExtension,
                     Component       * currentComponent)

            :CWwindow((CDocument *) theSpGlobals->theAppDocument,
                      CenterTopWinPlacement(CRect(10,20,610,440)), theTitle,
                      WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                      W_DOC, TASK_MENUBAR),
             itsExtension(theExtension),
             itsSchByCondWindow(NULL),
             theConditionComponent(NULL),
             theResultingComponent(NULL),
             theKeywords(NULL),
             theFieldTypes(NULL),
             theAnswerPtrs(NULL),
             theScrollingFrame(NULL),
             theComponentBeingEdited(currentComponent)
{

   int fieldType = 0;
   void * theAnswer = NULL;

   // create and clear the navigator

   itsNavigator = GetNavigator();
   itsNavigator->ClearTabStops();

   // if this window is editing an existing component, then we have to
   // find the corresponding MSText section and load the PKeyData
   // otherwise, we already have pointers to this information.

   if (theComponentBeingEdited)
   {
      CStringRWC searchMSText =
         GetCParm(theComponentBeingEdited->windowParms->data(), 1);

      keywordMSText = (MSText *) theSpGlobals->
         theParmsData->find(&searchMSText);

      // if there is NO information available, then we can not edit and
      // and must exit.

      if (!keywordMSText)
      {
         xvt_app_process_pending_events();
         xvt_dm_post_error ("Sorry, Suppose can not find necessary\n"
                            "information in the parameters file.\n"
                            "You can not edit this selection.");
         Close();
         return;
      }
      keywordPKeySet = keywordMSText->GetThePKeys();

      // set a pointer to the address of this window inside the component
      // ... this is done for editing control.

      theComponentBeingEdited->openedWindow=this;
   }
   else
   {
      keywordMSText  = keywordMSTextPointer;
      keywordPKeySet = keywordMSText->GetThePKeys();
   }

   // if there is no PKeyData for the keyword or extension, issue a warning
   // and exit.

   if (!keywordPKeySet)
   {
      xvt_dm_post_error("Suppose internal error:\n"
                        "Calling arguemnts to GPKeyword are wrong.");
      Close();
      return;
   }

   // set up the componentName field.

   new NText(this, CRect(8,12,64,32), "Name: ", 0L);

   // If we are editing, then the title is set as it was left on the exit
   // of the previous screen...and it this title is used if Reset is pushed.

   if (theComponentBeingEdited)
      componentName = new NEditControl(this, CRect(72,4,332,36),
         (CStringRW &) *theComponentBeingEdited, 0L);
   else
         // otherwise, use the window title for the title.
      componentName = new NEditControl(this, CRect(72,4,332,36),
                                       theTitle, 0L);

   // set up the universal buttons.

   okButton          = new NButton(this, CRect(  8,380,104,412),
                                   "Ok");
   useParmsButton    = new NButton(this, CRect(163,380,259,412),
                                   "Use Editor");
   resetButton       = new NButton(this, CRect(318,380,414,412),
                                   "Reset");
   cancelButton      = new NButton(this, CRect(496,380,592,412),
                                   "Cancel");

   okButton          ->SetCommand( 1);
   useParmsButton    ->SetCommand( 2);
   resetButton       ->SetCommand( 3);
   cancelButton      ->SetCommand( 4);

   okButton          ->SetGlue(BOTTOMSTICKY);
   useParmsButton    ->SetGlue(BOTTOMSTICKY);
   resetButton       ->SetGlue(BOTTOMSTICKY);
   cancelButton      ->SetGlue(BOTTOMSTICKY);

   // find out if Use Parms is legal.  Use parms will be legal if a
   // parmsForm PKey exists for the keyword.  Otherwise, it is NOT legal.

   if (!keywordMSText->FindPKeyData("parmsForm",
         theSpGlobals->theVariants->GetSelectedVariant(), 0))
      useParmsButton->Disable();

   // 544 = 68*8, 3000 = 375*8
   theScrollingFrame = new MyCScroller(this, CRect(8,40,592,372), 544, 3000);
   theScrollingFrame->IScroller(FALSE, TRUE, TRUE, 24, 240);

   // the maxFields value is set to the number of PKeys.  This number is
   // usually larger, but never smaller, than the number of fields needed.

   int maxFields = keywordPKeySet->entries();
   if (maxFields == 0) maxFields = 1;
   theFieldTypes = new int[maxFields];
   theAnswerPtrs = new void* [maxFields];
   for (int i=0; i<maxFields; i++) theAnswerPtrs[i]=NULL;

   // set up the starting vertical point.

   startPoint = 8;

   // Load the scroller with the fields of data.

   int fieldNumber = 0;
   numberOfFieldTypes = 0;
   while (fieldType > -1)
   {
      theAnswer = BuildField((CWindow *) this, 5, theScrollingFrame,
                              keywordMSText, &startPoint,
                              ++fieldNumber, &fieldType);
      if (fieldType == -1) break;
      theFieldTypes[numberOfFieldTypes]  =fieldType;
      theAnswerPtrs[numberOfFieldTypes++]=theAnswer;

      // make sure there is room for the next element

      theScrollingFrame->SetVirtualFrame( 544, startPoint+500);
   }

   theScrollingFrame->ShrinkToFit();

   // save the initial screen settings for reset.

   itsInitialSettings = "";
   StuffCParm((CStringRW &) itsInitialSettings,
              (char const *) componentName->GetTitle());
   StuffCParm((CStringRW &) itsInitialSettings,
              keywordMSText->data());
   BuildCParms (theFieldTypes,theAnswerPtrs,numberOfFieldTypes,
               (CStringRW &) itsInitialSettings);


   // if this object is being edited, set the values equal to the
   // way the window was when it was saved.

   if (theComponentBeingEdited)
      ResetObjects (theFieldTypes,theAnswerPtrs,numberOfFieldTypes, 2,
                    theComponentBeingEdited->windowParms->data());

   // wake up the view component keywords menu item.

   GetMenuBar()->SetEnabled (M_SIM_PREP_VIEWKEYWORDS, TRUE);

   // if theComponentBeingEdited is not null, perhaps we have
   // a conditions component that should be "found" and used.

   if (theComponentBeingEdited &&
       theComponentBeingEdited->conditionSerialNum >= 0)
   {
      RWOrderedIterator nextComp(*theSpGlobals->
                                 runStream->allComponents);
      Component * component;
      while ((component = (Component *) nextComp()) != NULL)
      {
         if (theComponentBeingEdited->conditionSerialNum ==
             component->componentNumber)
         {
            theConditionComponent = component;
            break;
         }
      }
   }

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), GPKEYWindow, 0L);

   itsNavigator->AppendTabStop(new CTabStop(okButton));
   itsNavigator->AppendTabStop(new CTabStop(useParmsButton));
   itsNavigator->AppendTabStop(new CTabStop(resetButton));
   itsNavigator->AppendTabStop(new CTabStop(cancelButton));
   itsNavigator->AppendTabStop(new CTabStop(componentName));
   theScrollingFrame->AppendToNavigator(itsNavigator);

   AdjustScrollingWinSize(theScrollingFrame, this);

   Show();

   itsNavigator->InitFocus();
}




void GPKeyword::DoCommand(long theCommand,void* theData)
{

   const char * theDefaultCondition = NULL;

   switch (theCommand)
   {
      case 2:                // Use Parms
      {
         if (xvt_dm_post_ask("Proceed","Cancel",NULL,
                             "You are about to enter a text editor "
                             "used to edit parameters manually.  "
                             "Suppose does not ensure correct keyword "
                             "records when you use this editor.")
            == RESP_2) break;
      }

      case 1:                // Ok
      {
         // deal with the condition component.  if year is
         // selected, and there is a condition component, we have
         // to clear up the condition component. The condition component
         // itself will be deleted, if not otherwise being used.

         for (int i=0; i<numberOfFieldTypes; i++)
         {
            if (theFieldTypes[i] == 2)
            {
               if (((ScheduleByObject *) theAnswerPtrs[i])->YearIsSelected())
               {
                  // if theConditionComponent is not otherwise being used
                  // then delete it.  in any event, set null because
                  // it is not being used in this component.
                  // Note that if the condition component was built by
                  // the expression editor it will be in the component
                  // list even if it is not wanted at this point.

                  if (theConditionComponent && !theSpGlobals->runStream->
                      allComponents->find(theConditionComponent))
                  delete theConditionComponent;

                  theConditionComponent = NULL;
               }
            }
         }

         if (theComponentBeingEdited)
         {
            CStringRW theCParms;

            if (!theComponentBeingEdited->keywords)
               theComponentBeingEdited->keywords = new CStringRW();

            int answerForm = theCommand;

            if (!BuildTheKeywords (keywordMSText,
                                   theFieldTypes,
                                   theAnswerPtrs,
                                   numberOfFieldTypes,
                                   answerForm,
                                   *theComponentBeingEdited->keywords)) break;

            StuffCParm(theCParms, componentName->GetTitle().data());
            StuffCParm(theCParms, keywordMSText->data());
            BuildCParms (theFieldTypes,theAnswerPtrs,
                         numberOfFieldTypes,theCParms);

            *theComponentBeingEdited->windowParms = theCParms;
            (CStringRW &) *theComponentBeingEdited=componentName->GetTitle();

            if (theConditionComponent)
            {
               theComponentBeingEdited->componentType=3;
               theComponentBeingEdited->conditionSerialNum=
                  theConditionComponent->componentNumber;

               // make sure that the condition component is in the run.
               // it will not be added if it is already present at each
               // place where theConditionBeingEdited is present.

               theSpGlobals->runStream->AddComponent(theConditionComponent,
                                                     theComponentBeingEdited);
            }
            else
            {
               // insure that theComponentBeingEdited does not refer
               // to theConditionComponent.

               theComponentBeingEdited->componentType=2;
               theComponentBeingEdited->conditionSerialNum=-1;

            }

            // make sure that the component is placed correctly in the
            // runstream.

            theSpGlobals->runStream->
                  AddComponent(theComponentBeingEdited);

            if (theCommand==2)  // Use Parms
            {
               Hide();
               xvt_scr_set_busy_cursor();
               new BuildExpression((CStringRW &) *theComponentBeingEdited,
                                   theComponentBeingEdited,
                                   theConditionComponent, 0);
            }
            SendUpdateMessage(COMPONENTSListChanged,NULL,this);
            theSpGlobals->theAppDocument->SetSave(TRUE);
         }
         else  // NOT theComponentBeingEdited
         {
            // the argument should be:
            // 0 if we are building for the UseParms feature.
            // 1 if we are building the standardKeyword

            if (BuildTheComponent(theCommand == 1))
            {
               theResultingComponent = theSpGlobals->runStream->
                     PPEChecking(keywordMSText,
                                 theResultingComponent);

               if (theCommand==1)  // Ok
               {
                  theSpGlobals->runStream->
                     AddComponent(theResultingComponent);
                  if (theConditionComponent)  theSpGlobals->runStream->
                     AddComponent(theConditionComponent,
                                  theResultingComponent);
                  SendUpdateMessage(COMPONENTSListChanged,NULL,this);
                  theSpGlobals->theAppDocument->SetSave(TRUE);

               }                    // Use Parms
               else
               {
                  Hide();
                  xvt_scr_set_busy_cursor();
                  new BuildExpression((CStringRW &) *theResultingComponent,
                                        theResultingComponent,
                                        theConditionComponent, 0);
               }
            }
            else break;
         }

         Close();
         break;
      }

      case 3:                // Reset
      {
         componentName->SetTitle(GetCParm(itsInitialSettings.data(),
                                          0));
         ResetObjects (theFieldTypes,theAnswerPtrs,numberOfFieldTypes, 2,
                       itsInitialSettings.data());
         break;
      }
      case 4:                // Cancel
      {
		 Close();
         break;
      }
      case 5:                // Condition (Create, or use existing).
      {
         if (itsSchByCondWindow &&
             G->GetDesktop()->FindWindow(itsSchByCondWindow))
         {
            G->GetDesktop()->SetFrontWindow(itsSchByCondWindow);
         }
         else
         {
            // find out if there is a default condition for
            // this keyword.

            PKeyData * defaultCondition = keywordMSText->
               FindPKeyData("defaultCondition",
                            theSpGlobals->theVariants->GetSelectedVariant(),
                            FALSE);

            if (defaultCondition)
               theDefaultCondition = defaultCondition->pString.data();

            if (theConditionComponent)  // a condition was already created, we
            {                           // are editing it.

               CStringRW theTitle = "Edit - ";
               theTitle += (CStringRW &) *theConditionComponent;
               if (theConditionComponent->openedWindow) itsSchByCondWindow =
                  (SchByCond *) theConditionComponent->openedWindow;
               else itsSchByCondWindow = NULL;
               if (itsSchByCondWindow)
                  G->GetDesktop()->SetFrontWindow(itsSchByCondWindow);
               else
               {
                  if (theConditionComponent->sourceWindow &&
                      strcmp("BuildExpression",
                             theConditionComponent->sourceWindow->data()) == 0)
                  {
                     itsSchByCondWindow = (SchByCond *)
                        new BuildExpression(theTitle, theConditionComponent,
                                            NULL, 1);
                  }
                  else
                  {
                     itsSchByCondWindow = new
                        SchByCond (this, 6,
                                   theDefaultCondition,
                                   theTitle,
                                   theConditionComponent,
                                   theComponentBeingEdited);
                  }
               }
            }
            else
            {
               CStringRW conditionTitle = "Condition - ";
               conditionTitle += componentName->GetTitle();
               itsSchByCondWindow = new SchByCond (this, 6,  // <--This #6 is the
                   theDefaultCondition, conditionTitle,      // following case 6:
                                NULL,   theComponentBeingEdited);

            }
         }
         break;
      }
      case 6:                // Condition (return condition component and
      {                      //            Notify Distroyed)

         theConditionComponent = (Component *) theData;
         itsSchByCondWindow = NULL;

         // if theConditionComponent is NULL, then no selection was made
         // or the window was closed with a CANCEL...switch to using YEAR.

         if (!theConditionComponent)
         {
            // find the pointer to the ScheduleByObject (it has field type 2).

            for (int i=0; i<numberOfFieldTypes; i++)
            {
               if (theFieldTypes[i] == 2)
               {
                  ((ScheduleByObject *) theAnswerPtrs[i])->SetYearButton( TRUE );
                  break;
               }
            }
         }
         break;
      }

      case EXTENSIONListChanged:
      {
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
                             "no longer being used.",
                             GetTitle().data());
         if (this != curFront) G->GetDesktop()->SetFrontWindow(curFront);
         break;
      }

      default:
         CWwindow::DoCommand(theCommand, theData);

   }        // end switch(theCommand)
}           // end function




void GPKeyword::SizeWindow(int theWidth, int theHeigth)
{

   // propogate the resize message first, then set the size of
   // the scroller.

   CWwindow::SizeWindow(theWidth, theHeigth);

   if (theWidth < 500 || theHeigth < 200)
   {
      if (theWidth  < 500) theWidth  = 500;
      if (theHeigth < 200) theHeigth = 200;

      // this call will cause a recursive call back to this routine.

      Size (CRect(80,100,80+theWidth,100+theHeigth));
   }
   if (theScrollingFrame)
   {
      theScrollingFrame->Size(CRect(8,40,theWidth-8,theHeigth-48));
      theScrollingFrame->DoSetOrigin(CPoint(0,0));
   }

   DoDraw();
}


BOOLEAN GPKeyword::Close( void )
{
   if (theComponentBeingEdited) theComponentBeingEdited->openedWindow=NULL;

   if (theConditionComponent && theConditionComponent->openedWindow)
   {
      theConditionComponent->openedWindow->Close();
   }
   else
   {
      if (itsSchByCondWindow &&
          G->GetDesktop()->FindWindow(itsSchByCondWindow))
          itsSchByCondWindow->Close();
   }

   return CWwindow::Close();
}





GPKeyword::~GPKeyword( void )
{
   if (theFieldTypes) delete []theFieldTypes;
   if (theAnswerPtrs) delete []theAnswerPtrs;
}





void GPKeyword::DoMenuCommand(MENU_TAG theMenuItem,
                              BOOLEAN isShiftkey,
                              BOOLEAN isControlKey)
{
   switch(theMenuItem)
   {
      case M_SIM_PREP_VIEWKEYWORDS:
      {
         CStringRW viewTitle = "Keywords - ";
         viewTitle += componentName->GetTitle();

         CStringRW * tempKeys = new CStringRW();

         if (BuildTheKeywords (keywordMSText,
                               theFieldTypes,
                               theAnswerPtrs,
                               numberOfFieldTypes,
                               1,
                               *tempKeys))
            new ViewKeywords(tempKeys->data(), viewTitle.data());

         break;
      }

      default:
         CWindow::DoMenuCommand(theMenuItem,isShiftkey, isControlKey);
         break;
   }
}






int GPKeyword::BuildTheComponent( int standardAnswerForm )
{
//  return 0 if NOT ok
//  return 1 if ok
//
//  standardAnswerForm = 1 if we are building the standardKeywords.
//                       0 if we are building for the UseParms feature.

   int answerForm = 2;
   if (standardAnswerForm) answerForm = 1;

   CStringRW theCParms;

   if (theResultingComponent)
   {
      delete theResultingComponent;
      theResultingComponent = NULL;
   }

   int CType = 2;                          // standard component
   if (theConditionComponent) CType = 3;   // attached to a condition

   theResultingComponent = CreateComponent(
      (const char *) componentName->GetTitle(),
      "GPKeyword", NULL, NULL, NULL,
      itsExtension.data(), CType);

   if (theConditionComponent)
      theResultingComponent->conditionSerialNum =
         theConditionComponent->componentNumber;

   if (BuildTheKeywords (keywordMSText,
       theFieldTypes,theAnswerPtrs,numberOfFieldTypes,answerForm,
       *theResultingComponent->keywords))
   {
      StuffCParm((CStringRW &) theCParms,
                 (char const *) componentName->GetTitle());
      StuffCParm((CStringRW &) theCParms, keywordMSText->data());
      if (standardAnswerForm)
         BuildCParms (theFieldTypes,theAnswerPtrs,numberOfFieldTypes,
                     (CStringRW &) theCParms);
      *theResultingComponent->windowParms = theCParms;

      return 1;
   }
   else
   {
      delete theResultingComponent;
      theResultingComponent = NULL;
      return 0;
   }
}
