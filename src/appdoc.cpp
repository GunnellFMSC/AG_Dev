/********************************************************************

   File Name:     appdoc.cpp
   Author:        kfd & nlc
   Date:          11/29/94

   updated:
		Author:		apg
		Date:		11/28/2017

********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include "suppdefs.hpp"
#include "supphelp.hpp"
#include "selectsi.hpp"

#include "runstr.hpp"
#include "postproc.hpp"
#include "stand.hpp"
#include "group.hpp"
#include "compon.hpp"
#include "variants.hpp"
#include "extens.hpp"
#include "programs.hpp"
#include "commands.hpp"
#include "selvar.hpp"
#include "fvslocs.hpp"
#include "spglobal.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"
#include "spprefer.hpp"
#include "cmdline.hpp"
#include "timescal.hpp"
#include "yearobj.hpp"
#include "spfunc2.hpp"
#include "grpmemb.hpp"
#include "dlstdgrp.hpp"
#include "sendupdt.hpp"
#include "reportwn.hpp"
#include "graphwn.hpp"
#include "readfvsw.hpp"
#include "simnotes.hpp"
#include "editlocs.hpp"
#include "editslf.hpp"
#include "editfvst.hpp"
#include "mdloutwin.hpp"
#include "cprefswin.hpp"

#include CDesktop_i
#include CApplication_i
#include CMenuBar_i

#include "opsys.hpp"

#include "warn_dlg.hpp"

#include <rw/ordcltn.h>
#include <rw/rwset.h>

#include "appdoc.hpp"
#include "suppdefs.hpp"
#include "mainwin.hpp"
#include "cmngwin.hpp"
#include "cmmodwin.hpp"
#include "cwwindow.hpp"
#include "usekey.hpp"
#include "schcond.hpp"
#include "postpwin.hpp"
#include "runwizw.hpp"
#include "reportd.hpp"
#include "avefvsw.hpp"
#include "dirsept.hpp"

XVT_HELP_INFO helpInfo;    // help file

// Define the Run-Time Type Identification for this class
PWRRegisterClass1(CsupposeDoc, CsupposeDocID, "CsupposeDoc", CDocument)


CsupposeDoc::CsupposeDoc (CApplication *theApplication,int theId )
            :CDocument(theApplication,theId),
             locFilePointer(NULL),
             itsFVSOutputNeedsScaning(0)
{

   // Create and initialize the internal data objects for this class

   // If the internal data object is derived from CModel, you may also
   // want to instantiate a CController for the data, and take advantage
   // of the automatic data propagation

   locFilePointer = NULL;

   // create helpfile pointer.

   FILE_SPEC itsHelpFilePointer;

   // if the helpfile name is on the command line, use it, otherwise use
   // the default.

   const char * temp = theSpGlobals->theCommandLine->getHelpFileName();
   if (temp) strcpy(itsHelpFilePointer.name, temp);
   else      strcpy(itsHelpFilePointer.name, "suppose");

   itsHelpFilePointer.type[0] = '0';
   itsHelpFilePointer.creator[0] = '0';

   xvt_fsys_save_dir();
   helpInfo = xvt_help_open_helpfile(&itsHelpFilePointer,0L);  // open it

   if (!helpInfo)
   {
      RWCString tmpName = theSpGlobals->theCommandLine->getFVSBIN();
      tmpName += SEPARATOR;
      if (temp) tmpName += temp;
      else      tmpName += "suppose";
      tmpName += ".csc";
      strcpy(itsHelpFilePointer.name, tmpName.data());
      helpInfo = xvt_help_open_helpfile(&itsHelpFilePointer,0L);  // open it
      if (!helpInfo) Warn ("Can't open help file.");
   }

   xvt_fsys_restore_dir();

   // set a print file output name.

#ifdef AIXV4

   char * PrintFileName = "suppose.ps";
   xvt_vobj_set_attr (NULL_WIN,
                      ATTR_PS_PRINT_FILE_NAME,
                      (long) PrintFileName);
   char * PrintCommand  = "lpr -r %s";
   xvt_vobj_set_attr (NULL_WIN,
                      ATTR_PS_PRINT_COMMAND,
                      (long) PrintCommand);
#endif

   // Set the RunStream class.
   if (theSpGlobals->runStream)
      delete theSpGlobals->runStream;
   theSpGlobals->runStream = new RunStream();
   Group *gr = new Group("All");
   theSpGlobals->runStream->AddGroupToSubset(gr);

   BuildWindow();
}




void  CsupposeDoc::BuildWindow(void)
{
   // save the pointer to the document in SpGlobals.

   theSpGlobals->theAppDocument = (PWR_CDocument *) this;

   // Create the default view(s) of this data

   // Instantiate a window of class CsupposeWin with appearance as
   // defined by the arguments to this method.
 
   // This is NOT the window defined in resources.

   // create main window/control panel
   itsSelectionsWindow = new CsupposeWin(this,
                                         AdjWinPlacement(CRect(10, 24, 738, 642)),
                                        "Main",
                                         WSF_SIZE | WSF_ICONIZABLE | WSF_CLOSE);
                                         //WSF_SIZE | WSF_ICONIZABLE | WSF_CLOSE | WSF_MAXIMIZED);

   itsSelectionsWindow->SetId(SELECTIONWindowID);         // set its id

   // make sure this window is activated.

   itsSelectionsWindow->Activate();

   // save a pointer to this window in SpGlobal

   theSpGlobals->theMainWindow = itsSelectionsWindow;

   // update menus (needed for WINDOWS, doesn't hurt Motif).

   UpdateMenus(itsSelectionsWindow->GetMenuBar());
}


void CsupposeDoc::LoadSimFileUsingCommandLine(void)
{
   // if a simulation file name is on the command line, then use it.

   const char * simFileName = theSpGlobals->theCommandLine->getSimFileName();

   if (simFileName)
   {
      xvt_fsys_save_dir();
      if (!itsXVTFilePointer) itsXVTFilePointer = SpGetFilePointer();
      strcpy(itsXVTFilePointer->name,simFileName);

      FILE * filePtr = fopen (itsXVTFilePointer->name,"r");
      xvt_fsys_restore_dir();
      if (!filePtr)
      {
         xvt_dm_post_warning ("Suppose can not open file:\n%s"
               "\nfor input.  Request ignored.", simFileName);
         return;
      }

      if (theSpGlobals->runStream) delete theSpGlobals->runStream;
      theSpGlobals->runStream = new RunStream();

      int code = theSpGlobals->runStream->ReadRun ( filePtr );
      fclose(filePtr);
      if (code)
      {
         xvt_dm_post_error ("An error occurred reading "
                            "an existing simulation file.");
         DoNew();
      }
      else
      {
         char * lastSep = strrchr(itsXVTFilePointer->name,(int) *SEPARATOR);
         if (lastSep)
         {
            char path[SZ_FNAME];
            int nchar = lastSep-itsXVTFilePointer->name;
            strncpy(path,itsXVTFilePointer->name,nchar);
            path[nchar]=0;
            SpChangeDirectory(path);
            xvt_fsys_save_dir();
         }

         SendUpdateMessage(STANDCountChanged,NULL,NULL);
         itsSelectionsWindow->SpUpdateFileName();
         SetSave(FALSE);

         // attempt to set the simulation starting and ending
         // year from the file.

         if (theSpGlobals->runStream->topKeywords)
            theSpGlobals->yearObj->SetStartAndEndFromTopSection(
               theSpGlobals->runStream->topKeywords->data());
      }
   }
}






void CsupposeDoc::DoCommand(long theCommand, void* theData)
{
  // Handle object messages here that have to do with accessing
  // data, or that create, destroy or otherwise manage the
  // window objects of the application.

   switch (theCommand)
   {
      case M_SIM_PREP_SIMULATION:
         DoMenuCommand(M_SIM_PREP_SIMULATION,0,0);			
	   //DoMenuCommand(MENU_TAG theMenuItem, BOOLEAN isShiftKey, BOOLEAN isControlKey)
         break;

      case M_SIM_PREP_TIME:
         DoMenuCommand(M_SIM_PREP_TIME,0,0);
         break;

      case M_SIM_PREP_MODEL:
         DoMenuCommand(M_SIM_PREP_MODEL,0,0);
         break;

      case M_SIM_PREP_MANAGEMENT:
         DoMenuCommand(M_SIM_PREP_MANAGEMENT,0,0);
         break;

      case M_SIM_PREP_MODEL_MOD:
         DoMenuCommand(M_SIM_PREP_MODEL_MOD,0,0);
         break;

      case M_SIM_PREP_USEKEYWORDS:
         DoMenuCommand(M_SIM_PREP_USEKEYWORDS,0,0);
         break;

      case M_SIM_PREP_POST_PROCESSOR:
         DoMenuCommand(M_SIM_PREP_POST_PROCESSOR,0,0);
         break;

      case M_SIM_PREP_RUN:
         DoMenuCommand(M_SIM_PREP_RUN,0,0);
         break;

      case M_AFTER_SIM_REPORTS:
         DoMenuCommand(M_AFTER_SIM_REPORTS,0,0);
         break;

      case M_AFTER_SIM_GRAPHICS:
         DoMenuCommand(M_AFTER_SIM_GRAPHICS,0,0);
         break;

//      case M_HELP_RUNWIZARD:
//         DoMenuCommand(M_HELP_RUNWIZARD,0,0);
//         break;

      case M_SIM_PREP_CURRENT:
         DoMenuCommand(M_SIM_PREP_CURRENT,0,0);
         break;

      case M_AFTER_SIM_READFVSOUTPUT:
         DoMenuCommand(M_AFTER_SIM_READFVSOUTPUT,0,0);
         break;

      case M_AFTER_SIM_AVERAGECASES:
         DoMenuCommand(M_AFTER_SIM_AVERAGECASES,0,0);
         break;

      case M_SIM_PREP_DELETESTAND:
      {
         itsDeleteStandWindow = (DeleteStandWindow *)
            FindWindow(DELETEStandWindowID);

         if (itsDeleteStandWindow)
         {
            G->GetDesktop()->SetFrontWindow(itsDeleteStandWindow);
            itsDeleteStandWindow->DoCommand(10,theData);
         }
         else
         {
            itsDeleteStandWindow = new DeleteStandWindow(this,theData);
            itsDeleteStandWindow->SetId(DELETEStandWindowID);
         }
         break;
      }

      case M_SIM_PREP_DELETEGROUP:
      {
         itsDeleteGroupWindow = (DeleteGroupWindow *)
            FindWindow(DELETEGroupWindowID);
         if (itsDeleteGroupWindow)
         {
            G->GetDesktop()->SetFrontWindow(itsDeleteGroupWindow);
            itsDeleteGroupWindow->DoCommand(10,theData);
         }
         else
         {
            itsDeleteGroupWindow = new DeleteGroupWindow(this,theData);
            itsDeleteGroupWindow->SetId(DELETEGroupWindowID);
         }
         break;

      }

      case M_FILE_QUIT:                   // exit button pushed
         DoMenuCommand(M_FILE_QUIT,0,0);  // shut down using menu command
         break;

      case NULLUpdateMessage:
         break;

      case SENDUpdateCommand:
      {
         CWindow * item = NULL;
         RWOrdered localWindowList = *itsWindows;
         RWOrderedIterator doto(localWindowList);
         while ((item = (CWindow *) doto()) != NULL)
         {
            if (G->GetDesktop()->FindWindow(item))
            {
               // don't send the message to the originating object, if it is set.

               if (item != ((UpdateMessage *) theData)->itsOriginatingWindow)
                  item->DoCommand(((UpdateMessage *) theData)->itsMessageNumber,
                                  ((UpdateMessage *) theData)->itsData);
            }
         }

         // deal with messages the application needs to receive here

         switch (((UpdateMessage *) theData)->itsMessageNumber)
         {
            case FVSCaseListIncreased:
            {
               itsFVSOutputNeedsScaning = 0;
               break;
            }
            case VARIANTSelectionChanged:
            {
               if (theSpGlobals->yearObj)
                  theSpGlobals->yearObj->VariantSelectionChanged();
               break;
            }
            default: break;
         }

         break;

      }

      default:
         CDocument::DoCommand(theCommand, theData);
         break;
   }
}






void CsupposeDoc::DoMenuCommand(MENU_TAG theMenuItem,
                                BOOLEAN isShiftKey, BOOLEAN isControlKey)
{
   xvt_scr_set_busy_cursor();

   switch (theMenuItem)
   {
      case M_FILE_OPEN:
         if (!ResetSuppose(0)) break;
         DoOpen();
         itsSelectionsWindow->SpUpdateFileName();
         break;

      case M_FILE_NEW:
         DoNew();
         break;

      case M_FILE_OPEN_LOC:
         SpSelectSimStandWin(1);
         break;

      case M_FILE_CHANGE_WORKING_DIR:
      {
         FILE_SPEC fileSpec;
         FILE_SPEC *filePtr = &fileSpec;
         filePtr->type[0] = NULL;
         xvt_fsys_get_dir(&filePtr->dir);
         switch (xvt_dm_post_dir_sel(filePtr,"Suppose Working Directory"))
         {
            case FL_OK:
            {
               char path[SZ_FNAME];
               xvt_fsys_convert_dir_to_str(&filePtr->dir,
                                           path, sizeof(path));
               SpChangeDirectory(path);
               xvt_fsys_save_dir();
            }
            case FL_BAD:
            case FL_CANCEL:
              break;
         }
         break;
      }

      case M_SIM_PREP_SIMULATION:
         SpSelectSimStandWin(0);
         break;

     case M_SIM_PREP_TIME:
         SetTimeScale();
         break;

      case M_SIM_PREP_MODEL:
      {
         itsModelOutWindow = (ModelOutWin *) FindWindow(ModelOutWindowID);
         if (itsModelOutWindow)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsModelOutWindow);
         else           // otherwise, create it
         {
            itsModelOutWindow = new ModelOutWin(this);
            itsModelOutWindow->SetId(ModelOutWindowID);
         }
         break;
      }
      case M_SIM_PREP_MANAGEMENT:
         SpManagementWindow();
         break;

      case M_SIM_PREP_MODEL_MOD:
         SpModelModWindow();
         break;

      case M_SIM_PREP_USEKEYWORDS:
      {
                        //  if window exists, display it.
         itsUseFVSKeyWin = (UseFVSKey *) FindWindow(USEFVSKEYWindowID);
         if (itsUseFVSKeyWin)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsUseFVSKeyWin);

         else           // otherwise, create it
         {
            itsUseFVSKeyWin = new UseFVSKey(this);
            itsUseFVSKeyWin->SetId(USEFVSKEYWindowID);
         }
         break;
      }

      case M_SIM_PREP_INSERT_FROM_FILE:
         theSpGlobals->theMainWindow->DoCommand(InsertFromFileCmd);
         break;

      case M_SIM_PREP_POST_PROCESSOR:
         PostProcessorWindow();
         break;

      case M_SIM_PREP_RUN:
         RunSimulation();
         break;

      case M_SIM_PREP_EDIT_SELECTION:
         theSpGlobals->theMainWindow->LaunchComponentWindow();
         break;

      case M_AFTER_SIM_REPORTS:
      {
         if (!theSpGlobals->theReportData)
            theSpGlobals->theReportData = new AllFVSCases (20);

         new GenerateReportWindow(this);

         if (!theSpGlobals->theReportData->entries() ||
             itsFVSOutputNeedsScaning)
         {
            DoMenuCommand(M_AFTER_SIM_READFVSOUTPUT,0,0);
         }
         break;
      }

      case M_AFTER_SIM_GRAPHICS:
      {
         if (!theSpGlobals->theReportData)
            theSpGlobals->theReportData = new AllFVSCases (20);

         new GenerateGraphWindow(this);

         if (!theSpGlobals->theReportData->entries() ||
             itsFVSOutputNeedsScaning)
         {
            DoMenuCommand(M_AFTER_SIM_READFVSOUTPUT,0,0);
         }
         break;
      }

/*      case M_HELP_RUNWIZARD:							//Commented out 05/21/09 AJSM
      {
                        //  if window exists, display it.
         itsRunWizardWin = (RunWizardWin *) FindWindow(RUNWIZWindowID);
         if (itsRunWizardWin)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsRunWizardWin);

         else           // otherwise, create it
         {
            itsRunWizardWin = new RunWizardWin (this);
            itsRunWizardWin->SetId(RUNWIZWindowID);
         }
         break;
      }
*/
      case M_SIM_PREP_CURRENT:
         {
            //  if group membership window exists
            itsGroupMembershipWin = (GroupMembershipWindow *)
                                    FindWindow(GROUPMEMBERSHIPWindowID);
            if (itsGroupMembershipWin)
            {
               // display it
               G->GetDesktop()->SetFrontWindow(itsGroupMembershipWin);
            }
            else
            {
               // otherwise, create it
               itsGroupMembershipWin = (GroupMembershipWindow *)
                                       new GroupMembershipWindow(this);
               itsGroupMembershipWin->SetId(GROUPMEMBERSHIPWindowID);
            }
            break;
         }
                                                   // Extras
      case M_SIM_PREP_VARIANT:
                        //  if window exists, display it.
         itsSelectVariantWin = (SelectVariant *) FindWindow(SELVARWindowID);
         if (itsSelectVariantWin)
            G->GetDesktop()->SetFrontWindow((PWR_CWindow*) itsSelectVariantWin);

         else           // otherwise, create it
         {
            itsSelectVariantWin = new SelectVariant (this);
            itsSelectVariantWin->SetId(SELVARWindowID);
         }
         break;

      case M_SIM_PREP_SIMNOTES:
      {
                        //  if window exists, display it.
         itsSimNotesWindow = (SimNotesWindow *) FindWindow(SIMNOTESWindowID);
         if (itsSimNotesWindow)
            G->GetDesktop()->SetFrontWindow((PWR_CWindow*) itsSimNotesWindow);

         else           // otherwise, create it
         {
            itsSimNotesWindow = new SimNotesWindow(this);
            itsSimNotesWindow->SetId(SIMNOTESWindowID);
         }
         break;
      }

      case M_SIM_PREP_VIEWKEYWORDS:
         xvt_dm_post_message ("No keywords to view from here.");
         break;

      case M_SIM_PREP_DELETESTAND:
         DoCommand(M_SIM_PREP_DELETESTAND,NULL);
         break;

      case M_SIM_PREP_DELETEGROUP:
         DoCommand(M_SIM_PREP_DELETEGROUP,NULL);
         break;

      case M_AFTER_SIM_READFVSOUTPUT:
      {
         // if the report data object is not yet created, then
         // create it (this should not be the case...but just in case).

         if (!theSpGlobals->theReportData)
              theSpGlobals->theReportData = new AllFVSCases (20);

                        //  if window exists, display it.
         itsReadFVSOutputWindow = (ReadFVSOutputWindow *)
                                  FindWindow(READFVSOUTPUTWindowID);
         if (itsReadFVSOutputWindow)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsReadFVSOutputWindow);

         else           // otherwise, create it
         {
            itsReadFVSOutputWindow = new ReadFVSOutputWindow(this);
            itsReadFVSOutputWindow->SetId(READFVSOUTPUTWindowID);
         }
         break;
      }

      case M_AFTER_SIM_AVERAGECASES:
      {
         if (!theSpGlobals->theReportData)
            theSpGlobals->theReportData = new AllFVSCases (20);

         itsAverageFVSOutputWindow = (AverageFVSOutputWindow *)
               FindWindow(AVERAGEFVSOUTPUTWindowID);
         if (itsAverageFVSOutputWindow) G->GetDesktop()->
               SetFrontWindow((CWindow*) itsAverageFVSOutputWindow);
         else
         {
            itsAverageFVSOutputWindow = new AverageFVSOutputWindow(this);
            itsAverageFVSOutputWindow->SetId(AVERAGEFVSOUTPUTWindowID);
         }
         break;
      }

      case M_PREFERENCES_EDIT:
      {
         // The user has asked to open current preferences
         CPrefsWin *aCPrefsWin = new CPrefsWin(this);
         aCPrefsWin->DoModal();
         break;
      }

      case M_DATA_PREP_LOCATIONS:
      {
                        //  if window exists, display it.
         itsEditLocsWindow = (EditLocationWin *) FindWindow(EDITLocsWindowID);
         if (itsEditLocsWindow)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsEditLocsWindow);

         else           // otherwise, create it
         {
            itsEditLocsWindow = new EditLocationWin (this);
            itsEditLocsWindow->SetId(EDITLocsWindowID);
         }
         break;
      }

      case M_DATA_PREP_STANDLIST:
      {
                        //  if window exists, display it.
         itsEditSLFWindow = (EditSLFWin *) FindWindow(EDITSLFWindowID);
         if (itsEditSLFWindow)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsEditSLFWindow);

         else           // otherwise, create it
         {
            itsEditSLFWindow = new EditSLFWin (this);
            itsEditSLFWindow->SetId(EDITSLFWindowID);
         }
         break;
      }

      case M_DATA_PREP_FVSTREEDATA:
      {
                        //  if window exists, display it.
         itsEditFVSTreeDataWindow = (EditFVSTreeDataWin *)
            FindWindow(EDITFVSTreeDataWindowID);
         if (itsEditFVSTreeDataWindow) G->GetDesktop()->
            SetFrontWindow((CWindow*) itsEditFVSTreeDataWindow);

         else           // otherwise, create it
         {
            itsEditFVSTreeDataWindow = new EditFVSTreeDataWin (this);
            itsEditFVSTreeDataWindow->SetId(EDITFVSTreeDataWindowID);
         }
         break;
      }

      case M_HELP_SUPPOSE_TUTORIAL:
      {
         ASK_RESPONSE aResponse;
         aResponse = xvt_dm_post_ask("Ok","Help","Cancel",
                                     "May Suppose install tutorial data in "
                                     "your working directory?");
         switch ( aResponse )
         {
            case RESP_DEFAULT:
            {
               // Make sure we are starting with a new file.

               if (!DoNew())
               {
                  // if do new did not work, then Display info on
                  // running the tutorial.
                  xvt_help_display_topic(helpInfo, RUNTUTORIAL);
                  break;
               }

               // Set up tutorial data, and set pointer to locations file.
               // if it worked, then display the main help.

               if (SetUpTutorialData()) xvt_help_display_topic(helpInfo,
                                                               XVT_TPC_TUTORIAL);
               else xvt_dm_post_warning ("Set up of tutorial data failed.");
               break;
            }
            case RESP_2:
               xvt_help_display_topic(helpInfo, RUNTUTORIAL);
               break;

            case RESP_3:
               break;
         }
      }

      default:
        {
          if ((theMenuItem >= M_HELP_DYNAMIC_ITEMS_BASE) && (theMenuItem <= M_HELP_DYNAMIC_ITEMS_MAX))
          {
            xvt_scr_launch_browser(((RWCollectableString*)itsDynamicHelpWebLinks[theMenuItem - M_HELP_DYNAMIC_ITEMS_BASE])->data());
          }
          else
          {
            CDocument::DoMenuCommand(theMenuItem, isShiftKey, isControlKey);
          }
        }
        break;
   }
}





