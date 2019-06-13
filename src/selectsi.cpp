/********************************************************************

   File Name:     selectsi.cpp   (Select Simulation Stands)
   Author:        nlc & kfd
   Date:          11/18/94 & 02/02/97

   Notes: see selectsi.hpp

********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "alllocd.hpp"
#include "stand.hpp"
#include "group.hpp"
#include "runstr.hpp"
#include "mainwin.hpp"
#include "spglobal.hpp"
#include "variants.hpp"
#include "selectsi.hpp"
#include "spprefer.hpp"
#include "suppdefs.hpp"
#include "readcomp.hpp"
#include "mylinetx.hpp"
#include "spfunc2.hpp"
#include "cretcomp.hpp"

#include CStringCollection_i
#include CText_i
#include CDocument_i
#include CStringRWC_i
#include CNavigator_i
#include NRadioNavigator_i
#include NButton_i
#include NGroupBox_i
#include NRadioButton_i
#include CStringRW_i
#include NText_i
#include NListBox_i
#include NCheckBox_i

#include <rw/ordcltn.h>
#include <rw/collstr.h>
#include <rw/regexp.h>
#include "msgwin.hpp"
#include "sendupdt.hpp"

#include "warn_dlg.hpp"

#include "supphelp.hpp"
extern XVT_HELP_INFO helpInfo;

SelectSimStandWin::SelectSimStandWin(CDocument * theDocument,
                                     FILE_SPEC * theLocFilePointer,
                                     FILE      * locFile)
                  :CWindow(theDocument,
                           RightWinPlacement(CRect(8,25,652,407)),
                           "Select Simulation Stands",
                           WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                           W_DOC,TASK_MENUBAR),
                   itsLocFilePointer(theLocFilePointer)
{
   AddDynamicHelpItems(GetMenuBar());

   xvt_scr_set_busy_cursor();

   // create and clear the navigator

   itsNavigator = GetNavigator();
   itsNavigator->ClearTabStops();

   // location or group radio buttons

   itsSelectionRadioGroup = new CRadioGroup(this,CPoint(4,1), FALSE);

   // By Locations, By Groups label wording is changed what you see.

   itsSelectionByLocation = itsSelectionRadioGroup->AddButton(CPoint(0,0),
                                                   "Pick Locations First",1);

   itsSelectionByGroup = itsSelectionRadioGroup->AddButton(CPoint(220,0),
                                                 "Pick Groups First",2);

   itsSelectionByLocationButton = (NRadioButton*) itsSelectionRadioGroup->
                                     FindSubview(itsSelectionByLocation);

   itsSelectionByGroupButton = (NRadioButton*) itsSelectionRadioGroup->
                                     FindSubview(itsSelectionByGroup);

   itsSelectionRadioGroup->SetSelectedButton(itsSelectionByLocation);

   // create the lists...

   itsLocList   = new RWOrdered ((size_t)   50);
   itsGroupList = new RWOrdered ((size_t)   50);
   itsStandList = new RWOrdered ((size_t) 2000);
   //itsStandList = new RWSortedVector((size_t) 2000);

   // location listbox

   itsLocationListBox = new NListBox(this,CRect(4,26,220,240),*itsLocList,
                                     CTL_FLAG_MULTIPLE);
   itsLocationListBox->SetCommand(3);
   itsLocationListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY);

   // group listbox

   itsGroupListBox = new NListBox(this,CRect(224,26,456,240),*itsGroupList,
                                  CTL_FLAG_MULTIPLE);
   itsGroupListBox->SetCommand(4);
   itsGroupListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY);

   // Addfile area

#ifdef AIXV4
   itsAddFileGroupBox = new NGroupBox (this,CRect(4,254,220,308),
                                       NULLString,0L);
   CText * tmpCText = new CText (this,CPoint(4,234),
                                 "Addfile processing ");
   tmpCText->SetGlue(BOTTOMSTICKY);

#else
   itsAddFileGroupBox = new NGroupBox (this,CRect(4,242,220,306),
                                       "Addfile processing ",0L);
#endif

   itsAddFileGroupBox->SetGlue(BOTTOMSTICKY);

//TODO Code to handle first checkbox
   itsAddFileRadioGroup = new CRadioGroup(this,CPoint(4,254), FALSE);

   itsAddFileYes  = itsAddFileRadioGroup->
      AddButton(CPoint(0,0),"Include addfiles/addkeys",20);

   // set YES the default

   itsAddFileRadioGroup->SetSelectedButton(itsAddFileYes);

   itsAddFileNo = itsAddFileRadioGroup->
      AddButton(CPoint(0,24),"Do not include them",20);

   itsAddFileYesButton = (NRadioButton*) itsAddFileRadioGroup->
                         FindSubview(itsAddFileYes);

   itsAddFileNoButton = (NRadioButton*) itsAddFileRadioGroup->
                         FindSubview(itsAddFileNo);

   itsAddFileRadioGroup->DoSetGlue(BOTTOMSTICKY);

   // stand selection criteria group radio buttons

#ifdef AIXV4
   itsStandSelectionGroupBox = new NGroupBox (this,CRect(224,254,456,308),
                                             NULLString,0L);
   tmpCText = new CText (this,CPoint(224,234),"A stand is listed... ");
   tmpCText->SetGlue(BOTTOMSTICKY);
#else
   itsStandSelectionGroupBox = new NGroupBox (this,CRect(224,242,456,306),
                                             "A stand is listed... ",0L);
#endif

   itsStandSelectionGroupBox->SetGlue(BOTTOMSTICKY);

   itsStandSelectionRadioGroup = new CRadioGroup(this,CPoint(224,254), FALSE);

   itsSelInAny = itsStandSelectionRadioGroup->
         AddButton(CPoint(0,0), "if in any selected group", 11);

   itsSelInEvery = itsStandSelectionRadioGroup->
         AddButton(CPoint(0,24),"if in every selected group", 12);

   itsSelInAnyButton = (NRadioButton*) itsStandSelectionRadioGroup->
      FindSubview(itsSelInAny);

   itsSelInEveryButton = (NRadioButton*) itsStandSelectionRadioGroup->
      FindSubview(itsSelInEvery);

   itsStandSelectionRadioGroup->SetSelectedButton(itsSelInEvery);

   itsStandSelectionRadioGroup->DoSetGlue(BOTTOMSTICKY);

   // available stands listbox

   itsAvailableNText = new NText(this, CRect(460,4,584,28),
                                 "Available Stands",0);

   itsStandListBox = new NListBox(this, CRect(460,26,640,268),*itsStandList,
                                  CTL_FLAG_MULTIPLE);
   itsStandListBox->SetCommand(5);
   itsStandListBox->SetDoubleCommand(10);
   itsStandListBox->SetGlue(ALLSTICKY);

   itsAllStandsButton = new NButton(this, CRect(460,272,640,304),
                                    "All Stands");
   itsAllStandsButton->SetCommand(13);
   itsAllStandsButton->SetGlue(BOTTOMSTICKY);

   NText * desiredStandText = new NText(this,
                                        CRect(338,314,450,338),
                                        "Desired stand:",0);
   desiredStandText->SetGlue(BOTTOMSTICKY);
   itsStandIDText = new MyNLineText(this, CPoint(460,310),
                                    (UNITS) 180, 14, 0, 30);
   itsStandIDText->SetGlue(BOTTOMSTICKY | LEFTSTICKY | RIGHTSTICKY);
   if (theSpGlobals->runStream->allStands->last())
      itsStandIDText->SetText(*((CStringRWC *)
                                theSpGlobals->runStream->allStands->last()));

   // current stand count.

   itsCountTextString = new CStringRW((RWSize_T) 40);
   itsCountNText = new NText(this, CRect(4,314,246,338),
                             *itsCountTextString , 0l);
   itsCountNText->SetGlue(BOTTOMSTICKY);
   SetCountString();

   // bottom buttons

   itsCloseButton = new NButton(this, CRect(493,346,640,378), "Close");
   itsCloseButton->SetCommand(7);
   itsCloseButton->SetGlue(BOTTOMSTICKY);

   itsDeleteSButton = new NButton(this, CRect(191,346,334,378), "Delete Stand");
   itsDeleteSButton->SetCommand(8);
   itsDeleteSButton->SetGlue(BOTTOMSTICKY);

   itsBareGroundButton = new NButton(this, CRect(342,346,485,378),
                                      "Bare Ground");
   itsBareGroundButton->SetCommand(9);
   itsBareGroundButton->SetGlue(BOTTOMSTICKY);

   itsAddStandsButton = new NButton(this, CRect(4,346,183,378),
                                    "Add Stand(s)");
   itsAddStandsButton->SetCommand(10);
   itsAddStandsButton->Disable();
   itsAddStandsButton->SetGlue(BOTTOMSTICKY);

                           // create the locations data
   itsLocationData = new AllLocationData(locFile);
   fclose(locFile);

   SpFillLocationList();

   // HELP file associations

   WINDOW ctl=GetXVTWindow();
   xvt_help_set_win_assoc(helpInfo, ctl, SELECTSIWindow, 0L);

   xvt_help_set_win_assoc (helpInfo,
                           itsSelectionByLocationButton->GetXVTWindow(),
                           SELECTSIWinByLocOrGroupButtons, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsSelectionByGroupButton->GetXVTWindow(),
                           SELECTSIWinByLocOrGroupButtons, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsLocationListBox->GetXVTWindow(),
                           SELECTSIWinLocListBox, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsGroupListBox->GetXVTWindow(),
                           SELECTSIWinGroupListBox, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsStandListBox->GetXVTWindow(),
                           SELECTSIWinStandListBox, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsAvailableNText->GetXVTWindow(),
                           SELECTSIWinStandListBox, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsAllStandsButton->GetXVTWindow(),
                           SELECTSIWinStandListBox, 0L);
// Can't figure out how to get the window of an NLineText object.
//   xvt_help_set_win_assoc (helpInfo,
//                         itsStandIDText->GetXVTWindow(),
//                         SELECTSIWinStandIDText, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           desiredStandText->GetXVTWindow(),
                           SELECTSIWinStandIDText, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsCloseButton->GetXVTWindow(),
                           SELECTSIWinCloseButton, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsDeleteSButton->GetXVTWindow(),
                           SELECTSIWinDelSButton, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsBareGroundButton->GetXVTWindow(),
                           SELECTSIWinBareGroundButton, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsAddStandsButton->GetXVTWindow(),
                           SELECTSIWinAddStandsButton, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsAddFileGroupBox->GetXVTWindow(),
                           SELECTSIWinAddFileYesOrNoButtons, 0L);
   //TODO Helpfile associations
   /*xvt_help_set_win_assoc (helpInfo,
                           itsAddFileYesButton->GetXVTWindow(),
                           SELECTSIWinAddFileYesOrNoButtons, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsAddFileNoButton->GetXVTWindow(),
                           SELECTSIWinAddFileYesOrNoButtons, 0L);*/
   xvt_help_set_win_assoc (helpInfo,
                           itsStandSelectionGroupBox->GetXVTWindow(),
                           SELECTSIWinStandSelectionButtons, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsSelInAnyButton->GetXVTWindow(),
                           SELECTSIWinStandSelectionButtons, 0L);
   xvt_help_set_win_assoc (helpInfo,
                           itsSelInEveryButton->GetXVTWindow(),
                           SELECTSIWinStandSelectionButtons, 0L);

   itsNavigator->AppendTabStop(new CTabStop(itsStandIDText));
   itsNavigator->AppendTabStop(new CTabStop(itsAddStandsButton));
   itsNavigator->AppendTabStop(new CTabStop(itsCloseButton));
   itsNavigator->AppendTabStop(new CTabStop
                               (new NRadioNavigator(itsSelectionRadioGroup)));
   itsNavigator->AppendTabStop(new CTabStop(itsLocationListBox));
   itsNavigator->AppendTabStop(new CTabStop(itsGroupListBox));
   itsNavigator->AppendTabStop(new CTabStop(itsStandListBox));
   //TODO TabStop
   /*itsNavigator->AppendTabStop(new CTabStop
                               (new NRadioNavigator(itsAddFileRadioGroup)));*/
   itsNavigator->AppendTabStop(new CTabStop(itsDeleteSButton));
   itsNavigator->AppendTabStop(new CTabStop
                               (new NRadioNavigator(itsStandSelectionRadioGroup)));

   Show();
   itsNavigator->InitFocus();
}






