/********************************************************************

   File Name:     sheltwin.hpp
   Author:        kfd, nlc rewrite 09/20/2006
   Date:          08/25/94

   Class:         ShelterwoodWin
   Inheritance:   CWwindow -> CMgtClassWindow -> ShelterwoodWin



   Contents:      ShelterwoodWin()
                  DoCommand
                  InitializeWithParms
                  BuildCParmSettingString
                  ResetTheComponentSettings
                  BuildKeywordString

   Contents description: routine specific

   InitializeWithParms
         return - none
         input  - none
         purpose: initialize window objects with parameters from the parms file

   BuildCParmSettingString
         return - CString of window settings
         input  - none
         purpose: convert window setting into string

   ResetTheComponentSettings
         return - none
         input  - const char * with screen settings
         purpose: for editing an existing component or reset
                  to the original factory settings.  Uses either
                  window settings from the component CParm or the
                  factory settings itsInitialSettings

   BuildKeywordString
         return - CStringRW with component keywords
         input  - none
         purpose: convert window entries into component keywords

*******************************************************************/

#ifndef sheltwin_h
#define sheltwin_h

#include "PwrDef.h"
#include "appdef.hpp"
#include "cmgtclas.hpp"

class NButton;
class CRadioGroup;
class ScheduleByObject;
class NRadioButton;
class BoxSliderObject;
class NCheckBox;
class NEditControl;
class CText;

class PKeyData;
class Component;

//------------------------------------------------------------------

class ShelterwoodWin : public CMgtClassWindow
{
   public:

      ShelterwoodWin(const CStringRW&                    = NULLString,
                     Component * theComponent            = NULL,
                     Component * theConditionComponent   = NULL);

      virtual void DoCommand(long theCommand,void* theData = NULL);

      void         InitializeWithParms(void);
      CStringRW    BuildCParmSettingString(void);
      void         ResetTheComponentSettings(const char * theSettings);
      CStringRW    BuildKeywordString(void);


   private:

      BoxSliderObject *  smallestTree;
      NCheckBox        * itsPrepCutCheckBox;

      CRadioGroup      * itsPrepCutRadioGroup;
      long               itsPrepCutFirst;
      long               itsPrepCutSecond;
      NRadioButton     * itsPrepCutFirstButton;
      NRadioButton     * itsPrepCutSecondButton;
      BoxSliderObject  * itsResidualBasal;
      BoxSliderObject  * itsResidualPercent;

      CText            * itsShelterCutText1;
      CText            * itsShelterCutText2;
      CText            * itsShelterCutText3;
      NEditControl     * itsShelterCutYears;

      CRadioGroup      * itsShelterCutRadioGroup;
      long               itsShelterCutFirst;
      long               itsShelterCutSecond;
      long               itsShelterCutThird;

      BoxSliderObject  * itsShelterBasal;
      BoxSliderObject  * itsShelterTrees;
      BoxSliderObject  * itsShelterPercent;

      NCheckBox        * itsRemovalCutCheckBox;
      NEditControl     * itsRemovalCutYears;
      BoxSliderObject  * removalCutResidual;
      BoxSliderObject  * smallestTreeRC;

      CStringRW          itsInitialSettings;
      BOOLEAN            noExistingCondition;
};
#endif //sheltwin_H