FILE_SPEC* CsupposeDoc::SpGetFilePointer(void)
{

   // if there is no current data file
   if (!itsXVTFilePointer)
   {
      // create a structure, fill it

      // itsXVTFilePointer is owned/managed by the base class -- do not delete
      itsXVTFilePointer = new FILE_SPEC;	  
	  
      strcpy(itsXVTFilePointer->name,NEWFILE);

      // set filetype (default is "key")

      strcpy(itsXVTFilePointer->type,"key");

      // use default directory from the pref file
	  DIRECTORY prefDir;
	  std::string sPrefDir = theSpGlobals->ReadPrefString("General Preferences", "defaultWorkingDirectory");
	  xvt_fsys_convert_str_to_dir((char*)sPrefDir.data(), &prefDir);
	  memcpy(&(itsXVTFilePointer->dir), &prefDir, sizeof(DIRECTORY));

	  //set the current directory to the one retreived.
	  xvt_fsys_set_dir(&itsXVTFilePointer->dir);
   }
   return itsXVTFilePointer;
}






BOOLEAN CsupposeDoc::DoNew(void)
{

   if (!ResetSuppose(0)) return FALSE;

   // busy the cursor.

   xvt_scr_set_busy_cursor();

   // reset the file name.

   if (itsXVTFilePointer) strcpy(itsXVTFilePointer->name,NEWFILE);

   // reset the RunStream class.

   if (theSpGlobals->runStream) delete theSpGlobals->runStream;
   theSpGlobals->runStream = new RunStream();

   Group *all = new Group("All");
   theSpGlobals->runStream->AddGroupToSubset(all);

   // Update file name and notify that the stand changed.

   SendUpdateMessage(CURRStandOrGroupChange, all);
   itsSelectionsWindow->SpUpdateFileName();
   itsSelectionsWindow->SpUpdateStandCount();

   SetSave(FALSE);

   return TRUE;
}






