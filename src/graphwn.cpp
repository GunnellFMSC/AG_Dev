/*********************************************************************

   File Name:    graphwn.cpp
   Author:       nlc
   Date:         11/96

   see graphwn.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include <float.h>

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
#include CPointRWC_i
#include CPolygon_i
#include CLine_i
#include CNavigator_i

#include "mycscrol.hpp"
#include "reportd.hpp"
#include "reportwn.hpp"
#include "graphwn.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "sendupdt.hpp"
#include "appdoc.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>

#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

#define VARIABLEChanged  100


GenerateGraphWindow::GenerateGraphWindow(CDocument * theDocument)
   :CWwindow(theDocument,
             CenterTopWinPlacement(CRect(10,20,610,440)), "Generate Graph",
             WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
             W_DOC, TASK_MENUBAR),
    itsPlot(NULL),
    itsColorIndex(-1),
    itsCurrentGraphType(GraphTypeLineOverYears)
{
   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   int ascentp, descent;
   xvt_dwin_get_font_metrics(GetCWindow()->GetXVTWindow(),
                             &itsLineHeight, &ascentp, &descent);
   itsLineHeight = itsLineHeight+ascentp+descent;

   new CText(this, CPoint(4,8), "Title: ");
   itsGraphTitle  = new NEditControl(this, CRect(56,4,332,36),
                                      NULLString, 0L);
   itsGraphTitle->SetGlue(TOPSTICKY | LEFTSTICKY | RIGHTSTICKY);

   CView * tempCView = (CView *) new CText(this, CPoint(340,8), "Type: ");
   tempCView->SetGlue(TOPSTICKY | RIGHTSTICKY);

   itsGraphTypeListButton = new NListButton(this, CRect(384,4,592,148),
                   theSpGlobals->theReportData->itsGraphTypesList, 0L);
   itsGraphTypeListButton->SetCommand( 4);
   itsGraphTypeListButton->SetGlue(TOPSTICKY | RIGHTSTICKY);
   itsGraphTypeListButton->SelectItem(itsCurrentGraphType-1);

   // set up CScroller.

   itsScrollingFrame = new MyCScroller(this, CRect(4,40,592,380),
                                       592-4-NScrollBar::NativeWidth(),
                                       itsLineHeight*10);
   itsScrollingFrame->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                              LEFTSTICKY   | RIGHTSTICKY);

   // set up the bottom buttons.

   itsWriteGraphButton   = new NButton(this, CRect(  4,384,120,416),
                                       "Write Graph");
   itsWriteGraphButton   ->Disable();

   itsClearGraphButton   = new NButton(this, CRect(130,384,246,416),
                                       "Clear Graph");
   itsReadFVSOutputButton= new NButton(this, CRect(256,384,396,416),
                                       "Read FVS Output");
   itsGraphsButton       = new NButton(this, CRect(406,384,496,416),
                                       "Graphs");
   itsCloseButton        = new NButton(this, CRect(506,384,596,416),
                                       "Close");

   itsWriteGraphButton   ->SetCommand( 1);
   itsClearGraphButton   ->SetCommand( 2);
   itsReadFVSOutputButton->SetCommand( M_AFTER_SIM_READFVSOUTPUT);
   itsGraphsButton       ->SetCommand( M_AFTER_SIM_GRAPHICS);
   itsCloseButton        ->SetCommand( 3);

   itsWriteGraphButton   ->SetGlue(BOTTOMSTICKY);
   itsClearGraphButton   ->SetGlue(BOTTOMSTICKY);
   itsReadFVSOutputButton->SetGlue(BOTTOMSTICKY);
   itsGraphsButton       ->SetGlue(BOTTOMSTICKY);
   itsCloseButton        ->SetGlue(BOTTOMSTICKY);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), GENERATEGraphs, 0L);

   aNav->AppendTabStop(new CTabStop(itsGraphTitle));
   aNav->AppendTabStop(new CTabStop(itsGraphTypeListButton));
   aNav->AppendTabStop(new CTabStop(itsWriteGraphButton));
   aNav->AppendTabStop(new CTabStop(itsClearGraphButton));
   aNav->AppendTabStop(new CTabStop(itsReadFVSOutputButton));
   aNav->AppendTabStop(new CTabStop(itsGraphsButton));
   aNav->AppendTabStop(new CTabStop(itsCloseButton));

   Show();
   aNav->InitFocus();

   ClearGraph();

}



void GenerateGraphWindow::DoCommand(long theCommand,void* theData)
{
   switch (theCommand)
   {
      case 1:   // Write Graph
      {
         WriteGraph();
         break;
      }
      case 2:   // Clear Graphic
      {
         itsColorIndex = -1;
         ClearGraph();
         break;
      }
      case 3:   // Close
      {
         Close();
         break;
      }

      case 4:   // Graph Type List Button
      {
         if (!GraphIsEmpty())
         {
#if XVTWS==WIN32WS
            switch(xvt_dm_post_ask("Cancel","C~lear","Write",
#else
            switch(xvt_dm_post_ask("Cancel","Clear","Write",
#endif
                                   "Changing graph types requires "
                                   "clearing graph."))
            {
               case RESP_DEFAULT:    // Cancel
               {
                  itsGraphTypeListButton->SelectItem(itsCurrentGraphType-1);
                  break;
               }
               case RESP_2:          // Clear
               {
                  itsColorIndex = -1;
                  ClearGraph();
                  break;
               }
               case RESP_3:          // Write
               {
                  WriteGraph();
                  itsColorIndex = -1;
                  ClearGraph();
                  break;
               }
            }
         }
         else ClearGraph();
         break;
      }

      case 5:   // Next Case
      case 6:   // Next Variable
      {
         GraphVariableSubview * theLastGuy =
            (GraphVariableSubview *) itsGraphVariableSubviews.last();
         FVSVariable * itsFVSVariable = theLastGuy->itsFVSVariable;
         if (itsFVSVariable) itsFVSVariable = itsFVSVariable->
            GetNextVariable(itsFVSVariable, (theCommand == 5));
         if (itsFVSVariable) NewChoice(itsFVSVariable);
         break;
      }
      case 7:   // New Choice
      {
         NewChoice(NULL);
         break;
      }
      case 8:   // Delete variable
      {
         GraphVariableSubview * aVToRemove = (GraphVariableSubview *) theData;
         if (aVToRemove && itsGraphVariableSubviews.entries())
         {
            // if there is only one variable, or if this is a scatter
            // plot, then if there are 2 or less variables or if the
            // variable is the X-axis variable, then simply clear.

            if (itsGraphVariableSubviews.entries() == 1 ||
                (itsCurrentGraphType == GraphTypeScatterYOverX &&
                 (aVToRemove == itsGraphVariableSubviews.first() ||
                  itsGraphVariableSubviews.entries() == 2)))
            {
               itsColorIndex = -1;
               ClearGraph();
            }
            else
            {
               itsScrollingFrame->Hide();

               // move the rest of the variables up.

               GraphVariableSubview * aGV;
               GraphVariableSubview * passItOn = NULL;
               CRect newRect, saveRect;
               RWOrderedIterator nextSubview(itsGraphVariableSubviews);
               while (aGV = (GraphVariableSubview *) nextSubview())
               {
                  if (passItOn)
                  {
                     saveRect = aGV->GetFrame();
                     aGV->DoSize(newRect);
                     newRect = saveRect;
                     passItOn = aGV;
                  }
                  else if (aGV == aVToRemove)
                  {
                     passItOn = aVToRemove;
                     newRect  = passItOn->GetFrame();
                  }
               }

               if (!passItOn) break;

               // delete all of the subviews inside the variable to remove.

               CSubview * aSubview;
               RWOrderedIterator subviewIter (*aVToRemove->GetSubviews());
               while (aSubview = (CSubview *) subviewIter())
               {
                  aVToRemove->RemoveSubview(aSubview);
                  delete aSubview;
               }

               itsScrollingFrame->RemoveSubview(aVToRemove);
               itsGraphVariableSubviews.removeAndDestroy(aVToRemove);

               itsScrollingFrame->Show();

               // invalidate last frame so that it will redraw

               RCT aRct = RCT(itsScrollingFrame->GetClippedFrame());
               xvt_dwin_invalidate_rect(GetXVTWindow(), &aRct);

               DisplayGraph();
            }
         }
         break;
      }
      case VARIABLEChanged: // The variable picker set a different variable
      {
         DisplayGraph();
         break;
      }
      case FVSCaseListDecreased:
      {
         if (!GraphIsEmpty())
         {
            CWindow * curTop = G->GetDesktop()->GetFrontWindow();
            G->GetDesktop()->SetFrontWindow(this);
            switch(xvt_dm_post_ask("Write","Clear",NULL,
                                   "Do you want to write this graph before "
                                   "it is cleared?"))
            {
               case RESP_DEFAULT:    // Write
               {
                  WriteGraph();
                  ClearGraph();
                  break;
               }
               case RESP_2:          // Clear
               {
                  ClearGraph();
                  break;
               }
            }
            G->GetDesktop()->SetFrontWindow(curTop);
         }
         break;
      }
      case FVSCaseListIncreased:     // reset show/hide some controls.
      {
         if (GraphIsEmpty())
         {
            itsNextCaseButton->DoHide();
            itsNextVariableButton->DoHide();
            itsNewChoiceButton->DoHide();
         }
         else
         {
            if (itsCurrentGraphType == GraphTypeBarYOverCase ||
                itsCurrentGraphType == GraphTypeScatterYOverX ||
                theSpGlobals->theReportData->entries()<2)
                 itsNextCaseButton->DoHide();
            else itsNextCaseButton->DoShow();
            itsNextVariableButton->DoShow();
            itsNewChoiceButton->DoShow();
         }
      }
      default:
         CWwindow::DoCommand(theCommand, theData);
   }
}


BOOLEAN GenerateGraphWindow::GraphIsEmpty( void )
{
   if (itsCurrentGraphType == GraphTypeScatterYOverX)
   {
      if (itsGraphVariableSubviews.entries() < 2) return TRUE;
      GraphVariableSubview * theFirstGuy =
         (GraphVariableSubview *) itsGraphVariableSubviews.first();
      if (theFirstGuy->itsFVSVariable == NULL) return TRUE;
   }
   GraphVariableSubview * theLastGuy =
      (GraphVariableSubview *) itsGraphVariableSubviews.last();
   return (theLastGuy->itsFVSVariable == NULL &&
           itsGraphVariableSubviews.entries() <= 1);
}


void GenerateGraphWindow::SizeWindow(int theWidth, int theHeigth)
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

      CRect aRect = itsScrollingFrame->GetLocalFrame();
      aRect.Right(aRect.Right()-NScrollBar::NativeWidth()-4);
      aRect.Left(aRect.Right()-134);

      // move the buttons...

      RWOrderedIterator nextV(itsGraphVariableSubviews);
      GraphVariableSubview * aV;
      while (aV = (GraphVariableSubview *) nextV())
      {
         if (itsCurrentGraphType == GraphTypeScatterYOverX  &&
             aV == itsGraphVariableSubviews.first())
         {
            CRect anR = aV->GetFrame();
            anR.Top(itsScrollingFrame->GetLocalFrame().Bottom()-36);
            anR.Bottom(anR.Top()+32);
            int center = itsScrollingFrame->GetLocalFrame().Right()/2;
            anR.Right(center);
            anR.Left(center-134);
            aV->DoSize(anR);
         }
         else
         {
            aRect.Top(aV->GetFrame().Top());
            aRect.Bottom(aV->GetFrame().Bottom());
            aV->DoSize(aRect);
         }
      }

      itsScrollingFrame->DoDraw();

      DisplayGraph();
   }
}


BOOLEAN GenerateGraphWindow::Close( void )
{
   return CWwindow::Close();
}


void GenerateGraphWindow::ClearGraph( void )
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

   itsPlot = NULL;

   itsCurrentGraphType = itsGraphTypeListButton->GetSelectPosition()+1;

   itsGraphVariableSubviews.clear();

   itsScrollingFrame->IScroller(FALSE, TRUE, TRUE, 24, 240);

   itsNextCaseButton = new NButton(itsScrollingFrame,
                                       CRect(240,4,354,36),
                                       "Next Case");
   itsNextCaseButton ->SetCommand( 5);

   itsNextVariableButton = new NButton(itsScrollingFrame,
                                       CRect(122,4,236,36),
                                       "Next Variable");
   itsNextVariableButton->SetCommand( 6);

   itsNewChoiceButton = new NButton(itsScrollingFrame,
                                       CRect(4,4,118,36),
                                       "New Choice");
   itsNewChoiceButton->SetCommand( 7);

   NewChoice(NULL);
}


void GenerateGraphWindow::NewChoice(FVSVariable * theVariableToDisplay)
{

   GraphVariableSubview * mostRecent = NULL;

   if (itsGraphVariableSubviews.entries()) mostRecent =
      (GraphVariableSubview *) itsGraphVariableSubviews.last();

   CRect aRect;
   GraphVariableSubview * newestChoice;

   if (mostRecent)
   {
      aRect = mostRecent->GetFrame();
      aRect.Top   (aRect.Bottom()+4);
      aRect.Bottom(aRect.Top()+32);
   }
   else
   {
      if (itsCurrentGraphType == GraphTypeScatterYOverX)
      {
         aRect.Top(itsScrollingFrame->GetLocalFrame().Bottom()-36);
         aRect.Bottom(aRect.Top()+32);
         int center = itsScrollingFrame->GetLocalFrame().Right()/2;
         aRect.Right(center);
         aRect.Left(center-134);
         newestChoice = new GraphVariableSubview(this, itsScrollingFrame,
                                                 aRect, theVariableToDisplay);
         itsGraphVariableSubviews.insert(newestChoice);
      }
      aRect.Top(4);
      aRect.Bottom(aRect.Top()+32);
      aRect.Right(itsScrollingFrame->GetLocalFrame().Right()-
                  NScrollBar::NativeWidth()-4);
      aRect.Left(aRect.Right()-134);
   }

   newestChoice = new GraphVariableSubview(this, itsScrollingFrame,
                                           aRect, theVariableToDisplay);
   itsGraphVariableSubviews.insert(newestChoice);
   itsScrollingFrame->ShrinkToFit();

   // this causes a call to DisplayGraph.

   DoCommand(VARIABLEChanged, this);
}


COLOR GenerateGraphWindow::NewColor(CEnvironment * theEnv)
{
   COLOR aColor;
   itsColorIndex++;
   if (itsColorIndex > 10) itsColorIndex = 0;
   switch (itsColorIndex)
   {
      case 0:  aColor = COLOR_RED;     break;
      case 1:  aColor = COLOR_GREEN;   break;
      case 2:  aColor = COLOR_BLUE;    break;
      case 3:  aColor = COLOR_MAGENTA; break;
      case 4:  aColor = COLOR_BLACK;   break;
      case 5:  aColor = COLOR_DKGRAY;  break;
      case 6:  aColor = COLOR_GRAY;    break;
      case 7:  aColor = COLOR_YELLOW;  break;
      case 8:  aColor = COLOR_CYAN;    break;
      case 9:  aColor = COLOR_LTGRAY;  break;
      case 10: aColor = COLOR_WHITE;   break;
      default: aColor = COLOR_BLACK;   break;
   }
   if (aColor == theEnv->GetBackgroundColor()) return NewColor(theEnv);
   else                                        return aColor;
}


void GenerateGraphWindow::DisplayGraph( void )
{
   if (GraphIsEmpty())
   {
      RCT aRct = RCT(GetClippedFrame());
      xvt_dwin_invalidate_rect(GetXVTWindow(), &aRct);

      // make sure that the controls are correctly set.

      DoCommand(FVSCaseListIncreased,this);
      return;
   }

   // if the plotting subview is not empty, then delete it and then
   // recreate it and draw the graph.

   if (itsPlot)
   {
      RemoveSubview(itsPlot);
      delete itsPlot;
   }

   CRect theRegion = itsNewChoiceButton->GetFrame();
   theRegion.Top(theRegion.Bottom()+4);
   theRegion.Bottom(itsScrollingFrame->GetLocalFrame().Bottom()-4);
   if (itsCurrentGraphType == GraphTypeScatterYOverX)
      theRegion.Bottom(theRegion.Bottom()-34);

   GraphVariableSubview * mostRecent =
      (GraphVariableSubview *) itsGraphVariableSubviews.last();

   theRegion.Right(mostRecent->GetFrame().Left()-4);

   itsPlot = new CSubview (itsScrollingFrame, theRegion);

#ifdef AIXV4
   itsPlot->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,10),FALSE);
#else
   itsPlot->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,8),FALSE);
#endif

   int yLabelMargin = xvt_dwin_get_text_width
      (itsPlot->GetCWindow()->GetXVTWindow(), "1.0", -1) + 2;

   int ascentp, descent, lineHeight;
   xvt_dwin_get_font_metrics(itsPlot->GetCWindow()->GetXVTWindow(),
                             &lineHeight, &ascentp, &descent);
   lineHeight = lineHeight+ascentp+descent;

   CEnvironment anEnv(*itsPlot->GetEnvironment());
   anEnv.SetPenColor(COLOR_BLACK);
   anEnv.SetBrushPattern(PAT_HOLLOW);

   int i;

   // find important scaling information.  Note this code define the max
   // and min years, even if they are not used in the graph.

   // Note: The min value for a variable is defined to be zero unless the
   // real min value is less than zero.

   int maxYr = SpNEGMAX;
   int minYr = SpPOSMAX;
   float scaleYr, theScaleForX, theMinForX;
   int nBars = 0;
   int barWidth = 13;

   float maxY[NumberGraphScaleGroups];
   float minY[NumberGraphScaleGroups];
   float scaleY[NumberGraphScaleGroups];

   for (i=0; i<NumberGraphScaleGroups; i++)
   {
      maxY[i] = -FLT_MAX;
      minY[i] = 0.0;
   }

   RWOrderedIterator nextV(itsGraphVariableSubviews);
   GraphVariableSubview * aV;

   // if the graph type is a scatter graph, skip the first variable in
   // the scaling sequence.

   if (itsCurrentGraphType == GraphTypeScatterYOverX) nextV();
   while (aV = (GraphVariableSubview *) nextV())
   {
      if (aV->itsFVSVariable &&
          aV->itsFVSVariable->itsObservations.entries())
      {
         float obs = aV->itsFVSVariable->GetMaxF();
         if (aV->itsFVSVariable->itsScaleGroup != GraphScaleGroupUndefined)
            if (maxY[aV->itsFVSVariable->itsScaleGroup] < obs)
                maxY[aV->itsFVSVariable->itsScaleGroup] = obs;
         if (maxY[0] < obs) maxY[0] = obs;

         obs = aV->itsFVSVariable->GetMinF();
         if (aV->itsFVSVariable->itsScaleGroup != GraphScaleGroupUndefined)
            if (minY[aV->itsFVSVariable->itsScaleGroup] > obs)
                minY[aV->itsFVSVariable->itsScaleGroup] = obs;
         if (minY[0] > obs) minY[0] = obs;

         FVSObservation * anObs = (FVSObservation *) aV->itsFVSVariable->
            itsObservations.at(0);

         if (itsCurrentGraphType == GraphTypeLineOverYears ||
             itsCurrentGraphType == GraphTypePhaseOverYears)
         {
            int year  = anObs->GetObsIDI();
            if (minYr > year) minYr = year;
            anObs = (FVSObservation *) aV->itsFVSVariable->itsObservations.
               at(aV->itsFVSVariable->itsObservations.entries()-1);
            year  = anObs->GetObsIDI();
            if (maxYr < year) maxYr = year;
         }

         // don't bother testing for other graph types.  Compute this sum
         // even if it is not needed.

         else nBars += aV->itsFVSVariable->itsObservations.entries();
      }
   }

   // set the maxY for scale group GraphScaleGroupPercent to 100
   // and for GraphScaleGroupProportion to 1.

   if (maxY[GraphScaleGroupPercent] < 100.)
      maxY[GraphScaleGroupPercent] = 100.;
   if (maxY[GraphScaleGroupProportion] < 1.)
      maxY[GraphScaleGroupProportion] = 1.;

   // set up the graph area.

   int graphLeft = yLabelMargin+5;
   int graphRight = itsPlot->GetLocalFrame().Right()-1;
   int graphTop = lineHeight/2;

   int height = itsPlot->GetLocalFrame().Bottom();
   if (itsCurrentGraphType == GraphTypeScatterYOverX) height -= lineHeight*2;
   else                                               height -= lineHeight*3;

   int nIncs = 5;
   int byInc = 2;
   if (itsCurrentGraphType == GraphTypePhaseOverYears)
   {
      nIncs = int(maxY[0]);
      if (nIncs > 20) nIncs = 20;
      byInc = 1;
   }

   int increment = height/nIncs;
   height = increment*nIncs;

   int graphBottom = height+graphTop;

   // build the Y axis.

   RWOrdered aList;
   int place=0;
   char label[5];
   for (i=0; i<=(nIncs*byInc); i+=byInc)
   {
      if (itsCurrentGraphType == GraphTypePhaseOverYears)
           sprintf(label,"%3d", (nIncs*byInc)-i);
      else sprintf(label,"%2.1f", (float(nIncs*byInc)-float(i))*.1);

      new CText(itsPlot, CPoint(0,place), label);

      aList.insert(new CPointRWC(yLabelMargin,place+graphTop));
      aList.insert(new CPointRWC(yLabelMargin+3,place+graphTop));
      place += increment;
      if (i<(nIncs*byInc))
         aList.insert(new CPointRWC(yLabelMargin+3,place+graphTop));
   }
   CPolygon * aPolygon = new CPolygon(itsPlot,&aList);
   aList.clearAndDestroy();

   // set up the scaling of variables.  First do the X axis.

   if (itsCurrentGraphType == GraphTypeLineOverYears ||
       itsCurrentGraphType == GraphTypePhaseOverYears)
   {
      if (maxYr <= minYr)
      {
         maxYr = 10;
         minYr =  0;
      }
      scaleYr = float(graphRight-graphLeft)/float(maxYr-minYr);
   }
   else if (itsCurrentGraphType == GraphTypeScatterYOverX)
   {
      GraphVariableSubview * xV = (GraphVariableSubview *)
         itsGraphVariableSubviews.first();
      theMinForX = xV->itsFVSVariable->GetMinF();
      if (theMinForX > 0.0) theMinForX = 0.0;
      theScaleForX = xV->itsFVSVariable->GetMaxF() - theMinForX;
      if (theScaleForX > FLT_MIN)
         theScaleForX = float(graphRight-graphLeft)/theScaleForX;
      else theScaleForX = 0.;
   }

   // the assumption is that we have a bar chart.

   else
   {
      int nB = int(float((graphRight-graphLeft))/barWidth);
      if (nB < nBars)
      {
         barWidth = 5;
         nB = (graphRight-graphLeft)/barWidth;
         if (nB < nBars)
         {
            xvt_dm_post_message ("You desire %d bars but have room for "
                                 "%d bars.  Resize your window or specify "
                                 "fewer variables.",nBars,nB);
         }
      }
   }

   // scale the Y axis...

   float gheight = graphBottom-graphTop;
   for (i=0; i<NumberGraphScaleGroups; i++)
   {
      if (maxY[i] != -FLT_MAX)
      {
         if (maxY[i]-minY[i] > FLT_MIN) scaleY[i] = gheight/(maxY[i]-minY[i]);
         else scaleY[i] = -1.;
      }
      else scaleY[i] = 0.;
   }

   int currLeft = graphLeft+1;
   int groupLeft = currLeft;     // used in bar charts.
   int barLabelSwitch = 0;

   nextV.reset();
   if (itsCurrentGraphType == GraphTypeScatterYOverX) nextV();
   while (aV = (GraphVariableSubview *) nextV())
   {
      if (aV->itsFVSVariable)
      {
         float theScaleForY;
         float theMaxForY;
         float theMinForY;
         if (aV->itsFVSVariable->itsScaleGroup == GraphScaleGroupUndefined)
         {
            // if the scale group is undefined, scale the variable
            // globally for all variables that have the same name in
            // this graph...if the graph type is GraphTypePhaseOverYears
            // then all the variables are scaled globally.

            theMaxForY = -FLT_MAX;
            theMinForY =  0.0;
            RWOrderedIterator nextV2(itsGraphVariableSubviews);
            GraphVariableSubview * aV2;
            while (aV2 = (GraphVariableSubview *) nextV2())
            {
               if (aV2->itsFVSVariable &&
                   aV2->itsFVSVariable->itsScaleGroup ==
                   GraphScaleGroupUndefined)
               {
                  BOOLEAN sameVar = (aV == aV2) ||
                     (itsCurrentGraphType == GraphTypePhaseOverYears);

                  if (!sameVar)
                  {
                     size_t hasAp = aV ->itsFVSVariable->first(047);
                     if (hasAp == RW_NPOS)
                        sameVar = strcmp(aV ->itsFVSVariable->data(),
                                         aV2->itsFVSVariable->data()) == 0;
                     else
                        sameVar = strncmp(aV ->itsFVSVariable->data(),
                                          aV2->itsFVSVariable->data(),
                                          (int) hasAp) == 0;
                  }
                  if (sameVar)
                  {
                     if (aV2->itsFVSVariable->GetMinF() < theMinForY)
                        theMinForY = aV2->itsFVSVariable->GetMinF();
                     if (aV2->itsFVSVariable->GetMaxF() > theMaxForY)
                        theMaxForY = aV2->itsFVSVariable->GetMaxF();
                  }
               }
            }
            if (theMaxForY != -FLT_MAX)
            {
               theScaleForY = theMaxForY - theMinForY;
               if (theScaleForY > FLT_MIN)
                  theScaleForY = gheight/theScaleForY;
               else theScaleForY = -1.;
            }
            else theScaleForY = 0.;
         }
         else
         {
            theScaleForY = scaleY[aV->itsFVSVariable->itsScaleGroup];
            theMaxForY   = maxY  [aV->itsFVSVariable->itsScaleGroup];
         }

         switch (itsCurrentGraphType)
         {
            case GraphTypeLineOverYears:
            {
               RWOrderedIterator nextObs(aV->itsFVSVariable->itsObservations);
               FVSObservation * anObs;
               while (anObs = (FVSObservation *) nextObs())
               {
                  int y = graphBottom;
                  float obs = anObs->GetValueF();
                  if (theScaleForY > -1.)
                  {
                     obs = theMaxForY - obs;
                     obs = obs*theScaleForY;
                     y = int(obs) + graphTop;
                  }
                  else if (obs > 0.) y = graphTop;

                  float year  = anObs->GetObsIDI()-minYr;
                  year = scaleYr*year;
                  int x = int(year)+graphLeft;
                  aList.insert(new CPointRWC(x,y));
               }
               CPolygon * aPolygon = new CPolygon(itsPlot,&aList);
               if (aV->itsLegendColor == COLOR_INVALID)
               {
                  anEnv.SetPenColor(NewColor(&anEnv));
                  aV->DrawLegendLine(&anEnv);
               }
               else anEnv.SetPenColor(aV->itsLegendColor);
               aPolygon->SetEnvironment(anEnv);
               aList.clearAndDestroy();
               break;
            }
            case GraphTypeScatterYOverX:
            {
               if (aV->itsLegendColor == COLOR_INVALID)
               {
                  anEnv.SetPenColor(NewColor(&anEnv));
                  aV->DrawLegendPoints(&anEnv);
               }
               else anEnv.SetPenColor(aV->itsLegendColor);

               GraphVariableSubview * xV = (GraphVariableSubview *)
                  itsGraphVariableSubviews.first();

               RWOrderedIterator nextXObs(xV->itsFVSVariable->itsObservations);
               FVSObservation * anXObs;
               while (anXObs = (FVSObservation *) nextXObs())
               {
                  float obs = anXObs->GetValueF() - theMinForX;
                  obs = obs*theScaleForX;
                  int x = int(obs)+graphLeft;

                  RWOrderedIterator nextObs(aV->itsFVSVariable->
                                            itsObservations);
                  FVSObservation * anObs;
                  while (anObs = (FVSObservation *) nextObs())
                  {
                     if (strcmp(anXObs->itsObservationIdentification,
                                anObs-> itsObservationIdentification) == 0)
                     {
                        int y = graphBottom;
                        obs = anObs->GetValueF();
                        if (theScaleForY > -1.)
                        {
                           obs = theMaxForY - obs;
                           obs = obs*theScaleForY;
                           y = int(obs) + graphTop;
                        }
                        else if (obs > 0.) y = graphTop;
                        PlotPoint (itsPlot,x,y,&anEnv);
                     }
                  }
               }
               break;
            }
            case GraphTypeBarYOverObsID:
            case GraphTypeBarYOverCase:
            {
               // the default is for a bar chart.

               if (aV->itsLegendColor == COLOR_INVALID)
               {
                  anEnv.SetPenColor(NewColor(&anEnv));
                  aV->DrawLegendBar(&anEnv);
               }
               else anEnv.SetPenColor(aV->itsLegendColor);

               RWOrderedIterator nextObs(aV->itsFVSVariable->itsObservations);
               FVSObservation * anObs;
               FVSObservation * lastObs = NULL;
               while (anObs = (FVSObservation *) nextObs())
               {
                  if (lastObs &&
                      strcmp(lastObs->itsObservationIdentification.data(),
                               anObs->itsObservationIdentification.data())
                      == 0 &&
                      lastObs->GetValueF() == anObs->GetValueF()) continue;

                  lastObs = anObs;

                  // draw the bar.

                  int y = graphBottom;
                  float obs = anObs->GetValueF();
                  if (theScaleForY > -1.)
                  {
                     obs = theMaxForY - obs;
                     obs = obs*theScaleForY;
                     y = int(obs) + graphTop;
                  }
                  else if (obs > 0.) y = graphTop;

                  if (y != graphBottom)
                  {
                     CPoint points[4];
                     points[0].H(currLeft+1);
                     points[0].V(graphBottom);
                     points[1].H(currLeft+1);
                     points[1].V(y);
                     points[2].H(currLeft+barWidth-1);
                     points[2].V(y);
                     points[3].H(currLeft+barWidth-1);
                     points[3].V(graphBottom);

                     CPolygon * aPolygon = new CPolygon(itsPlot,points,4);
                     aPolygon->SetFilled(TRUE);
                     PAT_STYLE tPat = anEnv.GetBrushPattern();
                     anEnv.SetBrushPattern(PAT_SOLID);
                     anEnv.SetBrushColor(anEnv.GetPenColor());
                     aPolygon->SetEnvironment(anEnv);
                     anEnv.SetBrushPattern(tPat);
                  }

                  // write the label.

                  const char * lab = anObs->itsObservationIdentification.
                                            data();
                  int labLen = xvt_dwin_get_text_width
                     (itsPlot->GetCWindow()->GetXVTWindow(),
                      (char *) lab, -1);

                  int xLabelMargin = graphBottom+6;
                  if (++barLabelSwitch % 2 == 0) xLabelMargin += lineHeight;

                  int place=(barWidth-labLen)/2;
                  if (place > barWidth+barWidth-2) place=barWidth+barWidth-2;
                  place=currLeft+place;

                  new CText(itsPlot,CPoint(place,xLabelMargin),lab);
                  currLeft += barWidth;
               }
               break;
            }
            case GraphTypePhaseOverYears:
            {
               CPoint aStartingPoint;
               CPoint anEndingPoint;
               CLine * aLine = NULL;
               if (aV->itsLegendColor == COLOR_INVALID)
               {
                  anEnv.SetPenColor(NewColor(&anEnv));
                  aV->DrawLegendLine(&anEnv);
               }
               else anEnv.SetPenColor(aV->itsLegendColor);

               RWOrderedIterator nextObs(aV->itsFVSVariable->itsObservations);
               FVSObservation * anObs;
               FVSObservation * lastObs = NULL;
               while (anObs = (FVSObservation *) nextObs())
               {
                  if (lastObs)
                  {
                     float year,obs;
                     UNITS x,y;
                     if (!aLine)
                     {
                        y = graphBottom;
                        obs = lastObs->GetValueF();
                        if (theScaleForY > -1.)
                        {
                           obs = theMaxForY - obs;
                           obs = obs*theScaleForY;
                           y = int(obs) + graphTop;
                        }
                        else if (obs > 0.) y = graphTop;

                        year = lastObs->GetObsIDI()-minYr;
                        year = scaleYr*year;
                        x = int(year)+graphLeft;

                        aStartingPoint.V(y);
                        aStartingPoint.H(x);
                     }

                     year = anObs->GetObsIDI()-minYr;
                     year = scaleYr*year;
                     x = int(year)+graphLeft;

                     anEndingPoint.V(y);
                     anEndingPoint.H(x);

                     if (aLine)
                     {
                        aStartingPoint = aLine->GetStartPoint();
                        aLine->Size(aStartingPoint,anEndingPoint);
                     }
                     else
                     {
                        aLine = new CLine(itsPlot,aStartingPoint,
                                          anEndingPoint);
                        aLine->SetArrows(FALSE,TRUE);
                        aLine->SetEnvironment(anEnv);
                     }

                     if (lastObs->GetValueF() != anObs->GetValueF())
                     {
                        anEndingPoint.H(x);
                        aStartingPoint = anEndingPoint;
                        obs = anObs->GetValueF();
                        if (theScaleForY > -1.)
                        {
                           obs = theMaxForY - obs;
                           obs = obs*theScaleForY;
                           y = int(obs) + graphTop;
                        }
                        else if (obs > 0.) y = graphTop;
                        anEndingPoint.V(y);
                        aLine = new CLine(itsPlot,aStartingPoint,
                                          anEndingPoint);
                        aLine->SetArrows(FALSE,TRUE);
                        aLine->SetEnvironment(anEnv);
                        aLine = NULL;
                     }
                  }
                  lastObs = anObs;
               }
               break;
            }
            default: break;
         }
      }

      // draw the X-axis for bar charts.  Remember that we are in a
      // while loop over variables.

      if (itsCurrentGraphType == GraphTypeBarYOverObsID ||
          itsCurrentGraphType == GraphTypeBarYOverCase)
      {
         CPoint points[4];
         points[0].H(groupLeft); points[0].V(graphBottom+5);
         points[1].H(groupLeft); points[1].V(graphBottom+2);
         points[2].H(currLeft);  points[2].V(graphBottom+2);
         points[3].H(currLeft);  points[3].V(graphBottom+5);
         new CPolygon(itsPlot,points,4);
      }
   }

   // draw the X axis for line charts and scatter graphs...we are not
   // in a loop over variables.

   if (itsCurrentGraphType == GraphTypeLineOverYears ||
       itsCurrentGraphType == GraphTypePhaseOverYears)
   {
      increment = (graphRight-graphLeft)/5;
      int yearInc = (maxYr-minYr)/5;
      int yearLab = minYr;
      int xLabelMargin = graphBottom+2;
      place=graphLeft;
      for (i=0; i<6; i++)
      {
         sprintf(label,"%d", yearLab);
         new CText(itsPlot,CPoint(place-graphLeft+1,
                                  xLabelMargin+5), label);
         yearLab += yearInc;
         aList.insert(new CPointRWC(place,xLabelMargin+3));
         aList.insert(new CPointRWC(place,xLabelMargin));
         place += increment;
         if (i<5) aList.insert(new CPointRWC(place,xLabelMargin));
      }
      aPolygon = new CPolygon(itsPlot,&aList);
      aList.clearAndDestroy();
      new CText(itsPlot,
                CPoint(itsPlot->GetLocalFrame().Right()/2,
                       itsPlot->GetLocalFrame().Bottom()-lineHeight-1),
                "Year");
   }
   else if (itsCurrentGraphType == GraphTypeScatterYOverX)
   {
      increment = (graphRight-graphLeft)/5;
      int xLabelMargin = graphBottom+2;
      place=graphLeft;
      for (i=0; i<12; i+=2)
      {
         sprintf(label,"%2.1f", (float) i*.1);
         new CText(itsPlot, CPoint(place-graphLeft+8,
                                   xLabelMargin+5), label);

         aList.insert(new CPointRWC(place,xLabelMargin+3));
         aList.insert(new CPointRWC(place,xLabelMargin));
         place += increment;
         if (i<10) aList.insert(new CPointRWC(place,xLabelMargin));
      }
      aPolygon = new CPolygon(itsPlot,&aList);
      aList.clearAndDestroy();
   }

   // make sure that the controls are correctly set.

   DoCommand(FVSCaseListIncreased,this);

   // cause the graphic area to be redrawn

   RCT aRct = RCT(GetClippedFrame());
   xvt_dwin_invalidate_rect(GetXVTWindow(), &aRct);
}


void GenerateGraphWindow::WriteGraph( void )
{
   if (GraphIsEmpty()) return;
   xvt_dm_post_message("Writing graphs is not yet implemented.");
}



void PlotPoint(CSubview * theSubview, int x, int y, CEnvironment * theEnv)
{
   CPoint points[4];

   points[0].H(x-1); points[0].V(y+1);
   points[1].H(x-1); points[1].V(y-1);
   points[2].H(x+1); points[2].V(y-1);
   points[3].H(x+1); points[3].V(y+1);

   CPolygon * aPolygon = new CPolygon(theSubview,points,4);
   PAT_STYLE tPat = theEnv->GetBrushPattern();
   theEnv->SetBrushColor(theEnv->GetPenColor());
   theEnv->SetBrushPattern(PAT_SOLID);
   aPolygon->SetEnvironment(*theEnv);
   theEnv->SetBrushPattern(tPat);
   aPolygon->SetFilled(TRUE);
}



GraphVariableSubview::GraphVariableSubview
   (GenerateGraphWindow  * theGraphWindow,
    MyCScroller          * theScrollingFrame,
    const CRect          & theRegion,
    FVSVariable          * theVariable)
                      :CSubview(theScrollingFrame, theRegion),
                       itsGraphWindow(theGraphWindow),
                       itsFVSVariable(theVariable),
                       itsLegendColor(COLOR_INVALID),
                       itsVariablePicker(NULL)
{
   CRect aRect = theRegion;
   aRect.Top   (0);
   aRect.Right (aRect.Right()-aRect.Left());
   aRect.Left  (20);
   aRect.Bottom(32);
   CStringRW aTitle;
   if (itsFVSVariable) aTitle = itsFVSVariable->BuildNameString(aTitle);
   else aTitle = "Pick Variable";

   itsVariablesButton = new NButton (this,aRect,aTitle,0L);
   itsVariablesButton->SetCommand(1);
}




GraphVariableSubview::~GraphVariableSubview( void )
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



void GraphVariableSubview::DoCommand(long theCommand,void * theData)
{
   switch (theCommand)
   {
      case 1:   // Variable Button Pushed.
      {
         if (itsVariablePicker)
            G->GetDesktop()->SetFrontWindow((CWindow*) itsVariablePicker);
         else itsVariablePicker = new PickVariableWindow
                 (this, 2, itsGraphWindow->itsCurrentGraphType);
         break;
      }
      case 2:   // Variable Picker Set a new variable.
      {
         CStringRW aTitle;
         itsVariablesButton->SetTitle(itsFVSVariable->
                                      BuildNameString(aTitle));
         itsGraphWindow->DoCommand(VARIABLEChanged, this);
         break;
      }
      default:
      {
         CSubview::DoCommand(theCommand,theData);
         break;
      }
   }
}


void GraphVariableSubview::DrawLegendLine(CEnvironment * theEnv)
{
   CPoint points[4];

   int top    = 1;
   int bottom = GetLocalFrame().Bottom()-1;
   int left   = 1;
   int right  = 16;

   points[0].H(left);       points[0].V(bottom);
   points[1].H(left+8);     points[1].V(top);
   points[2].H(left+8);     points[2].V(bottom);
   points[3].H(right);      points[3].V(top);

   CPolygon * aPolygon = new CPolygon(this,points,4);
   aPolygon->SetEnvironment(*theEnv);
   itsLegendColor=theEnv->GetPenColor();

   DoDraw();
}


void GraphVariableSubview::DrawLegendBar (CEnvironment * theEnv)
{
   CPoint points[4];

   int top    = 1;
   int bottom = GetLocalFrame().Bottom()-1;
   int left   = 8;
   int right  = 16;

   points[0].H(left);    points[0].V(bottom);
   points[1].H(left);    points[1].V(top);
   points[2].H(right);   points[2].V(top);
   points[3].H(right);   points[3].V(bottom);

   CPolygon * aPolygon = new CPolygon(this,points,4);
   PAT_STYLE tPat = theEnv->GetBrushPattern();
   theEnv->SetBrushPattern(PAT_SOLID);
   theEnv->SetBrushColor(theEnv->GetPenColor());
   aPolygon->SetEnvironment(*theEnv);
   theEnv->SetBrushPattern(tPat);
   aPolygon->SetFilled(TRUE);
   itsLegendColor=theEnv->GetPenColor();
   DoDraw();
}

void GraphVariableSubview::DrawLegendPoints(CEnvironment * theEnv)
{
   CPoint points[4];

   int bottom = GetLocalFrame().Bottom()-8;
   PlotPoint(this,3,bottom-2,theEnv);
   PlotPoint(this,6,11,theEnv);
   PlotPoint(this,16,15,theEnv);
   itsLegendColor=theEnv->GetPenColor();
   points[0].H(0);    points[0].V(bottom);
   points[1].H(0);    points[1].V(4);
   points[2].H(19);   points[2].V(4);
   points[3].H(19);   points[3].V(bottom);
   CPolygon * aPolygon = new CPolygon(this,points,4);
   theEnv->SetPenColor(COLOR_BLACK);
   aPolygon->SetEnvironment(*theEnv);
   aPolygon->SetFilled(TRUE);
   theEnv->SetPenColor(itsLegendColor);
   DoDraw();
}