void SelectSimStandWin::DoCommand(long theCommand,void* theData)
{
   xvt_scr_set_busy_cursor();

   switch (theCommand)
   {
      case 1:                      // by location button
      {
         itsGroupList->clear();
         itsGroupListBox->IListBox(*itsGroupList);

         itsAvailableNText->SetTitle("Available Stands");
         itsStandList->clear();
         itsStandListBox->IListBox(*itsStandList);

         SpFillLocationList();
         SpResetAddStandButton();

         break;
      }
      case 2:                      // by group button
      {
         itsLocList->clear();
         itsLocationListBox->IListBox(*itsLocList);

         itsAvailableNText->SetTitle("Available Stands");
         itsStandList->clear();
         itsStandListBox->IListBox(*itsStandList);

         SpFillGroupList();
         SpResetAddStandButton();

         break;
      }
      case 3:                      // Location List Box
      {
         if (itsSelectionRadioGroup->GetSelectedButton()==
             itsSelectionByLocation)
         {
            itsAvailableNText->SetTitle("Available Stands");
            itsStandList->clear();
            itsStandListBox->IListBox(*itsStandList);
            SpFillGroupList();
            if (itsLocationData->itsExGroups->entries() == 0)
               SpFillStandList();
         }
         else SpFillStandList();
         SpResetAddStandButton();
         break;
      }

      case 4:                      // Group List Box
      {
         if (itsSelectionRadioGroup->GetSelectedButton()==
             itsSelectionByLocation)
            SpFillStandList();
         else
         {

            itsAvailableNText->SetTitle("Available Stands");
            itsStandList->clear();
            itsStandListBox->IListBox(*itsStandList);
            SpFillLocationList();
         }
         SpResetAddStandButton();
         break;
      }
      case 5:                     // Stand List Box
      {
         // set the text line equal to the last selected item.
         int last=-1;
         for (int i=0; i<itsStandListBox->GetNumItems(); i++)
            if (itsStandListBox->IsItSelected(i)) last=i;
         if (last > -1)
            itsStandIDText->SetText(itsStandListBox->GetItem(last));
         else
            itsStandIDText->Clear();
         SpResetAddStandButton();
         break;
      }

      case 7:                      // close button
      {
         Close();
         break;
      }
      case 8:                      // Delete Stand button
      {
         CWindow::DoCommand(M_SIM_PREP_DELETESTAND,
                            theSpGlobals->runStream->allStands->last());
         break;
      }
      case 9:                      // Bare Ground
      {
         BareGround();
         break;
      }
      case 10:                     // accept selections or double click on
                                   // one stand.
      {
         SpAcceptStandList();
         SpResetAddStandButton();
         break;
      }
      case 11:                     // In any selected group radio button
      case 12:                     // In every selected group radio button
      {
         SpFillStandList();
         break;
      }
      case 13:                     // All Stands button selected
      {
         if(itsStandList->entries())
         {
            itsStandListBox->SelectAll();
            SpResetAddStandButton();
         }
         else
         {
            if (itsSelectionRadioGroup->GetSelectedButton()==
                itsSelectionByLocation &&
                itsLocationListBox->GetNumSelectedItems())
            {
               SpFillGroupList();
               itsGroupListBox->SelectAll();
            }
            else if (itsSelectionRadioGroup->GetSelectedButton()==
                     itsSelectionByGroup &&
                     itsGroupListBox->GetNumSelectedItems())
            {
               SpFillLocationList();
               itsLocationListBox->SelectAll();
            }
            else
            {
               itsStandSelectionRadioGroup->SetSelectedButton(itsSelInAny);
               if (itsSelectionRadioGroup->GetSelectedButton()==
                   itsSelectionByLocation)
               {
                  itsLocationListBox->SelectAll();
                  SpFillGroupList();
                  itsGroupListBox->SelectAll();
               }
               else
               {
                  itsGroupListBox->SelectAll();
                  SpFillLocationList();
                  itsLocationListBox->SelectAll();
               }
            }
            SpFillStandList();
         }
         break;
      }
      case 14:                     // NLineText do command.
      {
         // If using a pointer to data, object must remain in scope
         CStringRW aStandIDText(itsStandIDText->GetText());
         const char * pattern = aStandIDText.data();
         if (!(*pattern)) break;
         RWCRegexp * regExp  = NULL;

         if ((int) *pattern == '=')
         {
            if (*(++pattern))
            {
               regExp = new RWCRegexp(pattern);
               if (regExp->status() != RWCRegexp::OK)
               {
                  xvt_dm_post_error("Regular expression %s\nis wrong.",
                                    pattern);
                  delete regExp;
                  break;
               }
               else pattern = NULL;
            }
            else
            {
               xvt_dm_post_error("Regular expression is empty");
               break;
            }
         }

         if(!itsStandList->entries())
         {
            itsStandSelectionRadioGroup->SetSelectedButton(itsSelInAny);
            if (itsSelectionRadioGroup->GetSelectedButton()==
                itsSelectionByLocation)
            {
               itsLocationListBox->SelectAll();
               SpFillGroupList();
               itsGroupListBox->SelectAll();
            }
            else
            {
               itsGroupListBox->SelectAll();
               SpFillLocationList();
               itsLocationListBox->SelectAll();
            }
            SpFillStandList();
         }
         else itsStandListBox->DeselectAll();

         int loc = FindInRWOrderedSetOfCStringRWC(itsStandList,
                                                  pattern, regExp);
         if (loc >= 0)
         {
            itsStandListBox->SelectItem(loc);
            while ((loc = FindInRWOrderedSetOfCStringRWC(itsStandList,
                                                         pattern, regExp,
                                                         (size_t) loc)) > -1)
               itsStandListBox->SelectItem(loc++);
         }
         else xvt_dm_post_warning("Can not find \"%s\""
                                  "\nin Available Stands list.",
                                  itsStandIDText->GetText().data());
         if (regExp) delete regExp;
         SpResetAddStandButton();
         break;
      }
      case 20:                     // Inlcude/Do Not include Addfiles.
         break;

      case STANDCountChanged:
      case GROUPCountChanged:
      {
         SetCountString();
         break;
      }
      case EXDataUpdated:
      {
         FILE * locFile = fopen (itsLocFilePointer->name,"r");
         if (locFile)
         {
            xvt_scr_set_busy_cursor();
            delete itsLocationData;
            itsLocationData = new AllLocationData(locFile);
            fclose(locFile);
            if (itsSelectionRadioGroup->GetSelectedButton()==
                itsSelectionByLocation) DoCommand(1,NULL);
            else                        DoCommand(2,NULL);
         }
         else xvt_dm_post_message("Can not open file %s",
                                  itsLocFilePointer->name);
         break;
      }
      default:
      {
         CWindow::DoCommand(theCommand,theData);
         break;
      }
   }
}





