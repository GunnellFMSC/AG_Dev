/*********************************************************************

   File Name:    reportwn.cpp
   Author:       nlc
   Date:         11/96

   see reportwn.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include CText_i
#include NButton_i
#include NEditControl_i
#include NScrollText_i
#include NScrollBar_i
#include NListButton_i
#include NListBox_i
#include CMenuBar_i

#include CStringCollection_i
#include CUnits_i
#include CView_i
#include CWindow_i
#include CDocument_i
#include CStringRWC_i
#include CBoss_i
#include CApplication_i
#include CDesktop_i
#include CWindow_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "reportd.hpp"
#include "reportwn.hpp"
#include "graphwn.hpp"
#include "spglobal.hpp"
#include "sendupdt.hpp"
#include "appdoc.hpp"
#include "spfunc2.hpp"
#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>

#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

#define COLUMNChanged 100

GenerateReportWindow::GenerateReportWindow(CDocument * theDocument)
                     :CWwindow(theDocument,
                               CenterTopWinPlacement(CRect(10,20,610,440)), "Generate Report",
                               WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                               W_DOC, TASK_MENUBAR)
{

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   int ascentp, descent;
   xvt_dwin_get_font_metrics(GetCWindow()->GetXVTWindow(),
                             &itsLineHeight, &ascentp, &descent);
   itsLineHeight = itsLineHeight+ascentp+descent;

   new CText(this, CPoint(4,8), "Title: ");
   itsReportTitle  = new NEditControl(this, CRect(56,4,332,36),
                                      NULLString, 0L);
   itsReportTitle->SetGlue(TOPSTICKY | LEFTSTICKY | RIGHTSTICKY);

   CView * tempCView = (CView *) new CText(this, CPoint(340,8), "Type: ");
   tempCView->SetGlue(TOPSTICKY | RIGHTSTICKY);
   itsReportTypeListButton = new NListButton(this, CRect(384,4,592,148),
                   theSpGlobals->theReportData->itsReportTypesList, 0L);
   itsReportTypeListButton->SetCommand(4);
   itsReportTypeListButton->SetGlue(TOPSTICKY | RIGHTSTICKY);
   itsReportTypeListButton->SelectItem(0);

   // set up CScroller.

   itsScrollingFrame = new MyCScroller(this, CRect(4,40,592,380),
                                       592-4-NScrollBar::NativeWidth(),
                                       itsLineHeight*10);
   itsScrollingFrame->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                              LEFTSTICKY   | RIGHTSTICKY);

   // set up the bottom buttons.

   itsWriteReportButton   = new NButton(this, CRect(  4,384,120,416),
                                        "Write Report");
   itsClearReportButton   = new NButton(this, CRect(130,384,246,416),
                                        "Clear Report");
   itsReadFVSOutputButton = new NButton(this, CRect(256,384,396,416),
                                        "Read FVS Output");
   itsReportsButton       = new NButton(this, CRect(406,384,496,416),
                                        "Reports");
   itsCloseButton         = new NButton(this, CRect(506,384,596,416),
                                        "Close");

   itsWriteReportButton   ->SetCommand( 1);
   itsClearReportButton   ->SetCommand( 2);
   itsReadFVSOutputButton ->SetCommand( M_AFTER_SIM_READFVSOUTPUT);
   itsReportsButton       ->SetCommand( M_AFTER_SIM_REPORTS);
   itsCloseButton         ->SetCommand( 3);

   itsWriteReportButton   ->SetGlue(BOTTOMSTICKY);
   itsClearReportButton   ->SetGlue(BOTTOMSTICKY);
   itsReadFVSOutputButton ->SetGlue(BOTTOMSTICKY);
   itsReportsButton       ->SetGlue(BOTTOMSTICKY);
   itsCloseButton         ->SetGlue(BOTTOMSTICKY);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), GENERATEReports, 0L);

   aNav->AppendTabStop(new CTabStop(itsReportTitle));
   aNav->AppendTabStop(new CTabStop(itsReportTypeListButton));
   aNav->AppendTabStop(new CTabStop(itsWriteReportButton));
   aNav->AppendTabStop(new CTabStop(itsClearReportButton));
   aNav->AppendTabStop(new CTabStop(itsReadFVSOutputButton));
   aNav->AppendTabStop(new CTabStop(itsReportsButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));

   Show();
   aNav->InitFocus();

   ClearReport();
}




inline void MoveButton(NButton * theButton, int start)
{
   CRect aRect   = theButton->GetFrame();
   int len       = aRect.Right()-aRect.Left();
   aRect.Left (start+4);
   aRect.Right(aRect.Left()+len);
   theButton->DoSize(aRect);
}





void GenerateReportWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:   // Write Report
      {
         if (itsColumnSubviews.entries()) WriteReport();
         else xvt_dm_post_message("Nothing to write.");
         break;
      }
      case 2:   // Clear Report
      {
         if (ReportIsEmpty()) break;

         switch(xvt_dm_post_ask("Cancel","Ok","Write",
                                "Are you sure you want the report cleared?"))
         {
            case RESP_DEFAULT:    // Cancel
            {
               break;
            }
            case RESP_2:          // Clear
            {
               ClearReport();
               break;
            }
            case RESP_3:          // Write
            {
               WriteReport();
               break;
            }
         }
         break;
      }
      case 3:   // Close
      {
         Close();
         break;
      }

      case 4:   // Report Type List Button
      {
         if (!ReportIsEmpty())
         {
#if XVTWS==WIN32WS
            switch(xvt_dm_post_ask("Cancel","C~lear","Write",
#else
            switch(xvt_dm_post_ask("Cancel","Clear","Write",
#endif
                                   "Changing report types requires "
                                   "clearing report."))
            {
               case RESP_DEFAULT:    // Cancel
               {
                  int reportType = ((ColumnSubview *)
                     itsColumnSubviews.last())->
                     itsFVSVariable->itsFVSReport->GetFVSDataType();
                  itsReportTypeListButton->SelectItem(reportType-1);
                  break;
               }
               case RESP_2:          // Clear
               {
                  ClearReport();
                  break;
               }
               case RESP_3:          // Write
               {
                  WriteReport();
                  ClearReport();
                  break;
               }
            }
         }
         else ClearReport();
         break;
      }

      case 5:   // Next Case
      case 6:   // Next Variable
      {
         if (itsColumnSubviews.entries())
         {
            ColumnSubview * theLastColumn =
               (ColumnSubview *) itsColumnSubviews.last();
            FVSVariable * itsFVSVariable = theLastColumn->itsFVSVariable;
            itsFVSVariable = itsFVSVariable->
               GetNextVariable(itsFVSVariable, (theCommand == 5));
            if (itsFVSVariable) NewColumn(itsFVSVariable);
         }
         break;
      }
      case 7:   // New Column
      {
         NewColumn(NULL);
         break;
      }
      case 8:   // Delete Column (theData points to the one to delete).
      {
         if (theData && itsColumnSubviews.entries())
         {
            // if there is only one column, then just clear!

            if (itsColumnSubviews.entries() == 1) ClearReport();
            else
            {
               itsScrollingFrame->Hide();

               // first, move the rest of the subviews to the left.

               ColumnSubview * columnToRemove = (ColumnSubview *) theData;
               ColumnSubview * aColumnSubview;
               ColumnSubview * passItOn = NULL;
               CRect newRect, saveRect;
               RWOrderedIterator nextSubview(itsColumnSubviews);
               while (aColumnSubview = (ColumnSubview *) nextSubview())
               {
                  if (passItOn)
                  {
                     saveRect = aColumnSubview->GetFrame();
                     aColumnSubview->DoSize(newRect);
                     newRect = saveRect;
                     passItOn = aColumnSubview;
                  }
                  else if (aColumnSubview == columnToRemove)
                  {
                     passItOn = columnToRemove;
                     newRect  = passItOn->GetFrame();
                  }
               }

               if (!passItOn) break;

               // then move over the buttons.

               int newLeft = passItOn->GetFrame().Right()+4;
               if (passItOn == columnToRemove) newLeft =
                                                  passItOn->GetFrame().Left();

               MoveButton(itsNextCaseButton,newLeft);
               MoveButton(itsNextVariableButton,newLeft);
               MoveButton(itsNewColumnButton,newLeft);

               // delete all of the subviews inside the column to remove.

               CSubview * aSubview;
               RWOrderedIterator subviewIter (*columnToRemove->GetSubviews());
               while (aSubview = (CSubview *) subviewIter())
               {
                  columnToRemove->RemoveSubview(aSubview);
                  delete aSubview;
               }

               itsScrollingFrame->RemoveSubview(columnToRemove);
               itsColumnSubviews.removeAndDestroy(columnToRemove);

               // invalidate last frame so that it will redraw

               RCT aRct = RCT(itsScrollingFrame->GetClippedFrame());
               xvt_dwin_invalidate_rect(GetXVTWindow(), &aRct);

               DisplayTable();
               itsScrollingFrame->DoDraw(itsScrollingFrame->
                                         GetClippedFrame());
               itsScrollingFrame->Show();
            }
         }
         break;
      }
      case FVSCaseListDecreased:
      {
         if (!ReportIsEmpty())
         {
            CWindow * curTop = G->GetDesktop()->GetFrontWindow();
            G->GetDesktop()->SetFrontWindow(this);
            switch(xvt_dm_post_ask("Write","Clear",NULL,
                                   "Do you want to write this report before "
                                   "it is cleared."))
            {
               case RESP_DEFAULT:    // Write
               {
                  WriteReport();
                  ClearReport();
                  break;
               }
               case RESP_2:          // Clear
               {
                  ClearReport();
                  break;
               }
            }
            G->GetDesktop()->SetFrontWindow(curTop);
         }
         break;
      }
      case COLUMNChanged:
      {
         DisplayTable();
         break;
      }
      case FVSCaseListIncreased:     // reset show/hide some controls.
      {
         if (ReportIsEmpty())
         {
            itsNextCaseButton->DoHide();
            itsNextVariableButton->DoHide();
            itsNewColumnButton->DoHide();
         }
         else
         {
            if (itsReportTypeListButton->GetSelectPosition()+1 ==
                   ReportTypeOBSOverCase ||
                theSpGlobals->theReportData->entries()<2)
                 itsNextCaseButton->DoHide();
            else itsNextCaseButton->DoShow();
            itsNextVariableButton->DoShow();
            itsNewColumnButton->DoShow();
         }
      }

      default:
         CWwindow::DoCommand(theCommand, theData); break;
   }
}


void GenerateReportWindow::DoMenuCommand(MENU_TAG theMenuItem,
                                         BOOLEAN isShiftkey,
                                         BOOLEAN isControlKey)
{
   switch(theMenuItem)
   {
      case M_EDIT_COPY:
      {
         if (itsColumnSubviews.entries() && itsFirstColumnSubview &&
             itsFirstColumnSubview->itsColumnTitle)
         {
            // compute the amount of space we need on the clipboard.

            size_t aSize;
            int clipSize = 0;
            int eol_len  = strlen(EOL_SEQ);
            aSize = itsReportTitle->GetTitle().length();
            if (aSize) clipSize += aSize+eol_len;

            clipSize += itsFirstColumnSubview->itsColumnTitle->
                        GetTitle().length();

            RWOrderedIterator nextSubview(itsColumnSubviews);
            ColumnSubview * aColumnSubview;
            while (aColumnSubview = (ColumnSubview *) nextSubview())
            {
               aSize = aColumnSubview->itsVariablesButton->
                        GetTitle().length();
               if (aSize) clipSize += aSize+1;
               else clipSize++;
            }
            clipSize += eol_len;
            size_t rowNumber = 0;
            while (rowNumber < itsFirstColumnSubview->
                   itsObservationCTexts.entries())
            {
               aSize = ((CText *) itsFirstColumnSubview->
                        itsObservationCTexts.
                        at(rowNumber))->GetTitle().length();
               if (aSize)
               {
                  clipSize += aSize;
                  RWOrderedIterator nextSubview(itsColumnSubviews);
                  ColumnSubview * aColumnSubview;
                  while (aColumnSubview = (ColumnSubview *) nextSubview())
                  {
                     aSize = ((CText *) aColumnSubview->
                              itsObservationCTexts.
                              at(rowNumber))->GetTitle().length();
                     if (aSize) clipSize += aSize+1;
                     else clipSize++;
                  }
                  rowNumber++;
                  clipSize += eol_len;
               }
               else break;
            }

            // allocate the clipboard memory...if the ptr is not null,
            // we write the data.

            char * ptr;
            xvt_cb_open(TRUE);
            if (ptr = xvt_cb_alloc_data(++clipSize))
            {
               char * orgptr = ptr;
               size_t i;
               CStringRW c1 = itsReportTitle->GetTitle();
               if (aSize = c1.length())
               {
                  for (i=0; i<aSize; i++) *ptr++ = c1(i);
                  for (i=0; i<eol_len; i++) *ptr++ = EOL_SEQ[i];
               }
               c1 = itsFirstColumnSubview->itsColumnTitle->GetTitle();
               if (aSize = c1.length())
               {
                  for (i=0; i<aSize; i++) *ptr++ = c1(i);
               }
               RWOrderedIterator nextSubview(itsColumnSubviews);
               while (aColumnSubview = (ColumnSubview *) nextSubview())
               {
                  c1 = aColumnSubview->itsVariablesButton->
                       GetTitle();
                  *ptr++ = '\t';
                  if (aSize = c1.length())
                  {
                     for (i=0; i<aSize; i++) *ptr++ = c1(i);
                  }
               }
               for (i=0; i<eol_len; i++) *ptr++ = EOL_SEQ[i];
               rowNumber = 0;
               while (rowNumber < itsFirstColumnSubview->
                      itsObservationCTexts.entries())
               {
                  c1 = ((CText *) itsFirstColumnSubview->
                        itsObservationCTexts.
                        at(rowNumber))->GetTitle();
                  if (aSize = c1.length())
                  {
                     for (i=0; i<aSize; i++) *ptr++ = c1(i);
                     RWOrderedIterator nextSubview(itsColumnSubviews);
                     while (aColumnSubview = (ColumnSubview *) nextSubview())
                     {
                        c1 = ((CText *) aColumnSubview->
                              itsObservationCTexts.at(rowNumber))->GetTitle();
                        *ptr++ = '\t';
                        if (aSize = c1.length())
                        {
                           for (i=0; i<aSize; i++) *ptr++ = c1(i);
                        }
                     }
                     rowNumber++;
                     for (i=0; i<eol_len; i++) *ptr++ = EOL_SEQ[i];
                  }
                  else break;
               }
               *ptr++ = '\0';  // should not really need this, but I like it!
               if (!xvt_cb_put_data(CB_TEXT, NULL, ptr-orgptr-1,
                                    (PICTURE) NULL))
                  xvt_dm_post_error("Error putting text onto clipboard.");
               xvt_cb_free_data();
            }
            else xvt_dm_post_error("Cannot allocate clipboard memory.");
            xvt_cb_close();
            break;
         }
      }

      default:
         CWindow::DoMenuCommand(theMenuItem,isShiftkey, isControlKey);
         break;
   }
}


BOOLEAN GenerateReportWindow::ReportIsEmpty( void )
{
   return (!itsFirstColumnSubview ||
           (itsFirstColumnSubview &&
            itsFirstColumnSubview->itsObservationCTexts.entries() == 0));
}


void GenerateReportWindow::SizeWindow(int theWidth, int theHeigth)
{

   // propogate the resize message first, then set the size of
   // the scroller.

   CWwindow::SizeWindow(theWidth, theHeigth);

   if (theWidth < 500 || theHeigth < 200)
   {
      if (theWidth  < 500) theWidth  = 500;
      if (theHeigth < 200) theHeigth = 200;

      // this call will cause a recursive call back to this routine.

      Size (CRect(80,100,80+theWidth,100+theHeigth));
   }
   if (itsScrollingFrame)
   {
      itsScrollingFrame->Size(CRect(4,40,theWidth-4,theHeigth-40));
      itsScrollingFrame->DoSetOrigin(CPoint(0,0));
   }
   DoDraw();
}


BOOLEAN GenerateReportWindow::Close( void )
{
   // maybe we don't need this next command...didn't test it with/without!
   GetMenuBar()->SetEnabled(M_EDIT_COPY, FALSE);
   return CWwindow::Close();
}


void GenerateReportWindow::ClearReport( void )
{

   while (!itsScrollingFrame->GetSubviews()->isEmpty())
   {
      CSubview * aSubview;
      RWOrderedIterator subviewIterator (*itsScrollingFrame->GetSubviews());

      while ((aSubview = (CSubview *) subviewIterator()) != NULL)
      {
         RemoveSubview(aSubview);
         delete aSubview;
      }
   }

   itsColumnSubviews.clear();

   itsScrollingFrame->IScroller(TRUE, TRUE, TRUE, 24, 240);

   int start;
   const char * firstColumnTitle;

   int theReportType = itsReportTypeListButton->GetSelectPosition()+1;

   switch (theReportType)
   {
      case ReportTypeOBSOverYears:
           firstColumnTitle = "Year";         start= 60; break;
      // case ReportTypeOBSOverSpecies:
      //     firstColumnTitle = "Species";      start= 70; break;
      // case ReportTypeOBSOverSizeClass:
      //      firstColumnTitle = "Size Classes"; start=100; break;
      case ReportTypeOBSOverTreeIDS:
           firstColumnTitle = "Tree ID";      start= 80; break;
      case ReportTypeOBSOverCase:
           firstColumnTitle = "Case";         start= 60; break;
      case ReportTypeOBSOtherID:
           firstColumnTitle = "Obs Ident";      start= 100; break;
      default: Close();
   }
   itsFirstColumnSubview = new ColumnSubview(this,itsScrollingFrame,
                                             CRect(4,4,start-4,
                                                   itsLineHeight*10),
                                             firstColumnTitle, NULL);

   itsNextCaseButton     = new NButton(itsScrollingFrame,
                                       CRect(start,76,start+114,108),
                                       "Next Case");
   itsNextCaseButton     ->SetCommand( 5);

   itsNextVariableButton = new NButton(itsScrollingFrame,
                                       CRect(start,40,start+114,72),
                                       "Next Variable");
   itsNextVariableButton ->SetCommand( 6);

   itsNewColumnButton    = new NButton(itsScrollingFrame,
                                       CRect(start,4,start+114,36),
                                       "New Column");
   itsNewColumnButton    ->SetCommand( 7);

   NewColumn(NULL);

   DoDraw();

   // deactivate the copy menu item.

   GetMenuBar()->SetEnabled(M_EDIT_COPY, FALSE);
}


void GenerateReportWindow::DisplayTable( void )
{

   if (!itsColumnSubviews.entries())
   {
      GetMenuBar()->SetEnabled(M_EDIT_COPY, FALSE);

      // make sure that the controls are correctly set.

      DoCommand(FVSCaseListIncreased,this);
      return;
   }

   // build a list of observation identifications for use in defining
   // the contents of columnOne.  Do this in a nested scope so that the
   // binary tree is put on, then poped off the stack.

   RWOrdered columnOne;
   {
      RWBinaryTree columnOneBT;
      RWOrderedIterator nextSubview(itsColumnSubviews);
      ColumnSubview * aColumnSubview;
      while (aColumnSubview = (ColumnSubview *) nextSubview())
      {
         if (!aColumnSubview->itsFVSVariable) break;

         // iterate over the observations for the variable and load the
         // columnOneBT binary tree with the observation identifications.

         RWOrderedIterator nextObs(aColumnSubview->itsFVSVariable->
                                   itsObservations);
         FVSObservation * anObs;
         while (anObs = (FVSObservation *) nextObs())
         {
            if (!columnOneBT.contains(&anObs->itsObservationIdentification))
                 columnOneBT.insert(  &anObs->itsObservationIdentification);
         }
      }
      columnOne = columnOneBT.asOrderedCollection();
   }

   // display the first column, then the others.

   itsFirstColumnSubview->DisplayObservations(&columnOne);

   RWOrderedIterator nextSubview(itsColumnSubviews);
   ColumnSubview * aColumnSubview;
   while (aColumnSubview = (ColumnSubview *) nextSubview())
   {
      aColumnSubview->DisplayObservations(&columnOne);
   }

   GetMenuBar()->SetEnabled (M_EDIT_COPY, TRUE);

   // make sure that the controls are correctly set.

   DoCommand(FVSCaseListIncreased,this);
}


void GenerateReportWindow::WriteReport( void )
{
   if (itsColumnSubviews.entries() && itsFirstColumnSubview &&
       itsFirstColumnSubview->itsColumnTitle)
   {
      FILE_SPEC itsOutputFile;
      strcpy(itsOutputFile.name, ((CsupposeDoc*) theSpGlobals->
                                  theAppDocument)->SpGetFilePointer()->name);
      strcpy(itsOutputFile.type,"prn");
      xvt_fsys_get_dir(&itsOutputFile.dir);

      if (strncmp(itsOutputFile.name,NEWFILE,10) == 0)
      {
         strcpy(itsOutputFile.name," ");
      }
      else
      {
         size_t nLen = strlen(itsOutputFile.name);
         if (nLen > 4 && (strncmp(itsOutputFile.name+nLen-4,".",1) == 0))
         {
            strcpy(itsOutputFile.name+nLen-4,".prn");
         }
      }
      xvt_fsys_save_dir();
      switch (xvt_dm_post_file_save(&itsOutputFile, "Report file:"))
      {
         case FL_OK:
         {
            FILE * filePtr = fopen (itsOutputFile.name,"w");
            if (filePtr)
            {
               if (itsReportTitle->GetTitle().length())
               fprintf (filePtr,"%s\n",itsReportTitle->GetTitle().data());

               fprintf (filePtr,"%s",
                       itsFirstColumnSubview->itsColumnTitle->GetTitle().data());

               RWOrderedIterator nextSubview(itsColumnSubviews);
               ColumnSubview * aColumnSubview;
               while (aColumnSubview = (ColumnSubview *) nextSubview())
               {
                  fprintf (filePtr,"\t%s",aColumnSubview->
                          itsVariablesButton->GetTitle().data());
               }
               fprintf (filePtr,"\n");

               size_t rowNumber = 0;
               while (rowNumber < itsFirstColumnSubview->
                                  itsObservationCTexts.entries())
               {
                  CStringRW c1 = ((CText *) itsFirstColumnSubview->
                                  itsObservationCTexts.
                                  at(rowNumber))->GetTitle();
                  if (c1.length())
                  {
                     fprintf (filePtr,"%s",c1.data());

                     RWOrderedIterator nextSubview(itsColumnSubviews);
                     ColumnSubview * aColumnSubview;
                     while (aColumnSubview = (ColumnSubview *) nextSubview())
                     {
                        fprintf (filePtr,"\t%s",((CText *) aColumnSubview->
                                                 itsObservationCTexts.
                                                 at(rowNumber))->
                                 GetTitle().data());
                     }
                     rowNumber++;
                     fprintf (filePtr,"\n");
                  }
               }
               fclose(filePtr);
            }
            else
            {
               xvt_dm_post_warning ("Suppose can not open file:\n%s"
                                    "\nfor output.  Request ignored.",
                                    itsOutputFile.name);
            }
            break;
         }
         case FL_BAD:
         case FL_CANCEL:
         break;
      }
      xvt_fsys_restore_dir();
   }
}




void GenerateReportWindow::NewColumn(FVSVariable * theVariableToDisplay)
{

   ColumnSubview * mostRecent = itsFirstColumnSubview;

   if (itsColumnSubviews.entries()) mostRecent = (ColumnSubview *)
                                                 itsColumnSubviews.last();

   // get the dimensions of the most recent subview.  The new subview
   // will be to the right of the old one and will have the same height.

   CRect aRect = mostRecent->GetFrame();
   aRect.Left  (aRect.Right()+4);
   aRect.Right (aRect.Left()+110);
   ColumnSubview * newestColumn = new ColumnSubview(this, itsScrollingFrame,
                                                    aRect, NULL,
                                                    theVariableToDisplay);
   MoveButton(itsNextCaseButton,aRect.Right());
   MoveButton(itsNextVariableButton,aRect.Right());
   MoveButton(itsNewColumnButton,aRect.Right());

   itsColumnSubviews.insert(newestColumn);
   itsScrollingFrame->ShrinkToFit();

   // scroll as necessary to make sure the "new column" is inside the
   // visible part of the scroller.

   itsScrollingFrame->SetHIncrements(114,
                                     itsScrollingFrame->GetHPageIncrement());

   int diff = itsNewColumnButton->GetFrame().Right()-
              itsScrollingFrame->GetClippedFrame().Right();

   itsScrollingFrame->SetHIncrements(124,
                                     itsScrollingFrame->GetHPageIncrement());
   if (diff > 0) itsScrollingFrame->AutoScroll(124,0);

   DoCommand(COLUMNChanged, this);
}




ColumnSubview::ColumnSubview(GenerateReportWindow * theReportWindow,
                             MyCScroller          * theScrollingFrame,
                             const CRect&           theRegion,
                             const char           * theColumnTitle,
                             FVSVariable          * theVariable)
              :CSubview(theScrollingFrame, theRegion),
               itsReportWindow(theReportWindow),
               itsFVSVariable(theVariable),
               itsVariablePicker(NULL)
{
   if (theColumnTitle)
      itsColumnTitle = new CText(this,CPoint(4,6),theColumnTitle);
   else
   {
      itsColumnTitle = NULL;
      CRect aRect = theRegion;
      aRect.Top   (0);
      aRect.Right (aRect.Right()-aRect.Left());
      aRect.Left  (0);
      aRect.Bottom(32);
      CStringRW aTitle;
      if (itsFVSVariable) aTitle = itsFVSVariable->BuildNameString(aTitle);
      else aTitle = "Pick Variable";

      itsVariablesButton = new NButton (this,aRect,aTitle,0L);
      itsVariablesButton->SetCommand(1);
   }
}



ColumnSubview::~ColumnSubview( void )
{
   // if itsVariablePicker is not null, then the window exists.  We
   // need to notify it to close, but we need to do so in a way that
   // signals to it that it should not reference anything in this
   // class.  This is done by setting the reference to this class that
   // resides in the variable picker to NULL prior to calling close.

   if (itsVariablePicker)
   {
      itsVariablePicker->itsRelatedSubview = NULL;
      itsVariablePicker->Close();
   }
}



void ColumnSubview::DoCommand(long theCommand,void * theData)
{
   switch (theCommand)
   {
      case 1:   // Variable Button Pushed.
      {
         if (itsVariablePicker)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsVariablePicker);
         else itsVariablePicker = new PickVariableWindow
                 (this, 1, itsReportWindow->itsReportTypeListButton->
                  GetSelectPosition()+1);
         break;
      }
      case 2:   // Variable Picker Set a new variable.
      {
         CStringRW aTitle;
         itsVariablesButton->SetTitle(itsFVSVariable->BuildNameString(aTitle));
         itsReportWindow->DoCommand(COLUMNChanged, this);
         break;
      }
      default: CSubview::DoCommand(theCommand,theData); break;
   }
}


void ColumnSubview::DisplayObservations(RWOrdered * columnOne)
{

   // if itsColumnTitle is not NULL, then this is the first column
   // and the job is to display the list of observation identifications
   // in columnOne  (strip out the trailing $0 and $1 tags).

   size_t rowNumber = 0;
   CRect aRect = GetLocalFrame();

   if (itsColumnTitle)
   {
      aRect.Right(aRect.Right()-10);

      RWOrderedIterator nextObsID(*columnOne);
      CStringRWC * aObsID;
      CStringRW tmpString;
      size_t place;
      while (aObsID = (CStringRWC *) nextObsID())
      {
         // strip out the $0 or $1 part of the observation id.

         tmpString = *aObsID;
         place = tmpString.index("$");
         if (place != RW_NPOS) tmpString = tmpString(0, place);

         // if there is a CText for this obsID, use it; otherwise make one.

         CText * currCText;
         if (rowNumber < itsObservationCTexts.entries())
         {
            currCText = (CText *) itsObservationCTexts.at(rowNumber);
            currCText->SetTitle(tmpString);
         }
         else
         {
            aRect.Top(36+(rowNumber*(itsReportWindow->itsLineHeight+3)));
            aRect.Bottom(aRect.Top()+itsReportWindow->itsLineHeight+1);

            // adjust the size of the subview.

            CRect subviewRect = GetFrame();
            UNITS add = aRect.Bottom()-GetLocalFrame().Bottom();
            subviewRect.Bottom(subviewRect.Bottom()+add);
            DoSize(subviewRect);

            currCText = new CText(this, aRect, tmpString);
            currCText->SetPlacement(CText::RIGHT);
            itsObservationCTexts.insert(currCText);
         }
         rowNumber++;
      }

      // now clear the rest of the CTexts, if there are any.

      while (rowNumber < itsObservationCTexts.entries())
      {
         ((CText *) itsObservationCTexts.at(rowNumber++))->
         SetTitle(NULLString);
      }
   }
   else
   {

      // set up the column...prepare it for displaying the observations.

      aRect.Right(aRect.Right()-NScrollBar::NativeWidth()-10);

      // make sure that this subview is as tall as the first.

      CRect subviewRect = GetFrame();
      subviewRect.Bottom(itsReportWindow->itsFirstColumnSubview->
                         GetFrame().Bottom());
      DoSize(subviewRect);

      // insure that there are as many CTexts for this column as
      // there are in the first column...and that their placement
      // is almost identical.

      RWOrdered * theFirstColumnCTexts = &itsReportWindow->
                                          itsFirstColumnSubview->
                                          itsObservationCTexts;

      while (rowNumber < theFirstColumnCTexts->entries())
      {
         CText * currCText = NULL;
         CText * aFirstColumnCText = (CText *) theFirstColumnCTexts->
                                     at(rowNumber);
         if (rowNumber < itsObservationCTexts.entries())
         {
            currCText = (CText *) itsObservationCTexts.at(rowNumber);
            currCText->SetTitle(NULLString);
         }
         else
         {
            aRect.Top   (aFirstColumnCText->GetFrame().Top()   );
            aRect.Bottom(aFirstColumnCText->GetFrame().Bottom());
            currCText = new CText(this, aRect, NULLString);
            currCText->SetPlacement(CText::RIGHT);
            itsObservationCTexts.insert(currCText);
         }
         rowNumber++;
      }

      // now clear the rest of the CTexts, if there are any.

      while (rowNumber < itsObservationCTexts.entries())
      {
         ((CText *) itsObservationCTexts.at(rowNumber++))->
         SetTitle(NULLString);
      }

      // the last step is tricky...for each observation of a given variable,
      // find the row number where it's value shall be displayed, and
      // then set the title of the corresponding CText.  Good Luck!

      if (itsFVSVariable)
      {
         // iterate over the observations for the variable, find and
         // and load the CText corresponding to the correct row.

         RWOrderedIterator nextObs(itsFVSVariable->itsObservations);
         FVSObservation * anObs;
         while (anObs = (FVSObservation *) nextObs())
         {
            rowNumber = columnOne->index(&anObs->
                                         itsObservationIdentification);
            if (rowNumber != RW_NPOS &&
                rowNumber < itsObservationCTexts.entries())
                ((CText *) itsObservationCTexts.at(rowNumber))->
                                                 SetTitle(*anObs);
         }
      }
   }

   // DoDraw does not seem to work!

   RCT aRct = RCT(GetGlobalFrame());
   xvt_dwin_invalidate_rect(itsReportWindow->GetXVTWindow(), &aRct);
   itsReportWindow->itsScrollingFrame->ShrinkToFit();
}



PickVariableWindow::PickVariableWindow (CSubview * theRelatedSubview,
                                        int        theSubviewType,
                                        int        theType)
                   :CWwindow(theSpGlobals->theAppDocument,
                             CenterWinPlacement(CRect(100,100,600,364)),
                             "Pick Variable",
                             WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                             W_DOC, TASK_MENUBAR),
                    itsRelatedSubview(theRelatedSubview),
                    itsSubviewType(theSubviewType),
                    itsType(theType)
{
   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   // if the class is being built when no variable is
   // currently picked, then we load the lists with a cascade from
   // the top down (allCases -> aReport -> aVariable).

   // If a variable is already picked, then load the
   // lists from the bottom up (variable -> report -> case).

   // It is simplier to first simply build all the visual objects
   // then load the lists and set the selected items.  So, we will
   // initially load with a null list (except for the cases).

   // there will be two subview types:
   // itsSubviewType = 1 signifies that we are building a tabular report
   // itsSubviewType = 2 signifies that we are building a graph

   CText * aCasesCText = new CText(this, CPoint(4,8), "Cases: ");

   itsCasesListButton = new NListButton(this, CRect(76,4,496,260),
                                        *theSpGlobals->theReportData, 0L);
   itsCasesListButton->SetGlue(TOPSTICKY | LEFTSTICKY | RIGHTSTICKY);
   itsCasesListButton->SetCommand(1);

   RWOrdered tmpList((size_t) 1);
   new CText(this, CPoint(4,40), "Reports: ");
   itsReportsListButton = new NListButton(this, CRect(76,40,496,260),
                                          tmpList, 0L);
   itsReportsListButton->SetGlue(TOPSTICKY | LEFTSTICKY | RIGHTSTICKY);
   itsReportsListButton->SetCommand(2);

   new CText(this, CPoint(4,80), "Variables: ");
   itsVariablesListBox = new NListBox(this, CRect(76,80,170,224),
                                      tmpList, 0L);
   itsVariablesListBox->SetGlue(TOPSTICKY | LEFTSTICKY | BOTTOMSTICKY);
   itsVariablesListBox->SetCommand(3);
   itsVariablesListBox->SetDoubleCommand(4);

   itsUnitsCText    = new CText(this,CPoint(174,80),NULLString);
   itsNObsCText     = new CText(this,CPoint(174,104),NULLString);
   itsNObsMaxCText  = new CText(this,CPoint(174,128),NULLString);
   itsNObsMinCText  = new CText(this,CPoint(174,152),NULLString);

   itsOkButton     = new NButton(this, CRect(4,228,94,260),   "Ok");
   itsApplyButton  = new NButton(this, CRect(138,228,228,260),"Apply");
   itsDeleteButton = new NButton(this, CRect(272,228,362,260),"Delete");
   itsCloseButton  = new NButton(this, CRect(406,228,496,260),"Close");

   itsOkButton    ->SetGlue(BOTTOMSTICKY);
   itsApplyButton ->SetGlue(BOTTOMSTICKY);
   itsDeleteButton->SetGlue(BOTTOMSTICKY);
   itsCloseButton ->SetGlue(BOTTOMSTICKY);

   itsOkButton    ->SetCommand(4);
   itsApplyButton ->SetCommand(5);
   itsDeleteButton->SetCommand(6);
   itsCloseButton ->SetCommand(7);

   // figure out which list to present.

   FVSCase     * currCase;
   FVSReport   * currReport;
   FVSVariable * theFVSVariable = NULL;

   if (itsSubviewType == 1 && itsRelatedSubview) theFVSVariable =
      ((ColumnSubview *) itsRelatedSubview)->itsFVSVariable;
   if (itsSubviewType == 2 && itsRelatedSubview) theFVSVariable =
      ((GraphVariableSubview *) itsRelatedSubview)->itsFVSVariable;

   if (theFVSVariable)
   {
      LoadVariableList(theFVSVariable->itsFVSReport);
      itsVariablesListBox->IListBox(itsVariablesList);
      size_t i = itsVariablesList.index(theFVSVariable);
      if (i != RW_NPOS) itsVariablesListBox->SelectItem((int) i);
      else itsVariablesListBox->DeselectAll();

      currReport = theFVSVariable->itsFVSReport;
      currCase   = currReport->itsFVSCase;

      if (currCase == theSpGlobals->theReportData->itsOBSOverCaseCase)
      {
         itsCasesListButton->Hide();
         aCasesCText->SetTitle("You are displaying variables over cases.");
      }
      else
      {
         size_t i = currCase->itsAllFVSCases->index(currCase);
         if (i == RW_NPOS) i=0;
         itsCasesListButton->IListButton(*currCase->itsAllFVSCases,int(i));
      }
      LoadReportList(currCase);
      i = itsReportsList.index(currReport);
      if (i == RW_NPOS) i=0;
      itsReportsListButton->IListButton(itsReportsList,int(i));
   }
   else
   {
      if (theSpGlobals->theReportData->entries())
      {
         if ((itsSubviewType == 1 && itsType == ReportTypeOBSOverCase) ||
             (itsSubviewType == 2 && itsType == GraphTypeBarYOverCase))
         {
            // Make sure that OBSOverCaseCase is loaded.

            xvt_scr_set_busy_cursor();
            theSpGlobals->theReportData->LoadOBSOverCaseCase();

            currCase = theSpGlobals->theReportData->itsOBSOverCaseCase;
            itsCasesListButton->Hide();
            aCasesCText->SetTitle("You are displaying variables over cases.");
         }
         else
         {
            // if there is a case in use, and if it can be found, then
            // let it be the preselected case.

            int currPosition=0;
            if (theSpGlobals->theReportData->itsCaseInUse)
            {
               currPosition=theSpGlobals->theReportData->
                  index(theSpGlobals->theReportData->itsCaseInUse);
               if (currPosition == RW_NPOS) currPosition=0;
            }
            currCase = (FVSCase *) theSpGlobals->theReportData->
                  at(currPosition);
            itsCasesListButton->IListButton(*theSpGlobals->theReportData,
                                            currPosition);
         }
         if (!currCase) return;
         LoadReportList(currCase);
         itsReportsListButton->IListButton(itsReportsList, 0);
         currReport = (FVSReport *) itsReportsList.first();
         if (!currReport) return;
         LoadVariableList(currReport);
         itsVariablesListBox->IListBox(itsVariablesList);
         itsVariablesListBox->DeselectAll();
      }
   }
   DisplayStatisticsAboutVariable(theFVSVariable);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), VARIABLEPicker, 0L);

   aNav->AppendTabStop(new CTabStop(itsCasesListButton));
   aNav->AppendTabStop(new CTabStop(itsReportsListButton));
   aNav->AppendTabStop(new CTabStop(itsVariablesListBox));
   aNav->AppendTabStop(new CTabStop(itsOkButton));
   aNav->AppendTabStop(new CTabStop(itsApplyButton));
   aNav->AppendTabStop(new CTabStop(itsDeleteButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));
   Show();
   aNav->InitFocus();
}


void PickVariableWindow::DoCommand(long theCommand,void * theData)
{

   if (theCommand > 10000)
   {
      CWwindow::DoCommand(theCommand,theData);
      return;
   }

   FVSCase     * currCase       = NULL;
   FVSReport   * currReport     = NULL;
   FVSVariable * theFVSVariable = NULL;

   if (itsSubviewType == 1 && itsRelatedSubview) theFVSVariable =
      ((ColumnSubview *) itsRelatedSubview)->itsFVSVariable;
   else if (itsSubviewType == 2 && itsRelatedSubview) theFVSVariable =
      ((GraphVariableSubview *) itsRelatedSubview)->itsFVSVariable;

   switch (theCommand)
   {
      case 1:   // Case list button
      {
         // note that there will not be a visable case button if the report
         // type is ReportTypeOBSOverCase

         int item = itsCasesListButton->GetSelectPosition();
         if (item < 0) break;
         currCase = (FVSCase *) theSpGlobals->theReportData->at((size_t) item);
         if (!currCase) break;
         LoadReportList(currCase);
         if (currReport) currReport = currCase->GetReport(currReport->data());
         if (currReport)
         {
            size_t i = itsReportsList.index(currReport);
            if (i == RW_NPOS) item=0;
         }
         else
         {
            item = 0;
            currReport = (FVSReport *) itsReportsList.first();
         }
         itsReportsListButton->IListButton(itsReportsList, item);
         if (!currReport) break;

         LoadVariableList(currReport);
         itsVariablesListBox->IListBox(itsVariablesList);
         if (theFVSVariable)
            theFVSVariable = currReport->GetVariable(theFVSVariable->data());
         if (theFVSVariable)
         {
            RWOrderedIterator nextVar(itsVariablesList);
            FVSVariable * aVar;
            int i = 0;
            itsVariablesListBox->DeselectAll();
            while (aVar = (FVSVariable *) nextVar())
            {
               if (strcmp(theFVSVariable->data(),aVar->data()) == 0)
               {
                  itsVariablesListBox->SelectItem((int) i);
                  break;
               }
               i++;
            }
         }
         else itsVariablesListBox->DeselectAll();
         DisplayStatisticsAboutVariable(theFVSVariable);
         break;
      }
      case 2:   // Report list button
      {
         int item = -1;
         if (itsSubviewType == 1 &&
             itsType        == ReportTypeOBSOverCase)
            currCase = theSpGlobals->theReportData->itsOBSOverCaseCase;
         else
         {
            item = itsCasesListButton->GetSelectPosition();
            if (item < 0) break;
            currCase = (FVSCase *) theSpGlobals->
               theReportData->at((size_t) item);
         }

         if (!currCase) break;

         item = itsReportsListButton->GetSelectPosition();
         if (item < 0) break;

         currReport = (FVSReport *) itsReportsList.at((size_t) item);
         if (!currReport) break;

         LoadVariableList(currReport);
         itsVariablesListBox->IListBox(itsVariablesList);
         if (theFVSVariable)
            theFVSVariable = currReport->GetVariable(theFVSVariable->data());
         if (theFVSVariable)
         {
            RWOrderedIterator nextVar(itsVariablesList);
            FVSVariable * aVar;
            int i = 0;
            itsVariablesListBox->DeselectAll();
            while (aVar = (FVSVariable *) nextVar())
            {
               if (strcmp(theFVSVariable->data(),aVar->data()) == 0)
               {
                  itsVariablesListBox->SelectItem((int) i);
                  break;
               }
               i++;
            }
         }
         else itsVariablesListBox->DeselectAll();
         DisplayStatisticsAboutVariable(theFVSVariable);
         break;
      }
      case 3:   // Variable list button
      case 4:   // Variable list button double command
                // OK button
      case 5:   // Apply button
      {
         FVSVariable * anFVSVariable = NULL;
         int item;
         if ((itsSubviewType == 1 && itsType == ReportTypeOBSOverCase) ||
             (itsSubviewType == 2 && itsType == GraphTypeBarYOverCase))
            currCase = theSpGlobals->theReportData->itsOBSOverCaseCase;
         else
         {
            item = itsCasesListButton->GetSelectPosition();
            if (item < 0) break;
            currCase = (FVSCase *) theSpGlobals->theReportData->
               at((size_t) item);
         }
         if (currCase)
         {
            item = itsReportsListButton->GetSelectPosition();
            if (item >= 0) currReport = (FVSReport *)
                                 itsReportsList.at((size_t) item);
            if (currReport)
            {
               item = itsVariablesListBox->GetSelectPosition();
               if (item >= 0)
               {
                  anFVSVariable = (FVSVariable *) itsVariablesList.at(item);
                  if (anFVSVariable &&
                      anFVSVariable != theFVSVariable &&
                      theCommand    != 3)
                  {
                     if (itsSubviewType == 1 && itsRelatedSubview)
                     {
                        ((ColumnSubview *) itsRelatedSubview)->
                           itsFVSVariable = anFVSVariable;
                        ((ColumnSubview *) itsRelatedSubview)->DoCommand(2);
                     }
                     else if (itsSubviewType == 2 && itsRelatedSubview)
                     {
                        ((GraphVariableSubview *) itsRelatedSubview)->
                           itsFVSVariable = anFVSVariable;
                        ((GraphVariableSubview *) itsRelatedSubview)->
                           DoCommand(2);
                     }
                  }
                  if (theCommand == 4) Close();
               }
            }
         }
         if (theCommand != 4) DisplayStatisticsAboutVariable(anFVSVariable);
         break;
      }
      case 6:   // Delete button
      {
         // call the window that owns the associated subview, and tell
         // it to delete the subview.  This process will also close this
         // window.

         if (itsSubviewType == 1 && itsRelatedSubview)
            ((ColumnSubview *) itsRelatedSubview)->itsReportWindow->
               DoCommand(8, itsRelatedSubview);
         else if (itsSubviewType == 2 && itsRelatedSubview)
            ((GraphVariableSubview *) itsRelatedSubview)->itsGraphWindow->
            DoCommand(8, itsRelatedSubview);
         break;
      }
      case 7:   // Close button
      {
         Close();
         break;
      }
      case FVSCaseListIncreased:
      case FVSCaseListDecreased:
      {
         Close();
         break;
      }
      default: CWwindow::DoCommand(theCommand,theData); break;
   }
}



BOOLEAN PickVariableWindow::Close( void )
{
   // if itsRelatedColumn is NULL, it means that the column is
   // calling this close function from its own destructor.  In
   // this case, we should NOT attempt to notify itsRelatedColumn
   // that this window is closing.

   if (itsSubviewType == 1 && itsRelatedSubview)
      ((ColumnSubview *) itsRelatedSubview)->itsVariablePicker = NULL;
   else if (itsSubviewType == 2 && itsRelatedSubview)
      ((GraphVariableSubview *) itsRelatedSubview)->itsVariablePicker = NULL;

   return CWwindow::Close();
}



void PickVariableWindow::LoadReportList(FVSCase * currCase)
{
   if (itsSubviewType == 1)
   {
      currCase->LoadListWithReportsOfType(itsType, &itsReportsList);
   }
   else
   {
      switch (itsType)
      {
         case GraphTypeLineOverYears:
         {
            currCase->LoadListWithReportsOfType(FVSDataTypeOBSOverYears,
                                                &itsReportsList);
            break;
         }
         case GraphTypeScatterYOverX:
         {
            currCase->LoadListWithReportsOfType(FVSDataTypeOBSOverYears,
                                                &itsReportsList);
            currCase->LoadListWithReportsOfType(FVSDataTypeOBSOverTreeIDS,
                                                &itsReportsList, FALSE);
            break;
         }
         case GraphTypeBarYOverObsID:
         {
            currCase->LoadListWithReportsOfType(FVSDataTypeOBSOverOther,
                                                &itsReportsList);
            break;
         }
         case GraphTypeBarYOverCase:
         {
            currCase->LoadListWithReportsOfType(FVSDataTypeOBSOverCase,
                                                &itsReportsList);
            break;
         }
         case GraphTypePhaseOverYears:
         {
            currCase->LoadListWithReportsOfType(FVSDataTypeOBSOverYears,
                                                &itsReportsList);
            break;
         }
         default: break;
      }
   }
}


void PickVariableWindow::LoadVariableList(FVSReport * currReport)
{
   if (itsSubviewType == 1)
   {
      currReport->LoadListWithVariablesOfUnit(&itsVariablesList);
   }
   else
   {
      switch (itsType)
      {
         case GraphTypeLineOverYears:
         case GraphTypeScatterYOverX:
         case GraphTypeBarYOverObsID:
         case GraphTypeBarYOverCase:
         {
            currReport->LoadListWithVariablesOfUnit(&itsVariablesList,
                                                    FVSVariableUnitsClassVariable,
                                                    FALSE);
            break;
         }
         case GraphTypePhaseOverYears:
         {
            currReport->LoadListWithVariablesOfUnit(&itsVariablesList,
                                                    FVSVariableUnitsClassVariable);
            break;
         }
         default: break;
      }
   }
}




void PickVariableWindow::DisplayStatisticsAboutVariable(FVSVariable * aV)
{
   if (aV)
   {
      FVSObservation * aObs;
      itsUnitsCText->SetTitle(aV->GetUnitsDescription());
      CStringRW tmpS = "Number of observations = ";
      tmpS += ConvertToString(long(aV->itsObservations.entries()));
      itsNObsCText->SetTitle(tmpS);
      size_t tmpIndex = aV->GetMaxIndex();
      if (tmpIndex != RW_NPOS)
      {
         aObs = (FVSObservation *) aV->itsObservations.at(tmpIndex);
         tmpS = "Max = ";
         tmpS += (CStringRWC &) *aObs;
         tmpS += " for ";
         CStringRW aObsID = aObs->itsObservationIdentification;
         size_t place = aObsID.index("$");
         if (place != RW_NPOS) aObsID = aObsID(0, place);
         tmpS += aObsID;
         itsNObsMaxCText->SetTitle(tmpS);
      }
      else itsNObsMaxCText->SetTitle(NULLString);

      tmpIndex = aV->GetMinIndex();
      if (tmpIndex != RW_NPOS)
      {
         aObs = (FVSObservation *) aV->itsObservations.at(tmpIndex);
         tmpS = "Min = ";
         tmpS += (CStringRWC &) *aObs;
         tmpS += " for ";
         CStringRW aObsID = aObs->itsObservationIdentification;
         size_t place = aObsID.index("$");
         if (place != RW_NPOS) aObsID = aObsID(0, place);
         tmpS += aObsID;
         itsNObsMinCText->SetTitle(tmpS);
      }
      else itsNObsMaxCText->SetTitle(NULLString);
   }
   else
   {
      itsUnitsCText  ->SetTitle(NULLString);
      itsNObsCText   ->SetTitle(NULLString);
      itsNObsMaxCText->SetTitle(NULLString);
      itsNObsMinCText->SetTitle(NULLString);
   }
}
