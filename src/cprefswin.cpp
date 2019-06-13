/********************************************************************

File Name:     cprefswin.cpp
Author:        pmg
Date:          10/24/06

see cprefswin.hpp for notes.

********************************************************************/

#include "PwrDef.h"
#include "cprefswin.hpp"
#include "spfunc2.hpp"

#include CStringCollection_i
#include CEnvironment_i
#include CText_i
#include CWindow_i
#include NButton_i
#include NListButton_i
#include NEditControl_i
#include "opsys.hpp"

#define CMD_APPLY 1
#define CMD_CLOSE 2
#define CMD_MODIFIED 3
#define CMD_SETDIR 4

#define kCtlHeight 28

////////////////////////////////////////////////////////////////////////////
CPrefsWin::CPrefsWin(CDocument *theDocument)
	: CWindow(theDocument, CenterWinPlacement(CRect(0, 0, 640, 392)),
		"Modify Preferences", WSF_NO_MENUBAR, W_MODAL)
{
	int inputBoxWidth = 617;
   // Set modification flag
   itsModified = FALSE;

   // Default environment
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBackgroundColor((COLOR)xvt_vobj_get_attr(NULL_WIN, ATTR_BACK_COLOR));
   SetEnvironment(anEnv);

   // Display default variant identifier
   new CText(this, CPoint(25, (1*kCtlHeight)), "Default Variant:");
   itsDefaultVariant = new NEditControl(this, CRect(220, (1 * kCtlHeight) - 2, inputBoxWidth, (2 * kCtlHeight) - 2),
      theSpGlobals->thePreferences->defaultVariant);
   itsDefaultVariant->SetTextCommand(CMD_MODIFIED);

   // Display default locations file name
   new CText(this, CPoint(25, (2*kCtlHeight)), "Default Locations File:");
   itsDefaultLocationsFileName = new NEditControl(this, CRect(220, (2 * kCtlHeight) - 2, inputBoxWidth, (3 * kCtlHeight) - 2),
      theSpGlobals->thePreferences->defaultLocationsFileName);
   itsDefaultLocationsFileName->SetTextCommand(CMD_MODIFIED);

   // Display default parameters file name
   new CText(this, CPoint(25, (3*kCtlHeight)), "Default Parameters File:");
   itsDefaultParametersFileName = new NEditControl(this, CRect(220, (3 * kCtlHeight) - 2, inputBoxWidth, (4 * kCtlHeight) - 2),
      theSpGlobals->thePreferences->defaultParametersFileName);
   itsDefaultParametersFileName->SetTextCommand(CMD_MODIFIED);

   // Display default post processior information file name
   new CText(this, CPoint(25, (4*kCtlHeight)), "Default Database Name:");
   itsDefaultDataBaseName = new NEditControl(this, CRect(220, (4 * kCtlHeight) - 2, inputBoxWidth, (5 * kCtlHeight) - 2),
      theSpGlobals->thePreferences->defaultDataBaseName);
   itsDefaultDataBaseName->SetTextCommand(CMD_MODIFIED);

   // Display default PPE is used
   new CText(this, CPoint(25, (5*kCtlHeight)), "PPE Used:");
   itsDefaultUsePPE = new NEditControl(this, CRect(220, (5 * kCtlHeight) - 2, inputBoxWidth, (6 * kCtlHeight) - 2),
      (theSpGlobals->thePreferences->defaultUsePPE ? "Yes" : "No"));
   itsDefaultUsePPE->SetTextCommand(CMD_MODIFIED);

   // Display default editor
   new CText(this, CPoint(25, (6*kCtlHeight)), "Default Editor:");
   itsDefaultEditor = new NEditControl(this, CRect(220, (6 * kCtlHeight) - 2, inputBoxWidth, (7 * kCtlHeight) - 2),
      theSpGlobals->thePreferences->defaultEditor);
   itsDefaultEditor->SetTextCommand(CMD_MODIFIED);

   // Display default for segmenting the initial cycle is used
   new CText(this, CPoint(25, (7*kCtlHeight)), "Segment the initial cycle:");
   itsDefaultSegmentCycle = new NEditControl(this, CRect(220, (7 * kCtlHeight) - 2, inputBoxWidth, (8 * kCtlHeight) - 2),
      (theSpGlobals->thePreferences->defaultSegmentCycle ? "Yes" : "No"));
   itsDefaultSegmentCycle->SetTextCommand(CMD_MODIFIED);

   // Display default for segmenting the initial cycle is used
   databaseAnswers[0] = "Stands (FVS_StandInit)";
   databaseAnswers[1] = "Plots within stands (FVS_PlotInit)";
   databaseAnswers[2] = "Inventory Plots (FVS_StandInit_Plot)(e.g.: FIA plots)";
   databaseAnswers[3] = "Inventory Subplots (FVS_PlotInit_Plot)(e.g.: FIA subplots)";
   databaseAnswers[4] = "Conditions (FVS_StandInit_Cond)(e.g.: FIA conditions)";
   new CText(this, CPoint(25, (8 * kCtlHeight)), "Input Database Table");
   RWCString dbOrganization;
   switch (theSpGlobals->thePreferences->defaultProcessPlots)
   {// 0 = No, 1 = Yes, 2 = Plots, 3 = Subplots, 4 = Conditions
   case 0:
   default:
	   dbOrganization = databaseAnswers[0].data();
	   break;
   case 1:
	   dbOrganization = databaseAnswers[1].data();
	   break;
   case 2:
	   dbOrganization = databaseAnswers[2].data();
	   break;
   case 3:
	   dbOrganization = databaseAnswers[3].data();
	   break;
   case 4:
	   dbOrganization = databaseAnswers[4].data();
	   break;
   }

   RWOrdered * listButtonList = new RWOrdered(5);
   CStringRWC * dropboxOption = new CStringRWC(databaseAnswers[0].data());
   listButtonList->insert(dropboxOption);
   CStringRWC *dropboxOption2 = new CStringRWC(databaseAnswers[1].data());
   listButtonList->insert(dropboxOption2);
   CStringRWC *dropboxOption3 = new CStringRWC(databaseAnswers[2].data());
   listButtonList->insert(dropboxOption3);
   CStringRWC *dropboxOption4 = new CStringRWC(databaseAnswers[3].data());
   listButtonList->insert(dropboxOption4);
   CStringRWC *dropboxOption5 = new CStringRWC(databaseAnswers[4].data());
   listButtonList->insert(dropboxOption5);
   itsDefaultProcessPlots = new NListButton((CSubview*)this,
	   CRect(220, (8 * kCtlHeight) - 2, inputBoxWidth, (9 * kCtlHeight) + 152),
	   *listButtonList, 0L);
   ((NListButton *)itsDefaultProcessPlots)->SetCommand(CMD_MODIFIED);
   ((NListButton *)itsDefaultProcessPlots)->SelectItem(theSpGlobals->thePreferences->defaultProcessPlots);

   // Display default current working direcotry
   new CText(this, CPoint(25, (9*kCtlHeight)), "Default Working Directory:");
   itsDefaultWorkingDirectory = new NEditControl(this, CRect(220, (9 * kCtlHeight) - 2, inputBoxWidth, (10 * kCtlHeight) - 2),
      theSpGlobals->thePreferences->defaultWorkingDirectory);
   itsDefaultWorkingDirectory->SetTextCommand(CMD_MODIFIED);

   // Apply and Close
   itsSetDir = new NButton(this, CRect(220, (10 * kCtlHeight) + 4, inputBoxWidth, (11 * kCtlHeight) + 4),
                           "Set Default to Current Working Directory");
   itsSetDir->SetCommand(CMD_SETDIR);

   // Instructions
   double tempHolder = 11.4*kCtlHeight; //Variable created and next line edited to remove compiler warning (C4305)
   new CText(this, CPoint(25, (tempHolder)), "Applied changes will take effect upon next use of Suppose."); 

   // Apply and Close
   itsApply = new NButton(this, CRect(125, (int)(12.7*kCtlHeight), 275, (int)(13.7*kCtlHeight)), "Apply");
   itsApply->SetCommand(CMD_APPLY);

   itsClose = new NButton(this, CRect(325, (int)(12.7*kCtlHeight), 475, (int)(13.7*kCtlHeight)), "Close", CTL_FLAG_DEFAULT);
   itsClose->SetCommand(CMD_CLOSE);

   // Ensure objects created in the constructor are updated are drawn.
   xvt_dwin_invalidate_rect(GetXVTWindow(), NULL);
}

