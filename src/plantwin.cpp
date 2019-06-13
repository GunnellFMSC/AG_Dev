/********************************************************************

   File Name:     plantwin.cpp
   Author:        nlc
   Date:          01/15/98

********************************************************************/
#include "appdef.hpp"
#include "PwrDef.h"

#include "stand.hpp"
#include "runstr.hpp"
#include "group.hpp"
#include "compon.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include CStringCollection_i
#include CSubview_i
#include CStringRWC_i
#include NButton_i
#include NRadioButton_i
#include NListButton_i
#include CStringRW_i
#include CText_i
#include NLineText_i
#include NEditControl_i
#include CDesktop_i
#include CMenuBar_i
#include CRectangle_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "boxsldr.hpp"
#include "schedobj.hpp"
#include "plantwin.hpp"
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
#include "mylinetx.hpp"

#include "supphelp.hpp"    // for help #defines
extern XVT_HELP_INFO helpInfo;




PlantNaturalWin::PlantNaturalWin(const CStringRW& theTitle,
                                 int         fullEstab,
                                 Component * theComponent,
                                 Component * theConditionComponent)

                :CMgtClassWindow((CDocument *) theSpGlobals->theAppDocument,
#ifdef AIXV4

//  Set up the window size, as needed for the purpose.  This works on
//  AIX but not on Windows... so for Windows, Use the approach
//  of setting the window size below.

                                 (fullEstab ? CRect(5,20,605,516) :
                                              CRect(5,20,605,458)   ),
#else
                                 CenterTopWinPlacement(CRect(5,20,605,458)),
#endif
                                 theComponent,
                                 theConditionComponent, theTitle, NULL,
                                 WSF_ICONIZABLE | WSF_SIZE |
                                 WSF_CLOSE      | WSF_INVISIBLE,
                                 W_DOC, TASK_MENUBAR),
                 itsFullEstabInUse(fullEstab),
                 itsStockAdjRadioGroup(NULL),
                 itsSproutingRadioGroup(NULL),
                 itsPrepYearsLineText(NULL),
                 itsBurnPrepLineText(NULL),
                 itsMechPrepLineText(NULL),
                 itsInGrowthRadioGroup(NULL),
                 itsInGrowthOnDefault(0)
{

#ifndef AIXV4

   // size the window as necessary for the purpose.

   if (itsFullEstabInUse) Size(CenterTopWinPlacement(CRect(5,20,605,516)));

#endif

   // Build the species list.

   int nSpecies = theSpGlobals->theSpecies->getNumberSpecies();
   if (nSpecies > -1)
   {
      int spIndex;
      for (spIndex=0; spIndex<nSpecies; spIndex++)
      {
         itsSpeciesList.insert(new CStringRWC (
            theSpGlobals->theSpecies->getCommonName((const int) spIndex)));
      }
   }

   // Build the shade code list.

   itsShadeCodeList.insert(new CStringRWC("Uniform distribution"));
   itsShadeCodeList.insert(new CStringRWC("Most trees placed on dense plots"));
   itsShadeCodeList.insert(new CStringRWC("Most trees placed on sparse plots"));

   MakeStandardControls("Plant & Natural Regeneration");

   // move the reset button and place the "another" button

   CRect aRect = GetFrame();
   aRect.Bottom(aRect.Bottom()-4);
   aRect.Top(aRect.Bottom()-32);
   UNITS space = (aRect.Right()-540)/4;
   aRect.Left(92+space); aRect.Right(aRect.Left()+180);
   itsAnotherPlantNaturalButton = new NButton(this, aRect,
                                              "Another Plant/Natural", 0L);
   itsAnotherPlantNaturalButton->SetCommand(4);
   itsAnotherPlantNaturalButton->SetGlue(BOTTOMSTICKY);
   itsNavigator->InsertTabStop(1,new CTabStop(itsAnotherPlantNaturalButton));

   aRect.Left(aRect.Right()+space); aRect.Right(aRect.Left()+88);
   itsFreeFormButton->Size(aRect);
   aRect.Left(aRect.Right()+space); aRect.Right(aRect.Left()+88);
   itsResetButton->Size(aRect);
   aRect.Left(aRect.Right()+space); aRect.Right(aRect.Left()+88);
   itsCancelButton->Size(aRect);

   itsTopOfFreeSpace = 8;

   // find and define the MSText pointer for the rest of the window's life

   itsMSText = GetMSTextPtr("management.PlantNatural");

   // find out if the variant includes sprouting species.  If yes,
   // then create the sprouting species radio group.

   PKeyData * aPKeyData = NULL;
   if (itsMSText) aPKeyData = itsMSText->FindPKeyData("hasSproutingSpecies",
      (const char *) theSpGlobals->theVariants->GetSelectedVariant());

   itsHasSproutingSpecies = 1;
   if (aPKeyData) sscanf (aPKeyData->pString.data(),"%d",
                          &itsHasSproutingSpecies);
   itsHasSproutingSpecies = (itsHasSproutingSpecies == 1);

   int leftSide = 4;
   if (itsHasSproutingSpecies)
   {
      // create the stock adj radio group.

      new CText (itsScrollingFrame,
                 CPoint(leftSide,itsTopOfFreeSpace),"Sprouting:");

      itsSproutingRadioGroup = new CRadioGroup(itsScrollingFrame,
                                               CPoint(leftSide+80,
                                                      itsTopOfFreeSpace),
                                               FALSE);
      itsSproutingOn = itsSproutingRadioGroup->AddButton(CPoint(0,0),
                                                         "On", 7);
      itsSproutingOff = itsSproutingRadioGroup->AddButton(CPoint(60,0),
                                                          "Off", 7);

      itsSproutingOnButton = (NRadioButton*) itsSproutingRadioGroup->
                              FindSubview(itsSproutingOn);
      itsSproutingOffButton = (NRadioButton*) itsSproutingRadioGroup->
                              FindSubview(itsSproutingOff);

      aRect = itsSproutingOnButton->GetFrame();
      aRect.Right(aRect.Right()+20);
      itsSproutingOnButton->Size(aRect);
      aRect = itsSproutingOffButton->GetFrame();
      aRect.Right(aRect.Right()+40);
      itsSproutingOffButton->Size(aRect);

      aRect = itsSproutingRadioGroup->GetFrame();
      aRect.Right(aRect.Right()+40);
      itsSproutingRadioGroup->Size(aRect);

      itsSproutingRadioGroup->SetSelectedButton(itsSproutingOn);

      leftSide = 296;
   }

   // if the full estab is in use, then create the InGrow, yes/no
   // radio group.

   if (itsFullEstabInUse)
   {
      // create the stock adj radio group.

      new CText (itsScrollingFrame,CPoint(leftSide,itsTopOfFreeSpace),
                 "In Growth:");

      itsInGrowthRadioGroup = new CRadioGroup(itsScrollingFrame,
                                              CPoint(leftSide+80,
                                                     itsTopOfFreeSpace-7),
                                               FALSE);
      itsInGrowthOn = itsInGrowthRadioGroup->AddButton(CPoint(0,0),
                                                       "On", 7);
      itsInGrowthOff = itsInGrowthRadioGroup->AddButton(CPoint(60,0),
                                                        "Off", 7);

      itsInGrowthOnButton = (NRadioButton*) itsInGrowthRadioGroup->
                              FindSubview(itsInGrowthOn);
      itsInGrowthOffButton = (NRadioButton*) itsInGrowthRadioGroup->
                              FindSubview(itsInGrowthOff);

      aRect = itsInGrowthOnButton->GetFrame();
      aRect.Right(aRect.Right()+15);
      itsInGrowthOnButton->Size(aRect);
      aRect = itsInGrowthOffButton->GetFrame();
      aRect.Right(aRect.Right()+40);
      itsInGrowthOffButton->Size(aRect);

      aRect = itsInGrowthRadioGroup->GetFrame();
      aRect.Right(aRect.Right()+40);
      itsInGrowthRadioGroup->Size(aRect);

      if (itsMSText) aPKeyData = itsMSText->FindPKeyData("inGrowthDefault",
           (const char *) theSpGlobals->theVariants->GetSelectedVariant());

      if (aPKeyData) sscanf (aPKeyData->pString.data(),"%d",
                             &itsInGrowthOnDefault);
      itsInGrowthOnDefault = (itsInGrowthOnDefault == 1);

      if (itsInGrowthOnDefault)
           itsInGrowthRadioGroup->SetSelectedButton(itsInGrowthOn);
      else itsInGrowthRadioGroup->SetSelectedButton(itsInGrowthOff);

  }

   if (itsHasSproutingSpecies || itsFullEstabInUse) itsTopOfFreeSpace+=32;

   // create the date of disturbance text entry.

   aRect = itsScrollingFrame->GetFrame();

   aRect = CRect(4,itsTopOfFreeSpace,
                 itsScrollingFrame->GetFrame().Right()
                    -NScrollBar::NativeWidth()-8,
                 itsTopOfFreeSpace+88);

   CRectangle * aRectangle = new CRectangle(itsScrollingFrame,aRect);
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBrush(COLOR_BLACK,PAT_HOLLOW);
   anEnv.SetPen(COLOR_GRAY,2,PAT_SOLID,P_SOLID);
   aRectangle->SetEnvironment(anEnv);

   new CText (itsScrollingFrame,
              CPoint(8,itsTopOfFreeSpace-8),
              "  Date of disturbance is ...  ");

   // create ScheduleByObject with 100 as the ScheduleByObject DoCommand

   scheduleBy = new ScheduleByObject(this, 100, itsScrollingFrame,
                                     CPoint(8,itsTopOfFreeSpace+16), 50, 102);

   itsTopOfFreeSpace += 92;

   if (itsFullEstabInUse)
   {
      // create the stock adj radio group.

      itsStockAdjRadioGroup = new CRadioGroup(itsScrollingFrame,
                                              CPoint(4,itsTopOfFreeSpace-4),
                                              FALSE);

      itsStockAdjZero = itsStockAdjRadioGroup->AddButton(CPoint(0,0),
         "Establish only trees specified, StockAdj is Zero", 5);

      itsStockAdjSet = itsStockAdjRadioGroup->AddButton(CPoint(0,28),
         "Include predicted establishment, StockAdj is ", 5);

      itsStockAdjZeroButton = (NRadioButton*) itsStockAdjRadioGroup->
                              FindSubview(itsStockAdjZero);
      itsStockAdjSetButton = (NRadioButton*) itsStockAdjRadioGroup->
                              FindSubview(itsStockAdjSet);
      aRect = itsStockAdjSetButton->GetFrame();
      aRect.Right(aRect.Right()-20);
      itsStockAdjSetButton->Size(aRect);
      itsStockAdjLineText = new MyNLineText(itsScrollingFrame,
                                            CPoint(aRect.Right()+20,
                                                   itsTopOfFreeSpace+24),
                                            40, 6);
      itsStockAdjLineText->SetText("1.0");
      itsStockAdjLineText->Deactivate();
      itsStockAdjRadioGroup->SetSelectedButton(itsStockAdjZero);

      itsTopOfFreeSpace+=62;

      aRect = CRect(4,itsTopOfFreeSpace-8,
                    itsScrollingFrame->GetLocalFrame().Right()
                                   -NScrollBar::NativeWidth()-8,
                    itsTopOfFreeSpace+60);

      aRectangle = new CRectangle(itsScrollingFrame,aRect);
      aRectangle->SetEnvironment(anEnv);

      new CText (itsScrollingFrame,CPoint(8,itsTopOfFreeSpace),
                                          "Site preparation done");
      itsPrepYearsLineText = new MyNLineText(itsScrollingFrame,
                                             CPoint(170,itsTopOfFreeSpace-4),
                                             40, 6);
      itsPrepYearsLineText->SetText("1");

      new CText (itsScrollingFrame,CPoint(220,itsTopOfFreeSpace),
                 " year(s) after the date of disturbance.");

      itsTopOfFreeSpace+=32;

      new CText (itsScrollingFrame,CPoint(8,itsTopOfFreeSpace),
                                          "Percentage plots burned: ");
      itsBurnPrepLineText = new MyNLineText(itsScrollingFrame,
                                             CPoint(200,itsTopOfFreeSpace-4),
                                             40, 6);

      new CText (itsScrollingFrame,CPoint(255,itsTopOfFreeSpace),
                 " Percentage mechanically scarified:");
      itsMechPrepLineText = new MyNLineText(itsScrollingFrame,
                                             CPoint(510,itsTopOfFreeSpace-4),
                                             40, 6);
      itsTopOfFreeSpace+=36;
   }
   else itsTopOfFreeSpace+=8;

   new CText (itsScrollingFrame,CPoint(8,itsTopOfFreeSpace),
              "Planting/Natural done");
   itsPlantYearsLineText = new MyNLineText(itsScrollingFrame,
                                           CPoint(170,itsTopOfFreeSpace-4),
                                           40, 6);
   itsPlantYearsLineText->SetText("1");

   new CText (itsScrollingFrame,CPoint(220,itsTopOfFreeSpace),
              " year(s) after the date of disturbance.");

   itsTopOfFreeSpace+=32;

   PlantNaturalSubview * aPNSubview =
      new PlantNaturalSubview(this,itsScrollingFrame,
                              CRect(4,itsTopOfFreeSpace,
                                    itsScrollingFrame->GetLocalFrame().Right()
                                       -NScrollBar::NativeWidth()-8,
                                    itsTopOfFreeSpace+108));

   itsTopOfFreeSpace+=112;
   itsPlantNaturalSubviews.insert(aPNSubview);

   DoCommand (102,this);

   // itsComponent indicates that this is an
   // edit of an existing component; reset settings

   if (itsComponent)
   {
      if (itsComponent->windowParms)
      {
         itsInitialSettings = *itsComponent->windowParms;
         ResetTheComponentSettings();
      }
      itsConditionComponent = FindConditionComponent();
   }

   if (itsInitialSettings.length() == 0)
      itsInitialSettings = BuildCParmSettingString();

   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), PLANTNATURALWindow, 0L);

   itsScrollingFrame->ShrinkToFit();
   AdjustScrollingWinSize(itsScrollingFrame, this);

   Show();
   itsNavigator->InitFocus();
}




