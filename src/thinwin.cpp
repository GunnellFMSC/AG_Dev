/********************************************************************

   File Name:     thinwin.cpp   (thinfrom)
   Author:        kfd & nlc 10/2006
   Date:          10/17/94


   Contents:      ThinFromWindow()
                  DoCommand
                  DoMenuCommand


********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"
#include <math.h>

#include "compon.hpp"

#include CStringCollection_i
#include CStringRWC_i
#include NButton_i
#include NRadioButton_i
#include NListButton_i
#include CStringRW_i
#include CText_i
#include NCheckBox_i
#include NLineText_i
#include NEditControl_i
#include NGroupBox_i
#include CDesktop_i
#include CMenuBar_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "boxsldr.hpp"
#include "schedobj.hpp"
#include "thinwin.hpp"
#include "msgwin.hpp"
#include "warn_dlg.hpp"
#include "cparmrtn.hpp"

#include "spglobal.hpp"
#include "spparms.hpp"
#include "fvslocs.hpp"
#include "schcond.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "viewkey.hpp"

#include "suppdefs.hpp"
#include "supphelp.hpp"

#define ABOVE 0
#define BELOW 1

extern XVT_HELP_INFO helpInfo;



ThinFromWin::ThinFromWin(const CStringRW& theTitle,
                         int aboveOrBelow,
                         Component * theComponent,
                         Component * theConditionComponent)

            : CMgtClassWindow((CDocument *) theSpGlobals->theAppDocument,
                              CenterTopWinPlacement(CRect(5,20,605,458)), theComponent,
                              theConditionComponent, theTitle, NULL,
                              WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                              W_DOC, TASK_MENUBAR)
{

                  // determine if this is thin from above or below
   if (aboveOrBelow == 0)
      itsAboveOrBelow = ABOVE;
   else
      itsAboveOrBelow = BELOW;

   CStringRW name;
   if (itsAboveOrBelow == BELOW)
      name = "Thin from Below";
   else
      name = "Thin from Above";

   MakeStandardControls(name);

   new CText(itsScrollingFrame, CPoint(8,80),
             CStringRW("Specify residual density"));

                           // create ScheduleByObject
                           // *** 100 is reserved for the  ****
                           // *** ScheduleBy DoCommand     ****
   scheduleBy = new ScheduleByObject(this, 100, itsScrollingFrame,
                                     CPoint(8,8));


                           // specify residual density radio buttons
   itsDensityRadioGroup = new CRadioGroup(itsScrollingFrame,
                                          CPoint(24,120), FALSE);

   itsDensityFirst  = itsDensityRadioGroup->AddButton(CPoint(0,8),"",6);
   itsDensitySecond = itsDensityRadioGroup->AddButton(CPoint(0,116),"",7);
   itsDensityThird  = itsDensityRadioGroup->AddButton(CPoint(0,166),"",8);
   itsDensityRadioGroup->SetSelectedButton(itsDensityFirst);

   itsTreesPerAcre = new BoxSliderObject(itsScrollingFrame, this,
                                         CRect(56,108,554,158),
                                         "Trees per acre",
                                         "300 0 5000 100 500",
                                         SPLEFT);
   itsTreesPerAcre->SetCommand(23);

   CStringRW j=BuildSpacingFromTPA("300 0 5000 100 500");
   itsTreeSpacing = new BoxSliderObject(itsScrollingFrame, this,
                                        CRect(56,160,554,212),
                                        "Tree spacing (feet)", j, SPLEFT);

   itsTreeSpacing->SetCommand(24);

   itsBasalArea = new BoxSliderObject(itsScrollingFrame,
                                      CRect(56,216,554,266),
                                      "Basal area per acre",
                                      "250   0 2000   0 500",
                                      SPLEFT);
   itsBasalArea->DeactivateBoxSlider();

                           // specify residual density radio buttons

   new CText(itsScrollingFrame, CPoint(56,288), CStringRW("Percent of "));

   itsPercentRadioGroup = new CRadioGroup(itsScrollingFrame,
                                          CPoint(160,288), FALSE);
   itsPercentFirst  = itsPercentRadioGroup->AddButton(CPoint(0,0),
                                                      "trees per acre",9);
   itsPercentSecond = itsPercentRadioGroup->AddButton(CPoint(144,0),
                                                      "basal area at year of thin",10);
   itsPercentRadioGroup->SetSelectedButton(itsPercentFirst);

   itsPercentFirstButton = (NRadioButton*) itsPercentRadioGroup->
                                           FindSubview(itsPercentFirst);
   itsPercentFirstButton->Disable();

   itsPercentSecondButton = (NRadioButton*) itsPercentRadioGroup->
                            FindSubview(itsPercentSecond);
   itsPercentSecondButton->Disable();

   itsPercent = new BoxSliderObject(itsScrollingFrame,
                                    CRect(56,305,554,355),
                                    "", "70 0 100 0 100",
                                    SPLEFT);
   itsPercent->DeactivateBoxSlider();

   int top=363;

   itsPropLeft = new BoxSliderObject(itsScrollingFrame,
                                     CRect(16,top,554,top+50),
                                     "Proportion of trees left (spacing adjustment, 1-CutEff)",
                                     "0 0 1 0 1", SPLEFT);
   top += 58;

   new CText(itsScrollingFrame, CPoint(8,top),
             CStringRW("Specify tree size limits of thinning"));
   top += 20;

   itsDiameterLower = new BoxSliderObject(itsScrollingFrame,
                                          CRect(16,top,554,top+50),
                                          "Diameter lower limits (inches)",
                                          "0 0 10000 0 1000",
                                          SPLEFT);
   top += 58;

   itsDiameterUpper = new BoxSliderObject(itsScrollingFrame,
                                          CRect(16,top,554,top+50),
                                          "Diameter upper limits (inches)",
                                          "1000 0 10000 0 1000",
                                          SPLEFT);
   top += 58;

   itsHeightLower = new BoxSliderObject(itsScrollingFrame,
                                        CRect(16,top,554,top+50),
                                        "Height lower limits (feet)",
                                        "0 0 10000 0 1000 ",
                                        SPLEFT);
   top += 58;

   itsHeightUpper = new BoxSliderObject(itsScrollingFrame,
                                        CRect(16,top,554,top+50),
                                        "Height upper limits (feet)",
                                        "1000 0 10000 0 1000",
                                        SPLEFT);


                              // initialize with parm file values
   InitializeWithParms();
                              // save initial settings for reset
   itsInitialSettings =  BuildCParmSettingString();


                              // (itsComponent) indicates that this is an
                              // edit of an existing component; reset settings
   if (itsComponent)
   {
      if (itsComponent->windowParms)
         ResetTheComponentSettings(itsComponent->windowParms->data());

      itsConditionComponent = FindConditionComponent();

   }
   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), THINWINWindow, 0L);
   AdjustScrollingWinSize(itsScrollingFrame, this);
   Show();
   itsNavigator->InitFocus();
}





void ThinFromWin::DoCommand(long theCommand,void* theData)
{
   // NOTE:  checkboxes and radio buttons may seem to be unnecessarily
   // set (unset).  The reason for this is that the DoCommand is called
   // by other routines (InitializeWithParms, ResetTheSettings).  In order
   // to trigger correct action, these objects must be set.

   CEnvironment tempEnv;

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

      case 1:                    // Ok button was pressed
      {                          // SetSave set by SaveTheComponent
                                 // if there is not condition, change radio
                                 // button to by year
         if (!itsConditionComponent) scheduleBy->SetYearButton(TRUE);

         int usingFreeForm = itsFreeFormCommand==theCommand;
         CStringRW keywordLine = BuildKeywordString(itsAboveOrBelow);
         CStringRW settingsLine=NULLString;
         if (!usingFreeForm) settingsLine=BuildCParmSettingString();

         SaveTheComponent (usingFreeForm,
                          (itsAboveOrBelow == ABOVE) ? "ThinAbove" : "ThinBelow",
                          keywordLine,settingsLine);

         closeCalledByCloseCancel = FALSE;
         Close();
         break;
      }

      case 2:                     // Reset button was pressed.
      {
         if (itsComponent && itsComponent->windowParms)
              ResetTheComponentSettings(itsComponent->windowParms->data());
         else ResetTheComponentSettings(itsInitialSettings.data());
         break;
      }

      case 3:                     // Cancel button was pressed.
      {
         Close();
         break;
      }

      case 6:                    // radio button 1
         itsDensityRadioGroup->SetSelectedButton(itsDensityFirst);
         itsTreesPerAcre->ActivateBoxSlider();
         itsTreeSpacing->ActivateBoxSlider();
         itsBasalArea->DeactivateBoxSlider();
         itsPercent->DeactivateBoxSlider();
         itsPercentFirstButton->Disable();
         itsPercentSecondButton->Disable();
         break;

      case 7:                    // radio button 2
         itsDensityRadioGroup->SetSelectedButton(itsDensitySecond);
         itsBasalArea->ActivateBoxSlider();
         itsTreesPerAcre->DeactivateBoxSlider();
         itsTreeSpacing->DeactivateBoxSlider();
         itsPercent->DeactivateBoxSlider();
         itsPercentFirstButton->Disable();
         itsPercentSecondButton->Disable();
         break;

      case 8:                    // radio button 3
      {
         itsDensityRadioGroup->SetSelectedButton(itsDensityThird);
         itsPercent->ActivateBoxSlider();
         itsTreesPerAcre->DeactivateBoxSlider();
         itsTreeSpacing->DeactivateBoxSlider();
         itsBasalArea->DeactivateBoxSlider();
         itsPercentFirstButton->Enable();
         itsPercentSecondButton->Enable();
         break;
      }

      case 9:                    // minor radio button 1
      {
         itsPercentRadioGroup->SetSelectedButton(itsPercentFirst);
         break;
      }

      case 10:                    // minor radio button 2
      {
         itsPercentRadioGroup->SetSelectedButton(itsPercentSecond);
         break;
      }

      case 23:                   // itsTreesPerAcre Boxslider
      {
         CStringRW settingsTPA,settingsSpacing;
         settingsTPA=itsTreesPerAcre->GetBoxSliderSettings();
         settingsSpacing=BuildSpacingFromTPA(settingsTPA);
         itsTreeSpacing->ResetBoxSliderSettings(settingsSpacing);
         break;
      }

      case 24:                   // itsTreeSpacing Boxslider
      {
         CStringRW settingsTPA,settingsSpacing;
         settingsSpacing=itsTreeSpacing->GetBoxSliderSettings();
         settingsTPA=BuildTPAFromSpacing(settingsSpacing);
         itsTreesPerAcre->ResetBoxSliderSettings(settingsTPA);
         settingsSpacing=BuildSpacingFromTPA(settingsTPA);
         itsTreeSpacing->ResetBoxSliderSettings(settingsSpacing);
         break;
      }

      case 50:                   // ScheduleBy : Condition (existing)
      {
         CStringRW conditionTitle = "Edit - ";
         conditionTitle += (CStringRW&) *itsConditionComponent;
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
               this, "management.Thin", "defaultCondition",
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
               this, "management.Thin", "defaultCondition",
               conditionTitle, itsConditionComponent);
         break;
      }
      case 101:                  // ScheduleBy : Condition (return condition
                                 // string and Notify Distroyed)

         itsConditionComponent = (Component *) theData;
         itsSchByCondWindow = NULL;


         CheckForNULLMgtCondition();
         if (itsConditionComponent)
            scheduleBy->ResetConditionButtonCommand(50);

         break;

      default: CMgtClassWindow::DoCommand(theCommand, theData);

   }
}




void ThinFromWin::DoMenuCommand(MENU_TAG theMenuItem,
                                BOOLEAN isShiftkey,
                                BOOLEAN isControlKey)

{
   CStringRW keywordLine;

   switch(theMenuItem)
   {
      case M_SIM_PREP_VIEWKEYWORDS:
      {
         CStringRW viewTitle = "Keywords - ";
         viewTitle += itsComponentName->GetTitle();

         keywordLine = BuildKeywordString(itsAboveOrBelow);

         new ViewKeywords( keywordLine.data(),
                          (const char *) viewTitle);

         break;
      }

      default:
      CWindow::DoMenuCommand(theMenuItem,isShiftkey, isControlKey);
      break;
   }
}




void ThinFromWin::InitializeWithParms(void)
{
   const char * section = "management.Thin";
   PKeyData * parms;

   parms = GetPKeyData(section, "tpa");
   if (parms)
   {
      itsTreesPerAcre->ResetBoxSliderSettings(parms->pString.data());
      CStringRW j = BuildSpacingFromTPA(parms->pString.data());
      itsTreeSpacing->ResetBoxSliderSettings(j);
   }

   parms = GetPKeyData(section, "ba");
   if (parms) itsBasalArea->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "percent");
   if (parms) itsPercent->ResetBoxSliderSettings(parms->pString.data());

   parms = GetPKeyData(section, "propLeft");
   if (parms) itsPropLeft->ResetBoxSliderSettings(parms->pString.data());

   parms = GetPKeyData(section, "lowerDBH");
   if (parms) itsDiameterLower->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "upperDBH");
   if (parms) itsDiameterUpper->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "lowerHt");
   if (parms) itsHeightLower->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "upperHt");
   if (parms) itsHeightUpper->ResetBoxSliderSettings(parms->pString.data());


}




CStringRW ThinFromWin::BuildCParmSettingString(void)
{
   CStringRW settingsLine;

                           // save user entered name         0
   StuffCParm(settingsLine,
              (char const *)itsComponentName->GetTitle());

                            // save schedObj                 1
   StuffCParm((CStringRW &) settingsLine,
               scheduleBy->GetSchedObjSettings().data());

                           // save boxslider                 2
   StuffCParm((CStringRW &) settingsLine,
              (itsTreesPerAcre->GetBoxSliderSettings()).data());

                           // save boxslider                 3
   StuffCParm((CStringRW &) settingsLine,
              (itsBasalArea->GetBoxSliderSettings()).data());

                           // save boxslider                 4
   StuffCParm((CStringRW &) settingsLine,
              (itsPercent->GetBoxSliderSettings()).data());

                           // save radio selection           5
   if(itsDensityRadioGroup->GetSelectedButton() == itsDensityFirst)
      StuffCParm((CStringRW &) settingsLine, "0");
   if(itsDensityRadioGroup->GetSelectedButton() == itsDensitySecond)
      StuffCParm((CStringRW &) settingsLine, "1");
   if(itsDensityRadioGroup->GetSelectedButton() == itsDensityThird)
      StuffCParm((CStringRW &) settingsLine, "2");

                           // save radio selection           6
   if(itsPercentRadioGroup->GetSelectedButton() == itsPercentFirst)
      StuffCParm((CStringRW &) settingsLine, "0");
   else
      StuffCParm((CStringRW &) settingsLine, "1");

                           // save boxslider                 7
   StuffCParm(settingsLine,
              (itsDiameterLower->GetBoxSliderSettings()).data());

                           // save boxslider                 8
   StuffCParm(settingsLine,
              (itsDiameterUpper->GetBoxSliderSettings()).data());

                           // save boxslider                 9
   StuffCParm(settingsLine,
              (itsHeightLower->GetBoxSliderSettings()).data());

                           // save box slider                10
   StuffCParm(settingsLine,
              (itsHeightUpper->GetBoxSliderSettings()).data());

                           // save box slider                11
   StuffCParm(settingsLine,
              (itsPropLeft->GetBoxSliderSettings()).data());

   return settingsLine;
}




void ThinFromWin::ResetTheComponentSettings(const char * theSettings)
{

                           // reset user entered name          0
   CStringRW tmpString =  GetCParm((const char *) theSettings, 0);
   itsComponentName->SetTitle(tmpString.data());

                         // reset schedObj                     1
   tmpString =  GetCParm((const char *) theSettings, 1);
   scheduleBy->ResetSchedObjSettings(tmpString.data());

                        // reset user boxslider                2
   tmpString =  GetCParm((const char *) theSettings, 2);
   itsTreesPerAcre->ResetBoxSliderSettings(tmpString.data());

   tmpString = BuildSpacingFromTPA(tmpString.data());
   itsTreeSpacing->ResetBoxSliderSettings(tmpString.data());

                        // reset user boxslider                3
   tmpString =  GetCParm((const char *) theSettings, 3);
   itsBasalArea->ResetBoxSliderSettings(tmpString.data());

                        // reset user boxslider                4
   tmpString =  GetCParm((const char *) theSettings, 4);
   itsPercent->ResetBoxSliderSettings(tmpString.data());

                        // reset radio button                  5
   tmpString =  GetCParm((const char *) theSettings, 5);
   if (strchr(tmpString,'0')) DoCommand(6);
   if (strchr(tmpString,'1')) DoCommand(7);
   if (strchr(tmpString,'2')) DoCommand(8);

                        // reset radio button                  6
   tmpString =  GetCParm((const char *) theSettings, 6);
   if (strchr(tmpString,'0')) DoCommand(9);
   else DoCommand(10);

                           // reset user boxslider             7
   tmpString =  GetCParm((const char *) theSettings, 7);
   itsDiameterLower->ResetBoxSliderSettings(tmpString.data());

                           // reset user boxslider             8
   tmpString =  GetCParm((const char *) theSettings, 8);
   itsDiameterUpper->ResetBoxSliderSettings(tmpString.data());

                           // reset user boxslider             9
   tmpString =  GetCParm((const char *) theSettings, 9);
   itsHeightLower->ResetBoxSliderSettings(tmpString.data());

                           // reset user boxslider             10
   tmpString =  GetCParm((const char *) theSettings, 10);
   itsHeightUpper->ResetBoxSliderSettings(tmpString.data());

                           // reset user boxslider             11
   tmpString =  GetCParm((const char *) theSettings, 11);
   itsPropLeft->ResetBoxSliderSettings(tmpString.data());

}





CStringRW ThinFromWin::BuildKeywordString(long aboveOrBelow)
{
   char ab[2];
   strcpy(ab,aboveOrBelow == ABOVE ? "A" : "B");

   CStringRW keyLine,resid,cutEff,dbhLow,dbhHigh,htLow,htHigh;
   char line[82];

   cutEff  = itsPropLeft->GetTitle();
   float e = ConvertToFloat(cutEff.data());
   cutEff  = ConvertToString((float) 1.-e);
   dbhLow  = itsDiameterLower->GetTitle(),
   dbhHigh = itsDiameterUpper->GetTitle(),
   htLow   = itsHeightLower->GetTitle(),
   htHigh  = itsHeightUpper->GetTitle();

   int scheduleValue;

                              // get year or conditional scheduling
                              // and year of begin schedule
   scheduleBy->SpGetValue(scheduleValue);

                              // see which button pressed
   long selected = itsDensityRadioGroup->GetSelectedButton();

                              // the first: trees per acrea
   if (selected == itsDensityFirst)
   {
      resid=itsTreesPerAcre->GetTitle();
      sprintf(line,"thin%sTA   %10d%10s%10s%10s%10s%10s%10s\n",
              ab,scheduleValue,resid.data(),
              cutEff.data(),dbhLow.data(),dbhHigh.data(),htLow.data(),htHigh.data());
      keyLine += line;
   }

   if (selected == itsDensitySecond)
   {
      resid=itsBasalArea->GetTitle();
      sprintf(line,"thin%sBA   %10d%10s%10s%10s%10s%10s%10s\n",
              ab,scheduleValue,resid.data(),
              cutEff.data(),dbhLow.data(),dbhHigh.data(),htLow.data(),htHigh.data());
      keyLine += line;
   }

   if (selected == itsDensityThird)
   {
                              // see which button pressed
      long selected2 = itsPercentRadioGroup->GetSelectedButton();
      if (selected2 == itsPercentFirst)
      {
         resid = ConvertToString((float)((itsPercent->GetValue()) * .01));
         sprintf(line,"thin%sTA   %10d    PARMS(BTPA*%s, %s, %s, %s, %s, %s)\n",
                 ab,scheduleValue,resid.data(),
                 cutEff.data(),dbhLow.data(),dbhHigh.data(),htLow.data(),htHigh.data());
         keyLine += line;
      }

      if (selected2 == itsPercentSecond)
      {
         resid = ConvertToString((float)((itsPercent->GetValue()) * .01));
         sprintf(line,"thin%sBA   %10d    PARMS(BBA*%s, %s, %s, %s, %s, %s)\n",
                 ab,scheduleValue,resid.data(),
                 cutEff.data(),dbhLow.data(),dbhHigh.data(),htLow.data(),htHigh.data());
         keyLine += line;
      }
   }

   return keyLine;
}

CStringRW ThinFromWin::BuildSpacingFromTPA(const char * theSettings)
{
   float val, min1, max1, min2, max2, t;
   CStringRW settingsLine;

   sscanf(theSettings,"%f %f %f %f %f", &val, &min1, &max1, &min2, &max2);

   t = min1;
   min1 = sqrt(43560./max1);
   max1 = t > 1 ? sqrt(43560./min1) : 10000;
   t = min2;
   min2 = sqrt(43560./max2);
   max2 = t > 1 ? sqrt(43560./t) : 10000;

   t = val;
   val = val > 1 ? sqrt(43560./val) : max1;

   settingsLine  = ConvertToString(val);  settingsLine += " ";
   settingsLine += ConvertToString(min1); settingsLine += " ";
   settingsLine += ConvertToString(max1); settingsLine += " ";
   settingsLine += ConvertToString(min2); settingsLine += " ";
   settingsLine += ConvertToString(max2);

   return settingsLine;
}

CStringRW ThinFromWin::BuildTPAFromSpacing(const char * theSettings)
{
   float val, min1, max1, min2, max2, t;
   CStringRW settingsLine;

   sscanf(theSettings,"%f %f %f %f %f", &val, &min1, &max1, &min2, &max2);

   t = min1;
   min1 = max1 > 210. ? 0. : 43560./(max1*max1);
   max1 = max(min1+1.,43560./(t*t));
   t = min2;
   min2 = max2 > 210. ? 0. : 43560./(max2*max2);
   max2 = max(min2+1.,43560./(t*t));

   t = val;
   val = val > 210. ? 0. : 43560./(t*t);

   settingsLine  = ConvertToString(val);  settingsLine += " ";
   settingsLine += ConvertToString(min1); settingsLine += " ";
   settingsLine += ConvertToString(max1); settingsLine += " ";
   settingsLine += ConvertToString(min2); settingsLine += " ";
   settingsLine += ConvertToString(max2);

   return settingsLine;
}