short SelectSimStandWin::Close(void)
{

   delete itsLocList;
   delete itsGroupList;
   delete itsStandList;

   delete itsLocationData;

   return CWindow::Close();
}





void SelectSimStandWin::SpFillLocationList( void )
{
   itsLocList->clear();

   // if ByLocation option is being used, then fill the location list
   // with all locations.

   if (itsSelectionRadioGroup->GetSelectedButton()==itsSelectionByLocation)
   {
      for (size_t i = 0; i < itsLocationData->itsExLocations->entries(); i++)
      {
         itsLocList->insert(itsLocationData->itsExLocations->at(i));
      }
   }

   // otherwise (ByGroup option is being used), then fill the location list
   // with locations that contain one of the selected groups.

   else
   {
      for (size_t i = 0; i < itsGroupList->entries(); i++)
      {
         if (itsGroupListBox->IsItSelected(i))
         {
            ExGroup * anExGroup = (ExGroup *) itsGroupList->at(i);

            // check the location's group membership.

            if (anExGroup->itsExLocationMembership)
            {
               RWOrderedIterator exLocations(*anExGroup->
                                             itsExLocationMembership);
               ExLocation * anExLocation;
               while (anExLocation = (ExLocation *) exLocations())
               {
                  if (!itsLocList->contains(anExLocation) &&
                      anExLocation->itsExGroupMembership &&
                      anExLocation->itsExGroupMembership->contains(anExGroup))
                  {
                     itsLocList->insert(anExLocation);
                  }
               }
            }
         }
      }
   }
   SortRWOrdered(itsLocList);
   itsLocationListBox->IListBox(*itsLocList);
}





