/********************************************************************

   File Name:     sheltwin.cpp   (shelterwood)
   Author:        kfd, nlc rewrite 09/20/2006
   Date:          10/17/94


********************************************************************/
#include "appdef.hpp"
#include "PwrDef.h"

#include "stand.hpp"
#include "runstr.hpp"
#include "group.hpp"
#include "compon.hpp"


#include CStringCollection_i
#include CStringRWC_i
#include NButton_i
#include NRadioButton_i
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
#include "sheltwin.hpp"
#include "msgwin.hpp"      // for Tell();
#include "warn_dlg.hpp"

#include "spglobal.hpp"
#include "spparms.hpp"
#include "fvslocs.hpp"
#include "schcond.hpp"
#include "warn_dlg.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "cparmrtn.hpp"    // for Stuff/Get CParms
#include "viewkey.hpp"
#include "suppdefs.hpp"

#include "supphelp.hpp"    // for help #defines
extern XVT_HELP_INFO helpInfo;




ShelterwoodWin::ShelterwoodWin(const CStringRW& theTitle,
                Component * theComponent,
                Component * theConditionComponent)

               :CMgtClassWindow((CDocument *) theSpGlobals->theAppDocument,
                                CRect(5,20,605,458), theComponent,
                                theConditionComponent, theTitle, NULL,
                                WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                                W_DOC,TASK_MENUBAR)

{

   MakeStandardControls("Shelterwood",11,12,13);

   int top=8;
   scheduleBy = new ScheduleByObject(this, 100, itsScrollingFrame,
                                     CPoint(8,top));
   top += 80;

   smallestTree = new BoxSliderObject(itsScrollingFrame, this,
                                      CRect(8,top,554,top+78),
                                      "Smallest diameter cut in prep and shelterwood cuts",
                                      "5 0  1000 0  20",
                                      SPLEFT, 0);
   top += 62;

                           // prep cut checkbox
   itsPrepCutCheckBox = new NCheckBox(itsScrollingFrame,
                                      CRect(8,top,416,top+32),
                                      "Prep cut, specify residual density",
                                      0L);
   itsPrepCutCheckBox->SetSelectCommand(1);
   itsPrepCutCheckBox->SetDeselectCommand(2);
   top += 32;

                           // prep cut buttons
   itsPrepCutRadioGroup = new CRadioGroup(itsScrollingFrame, CPoint(24,top), FALSE);

   itsPrepCutFirst  = itsPrepCutRadioGroup->AddButton(CPoint(0,6),"",4);
   itsPrepCutSecond = itsPrepCutRadioGroup->AddButton(CPoint(0,64),"",5);
   itsPrepCutRadioGroup->SetSelectedButton(itsPrepCutFirst);

   itsPrepCutFirstButton = (NRadioButton*) itsPrepCutRadioGroup->
                                           FindSubview(itsPrepCutFirst);
   itsPrepCutFirstButton->Disable();

   itsPrepCutSecondButton = (NRadioButton*) itsPrepCutRadioGroup->
                                            FindSubview(itsPrepCutSecond);
   itsPrepCutSecondButton->Disable();


                           // prep cut box sliders
   itsResidualBasal = new BoxSliderObject(itsScrollingFrame, this,
                                          CRect(56,top,554,top+50),
                                          "Residual basal area per acre",
                                          "200 0  1000 0 500",
                                          SPLEFT);
   itsResidualBasal->DeactivateBoxSlider();
   top +=58;

                           // prep cut; residual percent
   itsResidualPercent = new BoxSliderObject(itsScrollingFrame, this,
                                            CRect(56,top,554,top+50),
                                            "Residual percent of maximum SDI in year of prep cut",
                                            "60 0 100 0 100",
                                            SPLEFT);
   itsResidualPercent->DeactivateBoxSlider();
   top +=58;


                           // shelterwood cut
   itsShelterCutText1 = new CText(itsScrollingFrame,
                                  CPoint(8,top+4),
                                  CStringRW("Shelterwood cut"));
   itsShelterCutText2 = new CText(itsScrollingFrame, CPoint(8,top+4),
                                  CStringRW("Shelterwood cut, scheduled "));
   itsShelterCutYears = new NEditControl(itsScrollingFrame,
                                         CRect(200,top,246,top+30), "10",
                                         CTL_FLAG_RIGHT_JUST);
   itsShelterCutText3 = new CText(itsScrollingFrame, CPoint(250,top+4),
                                  CStringRW("years after prep cut"));
   itsShelterCutText2->DoHide();
   itsShelterCutText3->DoHide();
   itsShelterCutYears->DoHide();
   top +=36;

                           // shelter cut buttons
   itsShelterCutRadioGroup = new CRadioGroup(itsScrollingFrame,CPoint(8,top), FALSE);

   itsShelterCutFirst  = itsShelterCutRadioGroup->AddButton(CPoint(0,6),"",6);
   itsShelterCutSecond = itsShelterCutRadioGroup->AddButton(CPoint(0,64),"",7);
   itsShelterCutThird = itsShelterCutRadioGroup->AddButton(CPoint(0,122),"",8);

   itsShelterCutRadioGroup->SetSelectedButton(itsShelterCutSecond);

                           // shelter cut box sliders
   itsShelterBasal = new BoxSliderObject(itsScrollingFrame,this,
                                         CRect(40,top,554,top+50),
                                         "Residual basal area per acre",
                                         "100 0 10000 0 300",
                                         SPLEFT);
   itsShelterBasal->DeactivateBoxSlider();
   top +=58;

   itsShelterTrees = new BoxSliderObject(itsScrollingFrame, this,
                                         CRect(40,top,554,top+50),
                                         "Residual trees per acre",
                                         " 50 0 10000 0 200",
                                         SPLEFT);
   top +=58;

   itsShelterPercent = new BoxSliderObject(itsScrollingFrame,this,
                                           CRect(40,top,554,top+50) ,
                                           "Residual percent of maximum SDI in year of cut",
                                           "20 0   100 0 100",
                                           SPLEFT);
   itsShelterPercent->DeactivateBoxSlider();
   top +=58;

                           // removal cut checkbox
   itsRemovalCutCheckBox = new NCheckBox(itsScrollingFrame,
                                         CRect(8,top,224,top+24),
                                         "Removal cut, scheduled", 0);
   itsRemovalCutCheckBox->SetSelectCommand(9);
   itsRemovalCutCheckBox->SetDeselectCommand(10);

   itsRemovalCutYears = new NEditControl(itsScrollingFrame,
                                         CRect(200,top,246,top+32), "10",
                                         CTL_FLAG_RIGHT_JUST);

   new CText(itsScrollingFrame, CPoint(252,top+4),
        CStringRW("years after previous entry."));
   top +=36;

   removalCutResidual = new BoxSliderObject(itsScrollingFrame, this,
                                           CRect(8,top,554,top+50) ,
                                           "Removal cut residual trees (cut from below)",
                                           "2 0 10000 0  50",
                                           SPLEFT);

   top += 58;
   smallestTreeRC = new BoxSliderObject(itsScrollingFrame, this,
                                      CRect(8,top,554,top+78),
                                      "Smallest diameter cut in removal cut",
                                      "6 0  1000 0  20",
                                      SPLEFT, 0);
   DoCommand(10);


                              // initialize with parm file values
   InitializeWithParms();
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

   AdjustScrollingWinSize(itsScrollingFrame, this);
   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), SHELTWINWindow, 0L);
   Show();
   itsNavigator->InitFocus();
}