void PlantNaturalWin::DoCommand(long theCommand,void* theData)
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
      {                          // ok button pushed
                                 // SetSave set by SaveTheComponent
                                 // if there is not condition, change radio
                                 // button to by year

         if (!itsConditionComponent)
            scheduleBy->SetYearButton(TRUE);

         const char * extension = "strp";
         if (itsFullEstabInUse) extension = "estb";

         int usingFreeForm = itsFreeFormCommand==theCommand;
         CStringRW keywordLine = BuildKeywordString();
         CStringRW settingsLine=NULLString;
         if (!usingFreeForm) settingsLine=BuildCParmSettingString();

         SaveTheComponent (usingFreeForm,"PlantNatural",keywordLine,settingsLine,extension);

         closeCalledByCloseCancel = FALSE;

         Close();
         break;
      }

      case 2:                    // Reset was pressed
      {
         ResetTheComponentSettings();
         break;
      }
      case 3:                    // Cancel was pressed
      {
         Close();
         break;
      }
      case 4:                   // Another Plant/Natural button.
      {
         PlantNaturalSubview * aPNSubview =
            new PlantNaturalSubview(this,itsScrollingFrame,
                                    CRect(4,itsTopOfFreeSpace,
                                          itsScrollingFrame->GetLocalFrame().
                                          Right()-NScrollBar::NativeWidth()-8,
                                          itsTopOfFreeSpace+108));

         itsTopOfFreeSpace+=112;
         itsPlantNaturalSubviews.insert(aPNSubview);

         itsScrollingFrame->ShrinkToFit();

         // scroll as necessary to make sure the new plant/natural
         // is inside the visible part of the scroller.

         itsScrollingFrame->SetVIncrements(112,itsScrollingFrame->
                                           GetVPageIncrement());

         int diff = itsAnotherPlantNaturalButton->GetFrame().Bottom()-
                    itsScrollingFrame->GetClippedFrame().Bottom();

         // I thought this following line would work correctly, but it does not
         // if (diff > 0) itsScrollingFrame->AutoScroll(0,diff);

         // So, it is replaced with the following two statements:
         itsScrollingFrame->SetVIncrements(112,
            itsScrollingFrame->GetVPageIncrement());
         if (diff > 0) itsScrollingFrame->AutoScroll(0,112);

         SetFocusOnNextElement(theData);

         // having a slight problem getting all the elements to draw.
         // so, here is a way to address the problem:

         RCT aRct = RCT(GetFrame());
         xvt_dwin_invalidate_rect(GetXVTWindow(), &aRct);
         break;
      }
      case 5:                    // StockAdj radio group
      {
         SetFocusOnNextElement(theData);
         break;
      }
      case 6:                    // MyLineText cr command.
      {
         SetFocusOnNextElement(theData);
         break;
      }
      case 7:                    // Sprouting and Ingrowth (both on and off).
      {
         SetFocusOnNextElement(theData);
         break;
      }

      case 50:                   // ScheduleBy : Condition (existing)
      {
         CStringRW conditionTitle = "Edit - ";
         conditionTitle += (CStringRW&) *itsConditionComponent;
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
               this, "management.PlantNatural", "defaultCondition",
               conditionTitle, itsConditionComponent);
         break;
      }

      case 51:                   // ScheduleBy : Condition (existing); set to
                                 // NULL since its closed if you're here
         itsSchByCondWindow = NULL;
         break;

      case 100:
      {
         CStringRW conditionTitle = "Condition - ";
         conditionTitle += itsComponentName->GetTitle();
         itsSchByCondWindow = BuildMgtConditionWindow(itsSchByCondWindow,
                   this, "management.PlantNatural", "defaultCondition",
                   conditionTitle, itsConditionComponent);
	     break;
      }
      case 101:

      // ScheduleBy : Condition (return condition
      // string and Notify Distroyed)
      {
         itsConditionComponent = (Component *) theData;
         itsSchByCondWindow = NULL;

         CheckForNULLMgtCondition();
         if (itsConditionComponent)
            scheduleBy->ResetConditionButtonCommand(50);
         break;
      }

      case 102: // ScheduleBy : status changed in some way.
      {
         SetFocusOnNextElement(theData);
         break;
      }

      default:

         CMgtClassWindow::DoCommand(theCommand, theData);
   }
}



