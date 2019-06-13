/********************************************************************

   File Name:     spprefer.cpp
   Author:        nlc
   Date:          11/30/2007

 see spprefer.hpp for notes.

 ********************************************************************/

#include "spprefer.hpp"
#include "spglobal.hpp"
#include "cmdline.hpp"
#include "spfuncs.hpp"
#include <ctype.h>

SpPreferences::SpPreferences( void )
{
   defaultVariant             = "ie";
   defaultLocationsFileName   = "suppose.loc";
   tutorialLocationsFileName  = "tutorial.loc";
   defaultParametersFileName  = "suppose.prm";
   defaultDataBaseName        = ".";  //ajsm
   defaultUsePPE              = 0;
   defaultSegmentCycle        = 1;
   defaultProcessPlots        = 0;
#ifdef AIXV4
   defaultEditor              = "emacs";
#else
   defaultEditor              = "NotePad";
#endif
   defaultWorkingDirectory    = ".";
}


void SpPreferences::upDateDefaultsFromParmsFile( void )
{
   const char *preferences = "preferences";

   // get the default variant identifier.
   PKeyData *parms = GetPKeyData(preferences,"defaultVariant",NULL,0,0);
   if (parms) defaultVariant = parms->pString.data();

   // get the default locations file name.
   parms = GetPKeyData(preferences,"defaultLocationsFile",(const char *) theSpGlobals->OS,0,0);
   if (parms) defaultLocationsFileName = parms->pString.data();

   // get the default post processior information file name, is this used?
   parms = GetPKeyData(preferences,"defaultDataBase",(const char *) theSpGlobals->OS,0,0);
   if (parms) defaultDataBaseName = parms->pString.data();

   // get the default value of use PPE, is this used?
   parms = GetPKeyData(preferences,"defaultUsePPE",NULL,0,0);
   if (parms) defaultUsePPE = tolower ((int) *(parms->pString.data())) == (int) 'y';

   // get the default editor.
   parms = GetPKeyData(preferences,"defaultEditor",(const char *) theSpGlobals->OS,0,0);
   if (parms) defaultEditor = parms->pString.data();

  // get the default value for segmenting the initial cycle, is this used?
   parms = GetPKeyData(preferences,"defaultSegmentCycle",NULL,0,0);
   if (parms) defaultSegmentCycle = tolower ((int) *(parms->pString.data())) == (int) 'y';

   // get the default value for segmenting the initial cycle, is this used?
   parms = GetPKeyData(preferences,"defaultProcessPlots",NULL,0,0);
   if (parms) defaultProcessPlots = tolower ((int) *(parms->pString.data())) == (int) 'y';

   // get the default working diectory.
   parms = GetPKeyData(preferences,"defaultWorkingDirectory",(const char *) theSpGlobals->OS,0,0);
   if (parms) defaultWorkingDirectory = parms->pString.data();
}


void SpPreferences::upDateDefaultsFromCommandLine( void )
{
   const char * temp;

   // get the default variant identifier.
   temp = theSpGlobals->theCommandLine->getDefVariant();
   if (temp) defaultVariant = temp;

   // get the default locations file name.
   temp = theSpGlobals->theCommandLine->getLocFileName();
   if (temp) defaultLocationsFileName = temp;

   // get the default parameters file name.
   temp = theSpGlobals->theCommandLine->getParmsFileName();
   if (temp) defaultParametersFileName = temp;

   // get the default post processior information file name, is this used?
   temp = theSpGlobals->theCommandLine->getDataBaseName();
   if (temp) defaultDataBaseName = temp;

   // get the default value of use PPE, is this used?
   temp = theSpGlobals->theCommandLine->getPPEYesNo();
   if (temp) defaultUsePPE = tolower ((int) *temp) == (int) 'y';

   // get the default editor.
   temp = theSpGlobals->theCommandLine->getEditor();
   if (temp) defaultEditor = temp;

   // get the default value for segmenting the initial cycle, is this used?
   temp = theSpGlobals->theCommandLine->getSegCycleYesNo();
   if (temp) defaultSegmentCycle = tolower ((int) *temp) == (int) 'y';

   // get the default working diectory.
   temp = NULL; // Need to implement --> theSpGlobals->theCommandLine->getEditor();
   if (temp) defaultWorkingDirectory = temp;
}