void ShelterwoodWin::DoCommand(long theCommand,void* theData)
{
   // NOTE:  checkboxes and radio buttons may seem to be unnecessarily
   // set (unset).  The reason for this is that the DoCommand is called
   // by other routines (InitializeWithParms, ResetTheSettings).  In order
   // to trigger correct action, these objects must be set.

   switch (theCommand)
   {
      case 1:                    // prep cut checked
      {
         itsPrepCutCheckBox->Select();
         itsPrepCutFirstButton->Enable();
         itsPrepCutSecondButton->Enable();
         if ((int)itsPrepCutRadioGroup->GetSelectedButton()== itsPrepCutFirst)
            itsResidualBasal->ActivateBoxSlider();
         if ((int)itsPrepCutRadioGroup->GetSelectedButton()== itsPrepCutSecond)
            itsResidualPercent->ActivateBoxSlider();

         itsShelterCutText1->DoHide();
         itsShelterCutText2->DoShow();
         itsShelterCutText3->DoShow();
         itsShelterCutYears->DoShow();
         DoDraw();
         break;
      }

      case 2:                    // prep cut unchecked
      {
         itsPrepCutCheckBox->Deselect();
         itsResidualBasal->DeactivateBoxSlider();
         itsResidualPercent->DeactivateBoxSlider();
         itsPrepCutFirstButton->Disable();
         itsPrepCutSecondButton->Disable();

         itsShelterCutText1->DoShow();
         itsShelterCutText2->DoHide();
         itsShelterCutText3->DoHide();
         itsShelterCutYears->Hide();
         itsShelterCutYears->DoHide();
         itsShelterCutYears->Draw(itsShelterCutYears->GetGlobalFrame());
         DoDraw();
         break;
      }

      case 4:                    // radio button 1
      {
         itsPrepCutRadioGroup->SetSelectedButton(itsPrepCutFirst);
         itsResidualBasal->ActivateBoxSlider();
         itsResidualPercent->DeactivateBoxSlider();
         break;
      }

      case 5:                    // radio button 2
      {
         itsPrepCutRadioGroup->SetSelectedButton(itsPrepCutSecond);
         itsResidualBasal->DeactivateBoxSlider();
         itsResidualPercent->ActivateBoxSlider();
         break;
      }

      case 6:                    // shelterwood cut section radio button 1
      {
         itsShelterCutRadioGroup->SetSelectedButton(itsShelterCutFirst);
         itsShelterBasal->ActivateBoxSlider();
         itsShelterTrees->DeactivateBoxSlider();
         itsShelterPercent->DeactivateBoxSlider();
         break;
      }

      case 7:                    // shelterwood cut section radio button 2
      {
         itsShelterCutRadioGroup->SetSelectedButton(itsShelterCutSecond);
         itsShelterBasal->DeactivateBoxSlider();
         itsShelterTrees->ActivateBoxSlider();
         itsShelterPercent->DeactivateBoxSlider();
         break;
      }

      case 8:                    // shelterwood cut section radio button 3
      {
         itsShelterCutRadioGroup->SetSelectedButton(itsShelterCutThird);
         itsShelterBasal->DeactivateBoxSlider();
         itsShelterTrees->DeactivateBoxSlider();
         itsShelterPercent->ActivateBoxSlider();
         break;
      }

      case 9:                    // removal cut checked
      {
         itsRemovalCutCheckBox->Select();
         removalCutResidual->ActivateBoxSlider();
         smallestTreeRC->ActivateBoxSlider();
         itsRemovalCutYears->Enable();
         break;
      }

      case 10:                    // removal cut unchecked
      {
         itsRemovalCutCheckBox->Deselect();
         removalCutResidual->DeactivateBoxSlider();
         smallestTreeRC->DeactivateBoxSlider();
         itsRemovalCutYears->Disable();
         break;
      }

      case 50:                   // ScheduleBy : Condition (existing)
      {
         CStringRW conditionTitle = "Edit - ";
         conditionTitle += (CStringRW&) *itsConditionComponent;
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
               this, "management.Shelterwood", "defaultCondition",
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
                   this, "management.Shelterwood", "defaultCondition",
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

     case 11:                   // ok button pushed
                                 // SetSave set by SaveTheComponent
                                 // if there is not condition, change radio
                                 // button to by year
     {
         if (!itsConditionComponent)
            scheduleBy->SetYearButton(TRUE);

         int usingFreeForm = itsFreeFormCommand==theCommand;
         CStringRW keywordLine = BuildKeywordString();
         CStringRW settingsLine=NULLString;
         if (!usingFreeForm) settingsLine=BuildCParmSettingString();

         SaveTheComponent (usingFreeForm,"Shelterwood",keywordLine,settingsLine);
         closeCalledByCloseCancel = FALSE;
         Close();
         break;
      }

      case 12:                    // Reset was pressed
      {
         if (itsComponent && itsComponent->windowParms)
              ResetTheComponentSettings(itsComponent->windowParms->data());
         else ResetTheComponentSettings(itsInitialSettings.data());
         break;
      }


      case 13:                    // Cancel was pressed
      {
         Close();
         break;
      }
      default: CMgtClassWindow::DoCommand(theCommand, theData);

   }
}





void ShelterwoodWin::InitializeWithParms(void)
{
   const char * section = "management.Shelterwood";
   PKeyData * parms;

   parms = GetPKeyData(section, "minDBH");
   if (parms) smallestTree->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "pcResidBA");
   if (parms) itsResidualBasal->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "pcPercentSDI");
   if (parms) itsResidualPercent->ResetBoxSliderSettings(parms->pString.data());

   parms = GetPKeyData(section, "afterPrepYrs");
   if (parms) itsShelterCutYears->SetTitle(parms->pString);

   parms = GetPKeyData(section, "residBA");
   if (parms) itsShelterBasal->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "residTPA");
   if (parms) itsShelterTrees->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "percentSDI");
   if (parms) itsShelterPercent->ResetBoxSliderSettings(parms->pString.data());

   parms = GetPKeyData(section, "afterRemovalYrs");
   if (parms) itsRemovalCutYears->SetTitle(parms->pString);

   parms = GetPKeyData(section, "removeTPA");
   if (parms) removalCutResidual->ResetBoxSliderSettings(parms->pString.data());
   parms = GetPKeyData(section, "minDBHrc");
   if (parms) smallestTreeRC->ResetBoxSliderSettings(parms->pString.data());
}





