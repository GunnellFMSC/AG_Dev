/********************************************************************

   File Name:     cmgtclas.hpp
   Author:        kfd   &  nlc
   Date:          02/95    12/95

   Class:         CMgtClassWindow
   Inheritance:   CWindow->CWwindow->CMgtClassWindow


   Purpose:       The class was designed for the management window screens.
                  It has several functions which eliminate the need to
                  duplicate code in the objects which inherit from it.

   Contents:

     CMgtClassWindow  - constructor

     BuildMgtConditionWindow - builds a condition window

     SaveTheComponent - update or build and save a component

     CheckForNULLMgtCondition - checks to see if condition was entered; if not

     Close - Does some housekeeping on close.

*******************************************************************/

#ifndef CMgtClassWindow_H
#define CMgtClassWindow_H

#include "PwrDef.h"
#include "appdef.hpp"
#include "suppdefs.hpp"
#include "cwwindow.hpp"

#include CStringRW_i

class ScheduleByObject;
class SchByCond;
class Component;
class NEditControl;
class MyCScroller;
class NButton;
class CNavigator;

class CMgtClassWindow: public CWwindow
{
  public:

      CMgtClassWindow(CDocument *theDocument,
                      const CRect& theRegion,
                      Component * theComponent   = NULL,
                      Component * theConditionComponent
                                                 = NULL,
                      const CStringRW& theTitle  = NULLString,
                      CStringRW * theExtension   = NULL,
                      long theWindowAttributes   = WSF_NONE,
                      WIN_TYPE theWindowType     = W_DOC,
                      int theMenuBarId           = TASK_MENUBAR);

      virtual BOOLEAN Close( void );

      virtual void DoCommand(long theCommand,void* theData);

      virtual void DoMenuCommand(MENU_TAG theMenuItem,
                                 BOOLEAN isShiftkey,
                                 BOOLEAN isControlKey);

      virtual CStringRW BuildKeywordString(void);


      SchByCond * BuildMgtConditionWindow(SchByCond * itsSchByCondWindow,
                                          CWindow * itsParent,
                                          const char * nameString,
                                          const char * defaultString,
                                          CStringRW&   conditionTitle,
                                          Component  * conditionComponent);

      void SaveTheComponent(int usingFreeForm,
                            const char * screenName,
                            CStringRW  & keywordLine,
                            CStringRW  & settingsLine,
                            const char * extension = "base");

      virtual void MakeStandardControls(const CStringRW & name,
                                        long okCommand     = 1,
                                        long resetCommand  = 2,
                                        long cancelCommand = 3,
                                        long freeFormCommand=105);

      void CheckForNULLMgtCondition( void );
      virtual void SizeWindow(int theWidth, int theHeigth);

      Component * FindConditionComponent( void );

      CNavigator       * itsNavigator;
      CStringRW          itsExtension;
      Component        * itsComponent;
      Component        * itsConditionComponent;
      SchByCond        * itsSchByCondWindow;
      int                noExistingCondition;
      long               itsOKCommand;
      long               itsResetCommand;
      long               itsCancelCommand;
      long               itsFreeFormCommand;
      ScheduleByObject * scheduleBy;
      NEditControl     * itsComponentName;
      MyCScroller      * itsScrollingFrame;
      NButton          * itsOKButton;
      NButton          * itsResetButton;
      NButton          * itsFreeFormButton;
      NButton          * itsCancelButton;
      BOOLEAN            closeCalledByCloseCancel;



};
#endif //CMgtClassWindow_H

