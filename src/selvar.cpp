/*********************************************************************

   File Name:    selvar.cpp
   Author:       nlc
   Date:         04/27/95

   see selvar.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"


#include NListBox_i
#include NListButton_i
#include NCheckBox_i
#include NText_i
#include NButton_i
#include NListEdit_i
#include CDesktop_i
#include CMenuBar_i
#include CDocument_i

#include "suppdefs.hpp"

#include CStringCollection_i
#include CWindow_i
#include CStringRWC_i

#include "warn_dlg.hpp"
#include "spglobal.hpp"
#include "variants.hpp"
#include "extens.hpp"
#include "commands.hpp"
#include "programs.hpp"
#include "spparms.hpp"
#include "spprefer.hpp"
#include "spfunc2.hpp"

#include "selvar.hpp"
#include "supphelp.hpp"  // for help #defines
extern XVT_HELP_INFO helpInfo;

SelectVariant::SelectVariant( CDocument * theDocument )
              :CWwindow(theDocument,
                        CenterTopWinPlacement(CRect(20,20,570,364)),
                        "Select Variant and Extension",
                        WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE,
                        W_DOC, TASK_MENUBAR)
{

                                                              // Close
    closeButton = new NButton(this, CRect(8,304,104,336), "Close");
    closeButton->SetCommand(1);

                                                              // Variant
    new NText(this, CRect(8,8,152,32), "FVS Variant", 0L);


    variantListButton = new NListButton(this, CRect(144,8,456,320),
                                        variantsList, 0, 0L);
    variantListButton->SetCommand(3);

    variantLocked = new NCheckBox(this, CRect(462,8,546,32),
                                  "Locked", 0L);
    variantLocked->SetSelectCommand(4);
    variantLocked->SetDeselectCommand(5);

                                                             // Program
    new NText(this, CRect(8,48,152,72), "FVS Program", 0L);

    programListButton = new NListButton(this, CRect(144,48,456,320),
                                        programList, 0, 0L);
    programListButton->SetCommand(6);

    programLocked = new NCheckBox(this, CRect(462,48,546,72),
                                  "Locked", 0L);
    programLocked->SetSelectCommand(7);
    programLocked->SetDeselectCommand(8);


                                                              // Command
    new NText(this, CRect(8,88,152,112), "Run Command", 0L);

    commandListButton = new NListButton(this, CRect(144,88,456,320),
                                        commandList, 0, 0L);
    commandListButton->SetCommand(9);

    commandLocked = new NCheckBox(this, CRect(462,88,546,112),
                                  "Locked", 0L);
    commandLocked->SetSelectCommand(10);
    commandLocked->SetDeselectCommand(11);

    // command list button and manual locking are not yet supported.

    commandLocked->Disable();
    commandListButton->Disable();

                                                              // Extensions
    new NText(this, CRect(8,128,152,152), "FVS Extensions", 0L);


    extensionsListBox = new NListBox (this, CRect(144,128,456,344),
                                      extensionsList, CTL_FLAG_MULTIPLE);

    extensionsListBox->SetCommand(12);

    extensionsLockButton = new NButton(this, CRect(462,128,546,160),
                                     "Lock", 0L);
    extensionsLockButton->SetCommand(13);


    // Set up the linkage to help.

    xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), SELVARWindow, 0L);

    // Initialize the window.

    ISelectVariant();
}



void SelectVariant::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                         // Close button pushed
      {
         Close();
         break;
      }

      case 3:                         // Variant List button
      {
         int selectedItem = variantListButton->GetSelectPosition();
         if (theSpGlobals->theVariants->theSelectedVariant !=
             selectedItem)
         {
            const char * varIWant = theSpGlobals->theVariants->
               listOfVariantsPKeyData[selectedItem]->data();
            theSpGlobals->theVariants->SetSelectedVariant(varIWant);
            theSpGlobals->thePrograms->PickProgram(0);
         }
         break;
      }

      case 4:                         // Variant Locked (Select)
      {
         if (xvt_dm_post_ask("Lock","Cancel", NULL,
                             "Are you sure you want to lock the variant? ")
             == RESP_DEFAULT)
         {
            theSpGlobals->theVariants->variantSelectionLocked = 1;
            theSpGlobals->thePrograms->PickProgram(0);
            ISelectVariant();
         }
         else variantLocked->Deselect();
         break;
      }

      case 6:                         // Program List button
      {
         int selectedItem = programListButton->GetSelectPosition();
         if (theSpGlobals->thePrograms->theSelectedProgram !=
             selectedItem)
         {
            CStringRW tmp = programListButton->GetFirstSelectedItem();
            const char * pgm = tmp.data();
            theSpGlobals->thePrograms->SelectThisProgram(pgm, 0);
            ISelectVariant();
         }
         break;
      }
      case 7:                         // Program Locked (Select)
      {
         if (xvt_dm_post_ask("Lock","Cancel", NULL,
                             "Are you sure you want to lock the program? ")
             == RESP_DEFAULT)
         {
            theSpGlobals->thePrograms->LockLegalProgram
               ((int) programListButton->GetSelectPosition());
            ISelectVariant();
         }
         else programLocked->Deselect();

         break;
      }
      case 9:                         // Command List button (not active)

         break;

      case 10:                        // Command Locked (Select) (not active)

         break;

      case 12:                        // Extensions List box
      {
         // if there are no selected extension items, then break.

         if (extensionsListBox->GetNumSelectedItems() == 0) break;

         // load a list of requestedExtensions

         RWOrdered requestedExtensions((size_t) extensionsListBox->
                                       GetNumSelectedItems());

         RWOrderedIterator nextPKeyData(theSpGlobals->theExtensions->
                                        extensionsMSTextPointer->thePKeys);
         PKeyData * oneExtension;
         int i=0;
         while (oneExtension = (PKeyData *) nextPKeyData())
         {
            if (extensionsListBox->IsItSelected(i++))
                requestedExtensions.insert(oneExtension);
         }

         // look for a program that meets the needs.  If found, then
         // fine, select it.  If the variant is not locked, then let
         // it change if necessary to select a program that has the
         // extensions.

         int proposedPgm = theSpGlobals->thePrograms->
            FindProgramThatContainsAllDesiredElements
            (requestedExtensions,
             theSpGlobals->theVariants->GetSelectedVariant());

         if (proposedPgm == -1 &&
             !theSpGlobals->theVariants->variantSelectionLocked)
         {
            // try the default variant.

            if (strcmp(theSpGlobals->thePreferences->defaultVariant,
                       theSpGlobals->theVariants->GetSelectedVariant()) != 0)
            {
               proposedPgm = theSpGlobals->thePrograms->
                  FindProgramThatContainsAllDesiredElements
                  (requestedExtensions,
                   theSpGlobals->thePreferences->defaultVariant);
            }

            // try any variant.

            if (proposedPgm == -1) proposedPgm = theSpGlobals->thePrograms->
               FindProgramThatContainsAllDesiredElements
               (requestedExtensions, NULL);
         }

         if (proposedPgm == -1)
         {
            xvt_dm_post_message("Suppose could not find a program that "
                                "contains all the desired extensions.");
            ISelectVariant();
         }
         if (proposedPgm > -1)
            theSpGlobals->thePrograms->SelectThisProgram(proposedPgm, 0);

         break;
      }

      case 13:                        // Extensions Lock
      {
         if (xvt_dm_post_ask("Lock","Cancel", NULL,
                             "Are you sure you want to lock the extensions? ")
             == RESP_DEFAULT)
         {
            //extensionsLockButton->Disable();
            //extensionsListBox->Disable();

            // load a list of requestedExtensions

            RWOrdered requestedExtensions((size_t) extensionsListBox->
                                          GetNumSelectedItems());
            RWOrderedIterator nextPKeyData(theSpGlobals->theExtensions->
                                           extensionsMSTextPointer->thePKeys);
            CStringRWC * oneExtension;
            int i=0;
            while (oneExtension = (CStringRWC *) nextPKeyData())
            {
               if (extensionsListBox->IsItSelected(i++))
                  requestedExtensions.insert(oneExtension);
            }
            RWOrderedIterator nextInUse(requestedExtensions);
            while (oneExtension = (CStringRWC *) nextInUse())
            {
               theSpGlobals->theExtensions->
                  AddExtensionInUse(*oneExtension, 0);
            }
            ISelectVariant();
         }
         break;
      }


      case EXTENSIONListChanged:
      case VARIANTSelectionChanged:
      case PROGRAMSelectionChanged:
      {
         ISelectVariant();
         break;
      }
      default:
      {
         CWwindow::DoCommand(theCommand,theData);
         break;
      }
   }
}



SelectVariant::~SelectVariant( void )
{
   variantsList.clearAndDestroy();
   programList.clearAndDestroy();
   commandList.clearAndDestroy();
   extensionsList.clearAndDestroy();
}



void SelectVariant::ISelectVariant( void )
{
   int position;

   variantsList.clearAndDestroy();
   position = theSpGlobals->theVariants->LoadVariantCollection(&variantsList);
   if (position == -2)
   {
      variantsList.insert(new CStringRWC ("No variants."));
      position = 0;
   }
   else if (position <= 0) position = 0;
   variantListButton->IListButton(variantsList, position);
   if (theSpGlobals->theVariants->variantSelectionLocked)
   {
      variantLocked->Select();
      variantLocked->Disable();
      variantListButton->Disable();
   }

   programList.clearAndDestroy();
   position = theSpGlobals->thePrograms->LoadProgramCollection(&programList);
   if (position == -2)
   {
      programList.insert(new CStringRWC ("No programs."));
      position = 0;
   }
   else if (position < 0) position = 0;
   programListButton->IListButton(programList, position);
   if (theSpGlobals->thePrograms->programSelectionLocked)
   {
      programLocked->Select();
      programLocked->Disable();
      programListButton->Disable();
   }

   commandList.clearAndDestroy();
   position = theSpGlobals->theCommands->LoadCommandCollection(&commandList);
   if (position == -2)
   {
      commandList.insert(new CStringRWC ("No command."));
      position = 0;
   }
   else if (position < 0) position = 0;
   commandListButton->IListButton(commandList, position);
   if (theSpGlobals->theCommands->commandSelectionLocked)
   {
      commandLocked->Select();
      commandLocked->Disable();
      commandListButton->Disable();
   }

   RWOrderedIterator nextPKeyData(theSpGlobals->theExtensions->
                                  extensionsMSTextPointer->thePKeys);
   PKeyData * oneExtension;

   if (extensionsList.entries() == 0)
   {
      while (oneExtension = (PKeyData *) nextPKeyData())
      {
         extensionsList.insert(new CStringRWC(oneExtension->pString));
      }
      nextPKeyData.reset();
      extensionsListBox->IListBox(extensionsList);
   }
   else extensionsListBox->DeselectAll();

   int i=0;
   while (oneExtension = (PKeyData *) nextPKeyData())
   {
      if (theSpGlobals->thePrograms->SelectedProgramContainsElement
          ((CStringRWC *) oneExtension) > -1) extensionsListBox->SelectItem(i);
      i++;
      extensionsListBox->SelectItem(0);  // force the list to start at 0
   }

   if (theSpGlobals->thePrograms->programSelectionLocked)
   {
      extensionsLockButton->SetTitle("Locked");
      extensionsLockButton->Disable();
      extensionsListBox->Disable();
   }
}