BOOLEAN CsupposeDoc::DoOpen()
{
         // This override sets a mask for the file type in the
         // open and save file dialog.
         // Also override DOES NOT call BuildWindow

   itsXVTFilePointer = SpGetFilePointer();

                  // with no name
   itsXVTFilePointer->name[0] = 0;
   strcpy(itsXVTFilePointer->type,"key");
   // use default directory
   xvt_fsys_get_dir(&itsXVTFilePointer->dir);

   // bring up a dialog to ask for file

   xvt_fsys_save_dir();
   switch (xvt_dm_post_file_open(itsXVTFilePointer, "Simulation file:"))
   {
      case FL_OK:
      {
         FILE * filePtr = fopen (itsXVTFilePointer->name,"r");
         if (!filePtr)
         {
            xvt_dm_post_warning ("Suppose can not open file:\n%s"
                                 "\nfor output.  Request ignored.",
                                 itsXVTFilePointer->name);
            xvt_fsys_restore_dir();
            return DoOpen();
         }

         if (theSpGlobals->runStream) delete theSpGlobals->runStream;
         theSpGlobals->runStream = new RunStream();

         xvt_scr_set_busy_cursor();

         int code = theSpGlobals->runStream->ReadRun ( filePtr );
         fclose(filePtr);
         if (code)
         {
            xvt_dm_post_warning ("An error occurred reading "
                                 "an existing simulation file.");
            xvt_fsys_restore_dir();
            DoNew();
         }
         else
         {
            // if it was successful, then save the dir as current.

            xvt_fsys_save_dir();

            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&itsXVTFilePointer->dir,
                                        path, sizeof(path));
            SpChangeDirectory(path);

            itsSelectionsWindow->SpUpdateStandCount();
            itsSelectionsWindow->SpUpdateFileName();
            itsSelectionsWindow->DoCommand(GROUPMembershipChanged);

            // attempt to set the simulation starting and
            // ending year from the file.

            if (theSpGlobals->runStream->topKeywords)
               theSpGlobals->yearObj->SetStartAndEndFromTopSection(
               theSpGlobals->runStream->topKeywords->data());
         }
         break;
      }
      case FL_BAD:
      case FL_CANCEL:
         break;
   }
   xvt_fsys_restore_dir();

   return FALSE;
}






