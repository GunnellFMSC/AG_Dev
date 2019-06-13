/********************************************************************

   File Name:     postpwin.cpp   (Select Post Processors)
   Author:        nlc
   Date:          11/03/95


********************************************************************/

#include "appdef.hpp"
#include "PwrDef.h"

#include "spglobal.hpp"
#include "postpwin.hpp"
#include "runstr.hpp"
#include "suppdefs.hpp"
#include "spfunc2.hpp"

#include CStringCollection_i
#include CDocument_i
#include CStringRWC_i
#include NButton_i
#include NListBox_i
#include NScrollText_i
#include CStringRW_i
#include CText_i
#include CDesktop_i

#include <rw/ordcltn.h>
#include <rw/collstr.h>
#include "msgwin.hpp"
#include "postproc.hpp"
#include "spparms.hpp"

#include "supphelp.hpp"  // for help #defines

extern XVT_HELP_INFO helpInfo;


PostProcWin::PostProcWin(CDocument *theDocument)
            :CWindow(theDocument,
                     CenterTopWinPlacement(CRect(8,25,592,433)),
                     "Select Post Processors",
                     WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                     W_DOC,TASK_MENUBAR)
{
   AddDynamicHelpItems(GetMenuBar());

   CText * tmp = new CText(this,CPoint(8,4),"Available post processors:");
   tmp->SetGlue(TOPSTICKY | LEFTSTICKY);

   LoadAvailableList();

   availableListBox = new NListBox(this, CRect(8,28,576,128),
                                   availableList, NULL);
   availableListBox->SetDoubleCommand(4);
   availableListBox->SetCommand(5);
   availableListBox->SetGlue(TOPSTICKY | RIGHTSTICKY | LEFTSTICKY);

   tmp = new CText(this,CPoint(8,132),"Included post processors:");
   tmp->SetGlue(TOPSTICKY | LEFTSTICKY);

   includedListBox = new NListBox(this, CRect(8,156,576,236),
                         *theSpGlobals->runStream->includedPostProcs, NULL);
   includedListBox->SetCommand(6);
   includedListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                            RIGHTSTICKY  | LEFTSTICKY);

   tmp = new CText(this, CPoint(8,240), "Description/Notes:");
   tmp->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   descriptionTextEdit = new NScrollText(this, CRect(8,264,576,364), FALSE,
               TRUE,  TX_BORDER | TX_WRAP | TX_READONLY);
   descriptionTextEdit->SetCommand(7);
   descriptionTextEdit->SetGlue(BOTTOMSTICKY | RIGHTSTICKY | LEFTSTICKY);

                           // bottom buttons

   closeButton = new NButton(this, CRect(8,368,136,400), "Close");
   closeButton->SetCommand(1);
   closeButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   deleteButton = new NButton(this, CRect(448,368,576,400), "Delete");
   deleteButton->SetCommand(2);
   deleteButton->Disable();
   deleteButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   includeButton = new NButton(this, CRect(228,368,356,400), "Include");
   includeButton->SetCommand(3);
   includeButton->Disable();
   includeButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), POSTPWINWindow, 0L);
   Show();
   G->GetDesktop()->SetFrontWindow(this);
}






PostProcWin::~PostProcWin( void )
{
   // delete PostProcs that are in the available list AND NOT in the
   // included list.  In this case, membership in the included list is
   // determined ONLY if the POINTERS are equal.

   PostProc * availPostProc;
   for (availPostProc = (PostProc *) availableList.pop(); availPostProc;
        availPostProc = (PostProc *) availableList.pop())
   {
      RWOrderedIterator nextIncludedPostProc(*theSpGlobals->runStream->
                    includedPostProcs);
      PostProc * inclPostProc;
      int needsDeletion = 1;
      while (inclPostProc = (PostProc *) nextIncludedPostProc())
      {
         if (inclPostProc == availPostProc) needsDeletion = 0;
      }
      if (needsDeletion) delete availPostProc;
   }
}






