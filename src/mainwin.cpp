/********************************************************************

   File Name:     mainwin.cpp
   Author:        kfd & nlc (05/08/98) & pg & nlc 10/2006
   Date:          09/14/94

  Contents:      CSupposeWin()
                 CsupposeWin::IsupposeWin
                 CsupposeWin::DoCommand
                 CsupposeWin::DoMenuCommand
                 CsupposeWin::SpUpdateFileName
                 CsupposeWin::SpUpdateStandCount
                 CsupposeWin::Key
                 ...

********************************************************************/

#include "appdef.hpp"

#include CDesktop_i
#include CImage_i
#include CMenuBar_i
#include CNavigator_i
#include CRadioGroup_i
#include CStringCollection_i
#include CStringRWC_i
#include CTaskWin_i
#include CTreeEvent_i
#include CWindow_i
#include NButton_i
#include NGroupBox_i
#include NListBox_i
#include NRadioNavigator_i
#include NText_i

#include "mainwin.hpp"
#include "appdoc.hpp"
#include "clearwin.hpp"
#include "compon.hpp"
#include "dirsept.hpp"
#include "express.hpp"
#include "gpkey.hpp"
#include "group.hpp"
#include "msgwin.hpp"
#include "plantwin.hpp"
#include "readcomp.hpp"
#include "runstr.hpp"
#include "schcond.hpp"
#include "seedwin.hpp"
#include "selectsi.hpp"
#include "sendupdt.hpp"
#include "sheltwin.hpp"
#include "spfunc2.hpp"
#include "spglobal.hpp"
#include "stand.hpp"
#include "suppdefs.hpp"
#include "supphelp.hpp"
#include "thinwin.hpp"
#include "viewkey.hpp"
#include "warn_dlg.hpp"


extern XVT_HELP_INFO helpInfo;    // help file

// Define the Run-Time Type Identification for this class
PWRRegisterClass1(CsupposeWin, CsupposeWinID, "CsupposeWin", CWindow)

CsupposeWin::CsupposeWin(CDocument *theDocument,
                         const CRect& theRegion, const CStringRW& theTitle,
                         long theAttributes, WIN_TYPE theWindowType,
                         int theMenuBar)
   : CWindow(theDocument, theRegion, theTitle, theAttributes, theWindowType, theMenuBar),
     itsCurrentComponent(NULL),
     itsCurrentGroup(NULL),
     itsCurrentStand(NULL),
     itsSource(NULL)
{
   AddDynamicHelpItems(GetMenuBar());

   // create and clear the navitagor
   itsNavigator = GetNavigator();
   itsNavigator->ClearTabStops();

   //  construct all the controls for the screen and set the command to be issued when selected
   itsSimPrepGrpBox = new NGroupBox(this, CRect(6, 6, 588, 100), "Simulation Preparation", 0L);
   itsSimPrepGrpBox->SetGlue(TOPSTICKY | LEFTSTICKY );

   itsSelectSimStand = new NButton(this, CRect(12, 24, 140, 56), "Select Stands");
   itsSelectSimStand->SetCommand(M_SIM_PREP_SIMULATION);
   itsSelectSimStand->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsSelectSimStand));

   itsSetTime = new NButton(this, CRect(146, 24, 274, 56), "Set Time Scale");
   itsSetTime->SetCommand(M_SIM_PREP_TIME);
   itsSetTime->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsSetTime));

   itsManagement = new NButton(this, CRect(280, 24, 448, 56), "Select Management");
   itsManagement->SetCommand(M_SIM_PREP_MANAGEMENT);
   itsManagement->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsManagement));

   itsSelectModelOut = new NButton(this, CRect(454, 24, 582, 56), "Select Outputs");
   itsSelectModelOut->SetCommand(M_SIM_PREP_MODEL);
   itsSelectModelOut->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsSelectModelOut));

   itsRunSimulation = new NButton(this, CRect(594, 24, 722, 56), "Run Simulation");
   itsRunSimulation->SetCommand(M_SIM_PREP_RUN);
   itsRunSimulation->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsRunSimulation));

   itsUseFVSKeywords = new NButton(this, CRect(12, 62, 140, 94), "Add Keywords");
   itsUseFVSKeywords->SetCommand(M_SIM_PREP_USEKEYWORDS);
   itsUseFVSKeywords->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsUseFVSKeywords));

   itsInsertFromFile = new NButton(this, CRect(146, 62, 274, 94), "Insert From File");
   itsInsertFromFile->SetCommand(InsertFromFileCmd);
   itsInsertFromFile->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsInsertFromFile));

   itsSelectPostProc = new NButton(this, CRect(280, 62, 448, 94),  "Select Post Processors");
   itsSelectPostProc->SetCommand(M_SIM_PREP_POST_PROCESSOR);
   itsSelectPostProc->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsSelectPostProc));

   itsSpecifyMod = new NButton(this, CRect(454, 62, 582, 94), "Select Modifiers");
   itsSpecifyMod->SetCommand(M_SIM_PREP_MODEL_MOD);
   itsSpecifyMod->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsSpecifyMod));

   itsStandText = new NText(this, CRect(662, 62, 722, 78), "Stands", CTL_FLAG_LEFT_JUST);
   itsStandText->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsStandCountText = new NText(this, CRect(594, 62, 654, 78), "", CTL_FLAG_RIGHT_JUST);
   itsStandCountText->SetGlue(TOPSTICKY | LEFTSTICKY);

   itsGroupText = new NText(this, CRect(662, 78, 722, 94), "Groups", CTL_FLAG_LEFT_JUST);
   itsGroupText->SetGlue(TOPSTICKY | LEFTSTICKY);
   itsGroupCountText = new NText(this, CRect(594, 78, 654, 94), "", CTL_FLAG_RIGHT_JUST);
   itsGroupCountText->SetGlue(TOPSTICKY | LEFTSTICKY);

   // Create the CTreeView and call its initializer method (required) -- images are from resources.
   itsExpandedImage  = new CImage(EXPANDEDIMAGE);
   itsCollapsedImage = new CImage(COLLAPSEDIMAGE);
   itsTermnodeImage  = new CImage(TERMNODEIMAGE);

   CFont aFont = xvt_dwin_get_font(GetXVTWindow());

#if ENABLE_TREE_VIEW_SOURCE
   itsSource = new MyCTreeSource(this);