void CsupposeDoc::UpdateMenus(CMenuBar* theMenuBar)
{
   if (theMenuBar)
   {
      // Set the Save menu item according the the needs of the document:
      theMenuBar->SetEnabled( M_FILE_SAVE, TRUE );
      theMenuBar->SetEnabled( M_FILE_SAVE_AS, TRUE );
      theMenuBar->SetEnabled( M_FILE_OPEN, TRUE );
      theMenuBar->SetEnabled( M_FILE_NEW, TRUE);

      // Set other menu items
      theMenuBar->SetEnabled( M_DATA_PREP, TRUE);
      theMenuBar->SetEnabled( M_SIM_PREP, TRUE);
      theMenuBar->SetEnabled( M_AFTER_SIM, TRUE);
      theMenuBar->SetEnabled( M_PREFERENCES, TRUE);
   }
}





void CsupposeDoc::SpSelectSimStandWin(int autoPrompt)
{

   itsSelectSimStandWin = (SelectSimStandWin *)
      FindWindow(SELECTSIMSTANDWindowID);
   if (itsSelectSimStandWin)
   {
      if (autoPrompt)
      {
         itsSelectSimStandWin->Close();
         itsSelectSimStandWin=NULL;
      }
      else
      {
         G->GetDesktop()->SetFrontWindow(itsSelectSimStandWin);
         return;
      }
   }

   // create an XVT file spec and load it with the default file
   // name, type, and current directory.

   if (!locFilePointer)
   {
      locFilePointer = new FILE_SPEC;
      strcpy(locFilePointer->name,
             theSpGlobals->thePreferences->defaultLocationsFileName);
      strcpy(locFilePointer->type,"loc");
      xvt_fsys_get_dir(&locFilePointer->dir);
   }

   // attempt to open the file.  If it can be opened, then we will
   // use it...if not...otherwise...

   FILE * locFile = NULL;

   ASK_RESPONSE aResp = RESP_DEFAULT;

   if (!autoPrompt)
   {
      locFile = fopen (locFilePointer->name,"r");
      if (!locFile)
      {
         aResp = xvt_dm_post_ask("Browse",
                                 "Create",
                                 "Cancel",
                                 "Suppose can not open locations file: %s",
                                 locFilePointer->name);
      }
   }

   if (!locFile)
   {
      switch (aResp)
      {
         case RESP_DEFAULT:  // Browse
         {
            xvt_fsys_save_dir();
            strcpy(locFilePointer->type,"loc");
            switch (xvt_dm_post_file_open(locFilePointer,
                                          "Locations file:"))
            {
               case FL_OK:
               {
                  locFile = fopen (locFilePointer->name,"r");
                  if (locFile)
                  {
                     // if the locations file is now opened, then
                     // set the current directory as the "default".

                     char path[SZ_FNAME];
                     xvt_fsys_convert_dir_to_str(&locFilePointer->dir,
                                                 path, sizeof(path));
                     SpChangeDirectory(path);
                     xvt_fsys_save_dir();
                  }
                  else xvt_dm_post_warning ("Can not open file: %s",
                                            locFilePointer->name);
               }
               case FL_BAD:
               case FL_CANCEL:
                 break;
            }
            xvt_fsys_restore_dir();
            break;
         }
         case RESP_2:        // Create
         {
            strcpy(locFilePointer->name,theSpGlobals->thePreferences->defaultLocationsFileName);
            strcpy(locFilePointer->type,"loc");
            xvt_fsys_get_dir(&locFilePointer->dir);
            locFile = fopen (locFilePointer->name,"w");
            if (locFile)
            {
               // find out if a data base of the default name exists

               FILE_SPEC aFS;

               xvt_str_copy(aFS.name, theSpGlobals->thePreferences->defaultDataBaseName);
               xvt_str_copy(aFS.type, "");  // extension not needed in this case...
               xvt_fsys_get_dir(&aFS.dir);

               int dbsExists = xvt_fsys_get_file_attr(&aFS, XVT_FILE_ATTR_EXIST);

               if (dbsExists)
               {
                  fprintf (locFile,"C \"Database\" %s \n",
                     theSpGlobals->thePreferences->defaultDataBaseName.data());
                  fclose(locFile);
               }
               else
               {
                  fprintf (locFile,"A \"Default Bare Ground\" "
                           "@ baregrnd.slf @ @\n");
                  fclose(locFile);

                  // create the slf file unless it already exists.


                  locFile = fopen ("baregrnd.slf","r");
                  if (locFile) fclose (locFile);
                  else
                  {
                     locFile = fopen ("baregrnd.slf","w");
                     if (locFile)
                     {
                        const char * bareGrnd = "BareGrnd";
                        fprintf (locFile,
                                 "A %s @ 0 %s @\nB %s @\nC %s Variant=%s @\n",
                                 bareGrnd,
                                 theSpGlobals->theVariants->GetSelectedVariant(),
                                 bareGrnd, bareGrnd,
                                 theSpGlobals->theVariants->GetSelectedVariant());
                        fclose(locFile);
                     }
                     else xvt_dm_post_warning ("Can not open file %s for output.",
                                             "baregrnd.slf");
                  }
               }

               // reopen the locations file for input.

               locFile = fopen (locFilePointer->name,"r");
            }
            else xvt_dm_post_warning ("Can not open file %s for output.",
                                      locFilePointer->name);
         }
         case RESP_3:        // Cancel
         {
            break;
         }
      }
   }

   if (locFile)
   {
      itsSelectSimStandWin = new SelectSimStandWin(this, locFilePointer,
                                                   locFile);
      itsSelectSimStandWin->SetId(SELECTSIMSTANDWindowID);
   }
}