void PostProcWin::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:                           // Close
         Close();
         break;

      case 2:                           // Delete
      {
         int aPos = includedListBox->GetSelectPosition();
         PostProc * selectedPostProc = NULL;
         if (aPos >= 0 &&
             aPos < theSpGlobals->runStream->includedPostProcs->entries())
            selectedPostProc = (PostProc *)
               theSpGlobals->runStream->includedPostProcs->at(aPos);
         if (selectedPostProc)
         {
            theSpGlobals->runStream->includedPostProcs->
               remove(selectedPostProc);
            includedListBox->IListBox(*theSpGlobals->runStream->
                                      includedPostProcs);
         }
         deleteButton->Disable();
         descriptionTextEdit->Clear();
         itsDocument->SetSave(TRUE);
         break;
      }
      case 3:                           // Include
      case 4:                           // Available (Double)
      case 5:                           // Available (Single)
      {
         includedListBox->DeselectAll();
         deleteButton->Disable();
         int aPos = availableListBox->GetSelectPosition();
         PostProc * selectedPostProc = NULL;
         if (aPos >= 0 && aPos < availableList.entries())
            selectedPostProc = (PostProc *) availableList.at(aPos);

         if (theCommand != 3) DisplayDescription(selectedPostProc);

         // if the command is 5, single click, then
         // if the selected post proc is already included, then
         // set "Include" disabled, otherwise set enabled and break.

         if (!selectedPostProc) break;

         if (theCommand == 5)
         {
            if (theSpGlobals->runStream->includedPostProcs->
                contains(selectedPostProc)) includeButton->Disable();
            else includeButton->Enable();
         }
         else
         {
            if (!theSpGlobals->runStream->includedPostProcs->
                contains(selectedPostProc))
            {
               theSpGlobals->runStream->includedPostProcs->
                                   insert(selectedPostProc);
               itsDocument->SetSave(TRUE);
            }
            includeButton->Disable();
            availableListBox->DeselectAll();
            includedListBox ->IListBox(*theSpGlobals->runStream->
                                       includedPostProcs);
         }
         break;
      }
      case 6:                           // Included (Single)
      {
         if (includedListBox->GetSelectPosition() > -1)
         {
            PostProc * selectedPostProc = (PostProc *)
                  (*theSpGlobals->runStream->includedPostProcs)
                  [includedListBox->GetSelectPosition()];
            DisplayDescription(selectedPostProc);
            includeButton->Disable();
            availableListBox->DeselectAll();
            deleteButton->Enable();
         }
         break;
      }
      default: CWindow::DoCommand(theCommand,theData); break;
   }
}




void PostProcWin::LoadAvailableList( void )
{
   // YUK.  We must open an iterator over all MSText and find those that
   // have as the prefix part of the name ppif.  Then we get the PKeyData
   // elements we are looking for and create a post processor object.  We
   // add the post processor object to the list of "available" post processors.

   // At this point, we will only define the post processor object by name
   // and set a pointer to the MSText.  The rest will be done ONLY if the
   // post processor is selected to be included in the run.

   if (theSpGlobals->theParmsData->entries())
   {
      RWBinaryTreeIterator nextMSText(*theSpGlobals->theParmsData);
      MSText * aMSText;
      while (aMSText = (MSText *) nextMSText())
      {
         if (strncmp(aMSText->data(),"ppif.", 5) == 0)
         {
            // To be valid, the post processors must have a name and a
            // command.

            PKeyData * namePKeyData    =
               aMSText->FindPKeyData("name",NULL,0);
            PKeyData * commandPKeyData =
               aMSText->FindPKeyData("command", theSpGlobals->OS,0);
            if (namePKeyData && commandPKeyData)
            {
               PostProc * availPostProc =
                  new PostProc (namePKeyData->pString.data());
               availPostProc->theMSText = aMSText;
               availPostProc->theMSTextName =
                  new CStringRW((CStringRWC &) *aMSText);
               availPostProc->command   =
                  new CStringRW(commandPKeyData->pString);
               availableList.insert(availPostProc);
            }
         }
      }
   }
}



void PostProcWin::DisplayDescription(PostProc * selectedPostProc)
{
   // if the description is not yet set, then find the description
   // and create a holder for it.

   if (selectedPostProc)
   {
      if (!selectedPostProc->description)
      {
         PKeyData * descriptionPKeyData =
            selectedPostProc->theMSText->FindPKeyData("description",NULL,0);
         if (descriptionPKeyData) selectedPostProc->description =
               new CStringRW(descriptionPKeyData->pString);
         else selectedPostProc->description =
            new CStringRW("No description available.");
      }
      descriptionTextEdit->SetText(*selectedPostProc->description);
   }
   else descriptionTextEdit->Clear();
}