////////////////////////////////////////////////////////////////////////////
void CPrefsWin::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
   case CMD_APPLY:
      {
         if (itsModified)
         {
            // Update default variant identifier
            theSpGlobals->thePreferences->defaultVariant = itsDefaultVariant->GetTitle().strip(RWCString::both, ' ');

            // Update default locations file name
            theSpGlobals->thePreferences->defaultLocationsFileName = itsDefaultLocationsFileName->GetTitle().strip(RWCString::both, ' ');

            // Update default parameters file name
            theSpGlobals->thePreferences->defaultParametersFileName = itsDefaultParametersFileName->GetTitle().strip(RWCString::both, ' ');

            // Update default post processior information file name
            theSpGlobals->thePreferences->defaultDataBaseName = itsDefaultDataBaseName->GetTitle().strip(RWCString::both, ' ');

            // Update default PPE is used
            RWCString aPrefValue = itsDefaultUsePPE->GetTitle().strip(RWCString::both, ' ');
            if (!aPrefValue.isNull())
               theSpGlobals->thePreferences->defaultUsePPE = tolower((int)aPrefValue[0]) == (int)'y';

            // Update default editor
            theSpGlobals->thePreferences->defaultEditor = itsDefaultEditor->GetTitle().strip(RWCString::both, ' ');

            // Update default PPE is used
            aPrefValue = itsDefaultSegmentCycle->GetTitle().strip(RWCString::both, ' ');
			if (!aPrefValue.isNull())
               theSpGlobals->thePreferences->defaultSegmentCycle = tolower((int)aPrefValue[0]) == (int)'y';
			
			// Update default PPE is used
			aPrefValue = ((NListButton *)itsDefaultProcessPlots)->GetFirstSelectedItem();
			if (!aPrefValue.isNull())
				for (int i = 0; i <= 4; i++) // 0 = No, 1 = Yes, 2 = Plots, 3 = Subplots, 4 = Conditions
					if (strcmp(aPrefValue.data(), databaseAnswers[i].data()) == 0)
						theSpGlobals->thePreferences->defaultProcessPlots = i;
            theSpGlobals->thePreferences->defaultWorkingDirectory = itsDefaultWorkingDirectory->GetTitle().strip(RWCString::both, ' ');
            theSpGlobals->thePreferences->saveDefaultsToPrefFile();
            SpChangeDirectory(theSpGlobals->thePreferences->defaultWorkingDirectory);

            xvt_fsys_save_dir();

         }

         itsModified = FALSE;
      }
      break;

   case CMD_CLOSE:
      {
         if (itsModified)
         {
            if (RESP_DEFAULT == xvt_dm_post_ask("Yes", "No", NULL, "Do you wish to apply the changes before closing?"))
               DoCommand(CMD_APPLY);
         }
         Close();
      }
      break;

   case CMD_SETDIR:
      {
         FILE_SPEC fileSpec;
         xvt_fsys_get_dir(&(&fileSpec)->dir);
         char path[SZ_FNAME];
         xvt_fsys_convert_dir_to_str(&(&fileSpec)->dir,
                                      path, sizeof(path));
         itsDefaultWorkingDirectory->SetTitle(path);
         itsModified = TRUE;
      }
      break;

   case CMD_MODIFIED:
      {
         itsModified = TRUE;
      }
      break;

   default:
      CWindow::DoCommand(theCommand,theData);
   }
}