void CsupposeDoc::SpManagementWindow()
{
                        //  if management window exists
   itsManagementWin = (CMngWin*)FindWindow(ManagementWindowID);
   if (itsManagementWin)
   {
                        // display it
      G->GetDesktop()->SetFrontWindow(itsManagementWin);
      
   }
   else{
                        // otherwise, create it
	   itsManagementWin = new CMngWin(this);

	   itsManagementWin->SetId(ManagementWindowID);
   }
}


void CsupposeDoc::SpModelModWindow()
{
                        //  if model modifiers window exists
   itsModelModWin = (CModelModWin*)FindWindow(ModelModWindowID);
   if (itsModelModWin)
   {
                        // display it
      G->GetDesktop()->SetFrontWindow(itsModelModWin);
      return;
   }
                        // otherwise, create it
   itsModelModWin = new CModelModWin(this);

   itsModelModWin->SetId(ModelModWindowID);
}



void CsupposeDoc::PostProcessorWindow()
{

   itsPostProcessorWin = (PostProcWin *) FindWindow(POSTPROCWindowID);
   if (itsPostProcessorWin)
   {
      G->GetDesktop()->SetFrontWindow(itsPostProcessorWin);
      return;
   }
   itsPostProcessorWin = new PostProcWin(this);
   itsPostProcessorWin->SetId(POSTPROCWindowID);
}





