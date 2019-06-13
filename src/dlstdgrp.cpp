/********************************************************************

   File Name:     dlstdgrp.cpp
   Author:        nlc
   Date:          10/96 & 02/19/97 (added delete group)

   Contents:     (see .hpp for description)

********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

#include CDesktop_i
#include CDocument_i
#include NButton_i
#include NListBox_i
#include NListButton_i
#include NText_i
#include NTextEdit_i
#include CText_i
#include CWindow_i
#include CApplication_i
#include CNavigator_i
#include CStringCollection_i

#include <rw/ordcltn.h>
#include <rw/vstream.h>  // for debugging.
#include <rw/gdlist.h>

typedef void *void_PTR;
declare(RWGDlist, void)

#include "dlstdgrp.hpp"
#include "spglobal.hpp"
#include "runstr.hpp"
#include "compon.hpp"
#include "group.hpp"
#include "stand.hpp"
#include "sendupdt.hpp"
#include "spfunc2.hpp"
#include "mainwin.hpp"

extern XVT_HELP_INFO helpInfo;    // help file

DeleteStandWindow::DeleteStandWindow(CDocument * theDocument, void * theData)
                  :CWwindow(theDocument,
                            CenterWinPlacement(CRect(100,100,400,578)),
                            "Delete Stand(s)",
                            WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE,
                            W_DOC, TASK_MENUBAR),
                   itsPreSetStand((Stand*) theData)
{
   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   new NText(this, CRect(8,8,292,26), "Select stands to delete from simulation:", 0L);

   itsTargetStandListBox = new NListBox(this, CRect(8,34,292,184),
                                        *theSpGlobals->runStream->allStands);
   itsTargetStandListBox->SetGlue(TOPSTICKY | RIGHTSTICKY | LEFTSTICKY);
   itsTargetStandListBox->DeselectAll();
   itsTargetStandListBox->SetCommand(3);

   itsAttachedComponentsList = new RWOrdered((size_t) 20);

   itsComponentsText = new NText(this, CRect(8,198,292,216),
                                 "And these attached components:", 0L);

   itsAttachedComponentsListBox = new NListBox(this, CRect(8,224,292,374),
                                               *itsAttachedComponentsList);
   itsAttachedComponentsListBox->SetGlue(TOPSTICKY | BOTTOMSTICKY | RIGHTSTICKY | LEFTSTICKY);
   itsAttachedComponentsListBox->SetCommand(4);

   itsUserNoteText = new NTextEdit(this, CRect(8,382,292,430));
   itsUserNoteText->ITextEdit(TX_NOMENU | TX_NOCUT | TX_NOCOPY | TX_NOPASTE | TX_DISABLED | TX_READONLY | TX_WRAP,
                              itsUserNoteText->GetFrame().Width(),
                              1000,
                              "Note: Items selected for deletion are only removed from the simulation and are not applied to the underlying data.",
                              FALSE, TRUE, BOTTOMSTICKY | RIGHTSTICKY | LEFTSTICKY);
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBackgroundColor((COLOR)xvt_vobj_get_attr(NULL_WIN, ATTR_BACK_COLOR));
   anEnv.SetForegroundColor(COLOR_BLUE);
   itsUserNoteText->SetEnvironment(anEnv);

   itsDeleteStandButton = new NButton(this, CRect(8,438,88,470), "Delete");
   itsDeleteStandButton->SetCommand(1);
   itsDeleteStandButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsDeleteStandButton->Disable();

   itsCloseButton = new NButton(this, CRect(212,438,292,470), "Close");
   itsCloseButton->SetCommand(2);
   itsCloseButton->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), DELETEStandWindow, 0L);

   aNav->AppendTabStop(new CTabStop(itsTargetStandListBox));
   aNav->AppendTabStop(new CTabStop(itsAttachedComponentsListBox));
   aNav->AppendTabStop(new CTabStop(itsDeleteStandButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));
   aNav->InitFocus();

   if (itsPreSetStand) DoCommand (10,itsPreSetStand);
}




void DeleteStandWindow::SizeWindow(int theWidth, int theHeight)
{
   // Call base class method first
   CWindow::SizeWindow(theWidth, theHeight);

   // Reset the margin to match new size
   itsUserNoteText->SetMargin(itsUserNoteText->GetFrame().Width());
}




void DeleteStandWindow::DoCommand(long theCommand, void* theData)
{
   switch (theCommand)
   {
      case 1:                    // Delete Stand Button was pressed.
      {
         RWGDlist(void) aStandToDeleteList;
         int selectedItems, selectedPosition;

         // Delete any attached components
         if (itsAttachedComponentsList->entries())
         {
            RWOrderedIterator nextComponent(*itsAttachedComponentsList);
            Component * component;
            while (component = (Component *) nextComponent())
            {
               theSpGlobals->runStream->DelComponent(component);
            }
         }

         for (selectedItems = itsTargetStandListBox->GetNumSelectedItems(),
              selectedPosition = itsTargetStandListBox->GetSelectPosition();
              selectedItems && (selectedPosition < itsTargetStandListBox->GetNumItems());
              selectedPosition++)
         {
            if (itsTargetStandListBox->IsItSelected(selectedPosition))
            {
               selectedItems--;
               Stand * selectedStand = (Stand *) theSpGlobals->runStream->allStands->at((size_t) selectedPosition);

               // Add the stand to a list for deletion -- if deleted now
               // the list become out of sync with selection
               aStandToDeleteList.insert((void *) selectedStand);
            }
         }

         // Delete groups in list, if any
         if (aStandToDeleteList.entries() > 0)
         {
            Stand *aStandToDelete;
            RWGDlistIterator(void) nextStandToDelete(aStandToDeleteList);
			   while (aStandToDelete = (Stand *)nextStandToDelete())
               theSpGlobals->runStream->DelStand(aStandToDelete);
            SendUpdateMessage(STANDCountChanged);
         }
         break;
      }

      case 2:                    // Close Button was pressed.
      {
         Close();
		 theSpGlobals->theMainWindow->DoEnable();	//12/28/07 ajsm
													//Fix to maintain focus of the management
													//window after deleting a stand.
         break;
      }

      case 3:                    // Selection of a stand.
      {
         itsAttachedComponentsList->clear();
         itsAttachedComponentsListBox->IListBox(*itsAttachedComponentsList);

         int selectedItems, selectedPosition;
         for (selectedItems = itsTargetStandListBox->GetNumSelectedItems(),
              selectedPosition = itsTargetStandListBox->GetSelectPosition();
              selectedItems && (selectedPosition < itsTargetStandListBox->GetNumItems());
              selectedPosition++)
         {
            if (itsTargetStandListBox->IsItSelected(selectedPosition))
            {
               selectedItems--;
               Stand * selectedStand = (Stand *) theSpGlobals->runStream->allStands->at((size_t) selectedPosition);
               if (selectedStand->componentsOrGroups &&
                  selectedStand->componentsOrGroups->entries())
               {
                  RWOrderedIterator next(*selectedStand->componentsOrGroups);
                  RWCollectable * item;
                  while (item = (RWCollectable *) next())
                  {
                     if (item->isA() == COMPONENT)
                     {
                        RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
                        Stand * stand;
                        int insertItem = 1;
                        while (stand = (Stand *) nextStand())
                        {
                           if (stand != selectedStand &&
                              stand->componentsOrGroups &&
                              stand->componentsOrGroups->contains(item))
                              insertItem = 0;
                        }
                        if (insertItem) itsAttachedComponentsList->insert(item);
                     }
                  }
                  itsAttachedComponentsListBox->IListBox(*itsAttachedComponentsList);
                  itsAttachedComponentsListBox->SelectAll();
               }
            }
         }

         if (itsAttachedComponentsList->entries())
            itsComponentsText->SetTitle("And these attached components:");
         else
            itsComponentsText->SetTitle("Stand(s) has no attached components.");

         itsDeleteStandButton->Enable();
         break;
      }

      case 4:                     // component selection is attempted.
      {
         xvt_dm_post_message("You can't select components.");
         itsAttachedComponentsListBox->SelectAll();
         break;
      }

      case 10:                    // A message being received that
                                  // sets itsPreSetStand.
      {
         if (theData)
         {
            itsPreSetStand = (Stand *) theData;
            size_t position = theSpGlobals->runStream->
                  allStands->index(itsPreSetStand);
            if (position != RW_NPOS)
            {
               itsTargetStandListBox->SelectItem(position);
               DoCommand(3);
            }
         }
         break;
      }

      case STANDNameChanged:
      case STANDCountChanged:
      {
         if(theSpGlobals->runStream->allStands->entries())
         {
            itsTargetStandListBox->IListBox(*theSpGlobals->runStream->allStands);
         }
         else
         {
            itsTargetStandListBox->Clear();
         }
         itsTargetStandListBox->DeselectAll();
         itsAttachedComponentsList->clear();
         itsAttachedComponentsListBox->IListBox(*itsAttachedComponentsList);
         itsComponentsText->SetTitle("And these attached components:");
         itsDeleteStandButton->Disable();
         break;
      }

      default:
         CWwindow::DoCommand(theCommand,theData);
   }
}



DeleteGroupWindow::DeleteGroupWindow(CDocument * theDocument, void * theData)
                  :CWwindow(theDocument,
                            CenterWinPlacement(CRect(100,100,400,578)),
                            "Delete Group(s)",
                            WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE,
                            W_DOC, TASK_MENUBAR),
                   itsPreSetGroup((Group *) theData)

{
   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   new NText(this, CRect(8,8,292,26), "Select groups to delete from simulation:", 0L);

   itsTargetGroupListBox = new NListBox(this, CRect(8,34,292,184),
                                        *theSpGlobals->runStream->allGroups);
   itsTargetGroupListBox->SetGlue(TOPSTICKY | RIGHTSTICKY | LEFTSTICKY);
   itsTargetGroupListBox->DeselectAll();
   itsTargetGroupListBox->SetCommand(3);

   itsAttachedComponentsList = new RWOrdered((size_t) 20);

   itsComponentsText = new NText(this, CRect(8,198,292,216),
                                 "And these attached components:", 0L);

   itsAttachedComponentsListBox = new NListBox(this, CRect(8,224,292,374),
                                               *itsAttachedComponentsList);
   itsAttachedComponentsListBox->SetGlue(TOPSTICKY | BOTTOMSTICKY | RIGHTSTICKY | LEFTSTICKY);
   itsAttachedComponentsListBox->SetCommand(4);

   itsUserNoteText = new NTextEdit(this, CRect(8,382,292,430));
   itsUserNoteText->ITextEdit(TX_NOMENU | TX_NOCUT | TX_NOCOPY | TX_NOPASTE | TX_DISABLED | TX_READONLY | TX_WRAP,
                              itsUserNoteText->GetFrame().Width(),
                              1000,
                              "Note: Items selected for deletion are only removed from the simulation and are not applied to the underlying data.",
                              FALSE, TRUE, BOTTOMSTICKY | RIGHTSTICKY | LEFTSTICKY);
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBackgroundColor((COLOR)xvt_vobj_get_attr(NULL_WIN, ATTR_BACK_COLOR));
   anEnv.SetForegroundColor(COLOR_BLUE);
   itsUserNoteText->SetEnvironment(anEnv);

   itsDeleteGroupButton = new NButton(this, CRect(8,438,88,470), "Delete");
   itsDeleteGroupButton->SetCommand(1);
   itsDeleteGroupButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsDeleteGroupButton->Disable();

   itsCloseButton = new NButton(this, CRect(212,438,292,470), "Close");
   itsCloseButton->SetCommand(2);
   itsCloseButton->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), DELETEGroupWindow, 0L);

   aNav->AppendTabStop(new CTabStop(itsTargetGroupListBox));
   aNav->AppendTabStop(new CTabStop(itsAttachedComponentsListBox));
   aNav->AppendTabStop(new CTabStop(itsDeleteGroupButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));
   aNav->InitFocus();

   if (itsPreSetGroup) DoCommand (10,itsPreSetGroup);
}




void DeleteGroupWindow::SizeWindow(int theWidth, int theHeight)
{
   // Call base class method first
   CWindow::SizeWindow(theWidth, theHeight);

   // Reset the margin to match new size
   itsUserNoteText->SetMargin(itsUserNoteText->GetFrame().Width());
}




void DeleteGroupWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:                    // Delete Group Button was pressed.
      {
         RWGDlist(void) aGroupToDeleteList;
         int selectedItems, selectedPosition;

         for (selectedItems = itsTargetGroupListBox->GetNumSelectedItems(),
              selectedPosition = itsTargetGroupListBox->GetSelectPosition();
              selectedItems && (selectedPosition < itsTargetGroupListBox->GetNumItems());
              selectedPosition++)
         {
            if (itsTargetGroupListBox->IsItSelected(selectedPosition))
            {
               selectedItems--;
               Group * selectedGroup = (Group *) theSpGlobals->runStream->allGroups->at((size_t) selectedPosition);

               // if the selected group is group "All", then issue a warning
               // message.

               if (strcmp(selectedGroup->data(),"All") == 0)
               {
                  if (xvt_dm_post_ask("Cancel","Yes",NULL,
                                    "Are you sure you want to delete group All?")
                     == RESP_DEFAULT) break;
               }
               if (xvt_dm_post_ask("Cancel","OK",NULL,
                                 "OK to delete group %s, it's components,"
                                 " and stand membership list?",
                                 selectedGroup->data())
                  == RESP_DEFAULT) break;

               // Add the group to a list for deletion -- if deleted now
               // the list become out of sync with selection
               aGroupToDeleteList.insert((void *) selectedGroup);
            }
         }

         // Delete groups in list, if any
         if (aGroupToDeleteList.entries() > 0)
         {
            Group *aGroupToDelete;
            RWGDlistIterator(void) nextGroupToDelete(aGroupToDeleteList);
			   while (aGroupToDelete = (Group *)nextGroupToDelete())
               theSpGlobals->runStream->DelGroup(aGroupToDelete);
            SendUpdateMessage(GROUPCountChanged);
         }
         break;
      }

      case 2:                    // Close Button was pressed.
      {
         Close();
		 theSpGlobals->theMainWindow->DoEnable();	//12/28/07 ajsm
													//Fix to maintain focus of the management
													//window after deleting a group.
         break;
      }

      case 3:                    // Selection of a group.
      {
         itsAttachedComponentsList->clear();
         itsAttachedComponentsListBox->IListBox(*itsAttachedComponentsList);

         int selectedItems, selectedPosition;
         for (selectedItems = itsTargetGroupListBox->GetNumSelectedItems(),
              selectedPosition = itsTargetGroupListBox->GetSelectPosition();
              selectedItems && (selectedPosition < itsTargetGroupListBox->GetNumItems());
              selectedPosition++)
         {
            if (itsTargetGroupListBox->IsItSelected(selectedPosition))
            {
               selectedItems--;
               Group * selectedGroup = (Group *) theSpGlobals->runStream->
                                       allGroups->at(selectedPosition);
               if (selectedGroup->componentList &&
                  selectedGroup->componentList->entries())
               {
                  RWOrderedIterator nextComponent(*selectedGroup->componentList);
                  Component * aComponent;
                  while (aComponent = (Component *) nextComponent())
                     itsAttachedComponentsList->insert(aComponent);

                  itsAttachedComponentsListBox->
                     IListBox(*itsAttachedComponentsList);
                  itsAttachedComponentsListBox->SelectAll();
               }
            }
         }

         if (itsAttachedComponentsList->entries())
            itsComponentsText->SetTitle("And these attached components:");
         else
            itsComponentsText->SetTitle("Group has no attached components.");

         if (itsTargetGroupListBox->GetNumSelectedItems() == itsTargetGroupListBox->GetNumItems())
         {
            itsDeleteGroupButton->Disable();
            xvt_dm_post_message("You can't delete all groups.");
         }
         else if (theSpGlobals->runStream->allGroups->entries() > 1)
         {
            itsDeleteGroupButton->Enable();
         }
         else
         {
            xvt_dm_post_message("You can't delete the last group.");
            Close();
         }
          break;
      }

      case 4:                     // component selection is attempted.
      {
         xvt_dm_post_message("You can't select components.");
         itsAttachedComponentsListBox->SelectAll();
         break;
      }

      case 10:                    // A message being received that
                                  // sets itsPreSetGroup.
      {
         if (theData)
         {
            itsPreSetGroup = (Group *) theData;
            size_t position = theSpGlobals->runStream->
                  allGroups->index(itsPreSetGroup);
            if (position != RW_NPOS)
            {
               itsTargetGroupListBox->SelectItem(position);
               DoCommand(3);
            }
         }
         break;
      }

      case GROUPNameChanged:
      case GROUPCountChanged:
      {
         if(theSpGlobals->runStream->allGroups->entries())
         {
            itsTargetGroupListBox->IListBox(*theSpGlobals->
                                                  runStream->allGroups);
         }
         else
         {
            itsTargetGroupListBox->Clear();
         }
         itsTargetGroupListBox->DeselectAll();
         itsAttachedComponentsList->clear();
         itsAttachedComponentsListBox->IListBox(*itsAttachedComponentsList);
         itsComponentsText->SetTitle("And these attached components:");
         itsDeleteGroupButton->Disable();
         break;
      }

      default:
         CWwindow::DoCommand(theCommand,theData);
   }
}
