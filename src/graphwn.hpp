/********************************************************************

   File Name:     graphwn.hpp
   Author:        nlc
   Date:          05/97

   Class:         GenerateGraphWindow
   Inheritance:   CWindow->CWwindow

   Purpose:  This class provides for generating simple graphics based
             on FVS outputs.  It's chief helper data is described in
             reportd.hpp
 *                                                                  *
 ********************************************************************/

#ifndef GenerateGraphWindow_H
#define GenerateGraphWindow_H

class NButton;
class NListButton;
class CText;
class NEditControl;
class MyCScroller;
class CNavigator;
class RWOrdered;
class FVSCase;
class FVSReport;
class FVSVariable;
class PickVariableWindow;

#include "cwwindow.hpp"
#include CSubview_i

class GenerateGraphWindow : public CWwindow
{
   public:

      GenerateGraphWindow( CDocument * theDocument );
      virtual BOOLEAN Close( void );
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual void SizeWindow(int theWidth, int theHeigth);

      BOOLEAN GraphIsEmpty( void );
      void ClearGraph( void );

   private:

      int             itsLineHeight;
      int             itsCurrentGraphType;
      NEditControl  * itsGraphTitle;
      NListButton   * itsGraphTypeListButton;

      CNavigator    * itsNavigator;
      MyCScroller   * itsScrollingFrame;
      NButton       * itsNextCaseButton;
      NButton       * itsNextVariableButton;
      NButton       * itsNewChoiceButton;
      RWOrdered       itsGraphVariableSubviews;
      NButton       * itsWriteGraphButton;
      NButton       * itsClearGraphButton;
      NButton       * itsReadFVSOutputButton;
      NButton       * itsGraphsButton;
      NButton       * itsCloseButton;
      CSubview      * itsPlot;
      int             itsColorIndex;

      void DisplayGraph( void );
      void WriteGraph( void );
      void NewChoice(FVSVariable * theVariableToDisplay);
      COLOR NewColor(CEnvironment * theEnv);

   friend class PickVariableWindow;
   friend class GraphVariableSubview;

};

void PlotPoint(CSubview * theSubview, int x, int y, CEnvironment * theEnv);

class GraphVariableSubview : public CSubview
{

   public:

      GraphVariableSubview (GenerateGraphWindow  * theGraphWindow,
                            MyCScroller          * theScrollingFrame,
                            const CRect          & theRegion,
                            FVSVariable          * theVariable);
      virtual ~GraphVariableSubview(void);
      virtual void DoCommand(long theCommand,void* theData=NULL);

   private:

      GenerateGraphWindow  * itsGraphWindow;
      FVSVariable          * itsFVSVariable;
      NButton              * itsVariablesButton;
      PickVariableWindow   * itsVariablePicker;
      COLOR                  itsLegendColor;

      void DrawLegendLine  (CEnvironment * theEnv);
      void DrawLegendBar   (CEnvironment * theEnv);
      void DrawLegendPoints(CEnvironment * theEnv);

   friend class GenerateGraphWindow;
   friend class PickVariableWindow;

};



#endif