void SelectSimStandWin::SpFillGroupList( void )
{

   itsGroupList->clear();

   // if ByLocation option is being used, then fill group list for each
   // selected location.

   if (itsSelectionRadioGroup->GetSelectedButton()==itsSelectionByLocation)
   {
      for (size_t i = 0; i < itsLocList->entries(); i++)
      {
         if (itsLocationListBox->IsItSelected((int) i))
         {
            ExLocation * anExLoc = (ExLocation *) itsLocList->at(i);

            // make sure the location is "loaded"...it will not do anything
            // if the data are already loaded.

            anExLoc->LoadLocationData();

            // check the location's group membership.
            if (anExLoc->itsExGroupMembership)
            {
               RWOrderedIterator exGroupsAtLoc(*anExLoc->itsExGroupMembership);
               ExGroup * anExGroup;
               while (anExGroup = (ExGroup *) exGroupsAtLoc())
               {
                  if (strcmp(anExGroup->data(),"All") == 0) continue;
                  if (!itsGroupList->contains(anExGroup))
                       itsGroupList->insert(anExGroup);
               }
            }
         }
      }
      SortRWOrdered(itsGroupList);
   }

   // otherwise (ByGroup option is being used), then fill group list for
   // all locations.

   else
   {

      // make sure all the location data is loaded.

      itsLocationData->LoadAllLocations();

      RWBinaryTreeIterator next(*itsLocationData->itsExGroups);
      ExGroup * exGroup;
      while (exGroup = (ExGroup *) next())
      {
         if (strcmp(exGroup->data(),"All") != 0)
            itsGroupList->insert(exGroup);
      }
   }
   itsGroupListBox->IListBox(*itsGroupList);
}