#endif // ENABLE_TREE_VIEW_SOURCE

   itsComponentTreeView = new CTreeView(this, CRect(6, 106, 514, 450));
   itsComponentTreeView->ITreeView(itsSource,
                                COLOR_BLACK,
                                aFont,
                                CTreeItem::kBestHeight,
                                10,  // CTreeItem::kBestIndent,
                                itsTermnodeImage,
                                itsExpandedImage,
                                itsCollapsedImage,
                                NULL,
                                TRUE, TRUE, TRUE, 10, 50, TRUE,
                                TOPSTICKY | BOTTOMSTICKY | LEFTSTICKY | RIGHTSTICKY);
   itsComponentTreeView->SetDrawRoot(TRUE);
   itsNavigator->AppendTabStop(new CTabStop(itsComponentTreeView));

   CTreeNodeItem * theRoot = itsComponentTreeView->GetRoot();
   theRoot->SortChildren(&itsSorter);
   theRoot->Expand();

   itsGroupText = new NText(this, CRect(522, 106, 722, 122), "Affected Stands...", CTL_FLAG_LEFT_JUST);
   itsGroupText->SetGlue(TOPSTICKY | RIGHTSTICKY);
   itsStandListBox = new NListBox(this, CRect(522, 124, 722, 450), itsStandList, 0L);
   itsStandListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY | RIGHTSTICKY);
   itsStandListBox->SetCommand(StandListSelectionCmd);
   itsNavigator->AppendTabStop(new CTabStop(itsStandListBox));

   itsEditSimContGrpBox = new NGroupBox(this, CRect(6, 456, 726, 550), "Edit Simulation", 0L);
   itsEditSimContGrpBox->SetGlue(BOTTOMSTICKY | LEFTSTICKY | RIGHTSTICKY);

   itsEditButton = new NButton(this, CRect(12, 474, 125, 506), "Edit Selection");
   itsEditButton->SetCommand(EditSelectionCmd);
   itsEditButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsEditButton));

   itsDeleteButton = new NButton(this, CRect(131, 474, 244, 506), "Delete");
   itsDeleteButton->SetCommand(DeleteCmd);
   itsDeleteButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsDeleteButton));

   itsWriteButton = new NButton(this, CRect(250, 474, 363, 506), "Write");
   itsWriteButton->SetCommand(WriteToFileCmd);
   itsWriteButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsWriteButton));

   itsAppendButton = new NButton(this, CRect(369, 474, 482, 506), "Append");
   itsAppendButton->SetCommand(AppendCmd);
   itsAppendButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsAppendButton));

   itsCopyButton = new NButton(this, CRect(488, 474, 601, 506), "Copy");
   itsCopyButton->SetCommand(CopyCmd);
   itsCopyButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsCopyButton));

   itsCutButton = new NButton(this, CRect(607, 474, 720, 506), "Cut");
   itsCutButton->SetCommand(CutCmd);
   itsCutButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsCutButton));

   itsChangeGroupMembership = new NButton(this, CRect(12, 512, 244, 544), "Change Group Membership");
   itsChangeGroupMembership->SetCommand(M_SIM_PREP_CURRENT);
   itsChangeGroupMembership->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsChangeGroupMembership));

   itsPasteButton = new NButton(this, CRect(250, 512, 720, 544), "Paste");
   itsPasteButton->SetCommand(PasteCmd);
   itsPasteButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsPasteButton));

   itsAfterSimGrpBox = new NGroupBox(this, CRect(6, 556, 540, 612), "After Simulation", 0L);
   itsAfterSimGrpBox->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   readFVSOutputButton= new NButton(this, CRect(12, 574, 182, 606), "Read FVS Outputs",0L);
   readFVSOutputButton->SetCommand(M_AFTER_SIM_READFVSOUTPUT);
   readFVSOutputButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(readFVSOutputButton));

   itsGenerateGraph = new NButton(this, CRect(188, 574, 358, 606), "Generate Graphs");
   itsGenerateGraph->SetCommand(M_AFTER_SIM_GRAPHICS);
   itsGenerateGraph->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsGenerateGraph));

   itsGenerateReport = new NButton(this, CRect(364, 574, 534, 606), "Generate Reports");
   itsGenerateReport->SetCommand(M_AFTER_SIM_REPORTS);
   itsGenerateReport->SetGlue(BOTTOMSTICKY | LEFTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsGenerateReport));

   itsExitButton = new NButton(this, CRect(546, 574, 722, 606), "Exit");
   itsExitButton->SetCommand(M_FILE_QUIT);
   itsExitButton->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);
   itsNavigator->AppendTabStop(new CTabStop(itsExitButton));

   //  attach F1-help to the current window and elements
   WINDOW theWindow = GetXVTWindow();

   xvt_help_set_win_assoc (helpInfo, theWindow, MAINWINWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsSelectSimStand->GetXVTWindow(),
                           SELECTSIWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsSetTime->GetXVTWindow(),
                           TIMESCALWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsSelectModelOut->GetXVTWindow(),
                           SELECTModelOutputButton, 0L);
   xvt_help_set_win_assoc (helpInfo, itsManagement->GetXVTWindow(),
                           CMNGWINWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsSpecifyMod->GetXVTWindow(),
                           SPECIFYModelModButton, 0L);
   xvt_help_set_win_assoc (helpInfo, itsUseFVSKeywords->GetXVTWindow(),
                           USEKEYWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsSelectPostProc->GetXVTWindow(),
                           POSTPWINWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsRunSimulation->GetXVTWindow(),
                           RUNSimulation, 0L);
   xvt_help_set_win_assoc (helpInfo, itsChangeGroupMembership->GetXVTWindow(),
                           GRPMEMBWindow, 0L);
   xvt_help_set_win_assoc (helpInfo, itsGenerateGraph->GetXVTWindow(),
                           GENERATEGraphs, 0L);
   xvt_help_set_win_assoc (helpInfo, itsGenerateReport->GetXVTWindow(),
                           GENERATEReports, 0L);
   xvt_help_set_win_assoc (helpInfo, itsExitButton->GetXVTWindow(),
                           EXITButton, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_TIME,
                            TIMESCALWindow, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_MODEL,
                            SELECTModelOutputButton, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_MANAGEMENT,
                            CMNGWINWindow, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_MODEL_MOD,
                            SPECIFYModelModButton, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_USEKEYWORDS,
                            USEKEYWindow, 0L);
/*   xvt_help_set_menu_assoc (helpInfo, theWindow,		//Commented out 05/21/09 AJSM
                            M_HELP_RUNWIZARD,
                            RUNWizard, 0L);
*/   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_POST_PROCESSOR,
                            POSTPWINWindow, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_RUN,
                            RUNSimulation, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_AFTER_SIM_REPORTS,
                            GENERATEReports, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_AFTER_SIM_GRAPHICS,
                            GENERATEGraphs, 0L);

   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_VARIANT,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_SIMNOTES,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_DELETESTAND,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_DELETEGROUP,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_VIEWKEYWORDS,
                            MENUbar, 0L);

   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_PREFERENCES_EDIT,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_DATA_PREP_LOCATIONS,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_DATA_PREP_STANDLIST,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_DATA_PREP_FVSTREEDATA,
                            MENUbar, 0L);

   DoCommand(STANDCountChanged, this);

   BuildTreeView();
   ResetWindow(StandToExpand(), 1);

   itsNavigator->InitFocus();
}




