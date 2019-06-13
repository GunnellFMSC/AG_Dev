/********************************************************************

  File Name:     readfvsw.cpp
  Author:        nlc
  Date:          10/25/96

  see readfvsw.hpp for notes.

 ********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include CText_i
#include NButton_i
#include NEditControl_i
#include NScrollText_i
#include NListBox_i
#include CMenuBar_i

#include CStringCollection_i
#include CScroller_i
#include CUnits_i
#include NListButton_i
#include CView_i
#include CWindow_i
#include CStringRWC_i
#include CBoss_i
#include CApplication_i
#include CDocument_i
#include CDesktop_i
#include CWindow_i
#include CNavigator_i

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include "reportd.hpp"
#include "reportwn.hpp"
#include "spglobal.hpp"
#include "sendupdt.hpp"
#include "spfunc2.hpp"

#include "appdoc.hpp"
#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

#include "dirsept.hpp"
#include "readfvsw.hpp"
#include "parser.hpp"
#include "readfire.hpp"
#include "rdtreels.hpp"

ReadFVSOutputWindow::ReadFVSOutputWindow(CDocument * theDocument)
                    :CWwindow(theDocument,
                              AdjWinPlacement(CRect(10,20,630,440)), "Read FVS Output",
                              WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                              W_DOC, TASK_MENUBAR)
{

   xvt_scr_set_busy_cursor();

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   // initalization of some variables.

   itsStopFlag = 0;
   itsOpenedFileName = NULLString;
   itsOpenedFilePtr = NULL;
   itsFVSOutputParser = NULL;

   new CText(this, CPoint(4,4), "Cases: ");

   itsLinesReadCText = new CText(this, CPoint(492,4), NULLString);
   itsLinesReadCText->DoHide();

   itsCaseListBox = new NListBox(this, CRect(4,28,616,344),
                                 *theSpGlobals->theReportData,
                                 CTL_FLAG_MULTIPLE);
   itsCaseListBox->SetGlue(TOPSTICKY   | LEFTSTICKY |
                           RIGHTSTICKY | BOTTOMSTICKY);

   CText * ct = new CText(this, CPoint(6,354), "File: ");
   ct->SetGlue(BOTTOMSTICKY);

   strcpy(itsFVSOutputFile.name, ((CsupposeDoc*) theSpGlobals->
                                  theAppDocument)->SpGetFilePointer()->name);
   strcpy(itsFVSOutputFile.type,"out");
   xvt_fsys_get_dir(&itsFVSOutputFile.dir);

   if (strncmp(itsFVSOutputFile.name,NEWFILE,10) == 0)
   {
      strcpy(itsFVSOutputFile.name,"*no input file*");
   }
   else
   {
      size_t nLen = strlen(itsFVSOutputFile.name);
      if (nLen > 4 && (strncmp(itsFVSOutputFile.name+nLen-4,".",1) == 0))
      {
         strcpy(itsFVSOutputFile.name+nLen-4,".out");
      }
   }

   itsCurrentFileEditControl  = new NEditControl(this, CRect(50,348,368,380),
                                                 itsFVSOutputFile.name, 0L);
   itsCurrentFileEditControl->SetGlue(BOTTOMSTICKY);
   itsCurrentFileEditControl->SetFocusCommands(0, 6);

   RWOrdered aFileTypeList;
   aFileTypeList.insert(new CStringRWC ("Std Output"));
   aFileTypeList.insert(new CStringRWC ("TreeList"));

   itsAverageButton       = new NButton(this, CRect(376,348,492,380),
                                        "Average");
   itsStopButton          = new NButton(this, CRect(500,348,616,380),
                                        "Stop Reading");
   itsReadButton          = new NButton(this, CRect(  4,384,120,416),
                                        "Read File");
   itsBrowseButton        = new NButton(this, CRect(128,384,218,416),
                                        "Browse");
   itsFileTypesListButton = new NListButton(this, CRect(226,384,368,470),
                                            aFileTypeList, 0L);
   itsFileTypesListButton->Disable();
   itsDeleteCasesButton   = new NButton(this, CRect(376,384,492,416),
                                        "Delete Cases");
   itsCloseButton         = new NButton(this, CRect(500,384,616,416),
                                        "Close");

   itsAverageButton      ->SetCommand(M_AFTER_SIM_AVERAGECASES);
   itsStopButton         ->SetCommand(5);
   itsReadButton         ->SetCommand(1);
   itsBrowseButton       ->SetCommand(2);
   itsFileTypesListButton->SetCommand(8);
   itsDeleteCasesButton  ->SetCommand(3);
   itsCloseButton        ->SetCommand(4);

   itsAverageButton      ->SetGlue(BOTTOMSTICKY);
   itsStopButton         ->SetGlue(BOTTOMSTICKY);
   itsReadButton         ->SetGlue(BOTTOMSTICKY);
   itsBrowseButton       ->SetGlue(BOTTOMSTICKY);
   itsFileTypesListButton->SetGlue(BOTTOMSTICKY);
   itsDeleteCasesButton  ->SetGlue(BOTTOMSTICKY);
   itsCloseButton        ->SetGlue(BOTTOMSTICKY);

   // Set up the linkage to help.

   Show();

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), READFVSOutput, 0L);

   aNav->AppendTabStop(new CTabStop(itsReadButton));
   aNav->AppendTabStop(new CTabStop(itsBrowseButton));
   aNav->AppendTabStop(new CTabStop(itsFileTypesListButton));
   aNav->AppendTabStop(new CTabStop(itsDeleteCasesButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));
   aNav->AppendTabStop(new CTabStop(itsCaseListBox));
   aNav->AppendTabStop(new CTabStop(itsCurrentFileEditControl));
   aNav->AppendTabStop(new CTabStop(itsAverageButton));
   aNav->AppendTabStop(new CTabStop(itsStopButton));

   aNav->InitFocus();
}




void ReadFVSOutputWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:   // Read File
      {
         ReadFVSOutput();
         break;
      }
      case 2:   // Browse
      {
         xvt_fsys_save_dir();
         strcpy(itsFVSOutputFile.type,"out");
         if (xvt_dm_post_file_open(&itsFVSOutputFile,"FVS Output File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&itsFVSOutputFile.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     itsFVSOutputFile.name);
            itsCurrentFileEditControl->SetTitle(path);
            DoCommand(1,this);
         }
         xvt_fsys_restore_dir();
         break;
      }

      case 3:   // Delete Cases
      {
         size_t numSelectedCases = itsCaseListBox->GetNumSelectedItems();
         if (numSelectedCases == 0)
         {
            itsCaseListBox->SelectAll();
            break;
         }
         else
         {
            switch(xvt_dm_post_ask("Delete","Cancel",NULL,
                                   "Deleting %d case(s) will clear "
                                   "all report and graphics windows.",
                                   numSelectedCases))
            {
                 case RESP_2:          // Cancel
               {
                  itsCaseListBox->DeselectAll();
                  break;
               }
               case RESP_DEFAULT:    // Delete
               {
                  xvt_scr_set_busy_cursor();

                  size_t numDeleted = 0;
                  if (theSpGlobals->theReportData->entries() ==
                      numSelectedCases)
                  {
                     numDeleted = theSpGlobals->theReportData->
                                  DeleteAllCases();
                  }
                  else
                  {
                     // process the list from the bottom up...so that the
                     // index numbers will continue to match the cases
                     // that should be deleted.

                     for (size_t i = itsCaseListBox->GetNumItems(); i > 0;
                          i--)
                     {
                        if (itsCaseListBox->IsItSelected((int) i-1))
                        {
                           if (theSpGlobals->theReportData->DeleteCase(i-1))
                              numDeleted++;
                        }
                     }
                  }
                  if (numDeleted) SendUpdateMessage (FVSCaseListDecreased);
                  break;
               }
            }
         }
         break;
      }

      case 4:   // Close
      {
         Close();
         break;
      }

      case 5:   // Stop
      {
         itsStopFlag = 1;
         break;
      }

      case 6: // TextCommand, lost key focus
      {
         itsReadButton->Activate();
         break;
      }

      case 8: // File type list button.
      {
         break;
      }

      case FVSCaseListIncreased:
      case FVSCaseListDecreased:
      {
         if (theSpGlobals->theReportData->entries()) itsCaseListBox->
             IListBox(*theSpGlobals->theReportData);
         else itsCaseListBox->Clear();
         break;
      }

      default:
      {
         CWwindow::DoCommand(theCommand, theData);
         break;
      }
   }
}



BOOLEAN ReadFVSOutputWindow::Close( void )
{
   // if the close was done during the file read, then the file pointer
   // will be non-null.  In this case close the file and send the update
   // message.  This send can not be done from the loop where the read
   // is taking place because by the time flow returns to that location
   // this object will be already deleted.  Note the comments in the read
   // loop code.

   if (itsOpenedFilePtr)
   {
      fclose(itsOpenedFilePtr);
      itsOpenedFilePtr = NULL;
      SendUpdateMessage (FVSCaseListIncreased);
   }

   if (itsFVSOutputParser)
   {
      delete itsFVSOutputParser;
      itsFVSOutputParser = NULL;
   }
   return CWwindow::Close();
}



void ReadFVSOutputWindow::ReadFVSOutput()
{

   if (strncmp(itsCurrentFileEditControl->GetTitle().data(),
               "*no input file*",15) == 0)
   {
      DoCommand(2,this);
   }
   else
   {
      itsStopFlag = 0;
      itsOpenedFileName = itsCurrentFileEditControl->GetTitle();
      itsOpenedFilePtr = fopen (itsOpenedFileName.data(),
                                "r");
      if (itsOpenedFilePtr)
      {
         // if there is no parser, create one.  Use it as long as the
         // window is in use.

         if (!itsFVSOutputParser) itsFVSOutputParser = new FVSParser();

         // get an index to the number of cases that currently exists.

         size_t caseCount = theSpGlobals->theReportData->entries();

         char record[151];
         char msg[30];
         long nrecs = 0;
         sprintf (msg,"Lines read = %i",nrecs);
         itsLinesReadCText->SetTitle(msg);
         itsLinesReadCText->DoShow();

         int quitReading = 0;
         while (fgets(record, 150, itsOpenedFilePtr) && !quitReading)
         {
            nrecs++;
            if (nrecs % 1000 == 0)
            {
               sprintf (msg,"Lines read = %i",nrecs);
               itsLinesReadCText->SetTitle(msg);

               xvt_app_process_pending_events();

               // when we process a pending event, we may have introduced
               // a close event.  If that is the case, itsFVSOutputParser
               // will be NULL.  As this results in the destruction of
               // both the parser and the window, we return PDQ.

               // and, processing a pending event can result simply in
               // a change in the state of the stop flag... if this is the
               // case, simply notify the parser of the read abort.

               if (itsFVSOutputParser)
               {
                  if (itsStopFlag)
                  {
                     itsFVSOutputParser->ParseClose();
                     break;
                  }
               }
               else
               {
                  if (theSpGlobals->theReportData->entries() > caseCount &&
                      caseCount > 0)
                  {
                     theSpGlobals->theReportData->itsCaseInUse = (FVSCase *)
                           theSpGlobals->theReportData->at(caseCount);
                  }
                  else
                  {
                     theSpGlobals->theReportData->itsCaseInUse = NULL;
                  }
                  return;
               }
            }
            quitReading = itsFVSOutputParser->
                  ParseFVSOutputRecord(record,
                                       itsOpenedFilePtr,
                                       itsOpenedFileName);
         }

         // we can not reach this code on a close event.

         // set the case in use.

         if (theSpGlobals->theReportData->entries() > caseCount &&
             caseCount > 0)
         {
            theSpGlobals->theReportData->itsCaseInUse = (FVSCase *)
                  theSpGlobals->theReportData->at(caseCount);
         }
         else
         {
            theSpGlobals->theReportData->itsCaseInUse = NULL;
         }

         fclose(itsOpenedFilePtr);
         itsFVSOutputParser->ParseClose();
         sprintf (msg,"Lines read = %i",nrecs);
         itsLinesReadCText->SetTitle(msg);
         xvt_app_process_pending_events();
         SendUpdateMessage (FVSCaseListIncreased);
      }
      else
      {
         xvt_dm_post_message("File can not be opened.");
      }
      itsOpenedFileName = NULLString;
      itsOpenedFilePtr  = NULL;
   }
}