CStringRW PlantNaturalWin::BuildCParmSettingString(void)
{
   CStringRW settingsLine;

   CStringRW t;
   t = itsComponentName->GetTitle();
   StuffCParm(settingsLine,t.data());
   t = scheduleBy->GetSchedObjSettings();
   StuffCParm(settingsLine,t.data());

   CStringRW tmp="-1 ";

   if (itsSproutingRadioGroup)
   {
      if (itsSproutingRadioGroup->GetSelectedButton() == itsSproutingOff)
           tmp="0 ";
      else tmp="1 ";
   }

   if (itsInGrowthRadioGroup)
   {
      if (itsInGrowthRadioGroup->GetSelectedButton() == itsInGrowthOff)
           tmp+="0 ";
      else tmp+="1 ";
   }
   else tmp+="-1 ";

   if (itsStockAdjRadioGroup)
   {
      tmp+=itsStockAdjLineText->GetText();
      if (itsStockAdjRadioGroup->GetSelectedButton() == itsStockAdjZero)
           tmp+=" 0 ";
      else tmp+=" 1 ";
   }
   else tmp+="-1 -1 ";

   if (itsPrepYearsLineText && itsPrepYearsLineText->GetText().length())
   {
      tmp+=itsPrepYearsLineText->GetText();
      tmp+=" ";
   }
   else tmp+="b ";

   if (itsBurnPrepLineText && itsBurnPrepLineText->GetText().length())
   {
      tmp+=itsBurnPrepLineText->GetText();
       tmp+=" ";
   }
   else tmp+="b ";

   if (itsMechPrepLineText && itsMechPrepLineText->GetText().length())
   {
      tmp+=itsMechPrepLineText->GetText();
      tmp+=" ";
   }
   else tmp+="b ";

   tmp+=ConvertToString((int) itsPlantNaturalSubviews.entries());

   // store the contents of itsPlantYearsLineText see comment in
   // ResetTheComponentSettings.

   tmp+=" ";
   tmp+=itsPlantYearsLineText->GetText();

   StuffCParm(settingsLine,tmp.data());

   PlantNaturalSubview * aPlantNatural;
   RWOrderedIterator nextSubview(itsPlantNaturalSubviews);
   while (aPlantNatural = (PlantNaturalSubview *) nextSubview())
   {
      CStringRW * aP = aPlantNatural->GetParms();
      if (aP->length()) StuffCParm(settingsLine,aP->data());
   }
   return settingsLine;
}