void CsupposeWin::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case STANDCountChanged:
      {
         // One or more stands are added/deleted to/from the run.  If a stand
         // is deleted or added, the current stand is always reset to the last
         // stand in the list.  If two different messages, one for adding stands
         // and the other for deleting them, were sent, then we would take
         // different actions.  For adding stands, this code would be used.
         // For deleting them we would see if the "Current" stand is still
         // in the list, and if so, then set it as the seleted current stand.
         // (see the processing of GROUPCountChanged for a template).

         RWCollectable * curr =  theSpGlobals->runStream->GetCurrent();
         if (curr && curr->isA() == STAND &&
             theSpGlobals->runStream->allStands->entries())
               DoCommand(CURRStandOrGroupChange, theSpGlobals->runStream->allStands->last());
         else
            DoCommand(CURRStandOrGroupChange, NULL);

         int nItems = 0;
         if (theSpGlobals->runStream->allStands)
            nItems = theSpGlobals->runStream->allStands->entries();

         if (nItems)
         {
            itsRunSimulation->Enable();
            itsChangeGroupMembership->Enable();
         }
         else
         {
            itsRunSimulation->Disable();
            itsChangeGroupMembership->Disable();
         }

         theSpGlobals->theAppDocument->SetSave(TRUE);
         SpUpdateStandCount();
         break;
      }

      case GROUPCountChanged:
      {
         // One or more groups are added/deleted to/from the run.

         RWCollectable * curr =  theSpGlobals->runStream->GetCurrent();
         if (curr && curr->isA() == GROUP) DoCommand(CURRStandOrGroupChange, curr);
         else                              DoCommand(CURRStandOrGroupChange, NULL);

         theSpGlobals->theAppDocument->SetSave(TRUE);
         SpUpdateStandCount();
         break;
      }

      case CURRStandOrGroupChange:
      { 
         // Set a "focus" on a stand or group.  theData
         // will be a pointer to the stand or group.  If the object
         // is not the current "subset", then make it the current subset.

         if (!theData) theData = theSpGlobals->runStream->allGroups->first();
         if (!theData) break;

         int sendMessage = 1;

         if (((RWCollectable *) theData)->isA() == STAND)
         {
            if (theSpGlobals->runStream->subsetStands->entries())
            sendMessage = theData != theSpGlobals->runStream->subsetStands->first();

            if (sendMessage)
            {
               theSpGlobals->runStream->subsetGroups->clear();
               theSpGlobals->runStream->subsetStands->clear();
               theSpGlobals->runStream->subsetStands->insert((Stand *) theData);
            }
         }
         else
         {
            if (((RWCollectable *) theData)->isA() != GROUP) break;

            if (theSpGlobals->runStream->subsetGroups->entries())
               sendMessage = theData != theSpGlobals->runStream->subsetGroups->first();

            if (sendMessage)
            {
               theSpGlobals->runStream->subsetStands->clear();
               theSpGlobals->runStream->subsetGroups->clear();
               theSpGlobals->runStream->subsetGroups->insert((Group *) theData);
            }
         }

         if (sendMessage)
            SendUpdateMessage(CURRStandOrGroupChange, theData, this);

         BuildTreeView();
         ResetWindow(StandToExpand(),1);
         break;
      }

      case GROUPMembershipChanged:
      {
         BuildTreeView();
         ResetWindow(StandToExpand(),1);
         break;
      }

      case GROUPNameChanged:
      case STANDNameChanged:
      case COMPONENTSListChanged:
      {
         RWOrdered selectionList = itsComponentTreeView->GetSelection();
         BuildTreeView();
         if ( selectionList.isEmpty() )
         {
            ResetWindow(StandToExpand(),1);
         }
         else
         {
            ResetWindow( selectionList.first() );
         }
         break;
      }

      case EditSelectionCmd:
        LaunchComponentWindow();
        break;

      case PasteCmd:
        DoMenuCommand(M_EDIT_PASTE, 0, 0);
        break;

      case CutCmd:
        DoMenuCommand(M_EDIT_CUT,   0, 0);
        break;

      case CopyCmd:
        DoMenuCommand(M_EDIT_COPY,  0, 0);
        break;

      case DeleteCmd:
        DoMenuCommand(M_EDIT_CLEAR, 0, 0);
        break;

      case InsertFromFileCmd:
      {
         xvt_fsys_save_dir();
         FILE_SPEC * fileSpec = new FILE_SPEC;
         fileSpec->name[0]=NULL;
         strcpy(fileSpec->type,"kcp");
         xvt_fsys_get_dir(&fileSpec->dir);
         if (xvt_dm_post_file_open(fileSpec, "Insert component from file:")
             == FL_OK)
         {
            int likelyInserts = 0;
            xvt_fsys_get_dir(&fileSpec->dir);
            FILE * filePtr = fopen (fileSpec->name,"r");
            if (filePtr)
            {
               ReadComponentsFromFile(filePtr, fileSpec, &itsComponentsFromFile);
               fclose(filePtr);

               likelyInserts=itsComponentsFromFile.entries();

               theSpGlobals->runStream->
                     ProcessPPEComponents(&itsComponentsFromFile);

               if (itsComponentsFromFile.entries())
               {

                  // set the current subset to the current stand or group
                  // if they are set...

                  Stand * aStand = NULL;
                  Group * aGroup = NULL;
                  if (itsCurrentStand || itsCurrentGroup)
                  {
                     aStand = (Stand *) theSpGlobals->runStream->
                           subsetStands->pop();
                     if (itsCurrentStand) theSpGlobals->runStream->
                           subsetStands->insert(itsCurrentStand);
                     aGroup = (Group *) theSpGlobals->runStream->
                           subsetGroups->pop();
                     if (itsCurrentGroup) theSpGlobals->runStream->
                           subsetGroups->insert(itsCurrentGroup);
                  }
                  Component * componentToInsert;
                  Component * place = itsCurrentComponent;
                  Component * placeArg = place;
                  int after = 0;
                  if (place)
                  {
                     const char * what = "Component(s) from file";
                     if (itsComponentsFromFile.entries() == 1)
                        what = ((Component *)
                                itsComponentsFromFile.first())->data();
                     DefinePlacementArgs(what,&placeArg,&after);
                  }
                  int initAfter = after;
                  for (size_t i=0; i<itsComponentsFromFile.entries(); i++)
                  {
                     componentToInsert = (Component *) itsComponentsFromFile(i);

                     if (placeArg &&
                         placeArg->isA() != componentToInsert->isA())
                     {
                        placeArg=place;
                        after = initAfter;
                     }

                     if (placeArg &&
                         placeArg->isA() != componentToInsert->isA())
                        placeArg=NULL;

                     theSpGlobals->runStream->
                           AddComponent(componentToInsert, placeArg, after);

                     placeArg = componentToInsert;
                     after = 1;

                  }

                  itsComponentsFromFile.clear();

                  if (aStand || aGroup)
                  {
                     theSpGlobals->runStream->subsetStands->clear();
                     theSpGlobals->runStream->subsetGroups->clear();
                     if (aStand) theSpGlobals->runStream->
                                       subsetStands->insert(aStand);
                     if (aGroup) theSpGlobals->runStream->
                                       subsetGroups->insert(aGroup);
                  }

                  SendUpdateMessage(COMPONENTSListChanged,NULL,this);
                  theSpGlobals->theAppDocument->SetSave(TRUE);

                  BuildTreeView();
                  ResetWindow(itsComponentsFromFile.last());
               }
               else
               {
                  if (likelyInserts)
                  {
                     BuildTreeView();
                     ResetWindow();
                  }
                  else xvt_dm_post_warning("Component(s) NOT imported.");
               }
            }
            else
            {
               char path[SZ_FNAME];
               xvt_fsys_convert_dir_to_str(&fileSpec->dir, path, sizeof(path));
               xvt_dm_post_message("%s%s%s could not be opened for input.",
                                   path, SEPARATOR, fileSpec->name);
            }
         }
         xvt_fsys_restore_dir();
         delete fileSpec;
         break;
      }

      case WriteToFileCmd:
      case AppendCmd:
      {
         xvt_fsys_save_dir();
         FILE_SPEC * fileSpec = new FILE_SPEC;
         fileSpec->name[0]=NULL;
         strcpy(fileSpec->type,"kcp");
         xvt_fsys_get_dir(&fileSpec->dir);

         const char * opSp;
         FL_STATUS reply;

         if (theData == itsAppendButton)
         {
            opSp = "a+";
            reply = xvt_dm_post_file_open(fileSpec, "Append to file:");
         }
         else
         {
            opSp = "w+";
            reply = xvt_dm_post_file_save(fileSpec, "Write to file:");
         }
         if (reply == FL_OK)
         {
            xvt_fsys_get_dir(&fileSpec->dir);
            FILE * filePtr = fopen (fileSpec->name,opSp);
            if (filePtr)
            {
               itsCurrentComponent->WriteComponent(NULL, NULL, filePtr, 1);
               fclose(filePtr);
            }
            else
            {
               char path[SZ_FNAME];
               xvt_fsys_convert_dir_to_str(&fileSpec->dir, path, sizeof(path));
               xvt_dm_post_warning("%s%s%s could not be opened for output",
                                   path, SEPARATOR, fileSpec->name);
            }
         }
         xvt_fsys_restore_dir();
         delete fileSpec;
         break;
      }

      case StandListSelectionCmd:
      {
         int pos = itsStandListBox->GetSelectPosition();
         Stand * aStand = NULL;
         if (pos >= 0 && pos < (int) itsStandList.entries())
            aStand = (Stand *) itsStandList.at((size_t) pos);
         ResetWindow(aStand);
         break;
      }

      // Handle tree events
      case TreeEventCmd:
      {
         if (!theData) break;

         // event types:
         // 0=FocusIn,  1=FocusOut,   2=SelectionChanged, 3=Expand,
         // 4=Collapse, 5=MouseClick, 6=TitleMouseClick

         CTreeEvent* anEvent = (CTreeEvent*) theData;

         RWCollectable * selectedElement = NULL;
         MyUserData * userData = NULL;
         CTreeItem* anItem = anEvent->GetTreeItem();
         if (anItem)
         {
            userData = (MyUserData *) anItem->GetUserData();
            if (userData) selectedElement = userData->itsItem;
         }
         int theEvent = (int) anEvent->GetType();
         switch (anEvent->GetType())
         {
            case CTreeEvent::MouseClick:
            {
               if (userData)
               {
                  switch (userData->itsKind)
                  {
                     case COMPONENT:
                     case PPECOMPONENT:
                     {
                        itsCurrentComponent = (Component *) userData->itsItem;
                        itsCurrentGroup=NULL;
                        itsCurrentStand=NULL;
                        break;
                     }
                     case GROUP:
                     {
                        itsCurrentComponent=NULL;
                        itsCurrentGroup=(Group *) userData->itsItem;
                        itsCurrentStand=NULL;
                        if (anEvent->GetButton())  //button 1 (right)
                        {
                           SendUpdateMessage(CURRStandOrGroupChange, itsCurrentGroup, NULL);
                           anItem->SetColor(COLOR_RED);
                        }

                        break;
                     }
                     case STAND:
                     {
                        itsCurrentComponent=NULL;
                        itsCurrentGroup=NULL;
                        itsCurrentStand=(Stand *) userData->itsItem;
                        if (anEvent->GetButton())  //button 1 (right)
                        {
                           SendUpdateMessage(CURRStandOrGroupChange, itsCurrentStand, NULL);
                           anItem->SetColor(COLOR_RED);
                        }
                        break;
                     }
                     default:
                     {
                        itsCurrentComponent= NULL;
                        itsCurrentGroup    = NULL;
                        itsCurrentStand    = NULL;
                     }
                  }
               }
               else
               {
                  itsCurrentComponent= NULL;
                  itsCurrentGroup    = NULL;
                  itsCurrentStand    = NULL;
               }
               ResetWindow(selectedElement);
               break;
            }
            case CTreeEvent::MouseDoubleClick:
            {
               // Perform Edit command
               if (itsEditButton->IsEnabled())
                  DoCommand(EditSelectionCmd, NULL);
               break;
            }
            case CTreeEvent::Collapse:
            {
               CTreeNodeItem * theRoot = itsComponentTreeView->GetRoot();
               if (anItem == theRoot)
                  theRoot->Expand();
               break;
            }
            case CTreeEvent::Expand:
            {
               if (selectedElement)
                  ExpandMostChildren(anItem);
               break;
            }
            default: break;
         }
         break;
      }

      default:
      {
         CWindow::DoCommand(theCommand,theData);
         break;
      }
   }
}




