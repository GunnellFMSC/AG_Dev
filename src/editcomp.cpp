/*********************************************************************

  File Name:    editcomp.cpp
  Author:       nlc & kfd
  Date:         01/27/95 (extended 12/05/95 and 04/22/96)
                Modified by pg and nlc Feb 2006

    see editcomp.hpp for notes.

*********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include "runstr.hpp"
#include "extens.hpp"
#include "compon.hpp"
#include "cretcomp.hpp"
#include "group.hpp"
#include "stand.hpp"

#include CImage_i
#include CTreeView_i
#include CTreeEvent_i
#include CEnvironment_i
#include NListBox_i
#include NLineText_i
#include NText_i
#include CText_i
#include NButton_i
#include NGroupBox_i
#include NEditControl_i
#include CDesktop_i
#include CNavigator_i
#include CStringCollection_i
#include CWindow_i
#include CStringRWC_i

#include "suppdefs.hpp"
#include "warn_dlg.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include CMenuBar_i
#include CTaskWin_i
#include CRect_i

#include "dirsept.hpp"
#include "appdoc.hpp"
#include "clearwin.hpp"
#include "seedwin.hpp"
#include "sheltwin.hpp"
#include "editcomp.hpp"
#include "readcomp.hpp"
#include "mainwin.hpp"
#include "viewkey.hpp"
#include "gpkey.hpp"
#include "express.hpp"
#include "schcond.hpp"
#include "thinwin.hpp"
#include "plantwin.hpp"

#include "sendupdt.hpp"
#include "suppdefs.hpp"
#include "supphelp.hpp"

extern XVT_HELP_INFO helpInfo;

#define ENABLE_TREE_VIEW_SOURCE 1


//inline UNITS max(UNITS x,UNITS y) {return((x)>(y) ? (x):(y));}
//inline UNITS min(UNITS x,UNITS y) {return((x)<(y) ? (x):(y));}

EditComponent::EditComponent(CDocument *theDocument)
  :CWwindow(theDocument,
            AdjWinPlacement(CRect(20,20,660,645)),
            "Edit Simulation",
            WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
            W_DOC, TASK_MENUBAR),
   itsCurrentComponent(NULL),
   itsCurrentGroup(NULL),
   itsCurrentStand(NULL),
   itsSource(NULL)
{

   // make sure there is room for this window.
RCT rct;
xvt_vobj_get_outer_rect(TASK_WIN, &rct);


   CRect taskFrame = CBoss::G->GetTaskWin()->GetFrame();
   if (taskFrame.Bottom()<630 || taskFrame.Right()<800)
   {
      UNITS screenHeight = (UNITS) xvt_vobj_get_attr(NULL, ATTR_SCREEN_HEIGHT);
      UNITS screenWidth  = (UNITS) xvt_vobj_get_attr(NULL, ATTR_SCREEN_WIDTH);
      UNITS addWidth = (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_DOCFRAME_WIDTH);
      UNITS addVert  = (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_DOCFRAME_HEIGHT) +
                       (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_TITLE_HEIGHT)    +
                       (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_MENU_HEIGHT); 
      taskFrame.Top(addVert);
      addVert = addVert + (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_TITLE_HEIGHT);
      taskFrame.Left(max(((screenWidth/2)-400)+addWidth,0));
      taskFrame.Bottom(min(max(taskFrame.Bottom(),(UNITS) 630)+addVert,screenHeight-addVert));
      taskFrame.Right (min(taskFrame.Left()+max(800+addWidth,taskFrame.Right()),
         screenWidth-addWidth));
      CBoss::G->GetTaskWin()->Size(taskFrame);
   }
   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   addToSimGroupBox = new NGroupBox (this,CRect(4,4,636,64),
       "Add stands and components to simulation",0L);
   addToSimGroupBox->SetGlue(TOPSTICKY | LEFTSTICKY | RIGHTSTICKY);

   selStandsButton = new NButton(this, CRect(8,24,126,56),"Select Stands");
   selStandsButton->SetCommand(10);
   selStandsButton->SetGlue(TOPSTICKY | LEFTSTICKY);

   useKeywordsButton = new NButton(this, CRect(394,24,512,56),"Use Keywords");
   useKeywordsButton->SetCommand(11);
   useKeywordsButton->SetGlue(TOPSTICKY | LEFTSTICKY);

   selMgmtActionsButton = new NButton(this, CRect(132,24,271,56),"Select Management");
   selMgmtActionsButton->SetCommand(12);
   selMgmtActionsButton->SetGlue(TOPSTICKY | LEFTSTICKY);

   readButton = new NButton(this, CRect(518,24,632,56),"Insert from file");;
   readButton->SetCommand(7);
   readButton->SetGlue(TOPSTICKY | LEFTSTICKY);

   selReportsButton = new NButton(this, CRect(277,24,388,56),"Select Outputs");
   selReportsButton->SetCommand(13);
   selReportsButton->SetGlue(TOPSTICKY | LEFTSTICKY);

   editSimGroupBox = new NGroupBox (this,CRect(4,414,636,510),
       "Edit simulation contents",0L);
   editSimGroupBox->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   editButton = new NButton(this, CRect(8,436,158,468), "Edit Selection");
   editButton->SetCommand(2);
   editButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   changeGroupsButton = new NButton(this, CRect(8,474,220,506),
      "Change Group Membership");
   changeGroupsButton->SetCommand(18);
   changeGroupsButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   pasteButton = new NButton(this, CRect(228,474,632,506));
   pasteButton->SetCommand(3);
   pasteButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   cutButton = new NButton(this, CRect(545,436,632,468),"Cut");
   cutButton->SetCommand(4);
   cutButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   copyButton = new NButton(this, CRect(450,436,537,468),"Copy");;
   copyButton->SetCommand(5);
   copyButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   deleteButton = new NButton(this, CRect(165,436,252,468),"Delete");
   deleteButton->SetCommand(6);
   deleteButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   writeButton = new NButton(this, CRect(260,436,347,468),"Write");
   writeButton->SetCommand(8);
   writeButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   appendButton = new NButton(this, CRect(355,436,442,468),"Append");
   appendButton->SetCommand(8);
   appendButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   runOptionsGroupBox = new NGroupBox (this,CRect(4,512,471,567),
       "Simulation",0L);
   runOptionsGroupBox->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   timeScaleButton= new NButton(this, CRect(8,531,158,563), "Set Time Scale",0L);
   timeScaleButton->SetCommand(14);
   timeScaleButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   postProcsButtion= new NButton(this, CRect(166,531,335,563), "Select Post Processors",0L);
   postProcsButtion->SetCommand(15);
   postProcsButtion->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   runSimButton= new NButton(this, CRect(343,531,467,563),"Run Simulation",0L);
   runSimButton->SetCommand(16);
   runSimButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   standCountText = new NText(this, CRect(485,524,634,544),NULLString,0L);
   standCountText->SetGlue(BOTTOMSTICKY | LEFTSTICKY );
   groupCountText = new NText(this, CRect(485,548,634,568),NULLString,0L);
   groupCountText->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   afterRunGroupBox = new NGroupBox (this,CRect(4,570,471,624),
       "After simulation",0L);
   afterRunGroupBox->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   readFVSOutputButton= new NButton(this, CRect(8,587,158,619), "Read FVS Outputs",0L);
   readFVSOutputButton->SetCommand(19);
   readFVSOutputButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   genGraphsButton= new NButton(this, CRect(166,587,312,619), "Generate Graphs",0L);
   genGraphsButton->SetCommand(20);
   genGraphsButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   genReportsButton= new NButton(this, CRect(320,587,467,619), "Generate Reports",0L);
   genReportsButton->SetCommand(21);
   genReportsButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY );

   //exitButton = new NButton(this, CRect(475,587,551,619), "Exit");
   //exitButton->SetCommand(17);
   //exitButton->SetGlue(BOTTOMSTICKY |  RIGHTSTICKY );

   //closeButton = new NButton(this, CRect(558,587,634,619), "Close");
   //closeButton->SetCommand(1);
   //closeButton->SetGlue(BOTTOMSTICKY | RIGHTSTICKY );

   NText * aText = new NText(this, CRect(444,69,632,88),
                             "Affected Stands", 0L);
   aText->SetGlue(TOPSTICKY | RIGHTSTICKY);

   // Create the CTreeView and call its initializer method (required).
   // Create the images from resources.

   expandedImage  = new CImage(EXPANDEDIMAGE);
   collapsedImage = new CImage(COLLAPSEDIMAGE);
   termnodeImage  = new CImage(TERMNODEIMAGE);

   CFont aFont = xvt_dwin_get_font(theSpGlobals->
                                   theMainWindow->GetXVTWindow());

#if ENABLE_TREE_VIEW_SOURCE
   itsSource = new MyCTreeSource(this);
#endif // ENABLE_TREE_VIEW_SOURCE

   componentTreeView = new CTreeView(this, CRect(8,73,436,412));
   componentTreeView->ITreeView(itsSource,
                                COLOR_BLACK,
                                aFont,
                                CTreeItem::kBestHeight,
                                10,   //CTreeItem::kBestIndent,
                                termnodeImage,
                                expandedImage,
                                collapsedImage,
                                NULL,
                                TRUE,TRUE,TRUE,10,50,TRUE,
                                BOTTOMSTICKY | TOPSTICKY |
                                RIGHTSTICKY  | LEFTSTICKY);

   componentTreeView->SetDrawRoot(TRUE);

   CTreeNodeItem * theRoot = componentTreeView->GetRoot();
   theRoot->SortChildren(&itsSorter);
   theRoot->Expand();

   // Stands
   standListBox = new NListBox(this, CRect(444,92,632,412), standList, 0L);
   standListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY | RIGHTSTICKY);
   standListBox->SetCommand(9);

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), EDITSimulationFile, 0L);

   //itsNavigator->AppendTabStop(new CTabStop(closeButton));
   itsNavigator->AppendTabStop(new CTabStop(selStandsButton));
   itsNavigator->AppendTabStop(new CTabStop(useKeywordsButton));
   itsNavigator->AppendTabStop(new CTabStop(selMgmtActionsButton));
   itsNavigator->AppendTabStop(new CTabStop(selReportsButton));
   itsNavigator->AppendTabStop(new CTabStop(readButton));
   itsNavigator->AppendTabStop(new CTabStop(componentTreeView));
   itsNavigator->AppendTabStop(new CTabStop(deleteButton));
   itsNavigator->AppendTabStop(new CTabStop(writeButton));
   itsNavigator->AppendTabStop(new CTabStop(appendButton));
   itsNavigator->AppendTabStop(new CTabStop(cutButton));
   itsNavigator->AppendTabStop(new CTabStop(copyButton));
   itsNavigator->AppendTabStop(new CTabStop(editButton));
   itsNavigator->AppendTabStop(new CTabStop(pasteButton));
   itsNavigator->AppendTabStop(new CTabStop(timeScaleButton));
   itsNavigator->AppendTabStop(new CTabStop(postProcsButtion));
   itsNavigator->AppendTabStop(new CTabStop(runSimButton));
   itsNavigator->AppendTabStop(new CTabStop(readFVSOutputButton));
   itsNavigator->AppendTabStop(new CTabStop(genGraphsButton));
   itsNavigator->AppendTabStop(new CTabStop(genReportsButton));
   //itsNavigator->AppendTabStop(new CTabStop(exitButton));

   DoCommand(STANDCountChanged,this);

   BuildTreeView();
   ResetWindow(StandToExpand(), 1);

   itsNavigator->InitFocus();
}




void EditComponent::DoCommand(long theCommand, void* theData)
{
   switch (theCommand)
   {
      //case 1:                         // Close button pushed
      //  Close();
      //  break;

      case 2:                         // Edit button pushed
        LaunchComponentWindow();
        break;

      case 3:                         // Paste button
        DoMenuCommand(M_EDIT_PASTE, 0, 0);
        break;

      case 4:                         // Cut button
        DoMenuCommand(M_EDIT_CUT,   0, 0);
        break;

      case 5:                         // Copy button
        DoMenuCommand(M_EDIT_COPY,  0, 0);
        break;

      case 6:                         // Delete button
        DoMenuCommand(M_EDIT_CLEAR, 0, 0);
        break;

      case 7:                         // Insert from file
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
               ReadComponentsFromFile(filePtr, fileSpec, &componentsFromFile);
               fclose(filePtr);

               likelyInserts=componentsFromFile.entries();

               theSpGlobals->runStream->
                     ProcessPPEComponents(&componentsFromFile);

               if (componentsFromFile.entries())
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
                     if (componentsFromFile.entries() == 1)
                        what = ((Component *)
                                componentsFromFile.first())->data();
                     DefinePlacementArgs(what,&placeArg,&after);
                  }
                  int initAfter = after;
                  for (size_t i=0; i<componentsFromFile.entries(); i++)
                  {
                     componentToInsert = (Component *) componentsFromFile(i);

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

                  componentsFromFile.clear();

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
                  ResetWindow(componentsFromFile.last());
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

      case 8:                         // Write to file
      {
         xvt_fsys_save_dir();
         FILE_SPEC * fileSpec = new FILE_SPEC;
         fileSpec->name[0]=NULL;
         strcpy(fileSpec->type,"kcp");
         xvt_fsys_get_dir(&fileSpec->dir);

         const char * opSp;
         FL_STATUS reply;

         if (theData == appendButton)
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

      case 9:  // Stand List Selection
      {
         int pos = standListBox->GetSelectPosition();
         Stand * aStand = NULL;
         if (pos >= 0 && pos < (int) standList.entries())
            aStand = (Stand *) standList.at((size_t) pos);
         ResetWindow(aStand);
         break;
      }

      case 10:  // Select stands
      {
         DoMenuCommand(M_BASICS_SIMULATION, 0, 0);
         break;
      }

      case 11:  // Use Keywords
      {
         DoMenuCommand(M_BASICS_USEKEYWORDS, 0, 0);
         break;
      }

      case 12:  // Select Mgmt Actions
      {
         DoMenuCommand(M_BASICS_MANAGEMENT, 0, 0);
         break;
      }

      case 13:  // Select Reports (model outputs)
      {
         DoMenuCommand(M_BASICS_MODEL, 0, 0);
         break;
      }

      case 14:
      {
         DoMenuCommand(M_BASICS_TIME, 0, 0);
         break;
      }

      case 15:
      {
         DoMenuCommand(M_BASICS_POST_PROCESSOR, 0, 0);
         break;
      }

      case 16:
      {
         DoMenuCommand(M_BASICS_RUN, 0, 0);
         break;
      }

      //case 17:
      //{
      //   DoMenuCommand(M_FILE_QUIT, 0, 0);
      //   break;
      //}

      case 18:
      {
         DoMenuCommand(M_BASICS_CURRENT, 0, 0);
         break;
      }

      case 19:
      {
         DoMenuCommand(M_EXTRAS_READFVSOUTPUT, 0, 0);
         break;
      }

      case 20:
      {
         DoMenuCommand(M_BASICS_GRAPHICS, 0, 0);
         break;
      }

      case 21:
      {
         DoMenuCommand(M_BASICS_REPORTS, 0, 0);
         break;
      }

      case STANDCountChanged:
      case GROUPCountChanged:
      {
         int nItems;
         if (theSpGlobals->runStream->allStands)
              nItems = theSpGlobals->runStream->allStands->entries();
         else nItems = 0;
         if (nItems==1) standCountText->SetTitle("1 Stand");
         else           standCountText->SetTitle(ConvertToString(nItems)+" Stands");

         if (nItems) 
         {
            runSimButton->Enable();
            changeGroupsButton->Enable();
         }
         else
         {
            runSimButton->Disable();
            changeGroupsButton->Disable();
         }

         if (theSpGlobals->runStream->allGroups)
              nItems = theSpGlobals->runStream->allGroups->entries();
         else nItems = 0;
         if (nItems==1) groupCountText->SetTitle("1 Group");
         else           groupCountText->SetTitle(ConvertToString(nItems)+" Groups");
         if (theData == this) break;
      }

      case GROUPMembershipChanged:
      case CURRStandOrGroupChange:
      {
         BuildTreeView();
         ResetWindow(StandToExpand(),1);
         break;
      }

      case GROUPNameChanged:
      case STANDNameChanged:
      case COMPONENTSListChanged:
      {
         RWOrdered selectionList = componentTreeView->GetSelection();
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

      // Handle tree events
      case TreeEventCmd:
      {
         if (!theData) break;

         // event types:
         // 0=FocusIn,  1=FocusOut,  2=SelectionChanged,  3=Expand,
         // 4=Collapse, 5=MouseClick,  6=TitleMouseClick

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
                           SendUpdateMessage(CURRStandOrGroupChange,
                                             itsCurrentGroup,this);
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
                           SendUpdateMessage(CURRStandOrGroupChange,
                                             itsCurrentStand,this);
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
               if (editButton->IsEnabled())
                  DoCommand(2, NULL);
               break;
            }
            case CTreeEvent::Collapse:
            {
               CTreeNodeItem * theRoot = componentTreeView->GetRoot();
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
        CWwindow::DoCommand(theCommand,theData);
   }
}




void EditComponent::Key( const CKey& theKey )
{
   switch (theKey.GetChar())
   {
      case K_DEL:
         // Perform Delete command
         if (deleteButton->IsEnabled())
            DoCommand(6, NULL);
         break;

      default:
         break;
   }

   // Call base class method
   CWindow::Key( theKey );
}




void EditComponent::ResetWindow(RWCollectable * selectedElement, int clearSelection)
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
      LoadSelectedList(componentTreeView->GetRoot(),
                       selectedElement,
                       selList);
      anItem = FindFirst(componentTreeView->GetRoot(),
                         selectedElement);
      componentTreeView->SetSelection(selList,TRUE);

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

      pasteButton->SetTitle(pasteTitle);
      pasteButton->Enable();
   }
   else
   {
      GetMenuBar()->SetEnabled (M_EDIT_PASTE, FALSE);
      pasteButton->SetTitle("Paste");
      pasteButton->Disable();
   }

   BuildStandList();

   if (clearSelection)
   {
      componentTreeView->ClearSelection();
      itsCurrentComponent = NULL;
      itsCurrentGroup = NULL;
      itsCurrentStand = NULL;
   }

   // These options are set based on selction
   if (itsCurrentComponent)
   {
      GetMenuBar()->SetEnabled (M_EDIT_CUT,            TRUE);
      GetMenuBar()->SetEnabled (M_EDIT_COPY,           TRUE);
      GetMenuBar()->SetEnabled (M_EDIT_CLEAR,          TRUE);
      GetMenuBar()->SetEnabled (M_EXTRAS_VIEWKEYWORDS, TRUE);
      cutButton->Enable();
      copyButton->Enable();
      deleteButton->Enable();
      editButton->Enable();
      writeButton->Enable();
      appendButton->Enable();
   }
   else
   {
      GetMenuBar()->SetEnabled (M_EDIT_CUT,            FALSE);
      GetMenuBar()->SetEnabled (M_EDIT_COPY,           FALSE);
      GetMenuBar()->SetEnabled (M_EDIT_CLEAR,          FALSE);
      GetMenuBar()->SetEnabled (M_EXTRAS_VIEWKEYWORDS, FALSE);
      cutButton->Disable();
      copyButton->Disable();
      if (itsCurrentStand || itsCurrentGroup)
      {
         deleteButton->Enable();
         GetMenuBar()->SetEnabled (M_EDIT_CLEAR, TRUE);
      }
      else
      {
         deleteButton->Disable();
         GetMenuBar()->SetEnabled (M_EDIT_CLEAR, FALSE);
      }
      editButton->Disable();
      writeButton->Disable();
      appendButton->Disable();
   }

   if (anItem)
   {
      ExpandMostChildren(anItem);
      componentTreeView->MakeItemVisible(anItem);
   }

   if (IsVisible()) Invalidate(componentTreeView->GetFrame());
   else             Show();
}




void EditComponent::BuildTreeView( void )
{
   xvt_scr_set_busy_cursor();

   // first clear the current tree.

   CTreeNodeItem * theRoot = componentTreeView->GetRoot();

   if (!theRoot) return;

   ClearChildren(theRoot);
   theRoot->SetUserData(NULL);
   treeSequence=0;

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
               newItem->SetUserData(new MyUserData(group,treeSequence++));
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
   componentTreeView->ScrollViews(CPoint(0,0));
   if (IsVisible())
      Invalidate(componentTreeView->GetFrame());
}


void EditComponent::BuildStandList( void )
{
   standList.clear();

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
                  if (!standList.contains(stand)) standList.insert(stand);
            }
         }
      }
      RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
      while ((stand = (Stand *) nextStand()) != NULL)
      {
         if (stand->componentsOrGroups &&
             stand->componentsOrGroups->contains(itsCurrentComponent))
            if (!standList.contains(stand)) standList.insert(stand);
      }
   }

   else if (itsCurrentGroup)
   {
      if (itsCurrentGroup->standMembership)
      {
         Stand * stand;
         RWOrderedIterator nextStand(*itsCurrentGroup->standMembership);
         while (stand = (Stand *) nextStand()) standList.insert(stand);
      }
   }

   else if (itsCurrentStand) standList.insert(itsCurrentStand);

   else
   {
      Stand * stand;
      RWOrderedIterator nextStand(*theSpGlobals->runStream->allStands);
      while ((stand = (Stand *) nextStand()) != NULL)
         standList.insert(stand);
   }

   standListBox->IListBox(standList);
}


void EditComponent::DoMenuCommand(MENU_TAG theMenuItem,
                                  BOOLEAN isShiftkey,
                                  BOOLEAN isControlKey)
{
   switch(theMenuItem)
   {
      case M_EXTRAS_VIEWKEYWORDS:
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
            CWindow::DoCommand(M_EXTRAS_DELETESTAND,itsCurrentStand);

         else if (itsCurrentGroup)
            CWindow::DoCommand(M_EXTRAS_DELETEGROUP,itsCurrentGroup);

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
        CWindow::DoMenuCommand(theMenuItem,isShiftkey,isControlKey);
        break;
   }
}


void EditComponent::LaunchComponentWindow( void )
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




void EditComponent::DefinePlacementArgs(const char * what,
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



void EditComponent::ClearChildren(CTreeNodeItem * aNode)
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



void EditComponent::SortAllChildren(CTreeItem * aNode,
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



void EditComponent::LoadSelectedList(CTreeItem* aNode,
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



void EditComponent::ExpandMostChildren(CTreeItem* aNode)
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



CTreeItem * EditComponent::FindFirst(CTreeItem* aNode,
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



CTreeNodeItem * EditComponent::AddStand(CTreeNodeItem * root,
                                        Stand * aStand)
{
   CTreeNodeItem * newItem = root->AddNodeChild("Stand: " + *aStand);
   newItem->Collapse();
   newItem->SetUserData(new MyUserData(aStand,treeSequence++));
   if (aStand == theSpGlobals->runStream->subsetStands->first()) newItem->SetColor(COLOR_RED);
   else                                                          newItem->SetColor(COLOR_BLACK);

   return newItem;
}



CTreeNodeItem * EditComponent::AddGroup(CTreeNodeItem * root,
                                        Group * aGroup)
{
   CTreeNodeItem * newItem = root->AddNodeChild("Group: " + *aGroup);
   newItem->Expand();
   newItem->SetUserData(new MyUserData(aGroup,treeSequence++));
   if (aGroup == theSpGlobals->runStream->subsetGroups->first()) newItem->SetColor(COLOR_RED);
   else                                                          newItem->SetColor(COLOR_BLACK);

   return newItem;
}


CTreeNodeItem * EditComponent::AddComponent(CTreeNodeItem * root,
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
         root->SetUserData(new MyUserData(aComponent,treeSequence++));
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
            newItem->SetUserData(new MyUserData(aComponent,treeSequence++));
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
         newItem->SetUserData(new MyUserData(aComponent,treeSequence++));
      }
   }
   return root;
}

int EditComponent::AddPPEList( CStringRWC strPpeName, RWOrdered & ppeList )
{
   CTreeNodeItem * theRoot = componentTreeView->GetRoot();

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
      newItem->SetUserData(new MyUserData(ppeComponent,treeSequence++));
   }

   return 0;
}


Stand * EditComponent::StandToExpand( void )
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



//RWDEFINE_COLLECTABLE(MyUserData,MYUSERDATA)
//
//  MyUserData::MyUserData(RWCollectable * a, int n)
//    :RWCollectable(),
//     itsItem(a),
//     itsSequence(n),
//     itsKind(a->isA())
//{}
//
//
//MyUserData::MyUserData(MyUserData * c)
//  :RWCollectable(),
//   itsItem(c->itsItem),
//   itsSequence(c->itsSequence),
//   itsKind(c->itsKind)
//{}
//
//
//MyUserData::MyUserData( void )
//  :RWCollectable(),
//   itsItem(NULL),
//   itsSequence(0),
//   itsKind(0)
//{}
//
//
//MyUserData::~MyUserData( void )
//{}
//
//
//MyCTreeSorter::MyCTreeSorter( void )
//  :CTreeStringSorter()
//{}
//
//
//CTreeSorter::Comparison MyCTreeSorter::Compare
//(const CTreeItem* theFirstItem,
// const CTreeItem* theSecondItem) const
//{
//   int s1 = -99999;
//   int s2 = 99999;
//   const RWCollectable * userData1 = theFirstItem->GetUserData();
//   if (userData1) s1 = ((MyUserData *) userData1)->itsSequence;
//   const RWCollectable * userData2 = theSecondItem->GetUserData();
//   if (userData2) s2 = ((MyUserData *) userData2)->itsSequence;
//
//   if      (s1 > s2) return CTreeSorter::GreaterThan;
//   else if (s1 < s2) return CTreeSorter::LessThan;
//   else              return CTreeSorter::EqualTo;
//}


//////////////////////////////////////////////////////////////////////
EditComponent::MyCTreeSource::MyCTreeSource(EditComponent* theEnclosure) :
   itsEnclosure(theEnclosure)
{
}

//////////////////////////////////////////////////////////////////////
EditComponent::MyCTreeSource::~MyCTreeSource()
{
}

//////////////////////////////////////////////////////////////////////
RWGVector(CTreeItemInfo)* EditComponent::MyCTreeSource::GetTreeData(CTreeItem * theParent) const
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
            newItem->SetUserData(new MyUserData(group,itsEnclosure->treeSequence++));
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

