/********************************************************************

   File Name:     seedwin.cpp
   Author:        kfd, nlc rewrite 09/20/2006
   Date:          08/25/94

********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "stand.hpp"
#include "group.hpp"
#include "compon.hpp"
#include "runstr.hpp"

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
#include CDesktop_i     // needed for conditions
#include CMenuBar_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "boxsldr.hpp"
#include "schedobj.hpp"
#include "seedwin.hpp"

#include "msgwin.hpp"      // for Tell();
#include "schcond.hpp"
#include "spparms.hpp"
#include "fvslocs.hpp"
#include "warn_dlg.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "cparmrtn.hpp"    // for Stuff/Get CParms
#include "suppdefs.hpp"


#include "supphelp.hpp"    // for help #defines
extern XVT_HELP_INFO helpInfo;


SeedTreeWin::SeedTreeWin(const CStringRW& theTitle,
                         Component * theComponent,
                         Component * theConditionComponent)

            :CMgtClassWindow((CDocument *) theSpGlobals->theAppDocument,
                             CenterTopWinPlacement(CRect(5,20,605,458)), theComponent,
                             theConditionComponent, theTitle, NULL,
                             WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                             W_DOC,TASK_MENUBAR)
{

                           // construct a SeedTreeWin object
                           // create ScheduleByObject

   MakeStandardControls("Seedtree",8,9,10);

   int top=8;
   scheduleBy = new ScheduleByObject(this, 100, itsScrollingFrame,
                                     CPoint(8,top));
   top += 80;

   smallestTree = new BoxSliderObject(itsScrollingFrame, this,
                                      CRect(8,top,554,top+78),
                                      "Smallest diameter cut in prep and seed cuts",
                                      "5 0  1000 0  20",
                                      SPLEFT, 0);
   top += 62;


   prepCutCheckBox = new NCheckBox(itsScrollingFrame,CRect(8,top,416,top+32),
                                   "Prep cut, specify residual density", 0);
   prepCutCheckBox->SetSelectCommand(1);
   prepCutCheckBox->SetDeselectCommand(2);
   top += 40;

                           // prep cut; residual density
   itsDensityGroup = new CRadioGroup(itsScrollingFrame,CPoint(24,top+8), FALSE);

   itsFirst = itsDensityGroup->AddButton(CPoint(0,0),"",4);
   itsDensityGroup->SetSelectedButton(itsFirst);

   itsFirstButton = (NRadioButton*) itsDensityGroup->FindSubview(itsFirst);
   itsFirstButton->Disable();

   itsSecond = itsDensityGroup->AddButton(CPoint(0,48),"",5);
   itsSecondButton = (NRadioButton*) itsDensityGroup->FindSubview(itsSecond);
   itsSecondButton->Disable();

                           // prep cut; basal area
   residualBasal = new BoxSliderObject(itsScrollingFrame, this,
                                       CRect(56,top,554,top+50),
                                       "Residual basal area per acre",
                                       "200 0  1000 0 500",
                                       SPLEFT);
   residualBasal->DeactivateBoxSlider();
   top +=58;

                           // prep cut; residual percent
   residualPercent = new BoxSliderObject(itsScrollingFrame, this,
                                         CRect(56,top,554,top+50),
                                         "Residual percent of maximum SDI in year of prep cut",
                                         "60 0 100 0 100",
                                         SPLEFT);
   residualPercent->DeactivateBoxSlider();
   top +=58;

                           // seed cut
   itsSeedCutText1 = new CText(itsScrollingFrame, CPoint(8,top+4),
                               CStringRW("Seed cut"));
   itsSeedCutText2 = new CText(itsScrollingFrame, CPoint(8,top+4),
                               CStringRW("Seed cut, scheduled "));
   seedCutYears = new NEditControl(itsScrollingFrame,
                                   CRect(180,top,226,top+30), "10",
                                   CTL_FLAG_RIGHT_JUST);
   itsSeedCutText3 = new CText(itsScrollingFrame, CPoint(235,top+4),
                               CStringRW("years after prep cut"));
   top +=36;
                            // seed cut; residual trees
   residualTrees = new BoxSliderObject(itsScrollingFrame, this,
                                       CRect(8,top,554,top+50),
                                       "Seed cut residual trees (cut from below)",
                                       "5 0 10000 0  50",
                                       SPLEFT);
   top +=58;

                          // removal cut
   removalCutCheckBox = new NCheckBox(itsScrollingFrame,CRect(8,top,190,top+24),
                                      "Removal cut, scheduled", 0);
   removalCutCheckBox->SetSelectCommand(6);
   removalCutCheckBox->SetDeselectCommand(7);

                         // removal cut; years after
   removalCutYears = new NEditControl(itsScrollingFrame,
                                      CRect(196,top,242,top+30), "10",
                                      CTL_FLAG_RIGHT_JUST);
   new CText(itsScrollingFrame, CPoint(251,top+4),
             CStringRW("years after seed cut"));
   top +=36;

   // removal cut residual.
   removalCutResidual = new BoxSliderObject(itsScrollingFrame, this,
                                        CRect(8,top,554,top+50),
                                        "Removal cut residual trees (cut from below)",
                                        "2 0 10000 0  50",
                                        SPLEFT);
   top += 58;
   smallestTreeRC = new BoxSliderObject(itsScrollingFrame, this,
                                      CRect(8,top,554,top+78),
                                      "Smallest diameter cut in removal cut",
                                      "6 0  1000 0  20",
                                      SPLEFT, 0);


                              // initialize with parm file values
   InitializeWithParms();

   DoCommand(2); // Prep cut deselected.
   DoCommand(7); // Removal cut deselected.

                              // save initial settings for reset
   itsInitialSettings = BuildCParmSettingString();

                              // (itsComponent) indicates that this is an
                              // edit of an existing component; reset settings
   if (itsComponent)
   {
      if (itsComponent->windowParms)
         ResetTheComponentSettings(itsComponent->windowParms->data());

      itsConditionComponent = FindConditionComponent();
   }

   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), SEEDWINWindow, 0L);
   AdjustScrollingWinSize(itsScrollingFrame, this);
   Show();
   itsNavigator->InitFocus();

}




void SeedTreeWin::DoCommand(long theCommand,void* theData)
{
   // NOTE:  checkboxes and radio buttons may seem to be unnecessarily
   // set (unset).  The reason for this is that the DoCommand is called
   // by other routines (InitializeWithParms, ResetTheSettings).  In order
   // to trigger correct action, these objects must be set.

   switch (theCommand)
   {
      case 1:                    // prep cut checked
      {
         prepCutCheckBox->Select();
         itsFirstButton->Enable();
         itsSecondButton->Enable();
         if ((int)itsDensityGroup->GetSelectedButton()== itsFirst)
            residualBasal->ActivateBoxSlider();
         if ((int)itsDensityGroup->GetSelectedButton()== itsSecond)
            residualPercent->ActivateBoxSlider();

         itsSeedCutText1->DoHide();
         itsSeedCutText2->DoShow();
         itsSeedCutText3->DoShow();
         seedCutYears->DoShow();
         DoDraw();

         break;
      }

      case 2:                    // prep cut unchecked
      {
         prepCutCheckBox->Deselect();
         residualBasal->DeactivateBoxSlider();
         residualPercent->DeactivateBoxSlider();
         itsFirstButton->Disable();
         itsSecondButton->Disable();

         itsSeedCutText1->DoShow();
         itsSeedCutText2->DoHide();
         itsSeedCutText3->DoHide();
         seedCutYears->Hide();
         seedCutYears->DoHide();
         seedCutYears->Draw(seedCutYears->GetGlobalFrame());
         DoDraw();
         break;
      }

      case 4:                    // radio button 1 - prep cut residual basal
         itsDensityGroup->SetSelectedButton(itsFirst);
         residualBasal->ActivateBoxSlider();
         residualPercent->DeactivateBoxSlider();
         break;

      case 5:                    // radio button 2 - prep cut residual percent
         itsDensityGroup->SetSelectedButton(itsSecond);
         residualBasal->DeactivateBoxSlider();
         residualPercent->ActivateBoxSlider();
         break;

      case 6:                    // removal cut checked
      {
         removalCutCheckBox->Select();
         removalCutResidual->ActivateBoxSlider();
         smallestTreeRC->ActivateBoxSlider();
         removalCutYears->Enable();
         break;
      }

      case 7:                    // removal cut unchecked
      {
         removalCutCheckBox->Deselect();
         removalCutResidual->DeactivateBoxSlider();
         smallestTreeRC->DeactivateBoxSlider();
         removalCutYears->Disable();
         break;
      }

      case 50:                   // ScheduleBy : Condition (existing)
      {
         CStringRW conditionTitle = "Edit - ";
         conditionTitle += (CStringRW &) *itsConditionComponent;
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
                  this, "management.Seedtree", "defaultCondition",
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
                  this, "management.Seedtree", "defaultCondition",
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

      case 105:                  // free form
      {
         if (xvt_dm_post_ask("Proceed","Cancel",NULL,
                             "You are about to enter a text editor "
                             "used to edit parameters manually.  "
                             "Suppose does not ensure correct keyword "
                             "records when you use this editor.")
            == RESP_2) break;
      }

      case 8:
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

         SaveTheComponent (usingFreeForm,"Seedtree",keywordLine,settingsLine);
         closeCalledByCloseCancel = FALSE;
         Close();
         break;
      }


      case 9:                   // reset button pushed
      {
         if (itsComponent && itsComponent->windowParms)
              ResetTheComponentSettings(itsComponent->windowParms->data());
         else ResetTheComponentSettings(itsInitialSettings.data());
         break;
      }


      case 10:                   // cancel button pushed
      {
         Close();
         break;
      }

      default: CMgtClassWindow::DoCommand(theCommand, theData);

   }
}




void SeedTreeWin::InitializeWithParms(void)
{
   const char * section = "management.Seedtree";
   PKeyData * parms;

   parms = GetPKeyData(section, "minDBH");
   if (parms) smallestTree->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "pcResidBA");
   if (parms) residualBasal->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "pcPercentSDI");
   if (parms) residualPercent->ResetBoxSliderSettings(parms->pString.data());

   parms = GetPKeyData(section, "afterPrepYrs");
   if (parms) seedCutYears->SetTitle(parms->pString);
   parms = GetPKeyData(section, "residTPA");
   if (parms) residualTrees->ResetBoxSliderSettings(parms->pString.data());

   parms = GetPKeyData(section, "afterRemovalYrs");
   if (parms) removalCutYears->SetTitle(parms->pString);
   parms = GetPKeyData(section, "removeTPA");
   if (parms) removalCutResidual->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "minDBHrc");
   if (parms) smallestTreeRC->ResetBoxSliderSettings(parms->pString.data());
}



CStringRW SeedTreeWin::BuildCParmSettingString(void)
{
   CStringRW settingsLine;

                           // save user entered name
   StuffCParm(settingsLine,itsComponentName->GetTitle().data());

                            // save schedObj
   StuffCParm((CStringRW &) settingsLine,
               scheduleBy->GetSchedObjSettings().data());

                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (smallestTree->GetBoxSliderSettings()).data());

                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (residualBasal->GetBoxSliderSettings()).data());

                           // save boxslider
   StuffCParm((CStringRW &)settingsLine,
              (residualPercent->GetBoxSliderSettings()).data());

                           // save radio selection (residual)
   if(itsDensityGroup->GetSelectedButton() == itsFirst)
      StuffCParm((CStringRW &) settingsLine, "0");
   else
      StuffCParm((CStringRW &) settingsLine, "1");

                           // prep cut check box
   if (prepCutCheckBox->IsSelected())
      StuffCParm((CStringRW &) settingsLine, "1");
   else
      StuffCParm((CStringRW &) settingsLine, "0");

                           // edit control seedCutYears
   StuffCParm((CStringRW &) settingsLine,
              (char const *) seedCutYears->GetTitle());

                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (residualTrees->GetBoxSliderSettings()).data());

                           // edit control removalCutYears
   StuffCParm((CStringRW &) settingsLine,
              (char const *) removalCutYears->GetTitle());

                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (removalCutResidual->GetBoxSliderSettings()).data());

                           // save boxslider
   StuffCParm((CStringRW &) settingsLine,
              (smallestTreeRC->GetBoxSliderSettings()).data());

                           // removal cut check box
   if (removalCutCheckBox->IsSelected())
      StuffCParm((CStringRW &) settingsLine, "1");
   else
      StuffCParm((CStringRW &) settingsLine, "0");

   return settingsLine;
}





void SeedTreeWin::ResetTheComponentSettings(const char * theSettings)
{

                         // reset user entered name
   CStringRW tmpString =  GetCParm((const char *) theSettings, 0);
   itsComponentName->SetTitle(tmpString.data());

                         // reset schedObj
   tmpString =  GetCParm((const char *) theSettings, 1);
   scheduleBy->ResetSchedObjSettings(tmpString.data());


                        // reset user boxslider
   tmpString =  GetCParm((const char *) theSettings, 2);
   smallestTree->ResetBoxSliderSettings(tmpString.data());


                        // reset user boxslider
   tmpString =  GetCParm((const char *) theSettings, 3);
   residualBasal->ResetBoxSliderSettings(tmpString.data());

                        // reset user boxslider
   tmpString =  GetCParm((const char *) theSettings, 4);
   residualPercent->ResetBoxSliderSettings(tmpString.data());

                        // reset radio button
   tmpString =  GetCParm((const char *) theSettings, 5);
   if (strchr(tmpString,'0'))
      DoCommand(4);
   else
      DoCommand(5);

                        // reset prep cut check box
   tmpString =  GetCParm((const char *) theSettings, 6);
   if (strchr(tmpString,'1'))
      DoCommand(1);
   else
      DoCommand(2);

                        // edit control seedCutYears
   tmpString =  GetCParm((const char *) theSettings, 7);
   seedCutYears->SetTitle(tmpString);

                        // reset user boxslider
   tmpString =  GetCParm((const char *) theSettings, 8);
   residualTrees->ResetBoxSliderSettings(tmpString.data());

                        // edit control removalCutYears
   tmpString =  GetCParm((const char *) theSettings, 9);
   removalCutYears->SetTitle(tmpString);

                        // reset user boxslider
   tmpString =  GetCParm((const char *) theSettings, 10);
   removalCutResidual->ResetBoxSliderSettings(tmpString.data());

                        // reset user boxslider
   tmpString =  GetCParm((const char *) theSettings, 11);
   smallestTreeRC->ResetBoxSliderSettings(tmpString.data());

   // removal cut check box
   tmpString =  GetCParm((const char *) theSettings,12);
   if (strchr(tmpString,'1'))
      DoCommand(6);
   else
      DoCommand(7);

}





CStringRW SeedTreeWin::BuildKeywordString(void)
{
   CStringRW keyLine,minDBH,minDBHrc,year,resid;
   char line[82];
   int aPrepYear, aSeedYear, anIntVal;
   char * zero   = "0";
   char * nines  = "999";
   char * cuteff  = "1";

   minDBH=smallestTree->GetTitle();

   // get year or conditional scheduling and year of begin schedule
   scheduleBy->SpGetValue(aPrepYear);

   // initially seed year same as prep year
   aSeedYear = aPrepYear;

   // prep cut box checked?
   if (prepCutCheckBox->IsSelected())
   {
      // change seed year
      anIntVal=(int) ConvertToLong(seedCutYears->GetTitle());
      aSeedYear = aPrepYear + anIntVal;
      year = ConvertToString(aPrepYear);

      // see which button pressed in prep cut box
      long selected = itsDensityGroup->GetSelectedButton();

      // residual basal area
      if (selected == itsFirst)
      {
         resid = residualBasal->GetTitle();
         keyLine = "* thinBBA year,resid,cuteff,mindbh,maxdbh,minht,maxht\n";
         sprintf(line,"thinBBA   %10s%10s%10s%10s%10s%10s%10s\n",
                        year.data(),resid.data(),
                        cuteff, minDBH.data(), nines, zero, nines);
         keyLine += line;
      }

      else // residual percent
      {
         resid = ConvertToString((float)((residualPercent->GetValue()) * .01));
         keyLine="* Parms(resid,cuteff,species,mindbh,maxdbh,cutCntl)\n";
         sprintf(line,"thinSDI   %10s   Parms(%s*BSDIMax, 1, 0, %s, 999, 0)\n",
                 year.data(), resid.data(), minDBH.data());
         keyLine += line;
      }
   }

   // seed cut

   resid = residualTrees->GetTitle();
   year = ConvertToString(aSeedYear);
   keyLine += "* thinBTA year,resid,cuteff,mindbh,maxdbh,minht,maxht\n";
   sprintf(line,"thinBTA   %10s%10s%10s%10s%10s%10s%10s\n",
                 year.data(),resid.data(),cuteff, minDBH.data(),
                 nines, zero, nines);
   keyLine += line;

   // removal cut box checked?
   if (removalCutCheckBox->IsSelected())
   {
      minDBH=smallestTreeRC->GetTitle();
      anIntVal=(int) ConvertToLong(removalCutYears->GetTitle());
      year = ConvertToString(aSeedYear+anIntVal);
      resid = removalCutResidual->GetTitle();
      sprintf(line,"thinBTA   %10s%10s%10s%10s%10s%10s%10s\n",
                  year.data(),resid.data(),cuteff, minDBH.data(),
                  nines, zero, nines);
      keyLine += line;
   }
   return keyLine;
}