BOOLEAN CsupposeDoc::DoClose(void)
{

   if (!ResetSuppose(1)) return FALSE;

   // delete the Suppose Global class.  This will delete the other global
   // classes that do not inherent XVT windows.

   if (locFilePointer) delete locFilePointer;

   delete theSpGlobals;

   xvt_app_destroy();  // this function will never return.
   return TRUE; //do not call the CDocument DoClose.
}





BOOLEAN CsupposeDoc::ResetSuppose( int partial )
{
   if ( NeedsSaving() && theSpGlobals->runStream->allStands->entries())
   {
      ASK_RESPONSE aResponse;
      aResponse = xvt_dm_post_ask("Save","Discard","Cancel",
                                  "Simulation file has changed, save or discard?");
      switch ( aResponse )
      {
         case RESP_2:
            SetSave(FALSE);    // discard savings
            break;
         case RESP_3:
            return FALSE;      // canceled so return
         default:
            DoSave();
            break;
      }
   }

   // make sure that all of the SchByCond windows are closed before
   // we close other open windows.  The SchByCond windows are linked
   // to their parents...and we get a NULL pointer error
   // if these guys aren't gone first.

   for (   SchByCond * aSchByCondWin = (SchByCond *) FindWindow(SCHBYCONDWindowID);
        aSchByCondWin; aSchByCondWin = (SchByCond *) FindWindow(SCHBYCONDWindowID))
   {
      aSchByCondWin->Close();
      RemoveWindow((CWindow *) aSchByCondWin);
   }

   // find the groupmembership window...issue a close to it if it is open.
   itsGroupMembershipWin = (GroupMembershipWindow *)
                           FindWindow(GROUPMEMBERSHIPWindowID);
   if (itsGroupMembershipWin)
   {
      itsGroupMembershipWin->Close();
      RemoveWindow((CWindow *) itsGroupMembershipWin);
   }

   // find itsRunWizardWin window.

   itsRunWizardWin = (RunWizardWin *) FindWindow(RUNWIZWindowID);

   // close the rest of the windows (other than the SELECTIONWindow
   // and the RunWizard window, if it is opened).

   int ndelete = 1;
   while (ndelete)
   {
      ndelete = 0;
      RWOrderedIterator nextWindow(*GetWindows());
      CWindow * aWindow;
      while (aWindow = (CWindow *) nextWindow())
      {
         if (aWindow != (CWindow *) itsSelectionsWindow &&
             aWindow != (CWindow *) itsRunWizardWin && aWindow != NULL)
         {
            ndelete++;
            aWindow->Close();
            RemoveWindow(aWindow);
         }
      }
   }

   // Make sure that the location file pointer is deleted and set to null.

   if (locFilePointer)
   {
      delete locFilePointer;
      locFilePointer = NULL;
   }

   // if this is a partial call to reset (call is made from DoClose) then
   // issue a close to the runWizard and return.

   if (partial)
   {
      if (itsRunWizardWin)
      {
         itsRunWizardWin->Close();
         RemoveWindow(itsRunWizardWin);
      }
      return TRUE;
   }

   // create and/or reset the preferences object.

   if (theSpGlobals->thePreferences) 
	   delete theSpGlobals->thePreferences;
   theSpGlobals->thePreferences = new SpPreferences();

   // set the default preferences from the command line.

   theSpGlobals->thePreferences->upDateDefaultsFromCommandLine();

   // updates default parameters file
   theSpGlobals->thePreferences->upDateDefaultsFromPrefFile();				
   
   // create and/or reset the parameters data.

   if (theSpGlobals->theParmsData) 
	   delete theSpGlobals->theParmsData;
   theSpGlobals->theParmsData = new SpParms (theSpGlobals->thePreferences->defaultParametersFileName);

   // create the commands object.

   if (theSpGlobals->theCommands) delete theSpGlobals->theCommands;
   theSpGlobals->theCommands = new SpCommands();

   // create and/or reset the programs object.

   if (theSpGlobals->thePrograms) delete theSpGlobals->thePrograms;
   theSpGlobals->thePrograms = new SpPrograms();

   // update the defaults from the parameters file.

   theSpGlobals->thePreferences->upDateDefaultsFromParmsFile();

   // update the default preferences from the preference file.

   theSpGlobals->thePreferences->upDateDefaultsFromPrefFile();

   // update (a second time) the default preferences from the command line.

   theSpGlobals->thePreferences->upDateDefaultsFromCommandLine();

   SpChangeDirectory(theSpGlobals->thePreferences->defaultWorkingDirectory.data());



   // delete the year object prior to creating the variants object
   // this is done here because the year object normally responds
   // to the variant selection code.

   if (theSpGlobals->yearObj)
   {
      delete theSpGlobals->yearObj;
      theSpGlobals->yearObj = 0;
   }

   // create and/or reset the variant data.

   if (theSpGlobals->theVariants) delete theSpGlobals->theVariants;
   theSpGlobals->theVariants = new SpVariants();

   // create and/or reset the extensions data.

   if (theSpGlobals->theExtensions) delete theSpGlobals->theExtensions;
   theSpGlobals->theExtensions = new SpExtensions();

   // pick/repick a program, given the variant and extensions data.
   // this call includes picking a command.

   theSpGlobals->thePrograms->PickProgram(1);

   // create and/or reset up the species data.

   if (theSpGlobals->theSpecies) delete theSpGlobals->theSpecies;
   theSpGlobals->theSpecies = new SpSpecies();

   // create and/or reset up the habitat/plant association data.

   if (theSpGlobals->theHabPa) delete theSpGlobals->theHabPa;
   theSpGlobals->theHabPa = new HabPa();

   // create and/or reset up the forest code data.

   if (theSpGlobals->theForests) delete theSpGlobals->theForests;
   theSpGlobals->theForests = new Forests();

   // create the year object

   theSpGlobals->yearObj = new YearObj();

   // Notify the run wizard of the reset.

   if (itsRunWizardWin) itsRunWizardWin->DoCommand(SUPPOSEResetCommand, NULL);

   return TRUE;
}





