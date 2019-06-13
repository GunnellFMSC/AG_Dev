/*********************************************************************

   File Name:    editlocs.cpp
   Author:       nlc
   Date:         04/11/97 and 10/2006

   see editlocs.hpp for notes.

   Updated
		Author:	 apg
		Date:	 1/25/2018

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include NText_i
#include NButton_i
#include CDesktop_i
#include CMenuBar_i
#include CDocument_i
#include NRadioButton_i
#include NLineText_i
#include NRadioNavigator_i
#include CNavigator_i

#include "sendupdt.hpp"
#include "suppdefs.hpp"
#include "spprefer.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "dirsept.hpp"

#include CWindow_i
#include CStringRWC_i

#include "spglobal.hpp"

#include "editlocs.hpp"
#include "mylinetx.hpp"

#include "supphelp.hpp"  // for help #defines

extern XVT_HELP_INFO helpInfo;

EditLocationWin::EditLocationWin( CDocument * theDocument )
                :CWwindow(theDocument,
                          CenterTopWinPlacement(CRect(10,24,576,330)),
                          "Edit Locations File",
                          WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                          W_DOC, TASK_MENUBAR),
                 itsEditingType(1),
                 itsCurrentTypeA(0),
                 itsCurrentTypeB(0),
                 itsCurrentTypeC(0)
{
   xvt_scr_set_busy_cursor();

   // create and clear the navigator

   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   new NText(this, CRect(4,8,126,32), "Locations File:", 0L);

   itsLocsFileNameNLineText = new MyNLineText ( this,
                                               CPoint(130,6),
                                               344, 1, 2, SZ_FNAME);

   itsLocsFileNameNLineText->SetText(theSpGlobals->thePreferences->
                                     defaultLocationsFileName);

   itsBrowseButton = new NButton(this, CRect(482,4,562,36), "Browse");
   itsBrowseButton->SetCommand(3);

   // location (type A) or group (type B) radio buttons

   itsABRadioGroup = new CRadioGroup(this, CPoint(4,40), FALSE);

   itsTypeA = itsABRadioGroup->AddButton(CPoint(0,0),
              "Edit records that define Locations using Stand List Files (Type A)",4);

   itsTypeB = itsABRadioGroup->AddButton(CPoint(0,22),
              "Edit records that define Group Codes and AddFiles (Type B)",5);

   itsTypeC = itsABRadioGroup->AddButton(CPoint(0,44),
              "Edit records that define Locations using Databases (Type C)",23);

   itsTypeAButton = (NRadioButton*) itsABRadioGroup->FindSubview(itsTypeA);
   itsTypeBButton = (NRadioButton*) itsABRadioGroup->FindSubview(itsTypeB);
   itsTypeCButton = (NRadioButton*) itsABRadioGroup->FindSubview(itsTypeC);

   itsABRadioGroup->SetSelectedButton(itsTypeA);

   itsRecordCountNText = new NText(this, CRect(4,115,124,139),
                                   NULLString, 0L);

   itsPreviousButton = new NButton(this, CRect(130,111,214,144), "<<Previous");
   itsPreviousButton->SetCommand(6);
   itsNextButton = new NButton(this, CRect(218,111,302,144), "Next>>");
   itsNextButton->SetCommand(7);
   itsNewButton = new NButton(this, CRect(306,111,390,144), "New");
   itsNewButton->SetCommand(8);
   itsDeleteButton = new NButton(this, CRect(394,111,478,144), "Delete");
   itsDeleteButton->SetCommand(9);
   itsDuplicateButton = new NButton(this, CRect(482,111,562,144), "Duplicate");
   itsDuplicateButton->SetCommand(10);

   itsLocNameNText = new NText(this, CRect(4,158,126,182),
                               "Location Name:", 0L);
   itsLocNameNLineText = new NLineText (this, CPoint(130,156), 344);

   itsSLFNameNText = new NText(this, CRect(4,192,126,206),
                               "Stand List File:", 0L);
   itsSLFNLineText = new MyNLineText ( this,
                                      CPoint(130,188),
                                      344, 13, 14, SZ_FNAME);
   itsSLFBrowseButton = new NButton(this, CRect(482,186,562,218), "Browse");
   itsSLFBrowseButton->SetCommand(15);


   itsGroupCodeNText = new NText(this, CRect(4,158,126,182),
                                 "Group Code:", 0L);
   itsGroupCodeNText->DoHide();
   itsGroupCodeNLineText = new MyNLineText ( this, CPoint(130,156),
                                            344, 0, 0, 50);
   itsGroupCodeNLineText->DoHide();
   itsAddFileNText = new NText(this, CRect(4,192,126,206),
                               "Add File(s):", 0L);
   itsAddFileNText->DoHide();
   itsAddFileNLineText = new NLineText ( this, CPoint(130,188), 344);
   itsAddFileNLineText->DoHide();
   itsAddFileBrowseButton = new NButton(this, CRect(482,186,562,218),
                                        "Browse");
   itsAddFileBrowseButton->DoHide();
   itsAddFileBrowseButton->SetCommand(20);

   itsDataSourceNText = new NText(this, CRect(4,192,126,206),
                            "Data Source Name:", 0L);
   itsDataSourceNText->DoHide();
   itsDataSourceNLineText = new MyNLineText ( this, CPoint(130,188),
                                            344, 0, 0, 50);
   itsDataSourceNLineText->DoHide();
   itsDataSourceBrowseButton = new NButton(this, CRect(482,186,562,218),
                                        "Browse");
   itsDataSourceBrowseButton->DoHide();
   itsDataSourceBrowseButton->SetCommand(24);

   itsUserNameNText = new NText(this, CRect(4,232,126,264),
                               "User Name:", 0L);
   itsUserNameNText->DoHide();
   itsUserNameNLineText = new MyNLineText ( this, CPoint(130,228),
                                            172, 0, 0, 50);
   itsUserNameNLineText->DoHide();
   itsPasswordNText = new NText(this, CRect(318,232,408,264),
                               "Password:", 0L);
   itsPasswordNText->DoHide();
   itsPasswordNLineText = new MyNLineText ( this, CPoint(410,228),
                                            150, 0, 0, 50);
   itsPasswordNLineText->DoHide();

   itsOkButton = new NButton(this, CRect(4,268,88,300), "Ok");
   itsOkButton->SetCommand(21);

   itsSaveButton = new NButton(this, CRect(163,268,247,300), "Save");
   itsSaveButton->SetCommand(11);

   itsSaveAsButton = new NButton(this, CRect(322,268,407,300), "Save As");
   itsSaveAsButton->SetCommand(12);

   itsCancelButton = new NButton(this, CRect(482,268,562,300), "Cancel");
   itsCancelButton->SetCommand(22);

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), EDITLOCSWindow, 0L);

   ReadFile();
   DisplayRecord();

   aNavigator->AppendTabStop(new CTabStop(itsLocsFileNameNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsBrowseButton));
   aNavigator->AppendTabStop(new CTabStop
                             (new NRadioNavigator(itsABRadioGroup)));
   aNavigator->AppendTabStop(new CTabStop(itsPreviousButton));
   aNavigator->AppendTabStop(new CTabStop(itsNextButton));
   aNavigator->AppendTabStop(new CTabStop(itsNewButton));
   aNavigator->AppendTabStop(new CTabStop(itsDeleteButton));
   aNavigator->AppendTabStop(new CTabStop(itsDuplicateButton));

   aNavigator->AppendTabStop(new CTabStop(itsLocNameNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsSLFNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsSLFBrowseButton));
   aNavigator->AppendTabStop(new CTabStop(itsGroupCodeNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsAddFileNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsAddFileBrowseButton));

   aNavigator->AppendTabStop(new CTabStop(itsOkButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveAsButton));
   aNavigator->AppendTabStop(new CTabStop(itsCancelButton));

   Show();
   aNavigator->InitFocus();
}






EditLocationWin::~EditLocationWin( void )
{
   itsTypeARecords.clearAndDestroy();
   itsTypeBRecords.clearAndDestroy();
   itsTypeCRecords.clearAndDestroy();
}




void EditLocationWin::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                         // File name CR command
      {
         ReadFile();
         DisplayRecord();
         break;
      }
      case 2:                         // File name char change command
      {
         break;
      }
      case 3:                         // File name Browse button
      {
         FILE_SPEC fileSpec;
         strcpy(fileSpec.type,"loc");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec, "Locations File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            itsLocsFileNameNLineText->SetText(path);
            ReadFile();
            DisplayRecord();
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 4:                         // Edit Type A radio button
      {
         itsGroupCodeNText     ->DoHide();
         itsGroupCodeNLineText ->DoHide();
         itsAddFileNText       ->DoHide();
         itsAddFileNLineText   ->DoHide();
         itsAddFileBrowseButton->DoHide();
         itsAddFileBrowseButton->DoHide();
         itsDataSourceNText    ->DoHide();       
         itsDataSourceNLineText->DoHide();   
         itsDataSourceBrowseButton->DoHide();
         itsUserNameNText      ->DoHide();         
         itsUserNameNLineText  ->DoHide();     
         itsPasswordNText      ->DoHide();         
         itsPasswordNLineText  ->DoHide();     

         itsLocNameNText       ->DoShow();
         itsLocNameNLineText   ->DoShow();
         itsSLFNameNText       ->DoShow();
         itsSLFNLineText       ->DoShow();
         itsSLFBrowseButton    ->DoShow();

         StoreCurrent();
         itsEditingType = 1;
         if (itsTypeARecords.entries()) DisplayRecord();
         else DoCommand (8,NULL);
         break;
      }
      case 5:                         // Edit Type B radio button
      {
         itsGroupCodeNText     ->DoShow();
         itsGroupCodeNLineText ->DoShow();
         itsAddFileNText       ->DoShow();
         itsAddFileNLineText   ->DoShow();
         itsAddFileBrowseButton->DoShow();

         itsLocNameNText       ->DoHide();
         itsLocNameNLineText   ->DoHide();
         itsSLFNameNText       ->DoHide();
         itsSLFNLineText       ->DoHide();
         itsSLFBrowseButton    ->DoHide();
         itsDataSourceNText    ->DoHide();       
         itsDataSourceNLineText->DoHide();   
         itsDataSourceBrowseButton->DoHide();
         itsUserNameNText      ->DoHide();         
         itsUserNameNLineText  ->DoHide();     
         itsPasswordNText      ->DoHide();         
         itsPasswordNLineText  ->DoHide();     

         StoreCurrent();
         itsEditingType = 2;
         if (itsTypeBRecords.entries()) DisplayRecord();
         else DoCommand (8,NULL);
         break;
      }
      case 23:                         // Edit Type C radio button
      {
         itsGroupCodeNText     ->DoHide();
         itsGroupCodeNLineText ->DoHide();
         itsAddFileNText       ->DoHide();
         itsAddFileNLineText   ->DoHide();
         itsAddFileBrowseButton->DoHide();

         itsLocNameNText       ->DoShow();
         itsLocNameNLineText   ->DoShow();
         itsSLFNameNText       ->DoHide();
         itsSLFNLineText       ->DoHide();
         itsSLFBrowseButton    ->DoHide();
         itsDataSourceNText    ->DoShow();       
         itsDataSourceNLineText->DoShow();   
         itsDataSourceBrowseButton->DoShow();
         itsUserNameNText      ->DoShow();         
         itsUserNameNLineText  ->DoShow();     
         itsPasswordNText      ->DoShow();         
         itsPasswordNLineText  ->DoShow();     

         StoreCurrent();
         itsEditingType = 3;
         if (itsTypeCRecords.entries()) DisplayRecord();
         else DoCommand (8,NULL);
         break;
      }
      case 6:                         // Previous
      {
         StoreCurrent();
         switch (itsEditingType)
         {
            case 1:
            {
               if (itsTypeARecords.entries())
               {
                  if (itsCurrentTypeA == 0) itsCurrentTypeA =
                                             itsTypeARecords.entries()-1;
                  else itsCurrentTypeA--;
               }
               else itsCurrentTypeA = 0;
               break;
            }
            case 2:
            {
               if (itsTypeBRecords.entries())
               {
                  if (itsCurrentTypeB == 0) itsCurrentTypeB =
                                             itsTypeBRecords.entries()-1;
                  else itsCurrentTypeB--;
               }
               else itsCurrentTypeB = 0;
               break;
            }
            case 3:
            {
               if (itsTypeCRecords.entries())
               {
                  if (itsCurrentTypeC == 0) itsCurrentTypeC =
                                            itsTypeCRecords.entries()-1;
                  else itsCurrentTypeC--;
               }
               else itsCurrentTypeC = 0;
               break;
            }
            default: break;
         }
         DisplayRecord();
         break;
      }
      case 7:                         // Next
      {
         StoreCurrent();
         switch (itsEditingType)
         {
            case 1:
            {
               itsCurrentTypeA++;
               if (itsCurrentTypeA >= itsTypeARecords.entries())
                  itsCurrentTypeA = 0;
               break;
            }
            case 2:
            {
               itsCurrentTypeB++;
               if (itsCurrentTypeB >= itsTypeBRecords.entries())
                  itsCurrentTypeB = 0;
               break;
            }
            case 3:
            {
               itsCurrentTypeC++;
               if (itsCurrentTypeC >= itsTypeCRecords.entries())
                  itsCurrentTypeC = 0;
               break;
            }
            default: break;
         }
         DisplayRecord();
         break;
      }
      case 8:                         // New
      {
         StoreCurrent();
         switch (itsEditingType)
         {
            case 1:
            {
               LocTypeARec * aRec = new LocTypeARec();
               if (itsTypeARecords.entries() == itsCurrentTypeA++)
                  itsTypeARecords.insert(aRec);
               else itsTypeARecords.insertAt(itsCurrentTypeA,aRec);
               break;
            }
            case 2:
            {
               LocTypeBRec * aRec = new LocTypeBRec();
               if (itsTypeBRecords.entries() == itsCurrentTypeB++)
                  itsTypeBRecords.insert(aRec);
               else itsTypeBRecords.insertAt(itsCurrentTypeB,aRec);
               break;
            }
            case 3:
            {
               LocTypeCRec * aRec = new LocTypeCRec();
               if (itsTypeCRecords.entries() == itsCurrentTypeC++)
                  itsTypeCRecords.insert(aRec);
               else itsTypeCRecords.insertAt(itsCurrentTypeC,aRec);
               break;
            }
            default: break; 
         }
         DisplayRecord();
         break;
      }
      case 9:                         // Delete
      {
         switch (itsEditingType)
         {
            case 1:
            {
               itsTypeARecords.removeAndDestroy(itsTypeARecords.
                                                at(itsCurrentTypeA));
               break;
            }
            case 2:
            {
               itsTypeBRecords.removeAndDestroy(itsTypeBRecords.
                                                at(itsCurrentTypeB));
               break;
            }
            case 3:
            {
               itsTypeCRecords.removeAndDestroy(itsTypeCRecords.
                                                at(itsCurrentTypeC));
               break;
            }
            default: break;

         }
         DisplayRecord();
         break;
      }
      case 10:                        // Duplicate
      {
         StoreCurrent();
         switch (itsEditingType)
         {
            case 1:
            {
               if (itsTypeARecords.entries())
               {
                  itsTypeARecords.insertAt(itsCurrentTypeA,
                     new LocTypeARec((LocTypeARec *)
                                    itsTypeARecords.at(itsCurrentTypeA)));
                  itsCurrentTypeA++;
               }
               else xvt_dm_post_error("Nothing to duplicate.");
               break;
            }
            case 2:
            {
               if (itsTypeBRecords.entries())
               {
                  itsTypeBRecords.insertAt(itsCurrentTypeB,
                     new LocTypeBRec((LocTypeBRec *)
                                    itsTypeBRecords.at(itsCurrentTypeB)));
                  itsCurrentTypeB++;
               }
               else xvt_dm_post_error("Nothing to duplicate.");
               break;
            }
            case 3:
            {
               if (itsTypeCRecords.entries())
               {
                  itsTypeCRecords.insertAt(itsCurrentTypeC,
                     new LocTypeCRec((LocTypeCRec *)
                                    itsTypeCRecords.at(itsCurrentTypeC)));
                  itsCurrentTypeC++;
               }
               else xvt_dm_post_error("Nothing to duplicate.");
               break;
            }
            default: break; 
         }
         DisplayRecord();
         break;
      }
      case 11:                        // Save button
      {
         WriteFile();
         break;
      }
      case 12:                        // Save As button
      {

         FILE_SPEC fileSpec;
         fileSpec.name[0]=0;
         strcpy(fileSpec.type,"loc");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_save(&fileSpec, "Locations File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            itsLocsFileNameNLineText->SetText(path);
            WriteFile();
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 13:                        // Stand List File name CR command
      {
         break;
      }
      case 14:                        // Stand List File name char change
      {
         break;
      }
      case 15:                        // Stand List File name Browse button
      {
         FILE_SPEC fileSpec;
         strcpy(fileSpec.type,"slf");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec, "Stand List File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            itsSLFNLineText->SetText(path);
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 20:                        // Add File Browse button
      case 24:                        // Database Browse button
      {
         FILE_SPEC fileSpec;
         strcpy(fileSpec.type,(theCommand==20) ? "kcp" : "mdb;*.accdb;*.sqlite;*.db");  // updated to allow for various database options
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec, (theCommand==20) ? "Add File:" : "Data Source")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);

            CStringRW tmp = itsAddFileNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            if (tmp.length())
            {
               tmp += " ";
               tmp += path;
            }
            else tmp = path;
            if (theCommand==20) itsAddFileNLineText->SetText(tmp);
            else                itsDataSourceNLineText->SetText(tmp);
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 21:                        // Ok button pushed
      {
         StoreCurrent();
         if (WriteFile()) Close();
         break;
      }
      case 22:                        // Cancel button pushed
      {
         Close();
         break;
      }
      case EXDataUpdated:             // Another window updated the data.
      {
         if (theData)
         {
            // if theData is not null, it contains the file name of the
            // file that was updated.

            if (strstr((const char *) theData,
                       itsLocsFileNameNLineText->GetText().data()) ||
                strstr(itsLocsFileNameNLineText->GetText().data(),
                       (const char *) theData))
            {
               CWindow * curFront = G->GetDesktop()->GetFrontWindow();
               if (curFront != this)
                  G->GetDesktop()->SetFrontWindow((CWindow*) this);
               if (xvt_dm_post_ask("Reload","Ignore",NULL,
                                   "File %s has changed.", theData)
                   == RESP_DEFAULT)
               {
                  ReadFile();
                  DisplayRecord();
               }
               if (curFront != this)
                  G->GetDesktop()->SetFrontWindow(curFront);
            }
         }
         break;
      }

      default:
      {
         CWwindow::DoCommand(theCommand,theData);
         break;
      }
   }
}



void EditLocationWin::DisplayRecord( void )
{
   CStringRW newTitle ("Record ");
   switch (itsEditingType)
   {
      case 1:
      {
         if (itsTypeARecords.entries())
         {
            if (itsCurrentTypeA >= itsTypeARecords.entries())
               itsCurrentTypeA = itsTypeARecords.entries()-1;
            LocTypeARec * display = (LocTypeARec *) itsTypeARecords.
               at(itsCurrentTypeA);
            itsLocNameNLineText->SetText(*display);
            itsSLFNLineText->SetText(display->SLFName);
            newTitle += ConvertToString((int) itsCurrentTypeA+1);
            newTitle += " of ";
            newTitle += ConvertToString((int) itsTypeARecords.entries());
            itsRecordCountNText->SetTitle(newTitle);
         }
         else
         {
            itsCurrentTypeA=0;
            itsLocNameNLineText->Clear();
            itsSLFNLineText->Clear();
            DoCommand (8,NULL);
         }
         break;
      }
      case 2:
      {
         if (itsTypeBRecords.entries())
         {
            if (itsCurrentTypeB >= itsTypeBRecords.entries())
               itsCurrentTypeB = itsTypeBRecords.entries()-1;
            LocTypeBRec * display = (LocTypeBRec *) itsTypeBRecords.
               at(itsCurrentTypeB);
            itsGroupCodeNLineText->SetText(*display);
            itsAddFileNLineText->SetText(display->addFiles);
            newTitle += ConvertToString((int) itsCurrentTypeB+1);
            newTitle += " of ";
            newTitle += ConvertToString((int) itsTypeBRecords.entries());
            itsRecordCountNText->SetTitle(newTitle);
         }
         else
         {
            itsCurrentTypeB=0;
            itsGroupCodeNLineText->Clear();
            itsAddFileNLineText->Clear();
            DoCommand (8,NULL);
         }
         break;
      }
      case 3:
      {
         if (itsTypeCRecords.entries())
         {
            if (itsCurrentTypeC >= itsTypeCRecords.entries())
               itsCurrentTypeC = itsTypeCRecords.entries()-1;
            LocTypeCRec * display = (LocTypeCRec *) itsTypeCRecords.
               at(itsCurrentTypeC);
            itsLocNameNLineText->SetText(*display);
            itsDataSourceNLineText->SetText(display->dataSourceName);
            itsUserNameNLineText->SetText(display->userName);
            itsPasswordNLineText->SetText(display->passWord);
            newTitle += ConvertToString((int) itsCurrentTypeC+1);
            newTitle += " of ";
            newTitle += ConvertToString((int) itsTypeCRecords.entries());
            itsRecordCountNText->SetTitle(newTitle);
         }
         else
         {
            itsCurrentTypeC=0;
            itsLocNameNLineText->Clear();
            itsDataSourceNLineText->Clear();
            itsUserNameNLineText->Clear();
            itsPasswordNLineText->Clear();
            DoCommand (8,NULL);
         }
         break;
      }
      default:  break;
   }
}



void EditLocationWin::StoreCurrent( void )
{
   switch (itsEditingType)
   {
      case 1:
      {
         if (itsTypeARecords.entries())
         {
            LocTypeARec * display = (LocTypeARec *) itsTypeARecords.
               at(itsCurrentTypeA);
            CStringRW tmp = itsLocNameNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            (CStringRWC &) *display = tmp;
            tmp = itsSLFNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            display->SLFName = tmp;
         }
         break;
      }
      case 2:
      {
         if (itsTypeBRecords.entries())
         {
            LocTypeBRec * display = (LocTypeBRec *) itsTypeBRecords.
               at(itsCurrentTypeB);
            CStringRW tmp = itsGroupCodeNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            (CStringRWC &) *display = tmp;
            tmp = itsAddFileNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            display->addFiles = tmp;
         }
         break;
      }
      case 3:
      {
         if (itsTypeCRecords.entries())
         {
            LocTypeCRec * display = (LocTypeCRec *) itsTypeCRecords.
               at(itsCurrentTypeC);
            CStringRW tmp = itsLocNameNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            (CStringRWC &) *display = tmp;
            tmp = itsDataSourceNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            display->dataSourceName = tmp;
            tmp = itsUserNameNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            display->userName = tmp;
            tmp = itsPasswordNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            display->passWord = tmp;
         }
         break;
      }
      default: break;
   }
}




int EditLocationWin::WriteFile( void )
{

   StoreCurrent();
   CStringRW fileName = itsLocsFileNameNLineText->GetText();
   fileName = fileName.strip(RWCString::both);
   if ((itsTypeARecords.entries() ||
        itsTypeBRecords.entries() ||
        itsTypeCRecords.entries()) &&
       fileName.length())
   {
      int nA = 0;
      int nB = 0;
      int nC = 0;
      LocTypeARec * aARec;
      LocTypeBRec * aBRec;
      LocTypeCRec * aCRec;
      RWOrderedIterator ARecIter(itsTypeARecords);
      RWOrderedIterator BRecIter(itsTypeBRecords);
      RWOrderedIterator CRecIter(itsTypeCRecords);
      while (aARec = (LocTypeARec *) ARecIter())
         if (aARec->length() && aARec->SLFName.length())  nA++;
      while (aBRec = (LocTypeBRec *) BRecIter())
         if (aBRec->length() && aBRec->addFiles.length()) nB++;
      while (aCRec = (LocTypeCRec *) CRecIter())
         if (aCRec->length() && aCRec->dataSourceName.length())  nC++;

      if (nA || nB || nC)
      {
         FILE * locFile = fopen (fileName.data(),"w");
         if (locFile)
         {
            ARecIter.reset();
            while (aARec = (LocTypeARec *) ARecIter())
            {
               const char *at = "@";
               if (aARec->DSN.length()) at = aARec->DSN.data();
               if (aARec->length() && aARec->SLFName.length())
                  fprintf(locFile,"A \"%s\" @ %s %s\n",
                          aARec->data(),aARec->SLFName.data(), at);
            }
            BRecIter.reset();
            while (aBRec = (LocTypeBRec *) BRecIter())
            {
               if (aBRec->length() && aBRec->addFiles.length())
                  fprintf(locFile,"B %s %s @\n",
                          aBRec->data(),aBRec->addFiles.data());
            }
            CRecIter.reset();
            while (aCRec = (LocTypeCRec *) CRecIter())
            {
               if (aCRec->length() && aCRec->dataSourceName.length())
                  fprintf(locFile,"C \"%s\" %s %s %s\n",
                          aCRec->data(),aCRec->dataSourceName.data(),
                          aCRec->userName.data(), aCRec->passWord.data());
            }
            fclose (locFile);
            xvt_dm_post_message("%d Type A, %d Type B, and %d Type C records output.",
                                nA, nB, nC);
            SendUpdateMessage(EXDataUpdated,
                              (void *) fileName.data(),this);
            return 1;
         }
         else xvt_dm_post_warning("File %s could not be opened.",
                                  fileName.data());
         return 0;
      }
      return 1;
   }
   else xvt_dm_post_warning("No file name specified.");
   return 0;
}


void EditLocationWin::ReadFile( void )
{
   itsTypeARecords.clearAndDestroy();
   itsTypeBRecords.clearAndDestroy();
   itsTypeCRecords.clearAndDestroy();
   itsCurrentTypeA = 0;
   itsCurrentTypeB = 0;
   itsCurrentTypeC = 0;

   CStringRW fileName = itsLocsFileNameNLineText->GetText();
   fileName = fileName.strip(RWCString::both);
   if (fileName.length())
   {
      FILE * locFile = fopen (fileName.data(),"r");
      if (locFile)
      {
         char * record= new char[2001];
         char * ptr;
         if (locFile != NULL)
         {
            while (fgets(record, 2000, locFile) &&
                   (ptr = strtok (record," ")))
            {
               switch ((int) *ptr)
               {
                  case 'A':                // Location records
                  {
                     if (*(ptr+2))
                     {
                        LocTypeARec * newLoc = new LocTypeARec(ptr+2);
                        itsTypeARecords.insert(newLoc);
                     }
                     break;
                  }

                  case 'B':                // Group records.
                  {
                     if (*(ptr+2))
                     {
                        LocTypeBRec * newGroup = new LocTypeBRec(ptr+2);
                        itsTypeBRecords.insert(newGroup);
                     }
                     break;
                  }
                  case 'C':                // Loc DSN records.
                  {
                     if (*(ptr+2))
                     {
                        LocTypeCRec * newLoc = new LocTypeCRec(ptr+2);
                        itsTypeCRecords.insert(newLoc);
                     }
                     break;
                  }
                  default: break;
               }
            }
         }
         delete []record;
         fclose(locFile);
      }
      else xvt_dm_post_warning ("Suppose can not open Locations File: %s",
                                fileName.data());
   }
}



RWDEFINE_COLLECTABLE(LocTypeARec,EDITLOC)

LocTypeARec::LocTypeARec( char * record )
            :CStringRWC ( )

{
   char * ptr = record;
   if (*ptr == '\"')
   {
      ptr++;
      if (!ptr) return;
      ptr = strtok (record,"\"");
   }
   else ptr = strtok (record," \n");
   if (ptr && *ptr != '@') (CStringRWC &) *this = ptr;
   if (ptr) ptr = strtok (NULL," \n");
   if (ptr) ptr = strtok (NULL," \n");
   if (ptr && *ptr != '@') SLFName = ptr;
   if (ptr) ptr = strtok (NULL," \n");
   if (ptr && *ptr != '@') DSN = ptr;
}


LocTypeARec::LocTypeARec( LocTypeARec * locTypeARec)
            :CStringRWC ((CStringRWC &) *locTypeARec)
{
   SLFName = locTypeARec->SLFName;
}


LocTypeARec::LocTypeARec( void )
            :CStringRWC ( )
{}

RWBoolean LocTypeARec::isEqual(const RWCollectable * t) const
{
   return this == t;
}


RWDEFINE_COLLECTABLE(LocTypeBRec,EDITGROUP)

LocTypeBRec::LocTypeBRec( char  * record )
            :CStringRWC ( )

{
   char * ptr = strtok (record," @\n");
   if (ptr) (CStringRWC &) *this = ptr;
   if (ptr) ptr = strtok (NULL,"@\n");
   if (ptr) addFiles = ptr;
}


LocTypeBRec::LocTypeBRec( LocTypeBRec * locTypeBRec)
            :CStringRWC ((CStringRWC &) *locTypeBRec)
{
   addFiles = locTypeBRec->addFiles;
}


LocTypeBRec::LocTypeBRec( void )
            :CStringRWC ( )
{}

RWBoolean LocTypeBRec::isEqual(const RWCollectable * t) const
{
   return this == t;
}


RWDEFINE_COLLECTABLE(LocTypeCRec,EDITDSN)

LocTypeCRec::LocTypeCRec( char  * record )
            :CStringRWC ( )

{
   char * ptr = record;
   if (*ptr == '\"')
   {
      ptr++;
      if (!ptr) return;
      ptr = strtok (record,"\"");
   }
   else ptr = strtok (record," \n");
   if (ptr) (CStringRWC &) *this = ptr;
   if (ptr) ptr = strtok (NULL," \n");
   if (ptr) dataSourceName = ptr;
   if (ptr) ptr = strtok (NULL," \n");
   if (ptr) userName = ptr;
   if (ptr) ptr = strtok (NULL," \n");
   if (ptr) passWord = ptr;
}


LocTypeCRec::LocTypeCRec( LocTypeCRec * locTypeCRec)
            :CStringRWC ((CStringRWC &) *locTypeCRec)
{
   dataSourceName = locTypeCRec->dataSourceName;
   userName       = locTypeCRec->userName;
   passWord       = locTypeCRec->passWord;
}


LocTypeCRec::LocTypeCRec( void )
            :CStringRWC ( )
{}


RWBoolean LocTypeCRec::isEqual(const RWCollectable * t) const
{
   return this == t;
}