CStringRW ShelterwoodWin::BuildCParmSettingString(void)
{
   CStringRW settingsLine;

                           // save user entered name
   StuffCParm(settingsLine,
              (char const *)itsComponentName->GetTitle());

                            // save schedObj
   StuffCParm((CStringRW &) settingsLine,
               scheduleBy->GetSchedObjSettings().data());

   StuffCParm((CStringRW &) settingsLine,
              (smallestTree->GetBoxSliderSettings()).data());
   StuffCParm((CStringRW &) settingsLine,
              (itsResidualBasal->GetBoxSliderSettings()).data());
   StuffCParm((CStringRW &) settingsLine,
              (itsResidualPercent->GetBoxSliderSettings()).data());

                           // save radio selection (prep cut)
   if(itsPrepCutRadioGroup->GetSelectedButton() == itsPrepCutFirst)
      StuffCParm((CStringRW &) settingsLine, "0");
   else
      StuffCParm((CStringRW &) settingsLine, "1");

                           // prep cut check box
   if (itsPrepCutCheckBox->IsSelected())
      StuffCParm((CStringRW &) settingsLine, "1");
   else
      StuffCParm((CStringRW &) settingsLine, "0");

                           // edit control shelterwoodYears
   StuffCParm((CStringRW &) settingsLine,
              (char const *) itsShelterCutYears->GetTitle());
   StuffCParm((CStringRW &) settingsLine,
              (itsShelterBasal->GetBoxSliderSettings()).data());
   StuffCParm((CStringRW &) settingsLine,
              (itsShelterTrees->GetBoxSliderSettings()).data());
   StuffCParm((CStringRW &) settingsLine,
              (itsShelterPercent->GetBoxSliderSettings()).data());

                           // save radio selection (shelter cut)
   if(itsShelterCutRadioGroup->GetSelectedButton() == itsShelterCutFirst)
      StuffCParm((CStringRW &) settingsLine, "0");
   if(itsShelterCutRadioGroup->GetSelectedButton() == itsShelterCutSecond)
      StuffCParm((CStringRW &) settingsLine, "1");
   if(itsShelterCutRadioGroup->GetSelectedButton() == itsShelterCutThird)
      StuffCParm((CStringRW &) settingsLine, "2");

   StuffCParm((CStringRW &) settingsLine,
              (removalCutResidual->GetBoxSliderSettings()).data());
   StuffCParm((CStringRW &) settingsLine,
              (smallestTreeRC->GetBoxSliderSettings()).data());

                           // edit control removalCutYears
   StuffCParm((CStringRW &) settingsLine,
              (char const *) itsRemovalCutYears->GetTitle());

                           // removal cut check box
   if (itsRemovalCutCheckBox->IsSelected())
      StuffCParm((CStringRW &) settingsLine, "1");
   else
      StuffCParm((CStringRW &) settingsLine, "0");

   return settingsLine;
}

