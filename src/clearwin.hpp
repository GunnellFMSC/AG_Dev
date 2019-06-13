/********************************************************************

   File Name:     clearwin.hpp
   Author:        kfd
   Date:          08/25/94

   Class:         ClearcutWin
   Inheritance:   CWwindow -> CMgtClassWindow -> ClearcutWin


   Contents:      (inherited)
      ClearcutWin - constructor
      DoCommand

                  (window specific)
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

      InitializeWithParms
         return - none
         input  - none
         purpose: initialize window objects with parameters from
                  the parms file

*******************************************************************/

#ifndef ClearcutWindows_H
#define ClearcWindows_H

#include "PwrDef.h"
#include "appdef.hpp"
#include "cmgtclas.hpp"

class CRadioGroup;
class ScheduleByObject;
class BoxSliderObject;
class NListButton;
class NEditControl;

class SchByCond;
class Component;

class ClearcutWin : public CMgtClassWindow
{
   public:

      ClearcutWin(const CStringRW&           = NULLString,
                  Component * theComponent   = NULL,
                  Component * theConditionComponent   = NULL);

      virtual void      DoCommand(long theCommand,void* theData=NULL);
      virtual CStringRW BuildKeywordString(void);

      CStringRW    BuildCParmSettingString(void);
      void         ResetTheComponentSettings(const char * theSettings);
      void         InitializeWithParms(void);

  private:

      BoxSliderObject *   smallestTree;
      BoxSliderObject *   largeTree;
      BoxSliderObject *   legacyTree;
      NListButton *       percentile;
      CRadioGroup *       itsLargeGroup;
      long                itsFirstButton;
      long                itsSecondButton;


      CStringRW           itsInitialSettings;
      BOOLEAN             noExistingCondition;

};
#endif  //ClearcutWindows_H
