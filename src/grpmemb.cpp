/********************************************************************

   File Name:     grpmemb.cpp
   Author:        nlc
   Date:          04/96

   Contents:     (see .hpp for description)

********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "grpmemb.hpp"
#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

#include CDesktop_i
#include CDocument_i
#include NButton_i
#include NCheckBox_i
#include NScrollBar_i
#include NText_i
#include CText_i
#include CApplication_i
#include NListButton_i
#include NEditControl_i

#include CStringCollection_i
#include CRadioGroup_i
#include CNavigator_i

#include <rw/ordcltn.h>
#include <rw/vstream.h>

#include "mylinetx.hpp"

#include "runstr.hpp"
#include "compon.hpp"
#include "group.hpp"
#include "stand.hpp"
#include "cwwindow.hpp"
#include "mainwin.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "sendupdt.hpp"

extern XVT_HELP_INFO helpInfo;    // help file

GroupMembershipWindow::GroupMembershipWindow(CDocument *theDocument)
                      :CWwindow(theDocument,
                                RightWinPlacement(CRect(20,20,765,440)),
                                "Change Group Membership",
                                WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                                W_DOC, TASK_MENUBAR)
{

   // get and clear a navigator.

   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   CRect tmpRect;

   UNITS start = 8;
   itsCloseButton        = new NButton(this, CRect(start,384,start+138,416),
                                       "Close");
   start += 146;
   itsSetMembershipButton = new NButton(this, CRect(start,384,start+138,416),
                                       "Set Membership");
   start += 146;
   itsSetMgmtIDsButton   = new NButton(this, CRect(start,384,start+138,416),
                                       "Set MgmtIds");

   itsCloseButton        ->SetCommand( 1);
   itsSetMembershipButton->SetCommand( 2);
   itsSetMgmtIDsButton   ->SetCommand( 3);

   itsCloseButton        ->SetGlue(BOTTOMSTICKY);
   itsSetMembershipButton->SetGlue(BOTTOMSTICKY);
   itsSetMgmtIDsButton   ->SetGlue(BOTTOMSTICKY);

   numberOfRows      = 0;
   numberOfColumns   = 0;
   heightOfNLineText = 0;

   tmpRect = GetFrame();
   tmpRect.Left  (tmpRect.Right()-NScrollBar::NativeWidth()-2);
   tmpRect.Right (tmpRect.Right()-2);
   tmpRect.Top   ((UNITS) 64);
   tmpRect.Bottom((UNITS) 376-NScrollBar::NativeWidth());

   itsVerticalScrollBar   = new NScrollBar(this,
                                           tmpRect,
                                           VERTICAL);

   itsVerticalScrollBar->IScrollBar( 0,   10,
                                     0,    1,
                                     5, TRUE,
                                  TRUE, BOTTOMSTICKY | RIGHTSTICKY |
                                        TOPSTICKY);

   tmpRect.Right (tmpRect.Left()-2);
   tmpRect.Left  ((UNITS) 250);
   tmpRect.Top   ((UNITS) 378-NScrollBar::NativeWidth());
   tmpRect.Bottom((UNITS) 378);

   itsHorizontalScrollBar = new NScrollBar(this,
                                           tmpRect,
                                           HORIZONTAL);

   itsHorizontalScrollBar->IScrollBar( 0,    5,
                                       0,    1,
                                       5, TRUE,
                                    TRUE, BOTTOMSTICKY | RIGHTSTICKY
                                                       | LEFTSTICKY);

   new NText(this, CRect(40,24,120,60), "Simulation\nStands", 0L);
   new NText(this, CRect(198,24,240,60), "Mgmt\nIds", 0L);
   new NText(this, CRect(250, 4,408,28), "Groups:", 0L);


   itsSetGroupMembershipWindow = NULL;
   itsSetMgmtIDWindow          = NULL;

#if XVTWS==WIN32WS

   // On Motif, SizeWindow is called automatically... and BuildElements is
   // called from SizeWindows.  Here BuildElements is called for Windows.

   tmpRect = GetFrame();

   BuildElements (int(tmpRect.Right()-tmpRect.Left()),
                  int(tmpRect.Bottom()-tmpRect.Top()));

#endif

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), GRPMEMBWindow, 0L);

   itsNavigator->AppendTabStop(new CTabStop(itsCloseButton));
   itsNavigator->AppendTabStop(new CTabStop(itsSetMembershipButton));
   itsNavigator->AppendTabStop(new CTabStop(itsSetMgmtIDsButton));
//   itsNavigator->AppendTabStop(new CTabStop(itsMapsButton));
   itsNavigator->AppendTabStop(new CTabStop(itsVerticalScrollBar));
   itsNavigator->AppendTabStop(new CTabStop(itsHorizontalScrollBar));
   itsNavigator->InitFocus();
}




void GroupMembershipWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:                // Close
      {
         Close();
         break;
      }

      case 2:                // Set Membership
      {
         itsSetGroupMembershipWindow = new SetGroupMembershipWindow(this);
         itsSetGroupMembershipWindow->DoModal();
         itsSetGroupMembershipWindow = NULL;
         break;
      }
      case 3:                // Set MgmtIDs
      {
         itsSetMgmtIDWindow = new SetMgmtIDWindow(this);
         itsSetMgmtIDWindow->DoModal();
         itsSetMgmtIDWindow = NULL;
         break;
      }

      case STANDCountChanged:
      case STANDNameChanged:
      case GROUPCountChanged:
      case GROUPNameChanged:
      {
         ResetWindow();
         break;
      }
      default:
      {
         CWindow::DoCommand(theCommand,theData);
         break;
      }
   }
}



void GroupMembershipWindow::SizeWindow(int theWidth, int theHeigth)
{
   xvt_scr_set_busy_cursor();

   if (IsVisible()) Hide();

   CWwindow::SizeWindow(theWidth, theHeigth);

   if (theWidth < 500 || theHeigth < 200)
   {
      if (theWidth  < 500) theWidth  = 500;
      if (theHeigth < 200) theHeigth = 200;

      // this call will cause a recursive call back to this routine.

      Size (CRect(80,100,(UNITS) 80+theWidth,(UNITS) 100+theHeigth));
   }

   BuildElements(theWidth, theHeigth);
}




void GroupMembershipWindow::BuildElements(int theWidth, int theHeigth)
{
   numberOfColumns = 0;

   for (int left = 250; (left +  100) < theWidth; left += 104)
   {
      numberOfColumns++;
      if (itsListOfGroupSubviews.entries() < numberOfColumns)
      {
         GroupMembershipSubview * gView =
                   new GroupMembershipSubview(this,
                                              CRect((UNITS) left,
                                                    30,(UNITS) left+100,60));
         itsListOfGroupSubviews.insert(gView);
         if (!heightOfNLineText)
              heightOfNLineText = int(gView->itsGroupID->GetFrame().Bottom() -
                                      gView->itsGroupID->GetFrame().Top());
      }
      else
      {
         ((CSubview *) itsListOfGroupSubviews.at(numberOfColumns-1))->DoShow();
      }
   }

   if (itsListOfGroupSubviews.entries() > numberOfColumns)
   {
      for (size_t i = numberOfColumns;
           i < itsListOfGroupSubviews.entries();
           i++)
      {
         ((CSubview *) itsListOfGroupSubviews.at(i))->DoHide();
      }
   }

   numberOfRows = 0;

   for (int top = 64;
        (top +  heightOfNLineText+2) < itsHorizontalScrollBar->
                                       GetFrame().Top();
         top += heightOfNLineText+2)
   {
      numberOfRows++;
      if (itsListOfStandSubviews.entries() < numberOfRows)
      {
         StandMgmtSubview * sView =
            new StandMgmtSubview(this,
                                 CRect(0,(UNITS) top, 242,
                                       (UNITS) top+heightOfNLineText));
         itsListOfStandSubviews.insert(sView);
      }
      else
      {
         ((CSubview *) itsListOfStandSubviews.at(numberOfRows-1))->DoShow();
      }
   }
   if (itsListOfStandSubviews.entries() > numberOfRows)
   {
      for (size_t i = numberOfRows;
           i < itsListOfStandSubviews.entries();
           i++)
      {
         ((CSubview *) itsListOfStandSubviews.at(i))->DoHide();
      }
   }

   // load the check boxes as members of GroupMembershipSubview

   CRect tmpRect;

   if (numberOfRows && numberOfColumns)
   {
      for (size_t icol = 0; icol < numberOfColumns; icol++)
      {
         GroupMembershipSubview * gView = (GroupMembershipSubview *)
                                     itsListOfGroupSubviews.at(icol);

         // resize the gView so that all the check boxes will fit.

         tmpRect = gView->GetFrame();
         tmpRect.Bottom(itsHorizontalScrollBar->GetFrame().Top()-2);
         gView->DoSize(tmpRect);
         if (gView->itsListOfCheckBoxes.entries() <= numberOfRows)
         {
            tmpRect = gView->GetFrame();
            tmpRect.Left((tmpRect.Right()-tmpRect.Left()-heightOfNLineText)/2);
            tmpRect.Right(tmpRect.Left()+heightOfNLineText);
            tmpRect.Bottom((UNITS) 32);
         }

         for (size_t irow = 0; irow < numberOfRows; irow++)
         {
            tmpRect.Top(tmpRect.Bottom()+2);
            tmpRect.Bottom(tmpRect.Top()+heightOfNLineText);
            if (gView->itsListOfCheckBoxes.entries() < irow+1)
            {
               CSubview * sView = (CSubview *)
                                  itsListOfStandSubviews.at(irow);
               NCheckBox * box = new NCheckBox(gView, tmpRect);
               box->SetSelectCommand  ( 10+irow);
               box->SetDeselectCommand(100+irow);
               gView->itsListOfCheckBoxes.insert(box);
            }
            else
            {
               ((CSubview *) gView->itsListOfCheckBoxes.at(irow))->DoShow();
            }
         }

         if (itsListOfStandSubviews.entries() > numberOfRows)
         {
            for (size_t irow = numberOfRows;
                 irow < gView->itsListOfCheckBoxes.entries();
                 irow++)
            {
               ((CSubview *) gView->itsListOfCheckBoxes.at(irow))->DoHide();
            }
         }
      }
   }

   ResetWindow();
}





void GroupMembershipWindow::ResetWindow( void )
{
   xvt_scr_set_busy_cursor();

   UNITS maxRange = ((UNITS) theSpGlobals->runStream->allStands->
                    entries())-((UNITS) numberOfRows);
   if (maxRange < 1) maxRange = 1;

   UNITS increment = UNITS(float(maxRange)*.1);
   if (increment < 1) increment = 1;
   if (increment > numberOfRows) increment = (UNITS) numberOfRows;

   UNITS current = itsVerticalScrollBar->GetPosition();
   if (current > maxRange) current = maxRange;
   itsVerticalScrollBar->SetRange(0, maxRange, current);
   itsVerticalScrollBar->SetIncrements(1, increment);

   maxRange = ((UNITS) theSpGlobals->runStream->allGroups->
              entries())-((UNITS) numberOfColumns)+1;
   if (maxRange < 1) maxRange = 1;

   increment = UNITS(float(maxRange)*.1);
   if (increment < 1) increment = 1;
   if (increment > numberOfColumns) increment = (UNITS) numberOfColumns;

   current = itsHorizontalScrollBar->GetPosition();
   if (current > maxRange) current = maxRange;
   itsHorizontalScrollBar->SetRange(0, maxRange, current);
   itsHorizontalScrollBar->SetIncrements(1, increment);

   LoadStandsAndGroups();

   if (!IsVisible()) Show();

}




void GroupMembershipWindow::HScroll(SCROLL_CONTROL theEventType,
                                    UNITS thePosition)
{
   if (theEventType == SC_THUMBTRACK)
   {
      if (thePosition < theSpGlobals->runStream->allGroups->entries())
      {
         ((GroupMembershipSubview *) itsListOfGroupSubviews.at(0))->
         itsGroupID->SetText(*((Group *) theSpGlobals->runStream->
                               allGroups->at(size_t(thePosition))));
      }
      else
      {
         ((GroupMembershipSubview *) itsListOfGroupSubviews.at(0))->
         itsGroupID->Clear();
      }
   }
   else
   {
      LoadStandsAndGroups();
   }
}




void GroupMembershipWindow::VScroll(SCROLL_CONTROL theEventType,
                                    UNITS thePosition)
{
   if (theEventType == SC_THUMBTRACK)
   {
      if (thePosition < theSpGlobals->runStream->allStands->entries())
      {
         ((StandMgmtSubview *) itsListOfStandSubviews.at(0))->
         IStandMgmtSubview((Stand *) theSpGlobals->runStream->
                           allStands->at(size_t(thePosition)),
                           (int) thePosition+1);
      }
      else
      {
         ((StandMgmtSubview *) itsListOfStandSubviews.at(0))->
         IStandMgmtSubview(NULL, (int) thePosition+1);
      }
   }
   else
   {
      LoadStandsAndGroups();
   }
}


void GroupMembershipWindow::LoadStandsAndGroups( void )
{
   if (numberOfRows && numberOfColumns)
   {
      size_t i;
      size_t stand = (size_t) itsVerticalScrollBar->GetPosition();
      for (i = 0; i < numberOfRows; i++)
      {
         if (stand < theSpGlobals->runStream->allStands->entries())
         {
            ((StandMgmtSubview *) itsListOfStandSubviews.at(i))->
            IStandMgmtSubview((Stand *) theSpGlobals->runStream->
                              allStands->at(stand),
                              stand+1);
         }
         else
         {
            ((StandMgmtSubview *) itsListOfStandSubviews.at(i))->
            IStandMgmtSubview(NULL, stand+1);
         }
         stand++;
      }

      size_t group = (size_t) itsHorizontalScrollBar->GetPosition();
      for (i = 0; i < numberOfColumns; i++)
      {
         if (group < theSpGlobals->runStream->allGroups->entries())
         {
            ((GroupMembershipSubview *) itsListOfGroupSubviews.at(i))->
            IGroupMembershipSubview((Group *) theSpGlobals->runStream->
                                    allGroups->at(group));
         }
         else
         {
            ((GroupMembershipSubview *) itsListOfGroupSubviews.at(i))->
            IGroupMembershipSubview((Group *) NULL);
         }
         group++;
      }
   }
}




StandMgmtSubview::StandMgmtSubview(CSubview * theEnclosure,
                                   const CRect& theRegion)
                 :CSubview(theEnclosure, theRegion)
{
   itsSequenceNumber = new CText(this,CPoint(2,4),"0000");
   itsSequenceNumber->SetPlacement(CText::RIGHT);

   itsStandID = new MyNLineText(this, CPoint(40,0), (UNITS) 150,
                                1, 2, 26);
   itsStandID->SetFocusCommands(NULLcmd, 1);

   itsMgmtID = new MyNLineText(this, CPoint(192,0), (UNITS) 50,
                               1, 2, 4);
   itsMgmtID->SetFocusCommands(NULLcmd, 1);

   itsStand = NULL;
   itsStandHasChanged = 0;
}



void StandMgmtSubview::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:
      case 2:
      {
         if (itsStand)
         {
            if (theCommand == 1 && itsStandHasChanged)
            {
               if (itsStandID->GetText().length() == 0)
               {
                  CStringRW name = "Stand";
                  name += ConvertToString(itsStand->standNumber);
                  itsStandID->SetText(name);
               }

               *((CStringRW *) itsStand) = itsStandID->GetText().data();
               if (itsStand->mgmtID)
               {
                  *itsStand->mgmtID = itsMgmtID->GetText();
               }
               else
               {
                  if (itsMgmtID->GetText().length())
                  {
                     itsStand->mgmtID = new CStringRW(itsMgmtID->
                                                      GetText().data());
                  }
               }
               SendUpdateMessage(STANDNameChanged, NULL, GetCWindow());
               itsStandHasChanged = 0;
            }
            if (theCommand == 2)
            {
               itsStandHasChanged = 1;
            }
         }
         break;
      }
      default:
      {
         CSubview::DoCommand(theCommand,theData);
         break;
      }
   }
}




void  StandMgmtSubview::IStandMgmtSubview(Stand * theStand,
                                          int theSequenceNumber)
{
   Hide();
   itsStand = theStand;
   itsStandHasChanged = 0;
   if (itsStand)
   {
      itsStandID->SetText(*itsStand);
      if (itsStand->mgmtID)
      {
         itsMgmtID->SetText(*itsStand->mgmtID);
      }
      else
      {
         itsMgmtID->Clear();
      }
   }
   else
   {
      itsStandID->Clear();
      itsMgmtID->Clear();
   }
   itsSequenceNumber->SetText(ConvertToString(theSequenceNumber));
   itsSequenceNumber->DoDraw();
   Show();
}



GroupMembershipSubview::GroupMembershipSubview(CSubview * theEnclosure,
                                               const CRect& theRegion)
                       :CSubview(theEnclosure, theRegion)
{

   itsGroupID = new MyNLineText(this, CPoint(0,0), (UNITS) 100,
                                1, 2, 40);
   itsGroupID->SetFocusCommands(NULLcmd, 1);
   itsGroup = NULL;
   itsGroupHasChanged = 0;
}





void GroupMembershipSubview::DoCommand(long theCommand,void* theData)
{
   if (theCommand > 500)
   {
      CSubview::DoCommand(theCommand,theData);
      return;
   }

   if (itsGroup)
   {
      if (theCommand == 1 && itsGroupHasChanged)
      {
         if (itsGroupID->GetText().length() == 0)
         {
            CStringRW name = "Group";
            name += ConvertToString(itsGroup->groupNumber);
            itsGroupID->SetText(name);
         }
         *((CStringRW *) itsGroup) = itsGroupID->GetText();
         itsGroupHasChanged = 0;
         SendUpdateMessage(GROUPNameChanged, NULL, GetCWindow());
      }
      if (theCommand == 2)
      {
         itsGroupHasChanged = 1;
      }
      if (theCommand >= 10 && theCommand < 200) // Box select/deselect
      {
		 size_t irow;
         int kode = 0;
         Stand * stand;
         GroupMembershipWindow * gWin = (GroupMembershipWindow *)
                                        GetCWindow();

		 if (theCommand < 100)                 // SelectCheckBox
         {
            irow = size_t(theCommand-10);
            if (gWin->itsListOfStandSubviews.entries() > irow)
            {
               stand = ((StandMgmtSubview *) gWin->itsListOfStandSubviews.
                                             at(irow))->itsStand;
               if (stand) kode = itsGroup->AddStandToGroup(stand);
               if (kode)
               {
                  xvt_dm_post_error("Stand can not be added to "
                                    "this group.");
                  ((NCheckBox *) itsListOfCheckBoxes.at(irow))->Deselect();
               }
               else
               {
                  theSpGlobals->theAppDocument->SetSave(TRUE);
                  SendUpdateMessage(GROUPMembershipChanged, NULL,
                                    GetCWindow());
               }
            }
         }
         else
         {
                                               // DeselectCheckBox
            irow = size_t(theCommand-100);
            if (gWin->itsListOfStandSubviews.entries() > irow)
            {
               stand = ((StandMgmtSubview *) gWin->itsListOfStandSubviews.
                                             at(irow))->itsStand;
               if (stand) kode = itsGroup->DelStandFromGroup(stand);
               if (kode)
               {
                  xvt_dm_post_error("Stand can not be deleted "
                                    "from this group.");
                  ((NCheckBox *) itsListOfCheckBoxes.at(irow))->Select();
               }
               else
               {
                  theSpGlobals->theAppDocument->SetSave(TRUE);
                  SendUpdateMessage(GROUPMembershipChanged, NULL,
                                    GetCWindow());
               }
            }
         }
      }
   }
   else
   {
      if (theCommand == 1 && itsGroupID->GetText().length() > 0)
      {
         itsGroup = new Group(itsGroupID->GetText().data());
         theSpGlobals->runStream->AddGroup(itsGroup);
         GroupMembershipWindow * gWin = (GroupMembershipWindow *)
                                        GetCWindow();
         for (size_t i = 0; i < gWin->numberOfRows; i++)
            ((NCheckBox *) itsListOfCheckBoxes.at(i))->Deselect();
         SendUpdateMessage(GROUPCountChanged);
         itsGroupHasChanged = 0;
         xvt_dm_post_message("Group \"%s\" has been created.",
                             itsGroup->data());
      }
   }
}

void GroupMembershipSubview::IGroupMembershipSubview(Group * theGroup)
{
   Hide();
   itsGroup = theGroup;
   itsGroupHasChanged = 0;

   GroupMembershipWindow * gWin = (GroupMembershipWindow *)
                                   GetCWindow();
   if (itsGroup)
   {
      itsGroupID->SetText(*itsGroup);

      // select/deselect the boxes.

      for (size_t i = 0; i < gWin->numberOfRows; i++)
      {
         Stand * stand = ((StandMgmtSubview *) gWin->
                         itsListOfStandSubviews.at(i))->itsStand;
         if (stand &&
             itsGroup->standMembership &&
             itsGroup->standMembership->contains(stand))
         {
            ((NCheckBox *) itsListOfCheckBoxes.at(i))->Select();
         }
         else
         {
            ((NCheckBox *) itsListOfCheckBoxes.at(i))->Deselect();
         }
      }
   }
   else
   {
      itsGroupID->Clear();
      for (size_t i = 0; i < gWin->numberOfRows; i++)
        ((NCheckBox *) itsListOfCheckBoxes.at(i))->Deselect();
   }
   Show();
}





SetMgmtIDWindow::SetMgmtIDWindow(GroupMembershipWindow *
                                 theGroupMembershipWindow)
                :CWindow(theSpGlobals->theAppDocument,
                         CRect(100,100,330,284), "Set Management ID",
                         WSF_DEFER_MODAL, W_MODAL),
                 itsGroupMembershipWindow(theGroupMembershipWindow)
{

   itsApplyButton = new NButton(this, CRect(8,144,88,176), "Apply");
   itsApplyButton->SetCommand(1);

   itsCloseButton = new NButton(this, CRect(142,144,222,176), "Close");
   itsCloseButton->SetCommand(2);

   new NText(this, CRect(8,8,222,32),
             "For all stands in group: ",0L);

   itsGroupListButton = new NListButton(this,
                                        CRect(8,40,222,340),
                                        *theSpGlobals->runStream->allGroups);
   if (theSpGlobals->runStream->subsetGroups->entries())
   {
      itsGroupListButton->SelectItem(theSpGlobals->runStream->allGroups->
                                     index(theSpGlobals->runStream->
                                           subsetGroups->first()));
   }

   new NText(this, CRect(8,80,222,104),
             "Change management ID to:",0L);

   itsNewMgmtID = new NEditControl(this, CRect(8,112,58,138), NULLString,
                                   CTL_FLAG_RIGHT_JUST);
   itsNewMgmtID->SetFocusCommands(NULLcmd,1);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), GRPMEMBWindow, 0L);

   GetNavigator();
}




void SetMgmtIDWindow::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                    // Apply Button was pressed or
                                 // CR was pushed in the mgmtID field.
      {
         Group * group = (Group *)
                         theSpGlobals->runStream->allGroups->at
                         (itsGroupListButton->GetSelectPosition());
         if (group->standMembership)
         {
            RWOrderedIterator nextStand(*group->standMembership);
            Stand * stand;
            while (stand = (Stand *) nextStand())
            {
               if (stand->mgmtID)
               {
                  *stand->mgmtID = itsNewMgmtID->GetTitle();
               }
               else
               {
                  stand->mgmtID = new CStringRW(itsNewMgmtID->GetTitle());
               }
            }
            theSpGlobals->theAppDocument->SetSave(TRUE);
            itsGroupMembershipWindow->LoadStandsAndGroups();
         }
         break;
      }

      case 2:                    // Close Button was pressed.
      {
         Close();
         break;
      }

      default:
         CWindow::DoCommand(theCommand,theData);
   }
}




SetGroupMembershipWindow::SetGroupMembershipWindow
                          (GroupMembershipWindow * theGroupMembershipWindow)
                         :CWindow(theSpGlobals->theAppDocument,
                                  CRect(100,100,400,364),
                                  "Set Membership",
                                  WSF_DEFER_MODAL,
                                  W_MODAL),
                           itsGroupMembershipWindow(theGroupMembershipWindow)
{

   itsApplyButton = new NButton(this, CRect(8,224,88,256), "Apply");
   itsApplyButton->SetCommand(1);

   itsCloseButton = new NButton(this, CRect(212,224,292,256), "Close");
   itsCloseButton->SetCommand(2);

   new NText(this, CRect(8,8,292,32),
             "Set membership for group: ",0L);

   itsTargetGroupListButton = new NListButton(this,
                                        CRect(8,40,292,340),
                                        *theSpGlobals->runStream->allGroups);
   if (theSpGlobals->runStream->subsetGroups->entries())
   {
      itsTargetGroupListButton->SelectItem(theSpGlobals->runStream->allGroups->
                                           index(theSpGlobals->runStream->
                                                 subsetGroups->first()));
   }

   new NText(this, CRect(8,80,292,104),
             "To stands in group:",0L);

   itsSource1GroupListButton = new NListButton(this,
                                               CRect(8,112,292,412),
                                               *theSpGlobals->runStream->allGroups);
   if (theSpGlobals->runStream->subsetGroups->entries())
   {
      itsSource1GroupListButton->SelectItem(theSpGlobals->runStream->allGroups->
                                            index(theSpGlobals->runStream->
                                                  subsetGroups->first()));
   }


   itsLogicalOperatorRadioGroup = new CRadioGroup(this, CPoint(8,152), FALSE);

   itsAndRButton = itsLogicalOperatorRadioGroup->AddButton(CPoint(0,0),"And...");

   itsOrRButton  = itsLogicalOperatorRadioGroup->AddButton(CPoint(75,0),"Or...");

   itsNotRButton = itsLogicalOperatorRadioGroup->AddButton(CPoint(140,0),
                                                           "Not... in group:");
   itsLogicalOperatorRadioGroup->SetSelectedButton(itsAndRButton);

   itsSource2GroupListButton = new NListButton(this,
                                               CRect(8,184,292,484),
                                               *theSpGlobals->runStream->allGroups);
   itsSource2GroupListButton->DeselectAll();

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), GRPMEMBWindow, 0L);

   GetNavigator();
}





void SetGroupMembershipWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:                    // Apply Button was pressed or
                                 // CR was pushed in the mgmtID field.
      {
         Group * targetGroup  = NULL;
         Group * source1Group = NULL;
         Group * source2Group = NULL;

         if (itsTargetGroupListButton->GetSelectPosition() != -1)
            targetGroup = (Group *)
                          theSpGlobals->runStream->allGroups->at
                          (itsTargetGroupListButton->GetSelectPosition());
         if (!targetGroup) break;

         if (itsSource1GroupListButton->GetSelectPosition() != -1)
            source1Group = (Group *)
                          theSpGlobals->runStream->allGroups->at
                          (itsSource1GroupListButton->GetSelectPosition());
         if (!source1Group) break;

         if (itsSource2GroupListButton->GetSelectPosition() != -1)
            source2Group = (Group *)
                          theSpGlobals->runStream->allGroups->at
                          (itsSource2GroupListButton->GetSelectPosition());

         // if there is a source2Group, then we must determine the logic for membership

         if (source2Group)
         {
            RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
            Stand * stand;
            while (stand = (Stand *) nextStand())
            {
               if (itsLogicalOperatorRadioGroup->
                   GetSelectedButton() == itsAndRButton)
               {
                  // stand must be in both

                  if ((source1Group->standMembership &&
                       source1Group->standMembership->contains(stand)) &&
                      (source2Group->standMembership &&
                       source2Group->standMembership->contains(stand)))
                  {
                     targetGroup->AddStandToGroup(stand);
                  }
                  else
                  {
                     targetGroup->DelStandFromGroup(stand);
                  }
               }
               else if (itsLogicalOperatorRadioGroup->
                        GetSelectedButton() == itsOrRButton)
               {
                  // stand must be in either

                  if ((source1Group->standMembership &&
                       source1Group->standMembership->contains(stand)) ||
                      (source2Group->standMembership &&
                       source2Group->standMembership->contains(stand)))
                  {
                     targetGroup->AddStandToGroup(stand);
                  }
                  else
                  {
                     targetGroup->DelStandFromGroup(stand);
                  }
               }
               else if (itsLogicalOperatorRadioGroup->
                        GetSelectedButton() == itsNotRButton)
               {
                  // stand must be in the first, but not in the second.

                  if ((source1Group->standMembership &&
                       source1Group->standMembership->contains(stand)) &&
                      (!source2Group->standMembership ||
                       !(source2Group->standMembership &&
                         source2Group->standMembership->contains(stand))))
                  {
                     targetGroup->AddStandToGroup(stand);
                  }
                  else
                  {
                     targetGroup->DelStandFromGroup(stand);
                  }
               }
            }
         }
         else
         {
            // if there is no source2Group, then copy the membership of
            // source1Group to the target group.

            // if the targetGroup and the source1Group are the same group,
            // we're done!

            if (targetGroup != source1Group)
            {
               if (source1Group->standMembership)
               {
                  if (targetGroup->standMembership)
                  {
                     RWOrderedIterator nextStand(*targetGroup->standMembership);
                     Stand * stand;
                     while (stand = (Stand *) nextStand())
                     {
                        if (!source1Group->standMembership->contains(stand))
                             targetGroup->DelStandFromGroup(stand);
                        if (!targetGroup->standMembership) break;
                     }
                  }
                  RWOrderedIterator nextStand(*source1Group->standMembership);
                  Stand * stand;
                  while (stand = (Stand *) nextStand())
                  {
                     targetGroup->AddStandToGroup(stand);
                  }
               }
               else if (targetGroup->standMembership)
                        targetGroup->ClearStandMembership();
            }
         }
         theSpGlobals->theAppDocument->SetSave(TRUE);
         itsGroupMembershipWindow->LoadStandsAndGroups();
         break;
      }

      case 2:                    // Close Button was pressed.
      {
         Close();
         break;
      }

      default:
         CWindow::DoCommand(theCommand,theData);
   }
}
