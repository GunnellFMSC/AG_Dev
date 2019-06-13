/********************************************************************

File Name:     cprefswin.cpp
Author:        pmg
Date:          10/24/06

see cprefswin.hpp for notes.

********************************************************************/

#include "PwrDef.h"
#include "cprefswin.hpp"
#include "spfunc2.hpp"

#include CEnvironment_i
#include CText_i
#include CWindow_i
#include NButton_i
#include NEditControl_i
#include "opsys.hpp"

#define CMD_APPLY 1
#define CMD_CLOSE 2
#define CMD_MODIFIED 3
#define CMD_SETDIR 4

#define kCtlHeight 28

////////////////////////////////////////////////////////////////////////////
CPrefsWin::CPrefsWin(CDocument *theDocument)
: CWindow(theDocument, CenterWinPlacement(CRect(0, 0, 600, 400)),
          "Modify Preferences", WSF_NO_MENUBAR, W_MODAL)
{
   // Set modification flag
   itsModified = FALSE;

   // Default environment
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBackgroundColor((COLOR)xvt_vobj_get_attr(NULL_WIN, ATTR_BACK_COLOR));
   SetEnvironment(anEnv);

   // Display default variant identifier
   new CText(this, CPoint(25, (1*kCtlHeight)), "Default Variant:");
   itsDefaultVariant = new NEditControl(this, CRect(220, (1*kCtlHeight)-2, 575, (2*kCtlHeight)-2),
      theSpGlobals->thePreferences->defaultVariant);
   itsDefaultVariant->SetTextCommand(CMD_MODIFIED);

   // Display default locations file name
   new CText(this, CPoint(25, (2*kCtlHeight)), "Default Locations File:");
   itsDefaultLocationsFileName = new NEditControl(this, CRect(220, (2*kCtlHeight)-2, 575, (3*kCtlHeight)-2),
      theSpGlobals->thePreferences->defaultLocationsFileName);
   itsDefaultLocationsFileName->SetTextCommand(CMD_MODIFIED);

   // Display default parameters file name
   new CText(this, CPoint(25, (3*kCtlHeight)), "Default Parameters File:");
   itsDefaultParametersFileName = new NEditControl(this, CRect(220, (3*kCtlHeight)-2, 575, (4*kCtlHeight)-2),
      theSpGlobals->thePreferences->defaultParametersFileName);
   itsDefaultParametersFileName->SetTextCommand(CMD_MODIFIED);

   // Display default post processior information file name
   new CText(this, CPoint(25, (4*kCtlHeight)), "Default Database Name:");
   itsDefaultDataBaseName = new NEditControl(this, CRect(220, (4*kCtlHeight)-2, 575, (5*kCtlHeight)-2),
      theSpGlobals->thePreferences->defaultDataBaseName);
   itsDefaultDataBaseName->SetTextCommand(CMD_MODIFIED);

   // Display default PPE is used
   new CText(this, CPoint(25, (5*kCtlHeight)), "PPE Used:");
   itsDefaultUsePPE = new NEditControl(this, CRect(220, (5*kCtlHeight)-2, 575, (6*kCtlHeight)-2),
      (theSpGlobals->thePreferences->defaultUsePPE ? "Yes" : "No"));
   itsDefaultUsePPE->SetTextCommand(CMD_MODIFIED);

   // Display default editor
   new CText(this, CPoint(25, (6*kCtlHeight)), "Default Editor:");
   itsDefaultEditor = new NEditControl(this, CRect(220, (6*kCtlHeight)-2, 575, (7*kCtlHeight)-2),
      theSpGlobals->thePreferences->defaultEditor);
   itsDefaultEditor->SetTextCommand(CMD_MODIFIED);

   // Display default for segmenting the initial cycle is used
   new CText(this, CPoint(25, (7*kCtlHeight)), "Segment the initial cycle:");
   itsDefaultSegmentCycle = new NEditControl(this, CRect(220, (7*kCtlHeight)-2, 575, (8*kCtlHeight)-2),
      (theSpGlobals->thePreferences->defaultSegmentCycle ? "Yes" : "No"));
   itsDefaultSegmentCycle->SetTextCommand(CMD_MODIFIED);

   // Display default for segmenting the initial cycle is used
   new CText(this, CPoint(25, (8*kCtlHeight)), "Process plots as stands:");
   itsDefaultProcessPlots = new NEditControl(this, CRect(220, (8*kCtlHeight)-2, 575, (9*kCtlHeight)-2),
      (theSpGlobals->thePreferences->defaultProcessPlots ? "Yes" : "No"));
   itsDefaultProcessPlots->SetTextCommand(CMD_MODIFIED);

   // Display default current working direcotry
   new CText(this, CPoint(25, (9*kCtlHeight)), "Default Working Directory:");
   itsDefaultWorkingDirectory = new NEditControl(this, CRect(220, (9*kCtlHeight)-2, 575, (10*kCtlHeight)-2),
      theSpGlobals->thePreferences->defaultWorkingDirectory);
   itsDefaultWorkingDirectory->SetTextCommand(CMD_MODIFIED);

   // Apply and Close
   itsSetDir = new NButton(this, CRect(220, (10*kCtlHeight)+4, 575, (11*kCtlHeight)+4), 
                           "Set Default to Current Working Directory");
   itsSetDir->SetCommand(CMD_SETDIR);

   // Instructions
   double tempHolder = 11.7*kCtlHeight; //Variable created and next line edited to remove compiler warning (C4305)
   new CText(this, CPoint(25, (tempHolder)), "Applied changes will take effect upon next use of Suppose."); 

   // Apply and Close
   itsApply = new NButton(this, CRect(125, (13*kCtlHeight), 275, (14*kCtlHeight)), "Apply");
   itsApply->SetCommand(CMD_APPLY);

   itsClose = new NButton(this, CRect(325, (13*kCtlHeight), 475, (14*kCtlHeight)), "Close", CTL_FLAG_DEFAULT);
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
//            RWCString aPrefValue = itsDefaultSegmentCycle->GetTitle().strip(RWCString::both, ' ');
            aPrefValue = itsDefaultSegmentCycle->GetTitle().strip(RWCString::both, ' ');
			if (!aPrefValue.isNull())
               theSpGlobals->thePreferences->defaultSegmentCycle = tolower((int)aPrefValue[0]) == (int)'y';
			
			// Update default PPE is used
//            RWCString aPrefValue = itsDefaultSegmentCycle->GetTitle().strip(RWCString::both, ' ');
            aPrefValue = itsDefaultProcessPlots->GetTitle().strip(RWCString::both, ' ');
			if (!aPrefValue.isNull())
               theSpGlobals->thePreferences->defaultProcessPlots = tolower((int)aPrefValue[0]) == (int)'y';

			// Update default current working direcotry
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