BOOLEAN CsupposeDoc::DoSave(void)
{

   if (!theSpGlobals->runStream->allStands->entries())
   {
      xvt_dm_post_note("No stands in the simulation.");
      return FALSE;
   }

   /*------------------------  DoSave -----------------------------------*/

                           // blank out name if dummies show on control panel
   if (strcmp(itsXVTFilePointer->name,NEWFILE) == 0)
         itsXVTFilePointer->name[0] = 0;

                           // save the file if there is file object and name
   if (itsXVTFilePointer && itsXVTFilePointer->name[0])
   {
      FILE * filePtr = fopen (itsXVTFilePointer->name,"w");
      if (!filePtr)
      {
         CStringRW tt = "Suppose can not open file:\n";
         tt += itsXVTFilePointer->name;
         tt += "\nfor output.  Request ignored.";
         Warn (tt.data());
         return DoSaveAs();
      }

                           // sets the saving flag to false; no multiple saves
      SetSave(FALSE);

      xvt_scr_set_busy_cursor();

      CStringRW * top = new CStringRW((RWSize_T) 200);

      *top =  "Comment";
      *top += "\nStarting year for simulation is ";
      *top += ConvertToString(theSpGlobals->yearObj->GetStartYear()).data();
      *top += "\nEnding year for simulation is   ";
      *top += ConvertToString(theSpGlobals->yearObj->GetEndYear()).data();
      *top += "\nMin and Max inventory years are ";
      *top += ConvertToString(theSpGlobals->yearObj->GetInvYear(1)).data();
      *top += " ";
      *top += ConvertToString(theSpGlobals->yearObj->GetInvYear(2)).data();
      *top += "\nCommon cycle length is          ";
      *top += ConvertToString(theSpGlobals->yearObj->GetCycleLength()).data();
      *top += "\nEnd";

      if (theSpGlobals->runStream->topKeywords)
         delete theSpGlobals->runStream->topKeywords;
      theSpGlobals->runStream->topKeywords = top;

      // Set the timing for all stands in the simulation.

      SetTimingKeywordsForAllStands();

      // write the simulation file

      theSpGlobals->runStream->WriteRun( filePtr );
      fclose(filePtr);

                           // update the filename on the control panel
      itsSelectionsWindow->SpUpdateFileName();
      return TRUE;
   }

                           // no file object
   else
      return DoSaveAs();
}





BOOLEAN CsupposeDoc::DoSaveAs(void)
{


   /*------------------------  DoSaveAs  -----------------------------------*/

   if (!theSpGlobals->runStream->allStands->entries())
   {
      xvt_dm_post_note("No stands in the simulation.");
      return FALSE;
   }

   itsXVTFilePointer = SpGetFilePointer();
   itsXVTFilePointer->name[0] = 0;
   // set filetype (default is "key")
   strcpy(itsXVTFilePointer->type,"key");
   // use default directory
   xvt_fsys_get_dir(&itsXVTFilePointer->dir);

   // bring up a dialog to ask for file
   xvt_fsys_save_dir();

                                        // get file name; process it
   switch (xvt_dm_post_file_save(itsXVTFilePointer, "Save As:"))
   {
      case FL_OK:
      {
         BOOLEAN t;
         t = DoSave();
         xvt_fsys_restore_dir();
         return t;
      }
      case FL_BAD:
      case FL_CANCEL:
         xvt_fsys_restore_dir();
         strcpy(itsXVTFilePointer->name,NEWFILE);
         return FALSE;
   }
   return FALSE;
}





void CsupposeDoc::SetTimingKeywordsForAllStands( void )
{
   char buffer[100];
   const char * invYearIsAt;

   // define the cycle length.  In the future, we will have to insure
   // that we account for all cycles.

   int cycleLen = theSpGlobals->yearObj->GetCycleLength();

   // get the default cycle length for the variant.

   PKeyData * cycleLenPKeyData = GetPKeyData("timing", "cycleLength");
   int defaultCycleLen = 10;
   if (cycleLenPKeyData) sscanf (cycleLenPKeyData->pString.data(),
                                    "%d", &defaultCycleLen);
   // compute the base number of cycles.

   int endSim   = theSpGlobals->yearObj->GetEndYear();
   int startSim = theSpGlobals->yearObj->GetStartYear();
   int baseNumberCycles = (endSim-startSim)/cycleLen;
   if ((endSim-startSim)%cycleLen) baseNumberCycles++;

   // open an iterator over all stands.

   RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
   Stand * stand;
   while ((stand = (Stand *) nextStand()) != NULL)
   {
      char * buffptr = buffer;
      invYearIsAt = NULL;
      if (stand->siteKeywords)
         invYearIsAt = strstr (stand->siteKeywords->data(),"InvYear  ");
      else
      {
         xvt_dm_post_error("Stand %s does not have an InvYear keyword.",
                           stand->data());
         continue;
      }

      // point to the location following the InvYear string.  It
      // will be the inventory year.  Convert to integer.

      invYearIsAt += 10;
      int standInvYear = (int) ConvertToLong(invYearIsAt);

      if (cycleLen != defaultCycleLen ) buffptr += sprintf (buffptr,
                                          "TimeInt%23d\n", cycleLen);

      int standNumberCycles = baseNumberCycles;

      if (startSim != standInvYear)
      {
         if (theSpGlobals->thePreferences->defaultSegmentCycle == 1) //ajsm
		 {
			 int yearsToSpan = startSim-standInvYear;
			 int newCycles   = yearsToSpan/cycleLen;
			 int shortCycle  = yearsToSpan%cycleLen;
			 if (shortCycle)
			 {
				buffptr += sprintf (buffptr,"TimeInt            1%10d\n",
									shortCycle);
				standNumberCycles++;
			 }
			 standNumberCycles = standNumberCycles+newCycles; 
		 }
		 else
		 {
			 buffptr += sprintf (buffptr,"TimeInt            1%10d\n",
								startSim-standInvYear);
			 standNumberCycles++;
		 }
      }

      if (standNumberCycles < 1) standNumberCycles = 1;

      // Write the NumCycle and keyword.

      buffptr += sprintf (buffptr,"NumCycle%12d",
                         standNumberCycles);
      // set/reset the stand's timing keywords to the buffer contents.

      if (stand->timingKeywords) *stand->timingKeywords = buffer;
      else stand->timingKeywords = new CStringRW(buffer);
   }
}