void PlantNaturalWin::ResetTheComponentSettings()
{

   itsComponentName->SetTitle(GetCParm(itsInitialSettings.data(), 0));
   scheduleBy->ResetSchedObjSettings(GetCParm(itsInitialSettings.data(),
                                              1).data());

   RWCTokenizer next(GetCParm(itsInitialSettings.data(),2));

   CStringRW tmp = next();
   int aInt = (int) ConvertToLong(tmp);

   if (itsSproutingRadioGroup)
   {
      if (aInt==1) itsSproutingRadioGroup->SetSelectedButton(itsSproutingOn);
      else         itsSproutingRadioGroup->SetSelectedButton(itsSproutingOff);
   }

   tmp = next();
   aInt = (int) ConvertToLong(tmp);

   if (itsInGrowthRadioGroup)
   {
      if (aInt==1) itsInGrowthRadioGroup->SetSelectedButton(itsInGrowthOn);
      else         itsInGrowthRadioGroup->SetSelectedButton(itsInGrowthOff);
   }

   if (itsStockAdjRadioGroup)
   {
      itsStockAdjLineText->SetText(next());
      tmp = next();
      if (tmp.first('1') != RW_NPOS)
           itsStockAdjRadioGroup->SetSelectedButton(itsStockAdjSet);
      else itsStockAdjRadioGroup->SetSelectedButton(itsStockAdjZero);
   }
   else
   {
      next();
      next();
   }

   tmp = next();
   if (itsPrepYearsLineText)
   {
       if (tmp.first('b') != RW_NPOS) itsPrepYearsLineText->Clear();
       else                           itsPrepYearsLineText->SetText(tmp);
   }

   tmp = next();
   if (itsBurnPrepLineText)
   {
       if (tmp.first('b') != RW_NPOS) itsBurnPrepLineText->Clear();
       else                           itsBurnPrepLineText->SetText(tmp);
   }

   tmp = next();
   if (itsMechPrepLineText)
   {
      if (tmp.first('b') != RW_NPOS) itsMechPrepLineText->Clear();
      else                           itsMechPrepLineText->SetText(tmp);
   }

   // reset the views that were created when the window opened (those
   // have a parameter string.

   tmp = next();
   int nViews = (int) ConvertToLong(tmp);

   // the itsPlantYearsLineText object is restored after the nViews is
   // recovered.  This order is used so that keywords created prior to
   // the proper recovery of the itsPlantYearsLineText could be edited.

   tmp = next();
   if (!tmp.isNull()) itsPlantYearsLineText->SetText(tmp);

   PlantNaturalSubview * aPlantNatural;
   for (aInt=0; aInt<nViews; aInt++)
   {
      if (aInt >= itsPlantNaturalSubviews.entries()) DoCommand (4,NULL);
      aPlantNatural = (PlantNaturalSubview *)
         itsPlantNaturalSubviews.at((size_t) aInt);
      aPlantNatural->Init(GetCParm(itsInitialSettings.data(),3+aInt));
   }

   // reset the remaining views (those with on parameter string).

   if (nViews<itsPlantNaturalSubviews.entries())
   {
      for (aInt=nViews; aInt<itsPlantNaturalSubviews.entries(); aInt++)
      {
         aPlantNatural = (PlantNaturalSubview *)
            itsPlantNaturalSubviews.at((size_t) aInt);
         aPlantNatural->Init(NULL);
      }
   }
}



