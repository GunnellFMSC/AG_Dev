/********************************************************************

   File Name:     reportwn.hpp
   Author:        nlc
   Date:          10/96

   Class:         GenerateReportWindow
   Inheritance:   CWindow->CWwindow

   Class:         ColumnSubview
   Inheritance:   ColumnSubview->CSubview

   Purpose:  This class provides for generating reports based on FVS
             outputs.  It's chief helper data is described in
             reportd.hpp
 *                                                                  *
 ********************************************************************/

#ifndef GenerateReportWindow_H
#define GenerateReportWindow_H

class NButton;
class NListButton;
class CNavigator;
class CText;
class NEditControl;
class MyCScroller;
class RWOrdered;
class FVSCase;
class FVSReport;
class FVSVariable;
class PickVariableWindow;
class GenerateGraphWindow;

#include "cwwindow.hpp"
#include CSubview_i

class ColumnSubview;

class GenerateReportWindow : public CWwindow
{
   public:

      GenerateReportWindow( CDocument * theDocument );
      void DisplayTable( void );
      virtual BOOLEAN Close( void );
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual void DoMenuCommand(MENU_TAG theMenuItem,
                                 BOOLEAN isShiftkey,
                                 BOOLEAN isControlKey);
      virtual void SizeWindow(int theWidth, int theHeigth);

      BOOLEAN ReportIsEmpty( void );
      void ClearReport( void );
      void WriteReport( void );
      void NewColumn(FVSVariable * theVariableToDisplay);

   private:

      int             itsLineHeight;
      NEditControl  * itsReportTitle;
      NListButton   * itsReportTypeListButton;

      CNavigator    * itsNavigator;
      ColumnSubview * itsFirstColumnSubview;
      MyCScroller   * itsScrollingFrame;
      RWOrdered       itsColumnSubviews;
      NButton       * itsNextCaseButton;
      NButton       * itsNextVariableButton;
      NButton       * itsNewColumnButton;

      NButton       * itsWriteReportButton;
      NButton       * itsClearReportButton;
      NButton       * itsReadFVSOutputButton;
      NButton       * itsReportsButton;
      NButton       * itsCloseButton;

   friend class ColumnSubview;
   friend class PickVariableWindow;

};


class ColumnSubview : public CSubview
{

   public:

      ColumnSubview(GenerateReportWindow * theReportWindow,
                    MyCScroller          * theScrollingFrame,
                    const CRect&           theRegion,
                    const char           * theColumnTitle,
                    FVSVariable          * theVariable);
      virtual ~ColumnSubview(void);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      void DisplayObservations(RWOrdered * columnOne);


   private:

      GenerateReportWindow * itsReportWindow;
      CText                * itsColumnTitle;
      FVSVariable          * itsFVSVariable;
      NButton              * itsVariablesButton;
      RWOrdered              itsObservationCTexts;
      PickVariableWindow   * itsVariablePicker;

   friend class GenerateReportWindow;
   friend class PickVariableWindow;

};



class PickVariableWindow : public CWwindow
{
   public:

      PickVariableWindow (CSubview * theRelatedSubview,
                          int        theSubviewType = 1,
                          int        theType        = 1);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual BOOLEAN Close( void );

   private:

      CSubview      * itsRelatedSubview;
      int             itsSubviewType;
      int             itsType;
      NListButton   * itsCasesListButton;
      NListButton   * itsReportsListButton;
      RWOrdered       itsReportsList;
      RWOrdered       itsVariablesList;
      NListBox      * itsVariablesListBox;
      NButton       * itsOkButton;
      NButton       * itsApplyButton;
      NButton       * itsDeleteButton;
      NButton       * itsCloseButton;
      CText         * itsUnitsCText;
      CText         * itsNObsCText;
      CText         * itsNObsMaxCText;
      CText         * itsNObsMinCText;

      void LoadReportList   (FVSCase   * currCase);
      void LoadVariableList (FVSReport * currReport);
      void DisplayStatisticsAboutVariable(FVSVariable * aV);

   friend class GenerateReportWindow;
   friend class GenerateGraphWindow;
   friend class ColumnSubview;
   friend class GraphVariableSubview;

};

#endif