BOOLEAN CsupposeWin::Close(void)
{
   theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   return CWindow::Close();
}




void CsupposeWin::SpUpdateFileName()
{
   /*------------------------  SpUpdateFileName  -----------------------------
    *
    *  SpUpdateFileName updates the control panel by rewriting the name of the
    *  current data file or an indication that no file has been opened
    *
   --------------------------------------------------------------------------*/

   // get the current file name from the document

   FILE_SPEC* aFilePtr = ((CsupposeDoc*) GetDocument())->SpGetFilePointer();
   CStringRW fullName(SZ_FNAME);

   if (!aFilePtr->name[0]) strcpy(aFilePtr->name,NEWFILE);

   if (strcmp(aFilePtr->name,NEWFILE)==0) fullName = NEWFILE;
   else
   {
      char path[SZ_FNAME];
      xvt_fsys_convert_dir_to_str(&aFilePtr->dir,path,sizeof(path));
      fullName = path;
      fullName+= SEPARATOR;
      fullName+= aFilePtr->name;
   }

   CStringRW title(100);
   title = G->GetTaskWin()->GetTitle();
   size_t colon = title.first(':');
   if (colon == RW_NPOS) title.append("  Simulation file:");
   colon = title.first(':');
   title.replace(colon+1,999,NULLString);

   G->GetTaskWin()->SetTitle(title+"  "+fullName);
}




void CsupposeWin::SpUpdateStandCount(void)
{
   /*------------------------  SpUpdateStandCount ----------------------------
    *
    *  SpUpdateStandCount updates the control panel by rewriting the number of
    *  groups and stands in the current subset and in the simulation.
    *
   --------------------------------------------------------------------------*/

   int totalStands;
   int totalGroups;
   int subSetStands;
   int subSetGroups;

   if (theSpGlobals->runStream->allStands)
        totalStands = theSpGlobals->runStream->allStands->entries();
   else totalStands = 0;
   if (totalStands > 0) 
   {
      itsRunSimulation->Enable();
      itsChangeGroupMembership->Enable();
   }
   else
   {
      itsRunSimulation->Disable();
      itsChangeGroupMembership->Disable();
   }

   if (theSpGlobals->runStream->allGroups)
        totalGroups = theSpGlobals->runStream->allGroups->entries();
   else totalGroups = 0;

   if (theSpGlobals->runStream->subsetStands)
        subSetStands = theSpGlobals->runStream->subsetStands->entries();
   else subSetStands = 0;

   if (theSpGlobals->runStream->subsetGroups)
        subSetGroups = theSpGlobals->runStream->subsetGroups->entries();
   else subSetGroups = 0;

   if (subSetGroups == 0 && subSetStands == 0 &&
       theSpGlobals->runStream->allGroups->entries())
   {
      subSetGroups = 1;
      theSpGlobals->runStream->subsetGroups->
         insert(theSpGlobals->runStream->allGroups->first());
   }

   CStringRW string;
   string = ConvertToString(totalStands);
   itsStandCountText->SetTitle(string);
   string = ConvertToString(totalGroups);
   itsGroupCountText->SetTitle(string);
}




void CsupposeWin::Key( const CKey& theKey )
{
   switch (theKey.GetChar())
   {
      case K_DEL:
         // Perform Delete command
         if (itsDeleteButton->IsEnabled())
            DoCommand(DeleteCmd, NULL);
         break;

      default:
         break;
   }

   // Call base class method
   CWindow::Key( theKey );
}