void CsupposeDoc::RunSimulation( void )
{

   if (theSpGlobals->runStream->allStands->entries())
   {
      if (strcmp((const char *) SpGetFilePointer()->name,NEWFILE) == 0)
      {
         if (!DoSaveAs()) return;
      }

      if (NeedsSaving())
      {
#if XVTWS==WIN32WS
         switch ( xvt_dm_post_ask("Save","Save ~As","Cancel",
#else
         switch ( xvt_dm_post_ask("Save","Save As","Cancel",
#endif
                  "Simulation has changed, save before running?"))
         {
            case RESP_DEFAULT:
            {
               DoSave();
               break;
            }
            case RESP_2:
            {
               if (!DoSaveAs()) return;
               break;
            }
            default:
               return;
         }
      }

      // Build the simulation script, write it, and give a last chance.

      const char * program = theSpGlobals->thePrograms->GetProgram();
      const char * command = theSpGlobals->theCommands->GetCommand();
      const char * theSimFileName = (const char *) SpGetFilePointer()->name;

      if (command && program && theSimFileName &&
          strlen(command) && strlen(program) && strlen(theSimFileName))
      {
         CStringRW commandString = command;

         // if there are post processors, append their commands.

         if (theSpGlobals->runStream->includedPostProcs->entries())
         {
            RWOrderedIterator nextIncludedPostProc(
               *theSpGlobals->runStream->includedPostProcs);
            PostProc * inclPostProc;
            while (inclPostProc = (PostProc *) nextIncludedPostProc())
            {
               commandString.append("\n ");
               commandString.append(*inclPostProc->command);
            }
            commandString.append("\n");
         }

         // edit in the program name, the run name, and the editor name
         // in the command string.

         char pgm[] = "!program!";
         int  pgmlen = strlen(pgm);
         char run[] = "!run!";
         int  runlen = strlen(run);
         char editor[] = "!editor!";
         int  editorlen = strlen(editor);
         char fvsbin[] = "!fvsbin!";
         int  fvsbinlen = strlen(fvsbin);
         size_t place;

         while ((place = commandString.index(pgm, pgmlen, 0,
                                             RWCString::ignoreCase))
                != RW_NPOS)
            commandString.replace(place,   pgmlen,
                                  program, strlen(program));

         place = commandString.index(run, runlen,
                                     0, RWCString::ignoreCase);

         if (place != RW_NPOS)
         {
            int simFileLen;
            const char * point = strrchr(theSimFileName,'.');
            if (point) simFileLen = point-theSimFileName;
            else       simFileLen = strlen(theSimFileName);

            if (simFileLen < 1)
            {
               xvt_dm_post_error("Simulation file name error.");
               return;
            }

            while (place != RW_NPOS)
            {
               commandString.replace(place,          runlen,
                                     theSimFileName, simFileLen);
               place = commandString.index(run, runlen, 0,
                                           RWCString::ignoreCase);
            }
         }

         place = commandString.index(editor, editorlen,
                                     0, RWCString::ignoreCase);

         if (place != RW_NPOS)
         {
            const char * defEd = theSpGlobals->thePreferences->defaultEditor;
            int edLen = 0;
            if (defEd) edLen = strlen(defEd);
            if (edLen)
            {
               while (place != RW_NPOS)
               {
                  commandString.replace(place, editorlen, defEd, edLen);
                  place = commandString.index(editor, editorlen, 0,
                                              RWCString::ignoreCase);
               }
            }
         }

         const char * replaceFVSBIN =
               theSpGlobals->theCommandLine->getFVSBIN();
         if (!replaceFVSBIN) replaceFVSBIN = "";

         while ((place = commandString.index(fvsbin, fvsbinlen, 0,
                                             RWCString::ignoreCase))
                != RW_NPOS)
            commandString.replace(place,
                                  fvsbinlen,
                                  replaceFVSBIN,
                                  strlen(replaceFVSBIN));

         // set up the shell command.

#if XVTWS==WIN32WS
         CStringRW shellCommand = theSimFileName;
#else
         CStringRW shellCommand = "ksh < ";
         shellCommand.append(theSimFileName);
#endif

         place = shellCommand.last((char) '.');
         if (place) shellCommand.remove(place);

#if XVTWS==WIN32WS
         shellCommand.append(".bat");
#else
         shellCommand.append(".sh &");
#endif

         CStringRW tmpString = theSimFileName;
         place = tmpString.last((char) '.');
         if (place) tmpString.remove(place);

#if XVTWS==WIN32WS
         tmpString.append(".bat");
#else
         tmpString.append(".sh");
#endif

         FILE * filePtr = fopen (tmpString.data(),"w");
         if (filePtr)
         {
            fputs(commandString.data(), filePtr);
            fclose(filePtr);
            if (xvt_dm_post_ask("Run","Cancel", NULL,
                                "Run command: %s",
                                shellCommand.data()) == RESP_DEFAULT)
            {
               OpSys(shellCommand.data());
               itsFVSOutputNeedsScaning = 1;
            }
         }
         else xvt_dm_post_error("Can not open %s", tmpString.data());
      }
      else xvt_dm_post_error("No command available.");
   }
   else xvt_dm_post_note("No stands in the simulation.");
}



BOOLEAN CsupposeDoc::SetUpTutorialData(void)
{
   const char * tutorial = "tutorial";
   const char * tmp = NULL;
   FILE * filePtr = NULL;
   RWOrdered * theFiles = NULL;
   BOOLEAN returnValue;
   PKeyData * tutorialFileList = NULL;

   PKeyData * locationsFileContents =
              GetPKeyData(tutorial,"locationsFileContents",
                          (const char *) theSpGlobals->thePreferences->
                          defaultVariant,0,0);

   if (locationsFileContents)
   {
      filePtr = fopen (theSpGlobals->thePreferences->
                       tutorialLocationsFileName,"w");

      if (filePtr)
      {
         tmp = locationsFileContents->pString.data();
         fputs(tmp, filePtr);
         fclose(filePtr);
      }
      else goto FAIL;
   }
   else goto FAIL;

   tutorialFileList = GetPKeyData(tutorial,"tutorialFileList",
                                  (const char *) theSpGlobals->thePreferences->
                                  defaultVariant,0,0);

   if (tutorialFileList)
   {
      theFiles = new RWOrdered((size_t) 20);
      LoadRWOrdered(tutorialFileList->pString.data(),theFiles);
      if (theFiles->entries())
      {
         RWOrderedIterator nextFile(*theFiles);
         CStringRWC * fileName;
         while (fileName = (CStringRWC *) nextFile())
         {
            PKeyData * thePKey =
                       GetPKeyData(tutorial,
                                   fileName->data(),
                                   NULL,0,0);
            if (thePKey)
            {
               filePtr = fopen (fileName->data(),"w");
               if (filePtr)
               {
                  tmp = thePKey->pString.data();
                  fputs(tmp, filePtr);
                  fclose(filePtr);
               }
               else goto FAIL;
            }
            else goto FAIL;
         }
      }
      else goto FAIL;
   }
   else goto FAIL;

   theSpGlobals->thePreferences->defaultLocationsFileName=
      theSpGlobals->thePreferences->tutorialLocationsFileName;
   returnValue = TRUE;
   goto FREE;

   FAIL:
   returnValue = FALSE;

   FREE:
   if (theFiles)
   {
      theFiles->clearAndDestroy();
      delete theFiles;
   }
   FreePKeyData(tutorial);
   return returnValue;
}