void SelectSimStandWin::SpFillStandList()
{
   itsStandList->clear();

   // build a list of selected items from the group list box.

   int nSelectedGroups = itsGroupListBox->GetNumSelectedItems();

   RWOrdered selectedGroups((size_t) nSelectedGroups);

   int i;
   for (i = 0; i < itsGroupList->entries(); i++)
   {
      if (itsGroupListBox->IsItSelected(i))
      selectedGroups.insert(itsGroupList->at((size_t) i));
   }

   // loop over the list of selected locations.

   for (i = 0; i < itsLocList->entries(); i++)
   {
      if (itsLocationListBox->IsItSelected((int) i))
      {
         ExLocation * anExLoc = (ExLocation *) itsLocList->at(i);
         anExLoc->LoadLocationData();
         if (anExLoc->itsExStands)
         {
            if (itsStandSelectionRadioGroup->GetSelectedButton()==
                itsSelInAny)
            {

               // stand can be in any selected group.

               RWOrderedIterator exStandsAtLoc(*anExLoc->itsExStands);
               ExStand * anExStand;
               while (anExStand = (ExStand *) exStandsAtLoc())
               {
                  RWOrderedIterator exGroupsAtStand(*anExStand->
                                                    itsExGroupMembership);
                  ExGroup * anExGroup;
                  while (anExGroup = (ExGroup *) exGroupsAtStand())
                  {
                     if (selectedGroups.contains(anExGroup))
                     {
                        itsStandList->insert(anExStand);
                        break;
                     }
                  }
               }
            }
            else
            {

               // stand must be in every selected group.

               RWOrderedIterator exStandsAtLoc(*anExLoc->itsExStands);
               ExStand * anExStand;
               while (anExStand = (ExStand *) exStandsAtLoc())
               {
                  BOOLEAN yesInsert = TRUE;
                  RWOrderedIterator nextSelectedExGroup(selectedGroups);
                  ExGroup * aSelectedExGroup;
                  while (aSelectedExGroup = (ExGroup *) nextSelectedExGroup())
                  {
                     if (!anExStand->itsExGroupMembership->
                         contains(aSelectedExGroup))
                     {
                        yesInsert = FALSE;
                        break;
                     }
                  }
                  if (yesInsert) itsStandList->insert(anExStand);
               }
            }
         }
      }
   }
   RWSortedVector * itsStandListSorted;
   if (itsStandList->entries())
   {
      CStringRW c = ConvertToString((int) itsStandList->entries());
      c += " Stands";
      itsAvailableNText->SetTitle(c);
	  itsStandListSorted = new RWSortedVector((size_t) (int) itsStandList->entries());
		//Move each item to a sorted vector.
	  for(size_t i = 0; i < itsStandList->entries(); i++)
	  {
		  ExStand* stnd = (ExStand*) itsStandList->at(i);
		  itsStandListSorted->insert(stnd);
	  }
		//Remove all the items and re-add them, in sorted order
	  itsStandList->clear();
	  ExStand* item;
	  RWSortedVectorIterator next(*itsStandListSorted);
	  while( item = (ExStand*)next() )
	  {
		  itsStandList->append(item);
	  }
   }
   else 
	   itsAvailableNText->SetTitle("Available Stands");

   itsStandListBox->IListBox(*itsStandList);

}