CStringRW PlantNaturalWin::BuildKeywordString(void)
{
   CStringRW tmp;
   CStringRW tmp2;
   size_t aLen;

   CStringRW keyLine="Estab";

   if (scheduleBy->YearIsSelected())
   {
      aLen = strlen(scheduleBy->SpGetTitle());
      if (aLen && aLen < 20)
      {
         keyLine.resize(20-aLen);
         keyLine+=scheduleBy->SpGetTitle();
      }
   }

   if (itsSproutingRadioGroup)
   {
      if (itsSproutingRadioGroup->GetSelectedButton() == itsSproutingOff)
           keyLine+="\nNoSprout";
      else keyLine+="\nSprout";
   }

   if (itsInGrowthRadioGroup)
   {
      if (itsInGrowthRadioGroup->GetSelectedButton() == itsInGrowthOff)
           keyLine+="\nNoInGrow";
      else keyLine+="\nInGrow";
   }

   if (itsStockAdjRadioGroup)
   {
      tmp2 = scheduleBy->SpGetTitle();
      tmp.remove(0);
      aLen = tmp2.length();
      if (aLen < 10) tmp.resize(10-aLen);
      else           tmp2.resize(10);
      tmp+=tmp2;
      keyLine+="\nStockAdj  ";
      keyLine+=tmp;

      if (itsStockAdjRadioGroup->GetSelectedButton() == itsStockAdjZero)
      {
         keyLine+="       0.0";
      }
      else
      {
         tmp2=itsStockAdjLineText->GetText();
         aLen=tmp2.length();
         tmp.remove(0);
         if (aLen < 10) tmp.resize((size_t) (10-aLen));
         else           tmp2.resize(10);
         tmp+=tmp2;
         keyLine+=tmp;
      }
   }

   if (itsPrepYearsLineText)
   {
      int yearI = (int) ConvertToLong (scheduleBy->SpGetTitle());
      int prepI = (int) ConvertToLong (itsPrepYearsLineText->GetText());
      yearI=yearI+prepI;
      CStringRW yearS = ConvertToString(yearI);
      aLen = yearS.length();
      tmp.remove(0);
      if (aLen < 10)
      {
         tmp.resize((size_t) (10-aLen));
         tmp+=yearS;
         yearS=tmp;
      }
      else yearS.resize(10);

      tmp2 = itsBurnPrepLineText->GetText();
      aLen = tmp2.length();
      if (aLen)
      {
         tmp.remove(0);
         if (aLen < 10) tmp.resize((size_t) (10-aLen));
         else           tmp2.resize(10);
         tmp+=tmp2;
         keyLine+="\nBurnPrep  ";
         keyLine+=yearS;
         keyLine+=tmp;
      }

      tmp2 = itsMechPrepLineText->GetText();
      aLen = tmp2.length();
      if (aLen)
      {
         tmp.remove(0);
         if (aLen < 10) tmp.resize((size_t) (10-aLen));
         else           tmp2.resize(10);
         tmp+=tmp2;
         keyLine+="\nMechPrep  ";
         keyLine+=yearS;
         keyLine+=tmp;
      }
   }

   RWOrderedIterator nextSubview(itsPlantNaturalSubviews);
   PlantNaturalSubview * aPlantNatural;
   while (aPlantNatural = (PlantNaturalSubview *) nextSubview())
         aPlantNatural->GetKeyword(&keyLine);

   return keyLine;
}



