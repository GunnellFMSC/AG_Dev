/********************************************************************


   File Name:     clearwin.cpp
   Author:        kfd
   Date:          08/25/94


   Contents:      (see .hpp for further description)
      ClearcutWin
      DoCommand
      DoMenuCommand
      BuildCParmSettingString
      ResetTheComponentSettings
      BuildKeywordString
      InitializeWithParms


********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "runstr.hpp"
#include "compon.hpp"

#include CStringCollection_i
#include CStringRWC_i
#include NButton_i
#include NRadioButton_i
#include NListButton_i
#include CText_i
#include NEditControl_i
#include CMenuBar_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "clearwin.hpp"
#include "boxsldr.hpp"
#include "schedobj.hpp"
#include "schcond.hpp"

#include "fvslocs.hpp"     // needed by spfuncs
#include "spfuncs.hpp"     // for PKeyData
#include "spfunc2.hpp"
#include "cparmrtn.hpp"    // for Stuff/Get CParms
#include "suppdefs.hpp"
#include "warn_dlg.hpp"    // for testing


#include "supphelp.hpp"    // for help #defines
extern XVT_HELP_INFO helpInfo;



ClearcutWin::ClearcutWin(const CStringRW& theTitle,
                         Component * theComponent,
                         Component * theConditionComponent)

            :CMgtClassWindow((CDocument *) theSpGlobals->theAppDocument,
                             CenterTopWinPlacement(CRect(5,20,605,458)), theComponent,
                             theConditionComponent, theTitle, NULL,
                             WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                             W_DOC,TASK_MENUBAR)
{

   MakeStandardControls("Clearcut");

                           // create ScheduleByObject
                           // *** 100 is reserved for the  ****
                           // *** ScheduleBy DoCommand     ****
   scheduleBy = new ScheduleByObject(this, 100, itsScrollingFrame,
                                     CPoint(8,8) );

                           // diameter of smallest tree

   smallestTree = new BoxSliderObject(itsScrollingFrame, this,
                                      CRect(8,88,554,166),
                                      "Diameter",
                                      "5.1 0 200 0 100",
                                      SPLEFT,
                                      "Specify diameter of the smallest tree cut");

                           // number of legacy trees
   legacyTree = new BoxSliderObject(itsScrollingFrame, this,
                                    CRect(8,176,554,254),
                                    "Number of legacy trees per acre",
                                    "5 0 50 0 50",
                                    SPLEFT,
                                    "Specify number of legacy trees");


                           // diameter of legacy trees
   new CText(itsScrollingFrame, CPoint(8,268),
             CStringRW("Specify minimum diameter of legacy trees"));

   largeTree = new BoxSliderObject(itsScrollingFrame, this,
                                   CRect(48,288,554,340),
                                   "Diameter",
                                   "30.0 0 100 0 100",
                                   SPLEFT);
   largeTree->ActivateBoxSlider();

   new CText(itsScrollingFrame, CPoint(48,356),
             CStringRW("FVS computes the diameter of the "));
   new CText(itsScrollingFrame, CPoint(48,388),
             CStringRW("percentile tree in the distribution of trees."));

   RWOrdered items;
   items.insert(new CStringRWC ("50th"));
   items.insert(new CStringRWC ("70th"));
   items.insert(new CStringRWC ("90th"));

   percentile = new NListButton(itsScrollingFrame,
                                CRect(296,348,386,448),items,2);
   percentile->Disable();

   itsLargeGroup = new CRadioGroup(itsScrollingFrame,CPoint(8,298), FALSE);

   itsFirstButton  = itsLargeGroup->AddButton(CPoint(0,8),"",5);
   itsSecondButton = itsLargeGroup->AddButton(CPoint(0,56),"",6);
   itsLargeGroup->SetSelectedButton(itsFirstButton);

                              // initialize with parm file values
   InitializeWithParms();
                              // save initial settings for reset
   itsInitialSettings =  BuildCParmSettingString();


                              // (itsComponent) indicates that this is an
                              // edit of an existing component; reset settings
   if (itsComponent)
   {
      if (itsComponent->windowParms->length())
         ResetTheComponentSettings(itsComponent->windowParms->data());

      itsConditionComponent = FindConditionComponent();

   }

   // HELP file associations.

   WINDOW ctl=GetXVTWindow();
   xvt_help_set_win_assoc(helpInfo, ctl, CLEARWINWindow, 0L);
   AdjustScrollingWinSize(itsScrollingFrame, this);

   Show();
   itsNavigator->InitFocus();

}





void ClearcutWin::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 105:                  // free form
      {
         if (xvt_dm_post_ask("Proceed","Cancel",NULL,
                             "You are about to enter a text editor "
                             "used to edit parameters manually.  "
                             "Suppose does not ensure correct keyword "
                             "records when you use this editor.")
            == RESP_2) break;
      }

      case 1:
      {
                                 // ok button pushed
                                 // SetSave set by SaveTheComponent
                                 // if there is not condition, change radio
                                 // button to by year
         if (!itsConditionComponent)
            scheduleBy->SetYearButton(TRUE);

         int usingFreeForm = itsFreeFormCommand==theCommand;
         CStringRW keywordLine = BuildKeywordString();
         CStringRW settingsLine=NULLString;
         if (!usingFreeForm) settingsLine=BuildCParmSettingString();

         SaveTheComponent (usingFreeForm,"Clearcut",keywordLine,settingsLine);
         closeCalledByCloseCancel = FALSE;
         Close();
         break;
      }

      case 2:                    // the reset button
         ResetTheComponentSettings(itsInitialSettings);
         break;

      case 3:                    // cancel button pushed

         Close();
         break;

      case 5:                    // specify diameter radio button1 (diameter)

         percentile->Disable();
         largeTree->ActivateBoxSlider();
         break;

      case 6:                    // specify diameter radio button2 (percentile)

         percentile->Enable();
         largeTree->DeactivateBoxSlider();
         break;

      case 50:                   // ScheduleBy : Condition (existing)
      {
         CStringRW conditionTitle = "Edit - ";
         conditionTitle += (CStringRW&) *itsConditionComponent;
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
               this, "management.Clearcut", "defaultCondition",
               conditionTitle, itsConditionComponent);
         break;
      }

      case 51:                   // ScheduleBy : Condition (existing); set to
                                 // NULL since its closed if you're here
         itsSchByCondWindow = NULL;
         break;

      case 100:                  // ScheduleBy : Condition (Create)
      {
         CStringRW conditionTitle = "Condition - ";
         conditionTitle += itsComponentName->GetTitle();
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
               this, "management.Clearcut", "defaultCondition",
               conditionTitle, itsConditionComponent);
         break;
      }
      case 101:                  // ScheduleBy : Condition (return condition
                                 // string and Notify Distroyed)
      {
         itsConditionComponent = (Component *) theData;
         itsSchByCondWindow = NULL;

         CheckForNULLMgtCondition();
         if (itsConditionComponent)
            scheduleBy->ResetConditionButtonCommand(50);

         break;
      }
      default: CMgtClassWindow::DoCommand(theCommand, theData);
   }
}





CStringRW ClearcutWin::BuildCParmSettingString(void)
{
   CStringRW settingsLine;

                           // save user entered name
   StuffCParm(settingsLine,
              (char const *)itsComponentName->GetTitle());                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (smallestTree->GetBoxSliderSettings()).data());

                           // save boxslider
   StuffCParm((CStringRW &)settingsLine,
              (legacyTree->GetBoxSliderSettings()).data());

                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (largeTree->GetBoxSliderSettings()).data());

                           // save listbutton selection
   char *aBuf = new char [20];
   sprintf (aBuf, "%d", percentile->GetSelectPosition());
   StuffCParm((CStringRW &) settingsLine, aBuf);
   delete [] aBuf;

                           // save user radio selection
   if(itsLargeGroup->GetSelectedButton() == itsFirstButton)
      StuffCParm((CStringRW &) settingsLine, "0");
   else
      StuffCParm((CStringRW &) settingsLine, "1");

                            // save schedObj
   StuffCParm((CStringRW &) settingsLine,
               scheduleBy->GetSchedObjSettings().data());

   return settingsLine;
}





void ClearcutWin::ResetTheComponentSettings(const char * theSettings)
{

   {
                           // reset user entered name
      CStringRW tmpString =  GetCParm((const char *) theSettings, 0);
      itsComponentName->SetTitle(tmpString.data());

                           // reset user boxslider
      tmpString =  GetCParm((const char *) theSettings, 1);
      smallestTree->ResetBoxSliderSettings(tmpString.data());

                           // reset user boxslider
      tmpString =  GetCParm((const char *) theSettings, 2);
      legacyTree->ResetBoxSliderSettings(tmpString.data());

                           // reset user boxslider
      tmpString =  GetCParm((const char *) theSettings, 3);
      largeTree->ResetBoxSliderSettings(tmpString.data());

                           // reset user listbutton selection
      tmpString =  GetCParm((const char *) theSettings, 4);
      percentile->SelectItem(atoi(tmpString.data()));

                           // reset user radio selection
      tmpString =  GetCParm((const char *) theSettings, 5);
      if (strcmp(tmpString.data(), "0") == 0)
      {
         itsLargeGroup->SetSelectedButton(itsFirstButton);
         DoCommand(5);      // activate percentile / deactivate boxslider
      }
      else
      {
         itsLargeGroup->SetSelectedButton(itsSecondButton);
         DoCommand(6);      // deactivate percentile / activate boxslider
      }

                            // reset schedObj
      tmpString =  GetCParm((const char *) theSettings, 6);
      scheduleBy->ResetSchedObjSettings(tmpString.data());

   }

}





CStringRW ClearcutWin::BuildKeywordString(void)
{
   char keywordLine[200];
   char yearField  [11];
                           // scheduleValue schedobj value
                           // scheduleButton schedobj button number
   char * one    = "1.0";
   char * zero   = "0.0";
   char * nines  = "999.0";

   sprintf (yearField,"%10s",scheduleBy->SpGetTitle());

                      // selected = which button: diameter or percentile?
   long selected = itsLargeGroup->GetSelectedButton();

                     // if diameter entry selected is diameter
   if (selected == itsFirstButton)
   {
      sprintf(keywordLine,
         "ThinDBH   %10s%10s%10s%10s%10s%10s%10s\n"
         "ThinDBH   %10s%10s%10s%10s%10s%10s%10s",
         yearField,
         smallestTree->GetTitle().data(),
         largeTree->GetTitle().data(),
         one, zero, zero, zero,
         yearField,
         largeTree->GetTitle().data(),
         nines, one, zero,
         legacyTree->GetTitle().data(), zero);
   }

                     // if diameter entry selected is by percentile
   else
   {
      int percentileIndex = percentile->GetSelectPosition()+3;
      sprintf(keywordLine,
         "ThinDBH   %10s    Parms(%s, DBHDist (3,%d), 1.0, 0.0, 0.0, 0.0)\n"
         "ThinDBH   %10s    Parms(DBHDist (3,%d), 999., 1.0, 0.0, %s, 0.0)",
         yearField,
         smallestTree->GetTitle().data(),
         percentileIndex,
         yearField,
         percentileIndex,
         legacyTree->GetTitle().data());
   }

   CStringRW keyLine;
   keyLine = keywordLine;
   return keyLine;
}





void ClearcutWin::InitializeWithParms(void)
{
   const char *f1, *f2, *f3;
   const char *section = "management.Clearcut";

   // *****************             smallest tress box slider

   PKeyData * parms = GetPKeyData(section,"f1");

   // if there are entries in the parms file for this

   if (parms)
   {

      // title

      f1 = parms->pString.data();

      // slider settings

      f2 = NULL;
      parms = GetPKeyData(section, "f1v");
      if (parms) f2 = parms->pString.data();

      // optional title

      f3 = NULL;
      parms = GetPKeyData(section, "f1opt");
      if (parms) f3 = parms->pString.data();

      smallestTree->IBoxSliderObject(f1,f2,f3);
   }

   // *****************             number of legacy trees

   parms = GetPKeyData(section, "f2");

   // if there are entries in the parms file for this

   if (parms)
   {

      // title

      f1 = parms->pString.data();

      // slider settings

      f2 = NULL;
      parms = GetPKeyData(section, "f2v");
      f2 = parms->pString.data();

      // optional title

      f3 = NULL;
      parms = GetPKeyData(section, "f2opt");
      if (parms) f3 = parms->pString.data();

      legacyTree->IBoxSliderObject(f1,f2,f3);
   }

   // *****************             diameter of large trees

   parms = GetPKeyData(section, "f3");

   // if there are entries in the parms file for this

   if (parms)
   {
      f1 = parms->pString.data();

      f2 = NULL;
      parms = GetPKeyData(section, "f3v");
      if (parms) f2 = parms->pString.data();

      f3 = NULL;
      parms = GetPKeyData(section, "f3opt");
      if (parms) f3 = parms->pString.data();

      largeTree->IBoxSliderObject(f1,f2,f3);
   }
}

