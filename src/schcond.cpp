/*********************************************************************

   File Name:    schcond.cpp
   Author:       nlc
   Date:         12/23/94

   see schcond.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include "compon.hpp"
#include "group.hpp"
#include "extens.hpp"
#include "stand.hpp"
#include "cretcomp.hpp"
#include "readcomp.hpp"
#include "dirsept.hpp"

#include CText_i
#include NButton_i
#include NListButton_i
#include NEditControl_i
#include NListBox_i
#include CNavigator_i
#include CMenuBar_i
#include CStringCollection_i
#include CUnits_i
#include CView_i
#include CWindow_i
#include CDocument_i
#include CStringRWC_i
#include CBoss_i
#include CApplication_i
#include CDesktop_i

#include "mycscrol.hpp"
#include "spparms.hpp"
#include "suppdefs.hpp"
#include "fvslocs.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>

#include "schcond.hpp"
#include "schedobj.hpp"
#include "boxsldr.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "viewkey.hpp"
#include "cparmrtn.hpp"
#include "express.hpp"

#include "supphelp.hpp"    // for help #defines
#include <float.h>

extern XVT_HELP_INFO helpInfo;    // help file

SchByCond::SchByCond(CWindow * theParent, int callBackNumber,
                     const char * defaultCondition,
                     const CStringRW & theTitle,
                     Component * currentComponent,
                     Component * dependentComponent)

          :CWwindow((CDocument *) theSpGlobals->theAppDocument,
                    RightWinPlacement(CRect(40,25,640,465)), theTitle,
                    WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                    W_DOC, TASK_MENUBAR),
           itsOriginalTitle(theTitle),
           IamBeingClosedCallBack(callBackNumber),
           myParentIs(theParent),
           theDefaultCondition(defaultCondition),
           theComponentBeingEdited(currentComponent),
           theDependentComponent(dependentComponent),
           closeCalledByCloseCancel(TRUE),
           theResultingComponent(NULL),
           theFieldTypes(NULL),
           theAnswerPtrs(NULL),
           listOfConditionsPKeyData(NULL),
           existingConditionListBox(NULL),
           existingConditionList(NULL),
           existingConditionInUse(0)
{

   // set window id

   this->SetId(SCHBYCONDWindowID);

   // create and clear the navigator

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   // set up the universal buttons.

   okButton          = new NButton(this, CRect(  8,400, 96,432), "Ok");
   useExistingButton = new NButton(this, CRect(112,400,262,432),
                                   "Attach to...");
   freeFormButton    = new NButton(this, CRect(278,400,384,432), "Use Editor");
   resetButton       = new NButton(this, CRect(400,400,488,432), "Reset");
   cancelButton      = new NButton(this, CRect(504,400,592,432), "Cancel");

   okButton          -> SetCommand( 1);
   useExistingButton -> SetCommand( 2);
   resetButton       -> SetCommand( 3);
   cancelButton      -> SetCommand( 4);
   freeFormButton    -> SetCommand( 5);

   new CText(this, CPoint(8,12), CStringRW("Name: "));

   if (theComponentBeingEdited)
   {
      componentName = new NEditControl(this, CRect(82,4,592,36),
         (CStringRW &) *theComponentBeingEdited, 0L);

      // save the address of the window being used to edit the component.

      theComponentBeingEdited->openedWindow=this;
      useExistingButton->Disable();
   }
   else
      componentName = new NEditControl(this, CRect(82,4,592,36),
                                       theTitle, 0L);
   existingConditionList = new RWOrdered (20);
   theSpGlobals->runStream->
                 BuildSubSetComponentList(existingConditionList,
                                          1, NULL, NULL,
                                          theDependentComponent);

   if (existingConditionList->isEmpty()) useExistingButton->Disable();

   libTypeList.insert(new CStringRWC("Condition library:"));
   libTypeList.insert(new CStringRWC("Your conditions:"));

   libTypeListButton = new NListButton(this, CRect(4,40,168,148),
                                       libTypeList, 0, 0L);
   libTypeListButton->SetCommand(6);

   parmsLibList.insert(new CStringRWC(" "));
   libraryListButton = new NListButton(this, CRect(172,40,592,312),
                                       parmsLibList, 0, 0L);
   libraryListButton->SetCommand(7);

                                              // 544 = 68*8, 3000 = 375*8
   theScrollingWindow = new MyCScroller(this, CRect(4,76,592,360), 544, 3000);

   waitingPeriodText = new CText(this, CPoint(8,375), CStringRW(
      "Years before condition can be tested after being found true: "));

   // Alternative wording? "Mandatory delay, in years, between events:"

   waitingPeriod = new NEditControl(this, CRect(504,364,592,396), "0", 0L);

   // do the Reset command.

   DoCommand(3, NULL);

   libraryListButton ->SetGlue(BOTTOMSTICKY | TOPSTICKY );
   theScrollingWindow->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                               RIGHTSTICKY  | LEFTSTICKY);

   okButton          -> SetGlue(BOTTOMSTICKY);
   useExistingButton -> SetGlue(BOTTOMSTICKY);
   freeFormButton    -> SetGlue(BOTTOMSTICKY);
   resetButton       -> SetGlue(BOTTOMSTICKY);
   cancelButton      -> SetGlue(BOTTOMSTICKY);
   waitingPeriod     -> SetGlue(BOTTOMSTICKY);
   waitingPeriodText -> SetGlue(BOTTOMSTICKY);

   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), SCHCONDWindow, 0L);

   aNav->AppendTabStop(new CTabStop(okButton));
   aNav->AppendTabStop(new CTabStop(useExistingButton));
   aNav->AppendTabStop(new CTabStop(freeFormButton));
   aNav->AppendTabStop(new CTabStop(resetButton));
   aNav->AppendTabStop(new CTabStop(cancelButton));
   aNav->AppendTabStop(new CTabStop(componentName));
   aNav->AppendTabStop(new CTabStop(libTypeListButton));
   aNav->AppendTabStop(new CTabStop(libraryListButton));
   aNav->AppendTabStop(new CTabStop(theScrollingWindow));
   //   theScrollingWindow->AppendToNavigator(aNav);
   aNav->AppendTabStop(new CTabStop(waitingPeriod));

   Show();

   aNav->InitFocus();
}




void SchByCond::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                // Ok
      {
         if (existingConditionInUse)
         {
            int position = existingConditionListBox->GetSelectPosition();
            if (position > -1)
            {
               theResultingComponent =
                  (Component *) (*existingConditionList)[position];
            }
            else theResultingComponent = NULL;
         }
         else BuildTheComponent();
         closeCalledByCloseCancel = FALSE;
         Close();
         break;
      }

      case 2:                // Use Existing Condition
      {
         ClearScroller();
         if (!existingConditionListBox)
         {
            existingConditionListBox = new NListBox(this, CRect(4,108,360,360),
                                                    *existingConditionList, 0L);
            existingConditionText = new CText(this, CPoint(8,76),
               CStringRW("Attach to an existing condition: "));

            existingConditionListBox->SetDoubleCommand(1);
         }

         existingConditionInUse = 1;
         DoCommand (20,NULL);
         ShowExisting();
         break;
      }

      case 3:                // Reset
      {
         ClearScroller();
         HideExisting();
         if (theComponentBeingEdited)
         {
            componentName->SetTitle((CStringRW &) *theComponentBeingEdited);
            int item = -2;
            ISchByCond(NULL);
            sscanf (GetCParm(theComponentBeingEdited->windowParms->data(),
                             0).data(),
                    "%d", &item);
            if (item > -1)
            {
               libraryListButton->SelectItem(item);
               LoadScroller();
               waitingPeriod->
                  SetTitle(GetCParm(theComponentBeingEdited->
                                    windowParms->data(),
                                    1));
               ResetObjects (theFieldTypes,theAnswerPtrs,
                             numberOfFieldTypes, 2,
                             theComponentBeingEdited->windowParms->data());

            }
          }
         else
         {
            ISchByCond(theDefaultCondition);
            componentName->SetTitle(itsOriginalTitle);
         }

         existingConditionInUse = 0;

         break;
      }

      case 4:                // Cancel
      {
         Close();
         break;
      }

      case 5:                // FreeForm
      {
         if (xvt_dm_post_ask("Proceed","Cancel",NULL,
                             "You are about to enter a text editor "
                             "used to edit parameters manually.  "
                             "Suppose does not ensure correct keyword "
                             "records when you use this editor.")
             == RESP_2) break;

         Hide();

         BuildTheComponent(TRUE);
         if (theResultingComponent)
         {
            CStringRW theTitle = "Edit - ";
            theTitle += (CStringRW &) *theResultingComponent;
            new BuildExpression(theTitle, theResultingComponent, NULL, 1);
         }
         closeCalledByCloseCancel = FALSE;
         Close();        
         break;
      }

      case 6:                // Library type list button
      {
         if (libTypeListButton->GetSelectPosition() == 0)
         {
            libraryListButton->IListButton(parmsLibList, 0);
            DoCommand(3);
         }
         else
         {
            if (usrLibList.isEmpty())
            {
               LoadUsrLibList();
               if (usrLibList.isEmpty())
               {
                  DoCommand(3);
               }
               else
               {
                  libraryListButton->IListButton(usrLibList, 0);
                  ClearScroller();
                  existingConditionInUse = 0;
                  HideExisting();
               }
            }
         }
         break;
      }

      case 7:                // LibraryListButton
      {
         if (libTypeListButton->GetSelectPosition() == 0)
         {
            ClearScroller();
            LoadScroller();
            existingConditionInUse = 0;
            HideExisting();
         }
         else
         {
            int selectedCondition = libraryListButton->GetSelectPosition();
            if (selectedCondition > 0)
            {
               theResultingComponent = (Component *) usrLibList.
                  at((size_t) selectedCondition);

               if (xvt_dm_post_ask("Ok","Cancel",NULL,
                                   "Ok to use: %s",
                                   theResultingComponent->data())
                   == RESP_DEFAULT)
               {
                  if (theComponentBeingEdited) theComponentBeingEdited->
                     SetComponent(theResultingComponent);
                  else theResultingComponent =
                       new Component(theResultingComponent);
                  closeCalledByCloseCancel = FALSE;
                  Close();
               }
               theResultingComponent = NULL;
            }
         }
         break;
      }

      case EXTENSIONListChanged:
      {
         if (theComponentBeingEdited &&
             theComponentBeingEdited->extension &&
             theSpGlobals->theExtensions->
                FindLegalExtension(theComponentBeingEdited->
                                   extension->data()) == -1)
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

      case 20:  // a branch form case 2
      case COMPONENTSListChanged:
      {
         if (theDependentComponent)
         {
            int beenDeleted = 1;
            if (theSpGlobals->runStream->allComponents->entries())
            {
               RWOrderedIterator
                     nextComponent(*theSpGlobals->runStream->allComponents);
               Component * component;
               while (component = (Component *) nextComponent())
               {
                  if (theDependentComponent == component)
                  {
                     beenDeleted = 0;
                     break;
                  }
               }
               if (beenDeleted) theDependentComponent = NULL;
            }
         }
      }
      case CURRStandOrGroupChange:
      {
         if (existingConditionListBox)
         {
            theSpGlobals->runStream->
               BuildSubSetComponentList(existingConditionList,
                                        1, NULL, NULL,
                                        theDependentComponent);
            existingConditionListBox->IListBox(*existingConditionList);
            if (!existingConditionList->isEmpty())
               existingConditionListBox->DeselectAll();
         }
         break;
      }
      default: CWwindow::DoCommand(theCommand,theData); break;
   }
}






void SchByCond::ClearScroller( void )
{
   if (theFieldTypes)
   {
      delete []theFieldTypes;
      theFieldTypes = NULL;
   }
   if (theAnswerPtrs)
   {
      delete []theAnswerPtrs;
      theAnswerPtrs = NULL;
   }

   // put the view keyword menu item to sleep.

   GetMenuBar()->SetEnabled (M_SIM_PREP_VIEWKEYWORDS, FALSE);

   // go through the scroller and delete the the subviews.
   // it seems we need to iterate until the scrolling window is empty.

   CNavigator * aNav = GetNavigator();
   CTabStop   * aStop;
   while (!theScrollingWindow->GetSubviews()->isEmpty())
   {
      CSubview * aSubview;
      RWOrderedIterator subviewIterator (*theScrollingWindow->GetSubviews());

      while ((aSubview = (CSubview *) subviewIterator()) != NULL)
      {
         if ((aStop = aNav->FindTabStopHavingView(aSubview)))
         {
            aNav->RemoveTabStop(aStop);
            delete aStop;
         }
         RemoveSubview(aSubview);
         delete aSubview;
      }
   }
   theScrollingWindow->IScroller(FALSE, TRUE, TRUE, 24, 240);
   HideCondition();
}





void SchByCond::LoadScroller( void )
{
   void * theAnswer = NULL;

   // if nothing is selected, then return.

   int selectedCondition = libraryListButton->GetSelectPosition();
   selectedCondition--;
   if (selectedCondition < 0) return;

   // get the PKey for the selected condition...use it to find
   // the data for the condition in the parameters file.

   CStringRWC searchString = "condition.";
   searchString += *listOfConditionsPKeyData[selectedCondition];

   numberConditionPKeys = 0;
   conditionMSTextPointer = (MSText *) theSpGlobals->
            theParmsData->find(&searchString);
   if (conditionMSTextPointer)
   {
      conditionPKeyDataCollection = conditionMSTextPointer->GetThePKeys();
      numberConditionPKeys = (int) conditionPKeyDataCollection->entries();
   }

   if (numberConditionPKeys <= 0)
   {
      xvt_dm_post_error("Suppose can not find data to define the\n"
                        "selected condition in the paramters file.\n"
                        "The needed major section is:\n%s\n",
                        searchString.data());

      return;
   }

   // the numberConditionPKeys is usually larger, but
   // never smaller, than the number of fields needed.

   theFieldTypes = new int[numberConditionPKeys];
   theAnswerPtrs = new void* [numberConditionPKeys];
   for (int i=0; i<numberConditionPKeys; i++)  theAnswerPtrs[i]=NULL;

   // set up the starting vertical point.

   startPoint = 8;

   int fieldNumber = 0;
   int fieldType = 0;
   numberOfFieldTypes = 0;
   while (fieldType > -1)
   {
      theAnswer = BuildField((CWindow *) NULL, 0, theScrollingWindow,
                              conditionMSTextPointer, &startPoint,
                              ++fieldNumber, &fieldType);
      if (fieldType == -1) break;
      theFieldTypes[numberOfFieldTypes]  =fieldType;
      theAnswerPtrs[numberOfFieldTypes++]=theAnswer;
      theScrollingWindow->SetVirtualFrame( 544, startPoint+500);
   }

   theScrollingWindow->ShrinkToFit();

   // check to see if there is a default waiting years PKey...if so,
   // set the default.

   PKeyData * waitYearsPKeyData = conditionMSTextPointer->
      FindPKeyData("waitYears",
                   theSpGlobals->theVariants->GetSelectedVariant(),
                   FALSE);

   if (waitYearsPKeyData) waitingPeriod->
                             SetTitle(waitYearsPKeyData->pString);

   // wake up view component keyword menu item

   if (theFieldTypes) GetMenuBar()->SetEnabled (M_SIM_PREP_VIEWKEYWORDS,
                                                TRUE);

   ShowCondition();
}






void SchByCond::ISchByCond(const char * defaultCondition)
{
   // delete the current conditions list.

   if (listOfConditionsPKeyData)
   {
      delete []listOfConditionsPKeyData;
      listOfConditionsPKeyData = NULL;
   }

   // make sure the conditionsLists are empty.

   libTypeListButton->SelectItem(0);
   if (!usrLibList.isEmpty())
   {
      usrLibList.removeAt((size_t) 0);
      usrLibList.clearAndDestroy();
   }

   parmsLibList.clearAndDestroy();
   parmsLibList.insert(new CStringRWC(">>>>>  No Selected Condition  <<<<<<"));
   libraryListButton->IListButton(parmsLibList, 0);

   // look for the conditions (rules) part of the parameters file.

   CStringRWC searchString = "conditions_list";
   int numberConditions = 0;
   RWOrdered * conditionsPKeyData = NULL;
   MSText * neededMSTextPointer =  (MSText *) theSpGlobals->
         theParmsData->find(&searchString);
   if (neededMSTextPointer)
   {
      conditionsPKeyData = neededMSTextPointer->GetThePKeys();
      numberConditions = (int) conditionsPKeyData->entries();
   }

   // if the number of conditions is zero, issue an
   // error message and return.

   if (numberConditions <= 0)
   {
      xvt_dm_post_error("Suppose can not find data for the\n"
                        "condition in the paramters file.\n"
                        "The needed major section is:\n%s\n",
                        searchString.data());
      return;
   }

   // otherwise, allocate memory for pointers to extensions
   // and load the pointers to the extension PKeydata.

   listOfConditionsPKeyData = new ptrToPKeyData [numberConditions];

   // iterate over all conditions in the list, build entries into
   // the list button's list...while were at it, look for the default.

   int indexToDefault=-1;
   numberConditionsInPtrList = 0;
   RWOrderedIterator nextCondition(*conditionsPKeyData);
   PKeyData * conditionData;
   while (conditionData = (PKeyData *) nextCondition())
   {
      listOfConditionsPKeyData[numberConditionsInPtrList] = conditionData;
      parmsLibList.insert(new CStringRWC(conditionData->pString));

      // search for the default.

      if (defaultCondition &&
          theDefaultCondition &&
          strcmp(theDefaultCondition,conditionData->data()) == 0)
         indexToDefault = numberConditionsInPtrList;
      numberConditionsInPtrList++;
   }

   // load the library list.

   libraryListButton->IListButton(parmsLibList, 0);
   if (indexToDefault > -1)
   {
      libraryListButton->SelectItem(indexToDefault+1);
      LoadScroller();
   }
}



BOOLEAN SchByCond::Close( void )
{
   if (closeCalledByCloseCancel && theResultingComponent &&
       theResultingComponent != theComponentBeingEdited)
   {
      delete theResultingComponent;
      theResultingComponent = NULL;
   }

   if (theComponentBeingEdited)
   {
      theComponentBeingEdited->openedWindow=NULL;
      theResultingComponent = theComponentBeingEdited;
   }

   if (myParentIs) myParentIs->DoCommand((long) IamBeingClosedCallBack,
                               (void *) theResultingComponent);
   return CWwindow::Close();
}






SchByCond::~SchByCond( void )
{
   // delete the current conditions list.

   parmsLibList.clearAndDestroy();
   if (!usrLibList.isEmpty())
   {
      // simply remove the first one because it is actually not
      // a component.

      usrLibList.removeAt((size_t) 0);
      usrLibList.clearAndDestroy();
   }
   libTypeList.clearAndDestroy();

   if (listOfConditionsPKeyData) delete []listOfConditionsPKeyData;
   if (theFieldTypes)            delete []theFieldTypes;
   if (theAnswerPtrs)            delete []theAnswerPtrs;
   if (existingConditionList)    delete existingConditionList;
}






void SchByCond::BuildIfKeyword( CStringRW * keywords )
{
   char tmp[21];
   int  wait;
   sscanf (waitingPeriod->GetTitle().data(),"%d", &wait);
   sprintf (tmp,"IF%18d\n",wait);
   *keywords = tmp;
}






void SchByCond::BuildTheComponent(int freeForm)
{

   if (theComponentBeingEdited)
   {
      (CStringRW &) *theComponentBeingEdited = componentName->GetTitle();

      // insure the component type is 1 (condition)...thus a quick edit
      // will fix the component type of a component read from a file.

      theComponentBeingEdited->componentType=1;
      theResultingComponent=theComponentBeingEdited;

      // SetSave only if we are editing.

      theSpGlobals->theAppDocument->SetSave(TRUE);
   }
   else
   {
      delete theResultingComponent;

      theResultingComponent = CreateComponent(
         (const char *) componentName->GetTitle(),
         "SchByCond", NULL, NULL, NULL, NULL, 1);
   }

   CStringRW theAnswerString;

   BuildIfKeyword (&theAnswerString);

   int built = BuildTheKeywords (conditionMSTextPointer, theFieldTypes,
                                 theAnswerPtrs, numberOfFieldTypes, 0,
                                 theAnswerString);
   if (built)
   {
      *theResultingComponent->keywords = theAnswerString;
      itsInitialSettings = "";
      char tmp[5];
      sprintf (tmp,"%4d",libraryListButton->GetSelectPosition());
      StuffCParm((CStringRW &) itsInitialSettings, tmp);
      StuffCParm((CStringRW &) itsInitialSettings,
                 waitingPeriod->GetTitle().data());
      BuildCParms (theFieldTypes,theAnswerPtrs,numberOfFieldTypes,
                  (CStringRW &) itsInitialSettings);

      *theResultingComponent->windowParms = itsInitialSettings;
   }
   else
   {
      if (theComponentBeingEdited)
         xvt_dm_post_error("Condition could not be built.  The condition\n"
                           "being edited was not changed.");
      else if (freeForm)
      {
         theAnswerString+="\nThen";
         *theResultingComponent->keywords = theAnswerString;
      }
      else
      {
         xvt_dm_post_error("Condition could not be built.  No condition\n"
                           "will be assigned for this activity.");
         delete theResultingComponent;
         theResultingComponent = NULL;
      }
   }
}





void SchByCond::DoMenuCommand(MENU_TAG theMenuItem,
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

         BuildIfKeyword (tempKeys);

         if (!BuildTheKeywords (conditionMSTextPointer,
               theFieldTypes, theAnswerPtrs, numberOfFieldTypes, 0,
               *tempKeys))
            xvt_dm_post_warning("You have not selected a condition!");
         else
            new ViewKeywords(tempKeys->data(), viewTitle.data());


         break;
      }

      default:
         CWindow::DoMenuCommand(theMenuItem,isShiftkey, isControlKey);
         break;
   }
}



void SchByCond::HideCondition( void )
{
   theScrollingWindow->DoHide();
   waitingPeriod     ->DoHide();
   waitingPeriodText ->DoHide();
   DoDraw();
}



void SchByCond::ShowCondition( void )
{
   theScrollingWindow->DoShow();
   waitingPeriod     ->DoShow();
   waitingPeriodText ->DoShow();
   DoDraw();
}




void SchByCond::HideExisting( void )
{
   if (existingConditionListBox)
   {
      existingConditionListBox->DoHide();
      existingConditionText   ->DoHide();
      DoDraw();
   }
}



void SchByCond::ShowExisting( void )
{
   if (existingConditionListBox)
   {
      existingConditionListBox->DoShow();
      existingConditionText   ->DoShow();
      DoDraw();
   }
}



void SchByCond::LoadUsrLibList( void )
{
   // try to open and read the usr lib (filename: condlib.kcp)

   xvt_fsys_save_dir();
   FILE_SPEC fileSpec;
   strcpy(fileSpec.type,"kcp");
   strcpy(fileSpec.name,"condlib.kcp");
   xvt_fsys_get_dir(&fileSpec.dir);
   FILE * filePtr = fopen (fileSpec.name,"r");
   if (!filePtr)
   {
      if (xvt_dm_post_ask ("No","Yes",NULL,
                           "condlib.kcp could not be opened. "
                           "Do you want to specify a different file?")
          == RESP_2)
      {
         while (!filePtr)
         {
            if (xvt_dm_post_file_open(&fileSpec, "User rule library:")
                == FL_OK)
            {
               xvt_fsys_get_dir(&fileSpec.dir);
               filePtr = fopen (fileSpec.name,"r");
               if (!filePtr)
               {
                  char path[SZ_FNAME];
                  xvt_fsys_convert_dir_to_str(&fileSpec.dir, path,
                                              sizeof(path));
                  xvt_dm_post_message("%s%s%s could not be opened for input.",
                                      path, SEPARATOR, fileSpec.name);
               }
            }
            else break;
         }
      }
   }
   if (filePtr)
   {
      RWOrdered tmpLibList;
      ReadComponentsFromFile(filePtr, &fileSpec, &tmpLibList, FALSE);
      fclose(filePtr);
      RWOrderedIterator nextRule(tmpLibList);
      Component * component;
      while (component = (Component *) nextRule())
      {
         if (component->componentType == 1)
         {
            if (usrLibList.isEmpty())
               usrLibList.insert(parmsLibList.first());
            usrLibList.insert(component);
         }
         else delete component;
      }
   }

   xvt_fsys_restore_dir();
}