BOOLEAN PlantNaturalWin::Close(void)
{
   itsSpeciesList.clearAndDestroy();
   itsShadeCodeList.clearAndDestroy();
   itsPlantNaturalSubviews.clear(); // don't destroy!

   return CMgtClassWindow::Close();
}




void PlantNaturalWin::SetFocusOnNextElement(void * theData)
{

   PlantNaturalSubview * aPlantNatural;

   if (theData == NULL || theData == this) return;
   else if (theData == itsAnotherPlantNaturalButton)
   {
      aPlantNatural = (PlantNaturalSubview *) itsPlantNaturalSubviews.last();
      aPlantNatural->itsSpeciesListButton->Activate();
   }
   else if (theData == itsStockAdjSetButton) itsStockAdjLineText->Activate();
   else if (theData == itsStockAdjLineText ||
            theData == itsStockAdjZeroButton)
   {
      if (itsPrepYearsLineText) itsPrepYearsLineText->Activate();
      else                      itsPlantYearsLineText->Activate();
   }
   else if (theData == itsPrepYearsLineText) itsBurnPrepLineText->Activate();
   else if (theData == itsBurnPrepLineText)  itsMechPrepLineText->Activate();
   else if (theData == itsMechPrepLineText)  itsPlantYearsLineText->Activate();
   else if (theData == itsPlantYearsLineText)
   {
      aPlantNatural = (PlantNaturalSubview *) itsPlantNaturalSubviews.first();
      aPlantNatural->itsSpeciesListButton->Activate();
   }
}