void CsupposeWin::ResetWindow(RWCollectable * selectedElement, int clearSelection)
{
   xvt_scr_set_busy_cursor();

   if (!selectedElement)
   {
      // get the "current" group or stand.

      selectedElement = theSpGlobals->runStream->subsetStands->first();
      if (!selectedElement) selectedElement =
         theSpGlobals->runStream->subsetGroups->first();
   }

   CTreeItem * anItem = NULL;
   if (selectedElement)
   {
      RWOrdered selList;
      LoadSelectedList(itsComponentTreeView->GetRoot(),
                       selectedElement,
                       selList);
      anItem = FindFirst(itsComponentTreeView->GetRoot(),
                         selectedElement);
      itsComponentTreeView->SetSelection(selList,TRUE);

      switch (selectedElement->isA())
      {
         case COMPONENT:
         case PPECOMPONENT:
         {
            itsCurrentComponent = (Component *) selectedElement;
            itsCurrentGroup=NULL;
            itsCurrentStand=NULL;
            break;
         }
         case GROUP:
         {
            itsCurrentComponent=NULL;
            itsCurrentGroup=(Group *) selectedElement;
            itsCurrentStand=NULL;
            break;
         }
         case STAND:
         {
            itsCurrentComponent=NULL;
            itsCurrentGroup=NULL;
            itsCurrentStand=(Stand *) selectedElement;
            break;
         }
         default:
         {
            itsCurrentComponent= NULL;
            itsCurrentGroup    = NULL;
            itsCurrentStand    = NULL;
         }
      }
   }
   else
   {
      itsCurrentComponent = NULL;
      itsCurrentGroup     = NULL;
      itsCurrentStand     = NULL;
   }

   if (theSpGlobals->runStream->GetComponentFromClipBoard())
   {
      GetMenuBar()->SetEnabled (M_EDIT_PASTE, TRUE);
      CStringRW pasteTitle = "Paste \"";
      pasteTitle+= *theSpGlobals->runStream->GetComponentFromClipBoard();
      pasteTitle+="\"";
      if (pasteTitle.length() > 55)
         pasteTitle.replace(50, 999, "...\"", 4);

      itsPasteButton->SetTitle(pasteTitle);
      itsPasteButton->Enable();
   }
   else
   {
      GetMenuBar()->SetEnabled (M_EDIT_PASTE, FALSE);
      itsPasteButton->SetTitle("Paste");
      itsPasteButton->Disable();
   }

   BuildStandList();

   if (clearSelection)
   {
      itsComponentTreeView->ClearSelection();
      itsCurrentComponent = NULL;
      itsCurrentGroup = NULL;
      itsCurrentStand = NULL;
   }

   // These options are set based on selction
   if (itsCurrentComponent)
   {
      GetMenuBar()->SetEnabled (M_EDIT_CUT,                TRUE);
      GetMenuBar()->SetEnabled (M_EDIT_COPY,               TRUE);
      GetMenuBar()->SetEnabled (M_EDIT_CLEAR,              TRUE);
      GetMenuBar()->SetEnabled (M_SIM_PREP_VIEWKEYWORDS,   TRUE);
      GetMenuBar()->SetEnabled (M_SIM_PREP_EDIT_SELECTION, TRUE);
      itsCutButton->Enable();
      itsCopyButton->Enable();
      itsDeleteButton->Enable();
      itsEditButton->Enable();
      itsWriteButton->Enable();
      itsAppendButton->Enable();
   }
   else
   {
      GetMenuBar()->SetEnabled (M_EDIT_CUT,                FALSE);
      GetMenuBar()->SetEnabled (M_EDIT_COPY,               FALSE);
      GetMenuBar()->SetEnabled (M_EDIT_CLEAR,              FALSE);
      GetMenuBar()->SetEnabled (M_SIM_PREP_VIEWKEYWORDS,   FALSE);
      GetMenuBar()->SetEnabled (M_SIM_PREP_EDIT_SELECTION, FALSE);
      itsCutButton->Disable();
      itsCopyButton->Disable();
      if (itsCurrentStand || itsCurrentGroup)
      {
         itsDeleteButton->Enable();
         GetMenuBar()->SetEnabled (M_EDIT_CLEAR, TRUE);
      }
      else
      {
         itsDeleteButton->Disable();
         GetMenuBar()->SetEnabled (M_EDIT_CLEAR, FALSE);
      }
      itsEditButton->Disable();
      itsWriteButton->Disable();
      itsAppendButton->Disable();
   }

   if (anItem)
   {
      ExpandMostChildren(anItem);
      itsComponentTreeView->MakeItemVisible(anItem);
   }

   if (IsVisible()) Invalidate(itsComponentTreeView->GetFrame());
   else             Show();
}




void CsupposeWin::BuildTreeView( void )
{
   xvt_scr_set_busy_cursor();

   // first clear the current tree.

   CTreeNodeItem * theRoot = itsComponentTreeView->GetRoot();

   if (!theRoot) return;

   ClearChildren(theRoot);
   theRoot->SetUserData(NULL);
   itsTreeSequence=0;

   // reload the list.

   CTreeNodeItem * currRoot = theRoot;

   theRoot->SetString("Simulation file contents:");

   // PPE top information
   if ( theSpGlobals->runStream->topPPEComponents )
   {
      AddPPEList("PPE Top Components:",*theSpGlobals->runStream->topPPEComponents);
   }

   if (theSpGlobals->runStream->allStands->entries())
   {
      // find out which stand to list as "expanded".

      Stand * toExpand = StandToExpand();

      // Add the stand information

      Stand * stand;
      RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
      while (stand = (Stand *) nextStand())
      {
         CTreeNodeItem * standRoot = AddStand(theRoot,stand);
         if (toExpand == stand)
			 standRoot->Expand();

#if !ENABLE_TREE_VIEW_SOURCE

         currRoot = standRoot;
         if (stand->componentsOrGroups)
         {
            RWOrderedIterator nextComponentOrGroup(*stand->componentsOrGroups);
            RWCollectable * componentOrGroup;
            while (componentOrGroup = (RWCollectable *) nextComponentOrGroup())
            {
               if (componentOrGroup->isA() == COMPONENT)
               {
                  currRoot = AddComponent(currRoot,(Component*) componentOrGroup);
               }
               else if (componentOrGroup->isA() == GROUP)
               {
                  currRoot = AddGroup(standRoot,(Group*) componentOrGroup);
                  if (((Group *) componentOrGroup)->componentList)
                  {
                     RWOrderedIterator nextComponent(*((Group *) componentOrGroup)->componentList);
                     Component * component;
                     while (component = (Component *) nextComponent())
                     {
                        currRoot = AddComponent(currRoot,component);
                     }
                     currRoot = standRoot;
                  }
               }
            }
         }

         // create a list of groups that
         // contain the stand and have no components.  First step
         // is to see if any of such groups exist.  IF they do, then
         // create the additional groups tree node and add the
         // groups to that node... (done in one pass).

         CTreeNodeItem * moreGroups = NULL;
         RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
         Group * group;
         while ((group = (Group *) nextGroup()) != NULL)
         {
            // skip if the group has a compoenent list.

            if (!group->componentList   &&
                group->standMembership  &&
                group->standMembership->contains(stand))
            {
               if (!moreGroups)
               {
                  moreGroups=standRoot-> AddNodeChild("Group(s) with no attached components:");
                  moreGroups->SetColor(COLOR_BLACK);
                  moreGroups->Collapse();
                  moreGroups->SetUserData(NULL);
               }
               CTreeItem * newItem = moreGroups->AddTerminalChild("Group: " + *group);
               newItem->SetColor(COLOR_BLACK);
               newItem->SetUserData(new MyUserData(group,itsTreeSequence++));
               if (group == theSpGlobals->runStream->subsetGroups->first())
               {
                  if (!moreGroups->GetParent()->IsCollapsed())
                  {
                     moreGroups->Expand();
                  }
                  newItem->SetColor(COLOR_RED);
               }
               else newItem->SetColor(COLOR_BLACK);
            }
         }

 #endif // !ENABLE_TREE_VIEW_SOURCE

      }
   }

   // create list of groups that have components but no stands
   // only add this list if there is at least 1 component defined.

   if (theSpGlobals->runStream->allComponents->entries())
   {
      CTreeNodeItem * moreGroups = NULL;
      RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
      Group * group;
      while ((group = (Group *) nextGroup()) != NULL)
      {
         // skip if the group has stand membership.

         if (!group->standMembership)
         {
            if (!moreGroups)
            {
               moreGroups=theRoot->AddNodeChild("Group(s) with no attached stands:");
               moreGroups->SetColor(COLOR_BLACK);
               moreGroups->Expand();
               moreGroups->SetUserData(NULL);
            }
            currRoot = AddGroup(moreGroups,group);
            if (group->componentList)
            {
               RWOrderedIterator next(*group->componentList);
               Component * component;
               while (component = (Component *) next())
               {
                  currRoot = AddComponent(currRoot,component);
               }
            }
         }
      }
   }

   // PPE bottom information

   if ( theSpGlobals->runStream->botPPEComponents )
   {
      AddPPEList("PPE Bottom Components:",*theSpGlobals->runStream->botPPEComponents);
   }

   // PPE report information

   if ( theSpGlobals->runStream->reportPPEComponents )
   {
      AddPPEList("PPE Report Components:",*theSpGlobals->runStream->reportPPEComponents);
   }

   SortAllChildren(theRoot,&itsSorter);
   itsComponentTreeView->ScrollViews(CPoint(0,0));
   if (IsVisible())
      Invalidate(itsComponentTreeView->GetFrame());
}


