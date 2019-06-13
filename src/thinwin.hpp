/********************************************************************

   File Name:     thinwin.hpp
   Author:        kfd
   Date:          08/25/94

   Class:         ThinFromWindow

   Inheritance:   CWwindow -> CMgtClassWindow -> ThinFromWin



   Contents:      ThinFromWin()
                  DoCommand
                  DoMenuCommand
                  Close
                  InitializeWithParms
                  BuildCParmSettingString
                  ResetTheComponentSettings
                  BuildKeywordStringAbove
                  BuildKeywordStringBelow


*******************************************************************/

#ifndef thinwin_h
#define thinwin_h

#include "PwrDef.h"
#include "appdef.hpp"
#include "cmgtclas.hpp"
#include "spparms.hpp"

class NButton;
class CRadioGroup;
class NRadioButton;
class ScheduleByObject;
class BoxSliderObject;
class NEditControl;

class ScheduleByObject;
class SchByCond;
class PKeyData;


class ThinFromWin : public CMgtClassWindow
{
  public:

      ThinFromWin(const CStringRW&           = NULLString,
                  int   aboveOrBelow         = 0,
                  Component * theComponent   = NULL,
                  Component * theConditionComponent   = NULL);

      virtual void DoCommand(long theCommand,void* theData=NULL);

      virtual void DoMenuCommand(MENU_TAG theMenuItem,
                                 BOOLEAN isShiftkey,
                                 BOOLEAN isControlKey);

      void         InitializeWithParms(void);
      CStringRW    BuildCParmSettingString(void);
      void         ResetTheComponentSettings(const char * theSettings);
      CStringRW    BuildKeywordString(long aboveOrBelow);
      CStringRW    BuildSpacingFromTPA(const char * theSettings);
      CStringRW    BuildTPAFromSpacing(const char * theSettings);

  private:

      CRadioGroup      * itsDensityRadioGroup;
      long               itsDensityFirst;
      long               itsDensitySecond;
      long               itsDensityThird;

      BoxSliderObject  * itsTreesPerAcre;
      BoxSliderObject  * itsTreeSpacing ;
      BoxSliderObject  * itsBasalArea   ;
      BoxSliderObject  * itsPercent     ;

      CRadioGroup      * itsPercentRadioGroup;
      long               itsPercentFirst;
      long               itsPercentSecond;
      NRadioButton     * itsPercentFirstButton;
      NRadioButton     * itsPercentSecondButton;

      BoxSliderObject  * itsPropLeft;

      BoxSliderObject  * itsDiameterLower;
      BoxSliderObject  * itsDiameterUpper;
      BoxSliderObject  * itsHeightLower;
      BoxSliderObject  * itsHeightUpper;

      long               itsAboveOrBelow;

      CStringRW          itsInitialSettings;
      BOOLEAN            noExistingCondition;

};
#endif //thinwin_H