void SelectSimStandWin::SpAcceptStandList()
{

   if (itsStandListBox->GetNumSelectedItems() == 0 &&
       itsStandIDText->GetText().length()      > 0)
   {
      DoCommand(14,itsStandIDText);
      SpAcceptStandList();
      return;
   }

   Group * allGrpPtr = NULL;

   // go through defined groups to see if "All" exists.

   RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
   Group * group;
   while ((group = (Group *) nextGroup()))
   {
      if (strcmp(group->data(),"All") == 0)
      {
         allGrpPtr = group;
         break;
      }
   }

   // if it does not then create it.

   if (allGrpPtr == NULL)
   {
      allGrpPtr = new Group ("All");
      theSpGlobals->runStream->AddGroup(allGrpPtr);
      SendUpdateMessage (GROUPCountChanged);
   }

   CStringRW warnMsg;
   int       desiredStandsRequiringBlockedVariants = 0;
   char * standBuffer = new char[2001];

   xvt_fsys_save_dir();
   FILE_SPEC * fileSpec = new FILE_SPEC;
   xvt_fsys_get_dir(&fileSpec->dir);
   fileSpec->type[0]=NULL;

   RWOrdered componentsFromFile;

   int includeAddFiles = (itsAddFileRadioGroup->GetSelectedButton()
                          == itsAddFileYes) ? 1 : 0;

   xvt_scr_set_busy_cursor();

   // count the number of stands added.

   int standsAdded = 0;

   // iterate over the stand list.

   for (size_t i = 0; i < itsStandList->entries(); i++)
   {
      if (itsStandListBox->IsItSelected((int) i))
      {
         ExStand * anExStand = (ExStand *) itsStandList->at(i);

         // pass the stands variant list to the variant selection class
         // if the return code is 0, we can't use the stand.  Otherwise
         // the stand has acted to limit the variants to those possible.

         if (theSpGlobals->theVariants->
             NewStandVariantsInteraction(anExStand->itsVariants))
         {
            // the stand can be used...create a new stand.

            Stand * aStand = new Stand(anExStand->data());

            // save the variant list.
            aStand->variants = new CStringRW (anExStand->itsVariants);

            // build the stand keywords
            anExStand->BuildAnExStand(standBuffer,2000);

            // define originID string for the stand.  This will be
            // in the first part of the buffer.

            char * ptr = strtok(standBuffer,"\n");
            aStand->originID = new CStringRW(ptr);
            ptr = strtok(NULL,"~");

            // define the site keywords for the stand.
            if (ptr) aStand->siteKeywords = new CStringRW(ptr);

            // define the tree keywords for the stand.
            ptr = strtok(NULL,"");
            if (ptr) aStand->treeKeywords = new CStringRW(ptr);

            // add the stand to the "All" group
            allGrpPtr->AddStandToGroup(aStand);

            // if the automatically added components are not added, then add them

            if (!allGrpPtr->autoAddsDone)
            {
               allGrpPtr->autoAddsDone = 1;
               RWBinaryTreeIterator next(*itsLocationData->itsExGroups);
               ExGroup * anExGroup;
               while (anExGroup = (ExGroup *) next())
               {
                  if (strcmp(anExGroup->data(),"All") == 0)
                  {
                     if (includeAddFiles && anExGroup->itsAddFiles)
                     {
                        RWOrderedIterator addFileForGroup(*anExGroup->itsAddFiles);
                        CStringRWC * anAddFile;
                        while (anAddFile = (CStringRWC *) addFileForGroup())
                        {
                           strcpy(fileSpec->name,anAddFile->data());

                           FILE* filePtr= fopen(fileSpec->name,"r");
                           if (filePtr)
                           {
                              ReadComponentsFromFile(filePtr, fileSpec,
                                                     &componentsFromFile);

                              theSpGlobals->runStream->
                                    ProcessPPEComponents(&componentsFromFile);

                              fclose (filePtr);
                              if (componentsFromFile.entries())
                              {
                                 Component * placeArg = NULL;
                                 for (size_t i=0; i<componentsFromFile.entries(); i++)
                                 {
                                    theSpGlobals->runStream->allComponents->
                                       insert((RWCollectable *) componentsFromFile.at(i));
                                    allGrpPtr->AddComponentToGroup((Component *)
                                                                   componentsFromFile.at(i),
                                                                   placeArg,1);
                                    // once the first component is inserted, the others
                                    // must follow.
                                    placeArg = (Component *) componentsFromFile.at(i);
                                 }
                                 componentsFromFile.clear();
                              }
                           }
                           else
                           {
                              if (warnMsg.length() > 0) warnMsg += "\n-------------\n";
                              warnMsg += "Add file could not be opened.\nStand = ";
                              warnMsg += anExStand->data();
                              warnMsg += "\nFile = ";
                              warnMsg += fileSpec->name;
                              if (warnMsg.length() > 500)
                              {
                                 warnMsg += "\n\n*** Too many errors ***";
                                 goto ENDProcessing;
                              }
                           }
                        }
                     }
                     if (includeAddFiles && anExGroup->itsAddKeys)
                     {
                        RWOrderedIterator addKeyForGroup(*anExGroup->itsAddKeys);
                        CStringRWC * anAddKey;
                        Component * placeArg = NULL;
                        while (anAddKey = (CStringRWC *) addKeyForGroup())
                        {
                           CStringRW nkey;
                           CStringRW okey = *anAddKey;
                           for (int i=0; i<anAddKey->length(); i++)
                           {
                              char c = okey(i);
                              if (c && c != 13) nkey+=c;
                           }
                           Component * aComp = CreateComponent("From Database","FreeForm",
                              nkey,NULL,NULL,NULL,-1,NULL);
                           theSpGlobals->runStream->allComponents->insert((RWCollectable *) aComp);
                           allGrpPtr->AddComponentToGroup(aComp, placeArg,1);
                        }
                     }
                  }
               }
            }

            // count the stands added.
            standsAdded++;

            // AddFiles...those part of slf.

            if (includeAddFiles && anExStand->itsAddFiles)
            {
               RWOrderedIterator addFileForStand(*anExStand->itsAddFiles);
               CStringRWC * anAddFile;
               while (anAddFile = (CStringRWC *) addFileForStand())
               {
                  strcpy(fileSpec->name,anAddFile->data());

                  FILE* filePtr= fopen(fileSpec->name,"r");
                  if (filePtr)
                  {
                     ReadComponentsFromFile(filePtr, fileSpec,
                                            &componentsFromFile);
                     fclose (filePtr);

                     theSpGlobals->runStream->
                           ProcessPPEComponents(&componentsFromFile);

                     if (componentsFromFile.entries())
                     {
                        RWCollectable * placeArg = NULL;
                        for (size_t i=0; i<componentsFromFile.entries(); i++)
                        {
                           theSpGlobals->runStream->allComponents->
                                         insert((RWCollectable *) componentsFromFile.at(i));
                           aStand->AddComponentOrGroupToStand(componentsFromFile.at(i),
                                                              placeArg,1);
                           // once the first component is inserted, the others
                           // must follow.
                           placeArg = componentsFromFile.at(i);
                        }
                        componentsFromFile.clear();
                     }
                  }
                  else
                  {
                     if (warnMsg.length() > 0) warnMsg += "\n-------------\n";
                     warnMsg += "Add file could not be opened.\nStand = ";
                     warnMsg += anExStand->data();
                     warnMsg += "\nFile = ";
                     warnMsg += fileSpec->name;
                     if (warnMsg.length() > 500)
                     {
                        warnMsg += "\n\n*** Too many errors ***";
                        goto ENDProcessing;
                     }
                  }
               }
            }
            if (includeAddFiles && anExStand->itsAddKeys)
            {
               RWOrderedIterator addKeyForStand(*anExStand->itsAddKeys);
               CStringRWC * anAddKey;
               RWCollectable * placeArg = NULL;
               while (anAddKey = (CStringRWC *) addKeyForStand())
               {
                  CStringRW nkey;
                  CStringRW okey = *anAddKey;
                  for (int i=0; i<anAddKey->length(); i++)
                  {
                     char c = okey(i);
                     if (c && c != 13) nkey+=c;
                  }
                  Component * aComp = CreateComponent("From Database","FreeForm",
                     nkey,NULL,NULL,NULL,-1,NULL);
                  theSpGlobals->runStream->allComponents->insert((RWCollectable *)aComp);
                  aStand->AddComponentOrGroupToStand((RWCollectable *)aComp, placeArg, 1);
               }
            }

            // The "ExGroup" membership of the stand is used to create new groups
            // and/or create membership in existing groups.

            // Iterate over the ExGroupMembership for the stand.

            RWOrderedIterator exGroupsAtStand(*anExStand->itsExGroupMembership);
            ExGroup * anExGroup;
            while (anExGroup = (ExGroup *) exGroupsAtStand())
            {
               // Iterate over the defined groups.

               RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
               Group * group;
               Group * theGroupPtr = NULL;
               while (group = (Group *) nextGroup())
               {
                  if (strcmp(group->data(),anExGroup->data()) == 0)
                  // it's there...so set a reference to it.
                  {
                     theGroupPtr = group;
                     break;
                  }
               }

               // it's not found in the defined groups list...

               if (!theGroupPtr)
               {

                  // if anExGroup is one of the selected groups, then add it...
                  // otherwise skip processing this group.

                  BOOLEAN groupIsSelected = FALSE;
                  for (size_t i = 0; i<itsGroupList->entries(); i++)
                  {
                     if (itsGroupListBox->IsItSelected((int) i) &&
                         anExGroup == itsGroupList->at(i))
                     {
                        groupIsSelected = TRUE;
                        break;
                     }
                  }

                  // ***  the NEXT line causes all exgroup add files to be loaded

                  groupIsSelected = TRUE;

                  if (groupIsSelected)
                  {
                     theGroupPtr = new Group (anExGroup->data());
                     theSpGlobals->runStream->AddGroup(theGroupPtr);
                     SendUpdateMessage (GROUPCountChanged);

                     // check the group code files.  If they are defined, then
                     // add them to the group...not to the stand.

                     if (includeAddFiles && anExGroup->itsAddFiles)
                     {
                        RWOrderedIterator addFileForGroup(*anExGroup->itsAddFiles);
                        CStringRWC * anAddFile;
                        while (anAddFile = (CStringRWC *) addFileForGroup())
                        {
                           strcpy(fileSpec->name,anAddFile->data());

                           FILE* filePtr= fopen(fileSpec->name,"r");
                           if (filePtr)
                           {
                              ReadComponentsFromFile(filePtr, fileSpec,
                                                     &componentsFromFile);
                              fclose (filePtr);

                              theSpGlobals->runStream->
                                    ProcessPPEComponents(&componentsFromFile);

                              if (componentsFromFile.entries())
                              {
                                 Component * placeArg = NULL;
                                 for (size_t i=0; i<componentsFromFile.entries(); i++)
                                 {
                                    theSpGlobals->runStream->allComponents->
                                                  insert((RWCollectable *) componentsFromFile.at(i));
                                    theGroupPtr->AddComponentToGroup((Component *)
                                                                     componentsFromFile.at(i),
                                                                     placeArg,1);
                                    // once the first component is inserted, the others
                                    // must follow.
                                    placeArg = (Component *) componentsFromFile.at(i);
                                 }
                                 componentsFromFile.clear();
                              }
                           }
                           else
                           {
                              if (warnMsg.length() > 0) warnMsg += "\n-------------\n";
                              warnMsg += "Add file could not be opened.\nStand = ";
                              warnMsg += anExStand->data();
                              warnMsg += "\nFile = ";
                              warnMsg += fileSpec->name;
                              if (warnMsg.length() > 500)
                              {
                                 warnMsg += "\n\n*** Too many errors ***";
                                 goto ENDProcessing;
                              }
                           }
                        }
                        theGroupPtr->autoAddsDone = 1;
                     }
                     if (includeAddFiles && anExGroup->itsAddKeys)
                     {
                        RWOrderedIterator addKeyForGroup(*anExGroup->itsAddKeys);
                        CStringRWC * anAddKey;
                        Component * placeArg = NULL;
                        while (anAddKey = (CStringRWC *) addKeyForGroup())
                        {
                           CStringRW nkey;
                           CStringRW okey = *anAddKey;
                           for (int i=0; i<anAddKey->length(); i++)
                           {
                              char c = okey(i);
                              if (c && c != 13) nkey+=c;
                           }
                           Component * aComp = CreateComponent("From Database","FreeForm",
                              nkey,NULL,NULL,NULL,-1,NULL);
                           theSpGlobals->runStream->allComponents->insert((RWCollectable *) aComp);
                           theGroupPtr->AddComponentToGroup(aComp, placeArg, 1);
                        }
                        theGroupPtr->autoAddsDone = 1;
                     }
                  }
               }

               // now add the stand to the existing or newly created group.

               if (theGroupPtr) theGroupPtr->AddStandToGroup(aStand);
            }
         }
         else
         {
            // the stand can not be used because it requires an unavailable
            // variant.

            desiredStandsRequiringBlockedVariants++;
         }
      }
   }

   ENDProcessing:

   itsStandListBox->DeselectAll();

   delete fileSpec;
   xvt_fsys_restore_dir();

   if (desiredStandsRequiringBlockedVariants)
   {
      if (warnMsg.length() > 0) warnMsg += "\n-------------\n";
      warnMsg += ConvertToString(desiredStandsRequiringBlockedVariants);
      warnMsg += " stand(s) were not processed because\n"
                 "the variants they require are either not\n"
                 "available or blocked from use due to\n"
                 "prior selections.";
   }

   // Write necessary error message.

   if (warnMsg.length() > 0) Warn (warnMsg);

   delete []standBuffer;

   if (standsAdded)
   {
      // signal that something should be saved to the simulation file.

      itsDocument->SetSave(TRUE);

      // Notify windows to update that stand lists.

      SendUpdateMessage (STANDCountChanged);

   }
}





