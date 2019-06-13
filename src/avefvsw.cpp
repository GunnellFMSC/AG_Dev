/********************************************************************

  File Name:     avefvsw.cpp
  Author:        nlc
  Date:          07/27/98

  see avefvsw.hpp for notes.

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
#include "avefvsw.hpp"

AverageFVSOutputWindow::AverageFVSOutputWindow(CDocument * theDocument)
                       :CWwindow(theDocument,
                                 RightWinPlacement(CRect(10,20,510,440)), "Average FVS Output",
                                 WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                                 W_DOC, TASK_MENUBAR)
{

   xvt_scr_set_busy_cursor();

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   new CText(this, CPoint(4,4), "Cases: ");

   itsCaseListBox = new NListBox(this, CRect(4,28,496,244),
                                 *theSpGlobals->theReportData,
                                 CTL_FLAG_MULTIPLE);
   itsCaseListBox->SetCommand(1);
   itsCaseListBox->SetGlue(TOPSTICKY | LEFTSTICKY |
                           BOTTOMSTICKY | RIGHTSTICKY);

   CText * aCT = new CText(this, CPoint(4,248), "Groups: ");
   aCT->SetGlue(BOTTOMSTICKY);

   itsGroupListBox = new NListBox(this, CRect(4,276,296,416),
                                 itsGroupList, CTL_FLAG_MULTIPLE);
   itsGroupListBox->SetCommand(2);
   itsGroupListBox->SetGlue(LEFTSTICKY | RIGHTSTICKY | BOTTOMSTICKY);

   itsAverageCasesButton  = new NButton(this, CRect(300,276,496,308),
                                        "Average Selected Cases");
   itsAverageGroupsButton = new NButton(this, CRect(300,312,496,344),
                                        "Average Selected Groups");
   itsReadFVSOutputButton = new NButton(this, CRect(300,348,496,380),
                                        "Read FVS Output");
   itsResetButton         = new NButton(this, CRect(300,384,396,416),
                                        "Reset");
   itsCloseButton         = new NButton(this, CRect(400,384,496,416),
                                        "Close");

   itsAverageCasesButton ->SetCommand(3);
   itsAverageGroupsButton->SetCommand(4);
   itsReadFVSOutputButton->SetCommand(M_AFTER_SIM_READFVSOUTPUT);
   itsResetButton        ->SetCommand(5);
   itsCloseButton        ->SetCommand(6);

   itsAverageCasesButton ->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);
   itsAverageGroupsButton->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);
   itsReadFVSOutputButton->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);
   itsResetButton        ->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);
   itsCloseButton        ->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);

   LoadGroupList();

   Show();

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), AVERAGEFVSOutput, 0L);

   aNav->AppendTabStop(new CTabStop(itsAverageCasesButton));
   aNav->AppendTabStop(new CTabStop(itsAverageGroupsButton));
   aNav->AppendTabStop(new CTabStop(itsReadFVSOutputButton));
   aNav->AppendTabStop(new CTabStop(itsResetButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));
   aNav->AppendTabStop(new CTabStop(itsCaseListBox));
   aNav->AppendTabStop(new CTabStop(itsGroupListBox));

   aNav->InitFocus();
}




void AverageFVSOutputWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1: // Cases list box
      {
         break;
      }
      case 2: // Groups list box
      {
         break;
      }
      case 3: // Average Selected Cases
      {
         size_t numSelectedCases = itsCaseListBox->GetNumSelectedItems();
         if (numSelectedCases == 0)
         {
            itsCaseListBox->SelectAll();
         }
         else
         {
            RWOrdered selectedItems(numSelectedCases);
            for (size_t i=0;
                 i < theSpGlobals->theReportData->entries(); i++)
            {
               if (itsCaseListBox->IsItSelected((int) i))
               {
                  FVSCase * aCase = (FVSCase *)
                        theSpGlobals->theReportData->at(i);

                  // make sure the case is actually a case and not
                  // an average or a treelist or something else.

                  if (aCase->first('c') == 0)
                     selectedItems.insert(aCase);
               }
            }

            // create the average and add it to the cases.

            char t[20];
            sprintf (t,"Average of %d cases",selectedItems.entries());

            FVSCase * theAve = theSpGlobals->theReportData->
                  AverageCases(&selectedItems,t);

            if (theAve)
            {
               theSpGlobals->theReportData->insert(theAve);
               SendUpdateMessage (FVSCaseListIncreased);
            }
            else itsCaseListBox->DeselectAll();
         }
         break;
      }

      case 4:   // Average Selected Groups
      {
         size_t numSelectedCases = itsGroupListBox->GetNumSelectedItems();
         if (numSelectedCases == 0)
         {
            itsGroupListBox->SelectAll();
         }
         else
         {
            int newAverages = 0;
            RWOrdered selectedCases;
            CStringRW aG;
            int nItems = itsGroupListBox->GetNumItems();
            for (int item=0; item<nItems; item++)
            {
               if (itsGroupListBox->IsItSelected(item))
               {
                  aG = itsGroupListBox->GetItem(item);
                  selectedCases.clear();
                  RWOrderedIterator nextC(*theSpGlobals->theReportData);
                  FVSCase * aC;
                  while (aC = (FVSCase *) nextC())
                  {
                     if (aC->first('c') == 0)
                     {
                        RWCString tmp = *aC;
                        size_t loc = tmp.last(')');
                        if (loc != RW_NPOS) tmp = tmp.remove(loc);
                        loc = tmp.first('(');
                        if (loc !=  RW_NPOS) tmp = tmp.remove(0,loc+1);
                        RWTokenizer nextG(tmp);
                        RWCString aGrp;
                        while (!(aGrp=nextG(",")).isNull())
                        {
                           aGrp = aGrp.strip(RWCString::both,' ');
                           if (strcmp(aG.data(),aGrp.data()) == 0)
                           {
                              selectedCases.insert(aC);
                              break;
                           }
                        }
                     }
                  }
                  if (selectedCases.entries() > 1)
                  {
                     // create an average for the group.

                     CStringRW t = "Group=(";
                     t += aG;
                     t += ")";

                     FVSCase * theAve = theSpGlobals->theReportData->
                           AverageCases(&selectedCases,t.data());
                     if (theAve)
                     {
                        newAverages++;
                        theSpGlobals->theReportData->insert(theAve);
                     }
                  }
               }
            }
            if (newAverages) SendUpdateMessage (FVSCaseListIncreased);
         }
         break;
      }

      case 5:   // Reset
      {
         itsCaseListBox->DeselectAll();
         LoadGroupList();
         break;
      }

      case 6:   // Close
      {
         Close();
         break;
      }

      case FVSCaseListIncreased:
      case FVSCaseListDecreased:
      {
         itsCaseListBox->IListBox(*theSpGlobals->theReportData);
         LoadGroupList();
         break;
      }

      default:
      {
         CWwindow::DoCommand(theCommand, theData);
         break;
      }
   }
}



BOOLEAN AverageFVSOutputWindow::Close( void )
{
   itsGroupList.clearAndDestroy();
   return CWwindow::Close();
}


void AverageFVSOutputWindow::LoadGroupList( void )
{
   itsGroupListBox->DeselectAll();
   itsGroupList.clearAndDestroy();

   RWCString tmp,group;
   RWOrderedIterator nextC(*theSpGlobals->theReportData);
   FVSCase * aCase;
   while (aCase = (FVSCase *) nextC())
   {
      if (aCase->first('c') == 0)
      {
         tmp = *aCase;
         size_t loc = tmp.last(')');
         if (loc == RW_NPOS) break;
         tmp = tmp.remove(loc);
         loc = tmp.first('(');
         if (loc ==  RW_NPOS) break;
         tmp = tmp.remove(0,loc+1);
         RWTokenizer nextG(tmp);
         while (!(group=nextG(",")).isNull())
         {
            group = group.strip(RWCString::both,' ');
            RWOrderedIterator nextHaveIt(itsGroupList);
            CStringRWC * haveIt;
            int yea = 0;
            while (haveIt = (CStringRWC *) nextHaveIt())
            {
               if (strcmp(haveIt->data(),group.data()) == 0)
               {
                  yea = 1;
                  break;
               }
            }
            if (!yea) itsGroupList.insert(new CStringRWC(group));
         }
      }
   }
   itsGroupListBox->IListBox(itsGroupList);
}