void CsupposeWin::BuildStandList( void )
{
   itsStandList.clear();

   if (itsCurrentComponent)
   {
      Stand * stand;
      Group * group;
      RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
      while ((group = (Group *) nextGroup()) != NULL)
      {
         if (group->componentList &&
             group->componentList->contains(itsCurrentComponent))
         {
            if (group->standMembership)
            {
               RWOrderedIterator nextStand(*group->standMembership);
               while (stand = (Stand *) nextStand())
                  if (!itsStandList.contains(stand)) itsStandList.insert(stand);
            }
         }
      }
      RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
      while ((stand = (Stand *) nextStand()) != NULL)
      {
         if (stand->componentsOrGroups &&
             stand->componentsOrGroups->contains(itsCurrentComponent))
            if (!itsStandList.contains(stand)) itsStandList.insert(stand);
      }
   }

   else if (itsCurrentGroup)
   {
      if (itsCurrentGroup->standMembership)
      {
         Stand * stand;
         RWOrderedIterator nextStand(*itsCurrentGroup->standMembership);
         while (stand = (Stand *) nextStand()) itsStandList.insert(stand);
      }
   }

   else if (itsCurrentStand) itsStandList.insert(itsCurrentStand);

   else
   {
      Stand * stand;
      RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
      while ((stand = (Stand *) nextStand()) != NULL)
         itsStandList.insert(stand);
   }

   itsStandListBox->IListBox(itsStandList);
}


void CsupposeWin::DoMenuCommand(MENU_TAG theMenuItem,
                                  BOOLEAN isShiftkey,
                                  BOOLEAN isControlKey)
{
   switch(theMenuItem)
   {
      case M_SIM_PREP_VIEWKEYWORDS:
      {
         if (itsCurrentComponent)
         {
            CStringRW viewTitle = "View - ";
            viewTitle += (CStringRW&) *itsCurrentComponent;

            new ViewKeywords(itsCurrentComponent->keywords->data(),
                             viewTitle.data());
         }
         break;
      }

      case M_EDIT_CLEAR:         // aka Delete
      {
         if (itsCurrentComponent)
         {
            if (itsCurrentComponent->openedWindow)
            {
               G->GetDesktop()->
                     SetFrontWindow(itsCurrentComponent->openedWindow);
               xvt_dm_post_warning("You can not delete a component "
                                   "you are editing.");
            }
            else
            {
               if (xvt_dm_post_ask ("Yes", "Cancel", NULL,
                                    "Delete \"%s\"",
                                    itsCurrentComponent->data()) ==
                   RESP_DEFAULT)
               {
                  theSpGlobals->runStream->DelComponent(itsCurrentComponent);
                  itsCurrentComponent = NULL;
                  theSpGlobals->theAppDocument->SetSave(TRUE);
                  SendUpdateMessage(COMPONENTSListChanged,NULL,this);
                  BuildTreeView();
                  ResetWindow();
               }
            }
         }

         else if (itsCurrentStand)
            CWindow::DoCommand(M_SIM_PREP_DELETESTAND,itsCurrentStand);

         else if (itsCurrentGroup)
            CWindow::DoCommand(M_SIM_PREP_DELETEGROUP,itsCurrentGroup);

         break;
      }

      case M_EDIT_CUT:
      {
         if (itsCurrentComponent && itsCurrentComponent->openedWindow)
         {
            G->GetDesktop()->
                  SetFrontWindow(itsCurrentComponent->openedWindow);
            xvt_dm_post_message("You can not cut a "
                                "component you are editing.");
         }
         else
         {                          // sequence is *very* important
            theSpGlobals->runStream->
                  PutComponentOnClipBoard(itsCurrentComponent);
            theSpGlobals->runStream->DelComponent(itsCurrentComponent);
            itsCurrentComponent=NULL;
            theSpGlobals->theAppDocument->SetSave(TRUE);
            SendUpdateMessage(COMPONENTSListChanged,NULL,this);
            BuildTreeView();
            ResetWindow();
         }
         break;
      }

      case M_EDIT_COPY:
      {
         if (itsCurrentComponent)
         {
            theSpGlobals->runStream->ClearComponentClipBoard();
            Component * nC;
            if (itsCurrentComponent->isA() == PPECOMPONENT)
               nC = new PPEComponent(itsCurrentComponent);
            else
               nC = new Component(itsCurrentComponent);
            theSpGlobals->runStream->PutComponentOnClipBoard(nC);

            ResetWindow(itsCurrentComponent);
         }
         break;
      }

      case M_EDIT_PASTE:
      {
         Component * component =
               theSpGlobals->runStream->GetComponentFromClipBoard();

         if (component)
         {
            Component * place = itsCurrentComponent;
            Component * placeArg = place;
            int after = 0;
            if (place && place->isA() == component->isA() )
            {
               if (place->isA() == PPECOMPONENT)
               {
                  if ( place->componentType == component->componentType )
                     DefinePlacementArgs(component->data(),
                                         &placeArg,&after);
               }
               else
                  DefinePlacementArgs(component->data(),
                                      &placeArg,&after);
            }

            component = new Component(component);
            component = theSpGlobals->runStream->
                  PPEChecking( NULL, component );

            // set the current subset to point to what has been selected.

            Stand * aStand = NULL;
            Group * aGroup = NULL;

            if (component->isA() == COMPONENT &&
                (itsCurrentStand || itsCurrentGroup))
            {
               aStand = (Stand *) theSpGlobals->runStream->subsetStands->pop();
               if (itsCurrentStand) theSpGlobals->runStream->subsetStands->
                                          insert(itsCurrentStand);
               aGroup = (Group *) theSpGlobals->runStream->subsetGroups->pop();
               if (itsCurrentGroup) theSpGlobals->runStream->subsetGroups->
                                          insert(itsCurrentGroup);
            }

            theSpGlobals->runStream->AddComponent(component,
                                                  placeArg,after);

            // set the current subset back to original state.

            if (aStand || aGroup)
            {
               theSpGlobals->runStream->subsetStands->clear();
               theSpGlobals->runStream->subsetGroups->clear();
               if (aStand) theSpGlobals->runStream->subsetStands->
                                 insert(aStand);
               if (aGroup) theSpGlobals->runStream->subsetGroups->
                                 insert(aGroup);
            }

            BuildTreeView();
            ResetWindow(component);

            theSpGlobals->theAppDocument->SetSave(TRUE);
            SendUpdateMessage(COMPONENTSListChanged,NULL,this);
         }
         break;
      }

      default:
        CWindow::DoMenuCommand(theMenuItem, isShiftkey, isControlKey);
        break;
   }
}