void SelectSimStandWin::SetCountString( void )
{
   int totalStands;
   int totalGroups;

   if (theSpGlobals->runStream->allStands)
        totalStands = theSpGlobals->runStream->allStands->entries();
   else totalStands = 0;

   if (theSpGlobals->runStream->allGroups)
        totalGroups = theSpGlobals->runStream->allGroups->entries();
   else totalGroups = 0;


   *itsCountTextString =  "Contents: ";
   *itsCountTextString += ConvertToString(totalStands);
   *itsCountTextString += " Stands ";
   *itsCountTextString += ConvertToString(totalGroups);
   *itsCountTextString += " Groups";

   itsCountNText->SetTitle(*itsCountTextString);
}



void SelectSimStandWin::SpResetAddStandButton( void )
{
   int numSelectedStands = itsStandListBox->GetNumSelectedItems();

   switch (numSelectedStands)
   {
      case 0:
      {
         itsAddStandsButton->SetTitle("Add Stand(s)");
         itsAddStandsButton->Disable();
         break;
      }
      case 1:
      {
         itsAddStandsButton->SetTitle("Add 1 Stand");
         itsAddStandsButton->Enable();
         break;
      }
      default:
      {
         CStringRW newTitle = "Add ";
         newTitle += ConvertToString(numSelectedStands);
         newTitle += " Stands";
         itsAddStandsButton->SetTitle(newTitle);
         itsAddStandsButton->Enable();
         itsAddStandsButton->Activate();
         break;
      }
   }
}