PlantNaturalSubview::PlantNaturalSubview(PlantNaturalWin * thePlantNaturalWin,
                                         MyCScroller     * theScrollingFrame,
                                         const CRect     & theRegion)
                    :CSubview(theScrollingFrame, theRegion),
                     itsPlantNaturalWin(thePlantNaturalWin)
{
   CRect aRect = GetLocalFrame();
   CRectangle * aRectangle = new CRectangle(this,aRect);
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBrush(COLOR_BLACK,PAT_HOLLOW);
   anEnv.SetPen(COLOR_GRAY,2,PAT_SOLID,P_SOLID);
   aRectangle->SetEnvironment(anEnv);

   int viewRight = aRect.Right()-4;

   itsPlantNaturalRadioGroup = new CRadioGroup(this,CPoint(2,2),FALSE);

   itsPlant = itsPlantNaturalRadioGroup->AddButton(CPoint(0,0),"Plant", 4);
   itsNatural = itsPlantNaturalRadioGroup->AddButton(CPoint(80,0),"Natural",5);

   itsPlantButton = (NRadioButton*) itsPlantNaturalRadioGroup->
                                    FindSubview(itsPlant);
   itsNaturalButton = (NRadioButton*) itsPlantNaturalRadioGroup->
                                      FindSubview(itsNatural);

   new CText(this,CPoint(200,6),"Species:");
   itsSpeciesListButton = new NListButton(this,CRect(300,4,viewRight,200),
                              itsPlantNaturalWin->itsSpeciesList, 0L);
   itsSpeciesListButton->DeselectAll();
   itsSpeciesListButton->SetCommand(1);

   new CText(this,CPoint(4,44),"Trees/acre:");
   itsTreesAcreNLineText = new MyNLineText(this,CPoint(100,40),40,6);

   new CText(this,CPoint(150,44),"Percent survival:");
   itsSurvivalNLineText = new MyNLineText(this,CPoint(300,40),40,6);

   new CText(this,CPoint(350,44),"Average age:");
   itsAveAgeNLineText = new MyNLineText(this,CPoint(450,40),40,6);

   new CText(this,CPoint(4,78),"Ave height:");
   itsAveHeightNLineText = new MyNLineText(this,CPoint(100,74),40,6);

   new CText(this,CPoint(150,78),"Shade code:");
   itsShadeCodeListButton = new NListButton(this,CRect(250,74,viewRight,170),
                                itsPlantNaturalWin->itsShadeCodeList, 0L);
   itsShadeCodeListButton->SetCommand(2);

   Init(NULL);
}



void PlantNaturalSubview::DoCommand(long theCommand,void * theData)
{
   switch (theCommand)
   {
      case 1:                    // Species selection
      case 2:                    // Shade code selection
      case 4:                    // Plant
      case 5:                    // Natural
      case 6:                    // MyLineText cr command.
      {
         if (!theData) break;
         else if (theData == itsPlantButton ||
                  theData == itsNaturalButton)
              itsSpeciesListButton->Activate();
         else if (theData == itsSpeciesListButton)
              itsTreesAcreNLineText->Activate();
         else if (theData == itsTreesAcreNLineText)
              itsSurvivalNLineText->Activate();
         else if (theData == itsSurvivalNLineText)
              itsAveAgeNLineText->Activate();
         else if (theData == itsAveAgeNLineText)
              itsAveHeightNLineText->Activate();
         else if (theData == itsAveHeightNLineText)
              itsShadeCodeListButton->Activate();
         else if (theData == itsShadeCodeListButton)
              itsPlantNaturalWin->itsAnotherPlantNaturalButton->Activate();
         break;
      }
   }
}



void PlantNaturalSubview::Init( const char * theParms )
{
   if (theParms) itsParms = theParms;
   else          itsParms.remove(0);

   if (itsParms.length())
   {
      RWCTokenizer next(itsParms);
      if (ConvertToLong(next().data()))
           itsPlantNaturalRadioGroup->SetSelectedButton(itsPlant);
      else itsPlantNaturalRadioGroup->SetSelectedButton(itsNatural);
      itsSpeciesListButton->SelectItem((int) ConvertToLong(next().data()));
      CStringRW aTok = next();
      if (aTok.first('b') == RW_NPOS) itsTreesAcreNLineText->SetText(aTok);
      else                            itsTreesAcreNLineText->Clear();
      aTok = next();
      if (aTok.first('b') == RW_NPOS) itsSurvivalNLineText->SetText(aTok);
      else                            itsSurvivalNLineText->Clear();
      aTok = next();
      if (aTok.first('b') == RW_NPOS) itsAveAgeNLineText->SetText(aTok);
      else                            itsAveAgeNLineText->Clear();
      aTok = next();
      if (aTok.first('b') == RW_NPOS) itsAveHeightNLineText->SetText(aTok);
      else                            itsAveHeightNLineText->Clear();
      itsShadeCodeListButton->SelectItem((int) ConvertToLong(next().data()));
   }
   else
   {
      itsPlantNaturalRadioGroup->SetSelectedButton(itsPlant);
      itsSpeciesListButton->DeselectAll();
      itsTreesAcreNLineText->Clear();
      itsSurvivalNLineText->SetText("100");
      itsAveAgeNLineText->Clear();
      itsAveHeightNLineText->Clear();
      itsShadeCodeListButton->SelectItem(0);
   }
}