void CsupposeWin::LaunchComponentWindow( void )
{
#if XVTWS==WIN32WS
   static char * far componentWindows[] =
#else
         static char * componentWindows[] =
#endif
   {
      "Clearcut",
      "Seedtree",
      "Shelterwood",
      "GPKeyword",
      "SchByCond",
      "BuildExpression",
      "ThinAbove",
      "ThinBelow",
      "FreeForm",
      "PlantNatural",
      NULL
    };

   // if the component has an opened window, then simply focus on it.

   if (itsCurrentComponent->openedWindow)
   {
      G->GetDesktop()->SetFrontWindow(itsCurrentComponent->openedWindow);
      return;
   }

   int index;
   for (int i = 0; componentWindows[i] != NULL; i++)
   {
      if (strcmp(componentWindows[i],
                 itsCurrentComponent->sourceWindow->data()) == 0)
      {
         index = i;
         break;
      }
   }

   CStringRW theTitle = "Edit - ";
   theTitle += (CStringRW &) *itsCurrentComponent;
   switch (index)
   {

      case 0:                         // launch the Clearcut
      {
         new ClearcutWin(theTitle,itsCurrentComponent);
         break;
      }

      case 1:                         // launch the Seedtree
      {
         new SeedTreeWin(theTitle,itsCurrentComponent);
         break;
      }

      case 2:                         // launch the Shelterwood
      {
         new ShelterwoodWin(theTitle,itsCurrentComponent);
         break;
      }


      case 3:                         // launch the GPKeyword
      {
         new GPKeyword (theTitle, NULL, NULLString, itsCurrentComponent);
         break;
      }

      case 4:                         // launch the SchByCondition
      {
         new SchByCond (NULL, 0, NULL, theTitle, itsCurrentComponent);
         break;
      }

      case 5:                         // launch the BuildExpression
      {

         // if itsCurrentComponent is of type 1 (a condition, signify that
         // fact in the call to BuildExpression.

         int type = 0;
         if (itsCurrentComponent->componentType == 1) type = 1;

         new BuildExpression(theTitle, itsCurrentComponent, NULL, type);

         break;
      }

      case 6:                         // launch the Thin from above
        new ThinFromWin(theTitle, 0, itsCurrentComponent);
        break;

      case 7:                         // launch the Thin from below
        new ThinFromWin(theTitle, 1, itsCurrentComponent);
        break;

      case 8:                         // launch the BuildExpression
        // to edit FreeForm
        new BuildExpression(theTitle, itsCurrentComponent, NULL, 2);
        break;

      case 9:                         // launch the PlantNatural window
      {
         int full = 1;
         if (itsCurrentComponent &&
             itsCurrentComponent->extension &&
             itsCurrentComponent->extension->compareTo("strp") == 0) full = 0;
         new PlantNaturalWin(theTitle, full, itsCurrentComponent);
         break;
      }
      default:
        xvt_dm_post_message("Sorry, Suppose can not edit %s",
                            itsCurrentComponent->data());
        break;
   }
}




void CsupposeWin::DefinePlacementArgs(const char * what,
                                        Component ** placeArg,
                                        int * after)
{
   if (placeArg && *placeArg && after)
   {
      RWCString c = what;
      RWCString p = (*placeArg)->data();
      if (c.length() > 35) c.replace(32, 999, "...\"", 4);
      if (p.length() > 35) p.replace(32, 999, "...\"", 4);

      switch(xvt_dm_post_ask("Use Rules","Before","After",
                             "Let Suppose Use Rules for placement, or\n"
                             "paste \"%s\"\nBefore \"%s\" or\nAfter \"%s",
                             c.data(),p.data(),p.data()))
      {
         case RESP_DEFAULT:    // Use Rules
         {
            *placeArg = NULL;
            break;
         }
         case RESP_2:          // Before
         {
            break;
         }
         case RESP_3:          // After
         {
            *after = 1;
            break;
         }
      }
   }
}



void CsupposeWin::ClearChildren(CTreeNodeItem * aNode)
{
   unsigned long i;
   while (i=aNode->GetNChildren())
   {
      CTreeItem* aChild = aNode->GetChild(i-1);
      aChild->SetUserData(NULL);
      ClearChildren((CTreeNodeItem *) aChild);
      aNode->RemoveChild(i-1);
   }
}



void CsupposeWin::SortAllChildren(CTreeItem * aNode,
                                    CTreeSorter * itsSorter)
{
   unsigned long i;
   for (i=0; i<aNode->GetNChildren(); i++)
   {
      CTreeNodeItem * aChild = (CTreeNodeItem *) aNode->GetChild(i);
      SortAllChildren(aChild,itsSorter);
   }
   aNode->SortChildren(itsSorter);
}



void CsupposeWin::LoadSelectedList(CTreeItem* aNode,
                                     RWCollectable * selectedElement,
                                     RWOrdered & selList)
{
   if (aNode && selectedElement)
   {
      MyUserData * userData = (MyUserData *) aNode->GetUserData();
      if (userData &&
          userData->itsItem == selectedElement) selList.insert(aNode);
      unsigned long i;
      for (i=0; i<aNode->GetNChildren(); i++)
      {
         CTreeNodeItem * aChild = (CTreeNodeItem *) aNode->GetChild(i);
         LoadSelectedList(aChild,selectedElement,selList);
      }
   }
}



void CsupposeWin::ExpandMostChildren(CTreeItem* aNode)
{

   // expand all the children that have user data (stands, groups, components)

   if (aNode)
   {
      if (aNode->GetUserData())
      {
         aNode->Expand();
         CTreeItem* n = aNode;
         while (n=n->GetParent()) n->Expand();
      }
      unsigned long i;
      for (i=0; i<aNode->GetNChildren(); i++)
         ExpandMostChildren(aNode->GetChild(i));
   }
}



CTreeItem * CsupposeWin::FindFirst(CTreeItem* aNode,
                                     RWCollectable * selectedElement)
{
   if (aNode && selectedElement)
   {
      MyUserData * userData = (MyUserData *) aNode->GetUserData();
      if (userData &&
          userData->itsItem == selectedElement) return aNode;
      unsigned long i;
      for (i=0; i<aNode->GetNChildren(); i++)
      {
         CTreeNodeItem * aChild = (CTreeNodeItem *) aNode->GetChild(i);
         CTreeItem * aNode = FindFirst(aChild,selectedElement);
         if (aNode) return aNode;
      }
   }
   return NULL;
}



CTreeNodeItem * CsupposeWin::AddStand(CTreeNodeItem * root,
                                        Stand * aStand)
{
   CTreeNodeItem * newItem = root->AddNodeChild("Stand: " + *aStand);
   newItem->Collapse();
   newItem->SetUserData(new MyUserData(aStand,itsTreeSequence++));
   if (aStand == theSpGlobals->runStream->subsetStands->first()) newItem->SetColor(COLOR_RED);
   else                                                          newItem->SetColor(COLOR_BLACK);

   return newItem;
}



CTreeNodeItem * CsupposeWin::AddGroup(CTreeNodeItem * root,
                                        Group * aGroup)
{
   CTreeNodeItem * newItem = root->AddNodeChild("Group: " + *aGroup);
   newItem->Expand();
   newItem->SetUserData(new MyUserData(aGroup,itsTreeSequence++));
   if (aGroup == theSpGlobals->runStream->subsetGroups->first()) newItem->SetColor(COLOR_RED);
   else                                                          newItem->SetColor(COLOR_BLACK);

   return newItem;
}