void SelectSimStandWin::SortRWOrdered(RWOrdered * list)
{
   if (list->entries() > 1)
   {
      RWBinaryTree sort;
      RWOrderedIterator next1(*list);
      RWCollectable * item;
      while (item = (RWCollectable *) next1()) sort.insert(item);
      list->clear();
      RWBinaryTreeIterator next2(sort);
      while (item = (RWCollectable *) next2()) list->insert(item);
   }
}




int SelectSimStandWin::FindInRWOrderedSetOfCStringRWC(RWOrdered * list,
                                                      const char * c,
                                                      const RWCRegexp * re,
                                                      size_t start)
{
   if (list->entries() > start && (c || re))
   {
      for (size_t i = start; i < list->entries(); i++)
      {
         CStringRWC * theString = (CStringRWC *) list->at(i);
         if (re)
         {
            if ((*theString)(*re) != NULLString) return i;
         }
         else
         {
            if (strcmp(theString->data(),c) == 0) return i;
         }
      }
   }
   return -1;
}


void SelectSimStandWin::BareGround()
{
   xvt_scr_set_busy_cursor();

   const char * bareGrndSlf = "baregrnd.slf";
   const char * bareGrnd    = "BareGrnd";

   // create the slf file unless it already exists.

   int bareGrndSlfChanged = 0;
   FILE * slfFile = fopen (bareGrndSlf,"r");
   if (slfFile)
   {
      fclose (slfFile);
      slfFile = NULL;
      ASK_RESPONSE aResponse =
         xvt_dm_post_ask("Append","No","Cancel",
                         "%s exists. Append another stand?",
                         bareGrndSlf);
      switch (aResponse)
      {
         case RESP_DEFAULT:
         {
            bareGrndSlfChanged = 1;
            slfFile = fopen (bareGrndSlf,"a");
            break;
         }
         case RESP_2:
         {
            break;
         }
         case RESP_3:
         {
            return;
         }
      }
   }
   else
   {
      bareGrndSlfChanged = 1;
      slfFile = fopen (bareGrndSlf,"w");
   }

   if (slfFile)
   {
      fprintf (slfFile,
               "A %s @ 0 %s @\nB %s @\nC %s Variant=%s @\n",
               bareGrnd,
               theSpGlobals->theVariants->GetSelectedVariant(),
               bareGrnd, bareGrnd,
               theSpGlobals->theVariants->GetSelectedVariant());
      fclose(slfFile);
   }
   else if (bareGrndSlfChanged)
   {
      bareGrndSlfChanged = 0;
      xvt_dm_post_warning ("Can not open file %s for output.",
                           bareGrndSlf);
   }

   // try to find the baregrnd the current list of locations.
   // if it is found, and if it was changed, then refresh
   // the location and group lists.

   int bareGrndSlfFound = 0;
   if (itsLocationData->itsExLocations->entries() > 0)
   {
      RWOrderedIterator next(*itsLocationData->itsExLocations);
      ExLocation * exLocation;
      while (exLocation = (ExLocation *) next())
      {
         if (exLocation->itsSLFileName &&
             strstr(exLocation->itsSLFileName,bareGrndSlf))
         {
            if (bareGrndSlfChanged)
            {
               exLocation->FreeLocationData();
               SendUpdateMessage (EXDataUpdated,(void *) bareGrndSlf,this);
               if (itsSelectionRadioGroup->GetSelectedButton()==
                   itsSelectionByLocation) DoCommand(1,NULL);
               else                        DoCommand(2,NULL);
            }
            bareGrndSlfFound = 1;
            break;
         }
      }
   }

   // if it was not found append the Bare Ground location
   // to the current locFile, recreate the alllocation data, and
   // refresh the window.

   if (!bareGrndSlfFound)
   {
      if (xvt_dm_post_ask("Append","Cancel",NULL,
                          "%s not in location file %s",
                          bareGrndSlf,
                          itsLocFilePointer->name) == RESP_DEFAULT)
      {
         FILE * locFile = fopen (itsLocFilePointer->name,"a");
         if (locFile)
         {
            fprintf (locFile,"\nA \"Default Bare Ground\" "
                     "@ baregrnd.slf @ @\n");
            fclose(locFile);
            SendUpdateMessage (EXDataUpdated,itsLocFilePointer->name);
         }
         else xvt_dm_post_warning ("Can not open file %s for output.",
                                   itsLocFilePointer->name);
      }
   }
}