//------------------------------------------------------------------
//      ResetTheComponentSettings
//------------------------------------------------------------------

void ShelterwoodWin::ResetTheComponentSettings(const char * theSettings)
{

                           // reset user entered name
   CStringRW tmpString =  GetCParm((const char *) theSettings, 0);
   itsComponentName->SetTitle(tmpString);

                         // reset schedObj
   tmpString =  GetCParm((const char *) theSettings, 1);
   scheduleBy->ResetSchedObjSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings, 2);
   smallestTree->ResetBoxSliderSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings, 3);
   itsResidualBasal->ResetBoxSliderSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings, 4);
   itsResidualPercent->ResetBoxSliderSettings(tmpString.data());

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
   itsShelterCutYears->SetTitle(tmpString);

   tmpString =  GetCParm((const char *) theSettings, 8);
   itsShelterBasal->ResetBoxSliderSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings, 9);
   itsShelterTrees->ResetBoxSliderSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings, 10);
   itsShelterPercent->ResetBoxSliderSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings, 11);
   if (strchr(tmpString,'0')) DoCommand(6);
   if (strchr(tmpString,'1')) DoCommand(7);
   if (strchr(tmpString,'2')) DoCommand(8);

   tmpString =  GetCParm((const char *) theSettings,12);
   removalCutResidual->ResetBoxSliderSettings(tmpString.data());

   tmpString =  GetCParm((const char *) theSettings,13);
   smallestTreeRC->ResetBoxSliderSettings(tmpString.data());

                        // edit control removalCutYears
   tmpString =  GetCParm((const char *) theSettings,14);
   itsRemovalCutYears->SetTitle(tmpString);

                        // removal cut check box
   tmpString =  GetCParm((const char *) theSettings,15);

   if (strchr(tmpString,'1')) DoCommand(9);
   else DoCommand(10);
}