CTreeNodeItem * CsupposeWin::AddComponent(CTreeNodeItem * root,
                                            Component * aComponent)
{
   CTreeItem * newItem;
   if (aComponent->componentType == 1)
   {
      if (((MyUserData *)root->GetUserData())->itsKind == COMPONENT)
      {
         return AddComponent(root->GetParent(),aComponent);
      }
      else
      {
         root = root->AddNodeChild(*aComponent);
         ((CTreeNodeItem *) root)->SetColor(COLOR_BLACK);
         ((CTreeNodeItem *) root)->Expand();
         root->SetUserData(new MyUserData(aComponent,itsTreeSequence++));
      }
   }
   else
   {
      if (((MyUserData *)root->GetUserData())->itsKind == COMPONENT)
      {
         Component * aComp =
               (Component *) ((MyUserData *)root->GetUserData())->itsItem;

         if (aComp->componentType      == 1  &&    // root is a condition component
             aComponent->componentType == 3  &&    // new component is inside an IF set
             aComp->componentNumber    == aComponent->conditionSerialNum)
                                                   // root is THE condition
         {
            newItem = root->AddTerminalChild(*aComponent);
            newItem->SetColor(COLOR_BLACK);
            newItem->SetUserData(new MyUserData(aComponent,itsTreeSequence++));
         }
         else
         {
            // current root is not a condition or is the wrong one.

            return AddComponent(root->GetParent(),aComponent);
         }
      }
      else
      {
         newItem = root->AddTerminalChild(*aComponent);
         newItem->SetColor(COLOR_BLACK);
         newItem->SetUserData(new MyUserData(aComponent,itsTreeSequence++));
      }
   }
   return root;
}

int CsupposeWin::AddPPEList( CStringRWC strPpeName, RWOrdered & ppeList )
{
   CTreeNodeItem * theRoot = itsComponentTreeView->GetRoot();

   if (!theRoot) return 1;

   // PPE information
   RWOrderedIterator nextPpeItem( ppeList );
   CTreeNodeItem * ppeItem = NULL;
   PPEComponent * ppeComponent;
   while ((ppeComponent = (PPEComponent *) nextPpeItem()) != NULL)
   {
      // skip if the ppeComponent has a compoenent list.

      if (!ppeItem)
      {
         ppeItem = theRoot->AddNodeChild( strPpeName);
         ppeItem->SetColor(COLOR_BLACK);
         ppeItem->Expand();
         ppeItem->SetUserData(NULL);
      }
      CTreeItem * newItem =
            ppeItem->AddTerminalChild( *ppeComponent );
      newItem->SetColor(COLOR_BLACK);
      newItem->SetUserData(new MyUserData(ppeComponent,itsTreeSequence++));
   }

   return 0;
}


Stand * CsupposeWin::StandToExpand( void )
{
   if (theSpGlobals->runStream->subsetGroups->entries())
   {
      Group * currGroup = (Group *)
            theSpGlobals->runStream->subsetGroups->first();
     if (currGroup->standMembership &&
          currGroup->standMembership->entries())
         return (Stand *) currGroup->standMembership->first();
   }
   else if (theSpGlobals->runStream->subsetStands->entries())
   {
      return (Stand *) theSpGlobals->runStream->subsetStands->first();
   }
   return NULL;
}



RWDEFINE_COLLECTABLE(MyUserData,MYUSERDATA)

  MyUserData::MyUserData(RWCollectable * a, int n)
    :RWCollectable(),
     itsItem(a),
     itsSequence(n),
     itsKind(a->isA())
{}


MyUserData::MyUserData(MyUserData * c)
  :RWCollectable(),
   itsItem(c->itsItem),
   itsSequence(c->itsSequence),
   itsKind(c->itsKind)
{}


MyUserData::MyUserData( void )
  :RWCollectable(),
   itsItem(NULL),
   itsSequence(0),
   itsKind(0)
{}


MyUserData::~MyUserData( void )
{}


MyCTreeSorter::MyCTreeSorter( void )
  :CTreeStringSorter()
{}


CTreeSorter::Comparison MyCTreeSorter::Compare(const CTreeItem* theFirstItem, const CTreeItem* theSecondItem) const
{
   int s1 = -99999;
   int s2 = 99999;
   const RWCollectable *userData1 = theFirstItem->GetUserData();
   if (userData1) s1 = ((MyUserData *) userData1)->itsSequence;
   const RWCollectable *userData2 = theSecondItem->GetUserData();
   if (userData2) s2 = ((MyUserData *) userData2)->itsSequence;

   if      (s1 > s2) return CTreeSorter::GreaterThan;
   else if (s1 < s2) return CTreeSorter::LessThan;
   else              return CTreeSorter::EqualTo;
}


//////////////////////////////////////////////////////////////////////
CsupposeWin::MyCTreeSource::MyCTreeSource(CsupposeWin* theEnclosure) :
   itsEnclosure(theEnclosure)
{
}

//////////////////////////////////////////////////////////////////////
CsupposeWin::MyCTreeSource::~MyCTreeSource()
{
}

//////////////////////////////////////////////////////////////////////
RWGVector(CTreeItemInfo)* CsupposeWin::MyCTreeSource::GetTreeData(CTreeItem * theParent) const
{
   CTreeNodeItem * aParentNode = (CTreeNodeItem *) theParent;
   RWGVector(CTreeItemInfo) * anInfoList = new RWGVector(CTreeItemInfo);

#if ENABLE_TREE_VIEW_SOURCE

   MyUserData * userData = (MyUserData *) aParentNode->GetUserData();
	if ((userData != NULL) && (userData->itsKind == STAND))
   {
      Stand * stand = (Stand *) userData->itsItem;
      CTreeNodeItem * standRoot = aParentNode;
      CTreeNodeItem * currRoot = standRoot;

      if (stand->componentsOrGroups)
      {
         RWOrderedIterator nextComponentOrGroup(*stand->componentsOrGroups);
         RWCollectable * componentOrGroup;
         while (componentOrGroup = (RWCollectable *) nextComponentOrGroup())
         {
            if (componentOrGroup->isA() == COMPONENT)
            {
               currRoot = itsEnclosure->AddComponent(currRoot,(Component*) componentOrGroup);
            }
            else if (componentOrGroup->isA() == GROUP)
            {
               currRoot = itsEnclosure->AddGroup(standRoot,(Group*) componentOrGroup);
               if (((Group *) componentOrGroup)->componentList)
               {
                  RWOrderedIterator nextComponent(*((Group *) componentOrGroup)->componentList);
                  Component * component;
                  while (component = (Component *) nextComponent())
                  {
                     currRoot = itsEnclosure->AddComponent(currRoot,component);
                  }
                  currRoot = standRoot;
               }
            }
         }
      }

      // create a list of groups that
      // contain the stand and have no components.  First step
      // is to see if any of such groups exist.  IF they do, then
      // create the additional groups tree node and add the
      // groups to that node... (done in one pass).
      CTreeNodeItem * moreGroups = NULL;
      RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
      Group * group;
      while ((group = (Group *) nextGroup()) != NULL)
      {
         // skip if the group has a compoenent list.

         if (!group->componentList &&
               group->standMembership &&
               group->standMembership->contains(stand))
         {
            if (!moreGroups)
            {
               moreGroups=standRoot-> AddNodeChild("Group(s) with no attached components:");
               moreGroups->SetColor(COLOR_BLACK);
               moreGroups->Collapse();
               moreGroups->SetUserData(NULL);
            }

            CTreeItem * newItem = moreGroups->AddTerminalChild("Group: " + *group);
            newItem->SetColor(COLOR_BLACK);
            newItem->SetUserData(new MyUserData(group,itsEnclosure->itsTreeSequence++));
            if (group == theSpGlobals->runStream->subsetGroups->first())
            {
               if (!moreGroups->GetParent()->IsCollapsed())
               {
                  moreGroups->Expand();
               }
               newItem->SetColor(COLOR_RED);
            }
            else
               newItem->SetColor(COLOR_BLACK);
         }
      }
   }

#endif // ENABLE_TREE_VIEW_SOURCE

   // Return an empty list since we already created our children
   return ( anInfoList );
}