void SpPreferences::upDateDefaultsFromPrefFile( void )
{
   std::string aTempStr;

   // get the default variant identifier.
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultVariant");
   if (!aTempStr.empty()) defaultVariant = aTempStr.c_str();

   // get the default locations file name.
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultLocationsFileName");
   if (!aTempStr.empty()) defaultLocationsFileName = aTempStr.c_str();

   // get the default parameters file name.
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultParametersFileName");
   if (!aTempStr.empty()) 
	   defaultParametersFileName = aTempStr.c_str();

   // get the default post processior information file name, is this used?
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultDataBaseName");
   if (!aTempStr.empty()) defaultDataBaseName = aTempStr.c_str();

   // get the default value of use PPE, is this used?
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultUsePPE");
   if (!aTempStr.empty()) defaultUsePPE = tolower((int)aTempStr[0]) == (int)'y';

   // get the default editor.
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultEditor");
   if (!aTempStr.empty()) defaultEditor = aTempStr.c_str();

   // get the default value for segmenting the initial cycle, is this used?
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultSegmentCycle");
   if (!aTempStr.empty()) defaultSegmentCycle = tolower((int)aTempStr[0]) == (int)'y';

   // get the default value for segmenting the initial cycle, is this used?
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultProcessPlots");
   if (!aTempStr.empty())
   {
	   int asciiChar = tolower((int)aTempStr[0]);
	   switch (asciiChar)// 0 = No, 1 = Yes, 2 = Plots, 3 = Subplots, 4 = Conditions
	   {
	   case (int)'n': //"Stands (FVS_StandInit)"
	   default:
		   defaultProcessPlots = 0;
		   break;
	   case (int)'y': //"Plots within stands (FVS_PlotInit)"
		   defaultProcessPlots = 1;
		   break;
	   case (int)'p': //"Inventory Plots (FVS_StandInit_Plot Table, e.g.: FIA plots)"
		   defaultProcessPlots = 2;
		   break;
	   case (int)'s': //"Inventory Subplots (FVS_PlotInit_Plot Table, e.g.: FIA subplots)"
		   defaultProcessPlots = 3;
		   break;
	   case (int)'c': //"Conditions (FVS_StandInit_Cond Table, e.g.: FIA conditions)"
		   defaultProcessPlots = 4;
		   break;
	   }
   }

   // get the default working diectory.
   aTempStr = theSpGlobals->ReadPrefString("General Preferences", "defaultWorkingDirectory");
   if (!aTempStr.empty()) defaultWorkingDirectory = aTempStr.c_str();
}


void SpPreferences::saveDefaultsToPrefFile( void )
{
   // save the default variant identifier.
   if (!defaultVariant.isNull())
      theSpGlobals->WritePrefString("General Preferences", "defaultVariant", defaultVariant);
   else
      theSpGlobals->ClearPrefString("General Preferences", "defaultVariant");

   // save the default locations file name.
   if (!defaultLocationsFileName.isNull())
      theSpGlobals->WritePrefString("General Preferences", "defaultLocationsFileName", defaultLocationsFileName);
   else
      theSpGlobals->ClearPrefString("General Preferences", "defaultLocationsFileName");

   // save the default parameters file name.
   if (!defaultParametersFileName.isNull())
      theSpGlobals->WritePrefString("General Preferences", "defaultParametersFileName", defaultParametersFileName);
   else
      theSpGlobals->ClearPrefString("General Preferences", "defaultParametersFileName");

   // save the default post processior information file name, is this used?
   if (!defaultDataBaseName.isNull())
      theSpGlobals->WritePrefString("General Preferences", "defaultDataBaseName", defaultDataBaseName);
   else
      theSpGlobals->ClearPrefString("General Preferences", "defaultDataBaseName");

   // save the default value of use PPE, is this used?
   theSpGlobals->WritePrefString("General Preferences", "defaultUsePPE", (defaultUsePPE ? "y" : "n"));

   // save the default editor.
   if (!defaultEditor.isNull())
      theSpGlobals->WritePrefString("General Preferences", "defaultEditor", defaultEditor);
   else
      theSpGlobals->ClearPrefString("General Preferences", "defaultEditor");

   // save the default value of use PPE, is this used?
   theSpGlobals->WritePrefString("General Preferences", "defaultSegmentCycle", (defaultSegmentCycle ? "y" : "n"));

   // save the default value of use PPE, is this used?
   RWCString dbOrganization;
   switch (defaultProcessPlots)
   {// 0 = No, 1 = Yes, 2 = Plots, 3 = Subplots, 4 = Conditions
   case 0:
   default:
	   dbOrganization = "n";
	   break;
   case 1:
	   dbOrganization = "y";
	   break;
   case 2:
	   dbOrganization = "p";
	   break;
   case 3:
	   dbOrganization = "s";
	   break;
   case 4:
	   dbOrganization = "c";
	   break;
   }
   theSpGlobals->WritePrefString("General Preferences", "defaultProcessPlots", dbOrganization);

   // save the default working diectory.
   if (!defaultWorkingDirectory.isNull())
      theSpGlobals->WritePrefString("General Preferences", "defaultWorkingDirectory", defaultWorkingDirectory);
   else
      theSpGlobals->ClearPrefString("General Preferences", "defaultWorkingDirectory");
}