CStringRW ShelterwoodWin::BuildKeywordString(void)
{
   CStringRW keyLine,minDBH,minDBHrc,year,resid;
   char line[82];
   int aPrepYear, aShelterCutYear, anIntVal;
   char * zero   = "0";
   char * nines  = "999";
   char * cuteff  = "1";

   minDBH=smallestTree->GetTitle();

   // get year or conditional scheduling and year of begin schedule
   scheduleBy->SpGetValue(aPrepYear);

   // initially seed year same as prep year
   aShelterCutYear = aPrepYear;

   // prep cut box checked?
   if (itsPrepCutCheckBox->IsSelected())
   {
      // change seed year
      anIntVal=(int) ConvertToLong(itsShelterCutYears->GetTitle());
      aShelterCutYear = aPrepYear + anIntVal;
      year = ConvertToString(aPrepYear);

      // see which button pressed in prep cut box
      long selected = itsPrepCutRadioGroup->GetSelectedButton();

      // residual basal area
      if (selected == itsPrepCutFirst)
      {
         resid = itsResidualBasal->GetTitle();
         keyLine += "* thinBBA year,resid,cuteff,mindbh,maxdbh,minht,maxht\n";
         sprintf(line,"thinBBA   %10s%10s%10s%10s%10s%10s%10s\n",
                        year.data(),resid.data(),
                        cuteff, minDBH.data(), nines, zero, nines);
         keyLine += line;
      }
      else
      {
                              // the other: residual percent
         resid = ConvertToString((float)((itsResidualPercent->GetValue()) * .01));
         keyLine += "* Parms(resid,cuteff,species,mindbh,maxdbh,cutCntl)\n";
         sprintf(line,"thinSDI   %10s   Parms(%s*BSDIMax, 1, 0, %s, 999, 0)\n",
                 year.data(), resid.data(), minDBH.data());
         keyLine += line;
      }
   }

   // shelterwood cut
   long selected = itsShelterCutRadioGroup->GetSelectedButton();
   year = ConvertToString(aShelterCutYear);

   if (selected == itsShelterCutFirst)
   {
      resid = itsShelterBasal->GetTitle();
      keyLine += "* thinBBA year,resid,cuteff,mindbh,maxdbh,minht,maxht\n";
      sprintf(line,"thinBBA   %10s%10s%10s%10s%10s%10s%10s\n",
              year.data(),resid.data(),
              cuteff, minDBH.data(), nines, zero, nines);
      keyLine += line;
   }

   if (selected == itsShelterCutSecond)
   {
      resid = itsShelterTrees->GetTitle();
      keyLine += "* thinBTA year,resid,cuteff,mindbh,maxdbh,minht,maxht\n";
      sprintf(line,"thinBTA   %10s%10s%10s%10s%10s%10s%10s\n",
              year.data(),resid.data(),
              cuteff, minDBH.data(), nines, zero, nines);
      keyLine += line;
   }

   if (selected == itsShelterCutThird)
   {      
	  resid = ConvertToString((float)((itsShelterPercent->GetValue()) * .01));
      keyLine += "* Parms(resid,cuteff,species,mindbh,maxdbh,cutCntl)\n";
      sprintf(line,"thinSDI   %10s   Parms(%s*BSDIMax, 1, 0, %s, 999, 0)\n",
              year.data(), resid.data(), minDBH.data());
      keyLine += line;
   }

   // removal cut box checked?
   if (itsRemovalCutCheckBox->IsSelected())
   {
      minDBH=smallestTreeRC->GetTitle();
      anIntVal=(int) ConvertToLong(itsRemovalCutYears->GetTitle());
      year = ConvertToString(aShelterCutYear+anIntVal);
      resid = removalCutResidual->GetTitle();
      keyLine += "* thinBTA year,resid,cuteff,mindbh,maxdbh,minht,maxht\n";
      sprintf(line,"thinBTA   %10s%10s%10s%10s%10s%10s%10s\n",
                  year.data(),resid.data(),cuteff, minDBH.data(),
                  nines, zero, nines);
      keyLine += line;
   }
   return keyLine;
}
