/********************************************************************

   File Name:     seed.hpp
   Author:        kfd, nlc rewrite 09/20/2006
   Date:          08/25/94


   Class:         SeedTreeWin
   Inheritance:   CWwindow -> CMgtClassWindow -> SeedTreeWin


   Contents:      SeedTreeWin()
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

#ifndef Seedtree_H
#define Seedtree_H

#include "PwrDef.h"
#include "appdef.hpp"
#include "spparms.hpp"
#include "cmgtclas.hpp"

class NButton;
class CRadioGroup;
class NRadioButton;
class ScheduleByObject;
class BoxSliderObject;
class NCheckBox;
class NLineText;
class NEditControl;
class CText;

class PKeyData;
class Component;


class SeedTreeWin : public CMgtClassWindow
{
   public:

      SeedTreeWin(const CStringRW&           = NULLString,
                  Component * theComponent   = NULL,
                  Component * theConditionComponent   = NULL);
      virtual void       DoCommand(long theCommand,void* theData=NULL);
      virtual CStringRW  BuildKeywordString(void);

      void         InitializeWithParms(void);
      CStringRW    BuildCParmSettingString(void);
      void         ResetTheComponentSettings(const char * theSettings);

  private:

      BoxSliderObject *  smallestTree;
      NCheckBox  *       prepCutCheckBox;

      CRadioGroup *      itsDensityGroup;
      NRadioButton *     itsFirstButton;
      NRadioButton *     itsSecondButton;
      long               itsFirst;
      long               itsSecond;
      BoxSliderObject *  residualBasal;
      BoxSliderObject *  residualPercent;

      CText *            itsSeedCutText1;
      CText *            itsSeedCutText2;
      CText *            itsSeedCutText3;
      NEditControl *     seedCutYears;
      BoxSliderObject *  residualTrees;
      NCheckBox  *       removalCutCheckBox;
      NEditControl *     removalCutYears;
      BoxSliderObject *  removalCutResidual;
      BoxSliderObject *  smallestTreeRC;

      CStringRW      itsInitialSettings;
      BOOLEAN        noExistingCondition;
};
#endif //Seedtree_H