CStringRW * PlantNaturalSubview::GetParms( void )
{
   // clear the parameters string.

   if (itsParms.length()) itsParms = itsParms.remove(0);
   if (NumberTrees() > 0.0)
   {
      if (itsPlantNaturalRadioGroup->GetSelectedButton() ==
          itsPlant) itsParms = "1 ";
      else          itsParms = "0 ";
      itsParms+=ConvertToString(itsSpeciesListButton->GetSelectPosition());
      itsParms+=" ";
      itsParms+=itsTreesAcreNLineText->GetText();
      itsParms+=" ";
      itsParms+=itsSurvivalNLineText->GetText();
      if (itsAveAgeNLineText->GetText().length())
      {
         itsParms+=" ";
         itsParms+=itsAveAgeNLineText->GetText();
      }
      else itsParms+=" b";
      if (itsAveHeightNLineText->GetText().length())
      {
         itsParms+=" ";
         itsParms+=itsAveHeightNLineText->GetText();
      }
      else itsParms+=" b";
      itsParms+=" ";
      itsParms+=ConvertToString(itsShadeCodeListButton->GetSelectPosition());
   }
   return &itsParms;
}




void PlantNaturalSubview::GetKeyword(CStringRW * theKeyword)
{

   if (theKeyword && NumberTrees() > 0.0)
   {
      if (itsPlantNaturalRadioGroup->GetSelectedButton() ==
          itsPlant) *theKeyword+="\nPlant     ";
      else          *theKeyword+="\nNatural   ";

      CStringRW tmp;
      int yearI = (int) ConvertToLong (itsPlantNaturalWin->
                                       scheduleBy->SpGetTitle());
      int plantI = (int) ConvertToLong (itsPlantNaturalWin->
                                        itsPlantYearsLineText->GetText());
      yearI=yearI+plantI;
      CStringRW tmp2 = ConvertToString(yearI);
      size_t aLen = tmp2.length();
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      else           tmp2.resize(10);
      tmp+=tmp2;
      *theKeyword+=tmp;

      int aInt = itsSpeciesListButton->GetSelectPosition();
      const char * cptr = theSpGlobals->theSpecies->
                          getAlphaCode((const int) aInt);
      if (!cptr) return;
      aLen = strlen(cptr);
      tmp.remove(0);
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      tmp+=cptr;
      *theKeyword+=tmp;

      tmp2 = itsTreesAcreNLineText->GetText();
      aLen = tmp2.length();
      tmp.remove(0);
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      else           tmp2.resize(10);
      tmp+=tmp2;
      *theKeyword+=tmp;

      tmp2 = itsSurvivalNLineText->GetText();
      aLen = tmp2.length();
      tmp.remove(0);
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      else           tmp2.resize(10);
      tmp+=tmp2;
      *theKeyword+=tmp;

      tmp2 = itsAveAgeNLineText->GetText();
      aLen = tmp2.length();
      tmp.remove(0);
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      else           tmp2.resize(10);
      tmp+=tmp2;
      *theKeyword+=tmp;

      tmp2 = itsAveHeightNLineText->GetText();
      aLen = tmp2.length();
      tmp.remove(0);
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      else           tmp2.resize(10);
      tmp+=tmp2;
      *theKeyword+=tmp;

      tmp2 = ConvertToString(itsShadeCodeListButton->GetSelectPosition());
      aLen = tmp2.length();
      tmp.remove(0);
      if (aLen < 10) tmp.resize((size_t) (10-aLen));
      else           tmp2.resize(10);
      tmp+=tmp2;
      *theKeyword+=tmp;
   }
}




float PlantNaturalSubview::NumberTrees( void )
{
   int aInt = itsSpeciesListButton->GetSelectPosition();
   if (aInt > -1 &&
       theSpGlobals->theSpecies->getAlphaCode((const int) aInt))
   {
      float nP = ConvertToFloat(itsTreesAcreNLineText->GetText().data());
      float sR = ConvertToFloat(itsSurvivalNLineText->GetText().data());
      sR = sR*.01;
      if (nP > 0.0 && sR > 0.0) return (nP*sR);
   }
   return 0.0;
}
