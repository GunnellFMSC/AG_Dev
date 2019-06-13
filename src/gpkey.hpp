/********************************************************************

   File Name:    gpkey.hpp
   Author:       nlc
   Date:         12/15/94

   Purpose:  This class provides for support of FVS keywords that
             are generated using the suppose parameters file.

 *                                                                  *
 ********************************************************************/

#ifndef GPKeyword_H
#define GPKeyword_H

class Component;
class NButton;
class NText;
class NEditControl;
class MyCScroller;
class PKeyData;
class RWOrdered;
class SchByCond;
class MSText;
class CNavigator;

#include "cwwindow.hpp"
#include CStringRW_i

class GPKeyword : public CWwindow
{
  public:

    GPKeyword(const CStringRW & theTitle             = NULLString,
              MSText          * keywordMSTextPointer = NULL,
              const CStringRW & theExtension         = NULLString,
              Component       * currentComponent     = NULL);

    virtual ~GPKeyword( void );
    virtual BOOLEAN Close( void );
    virtual void DoCommand(long theCommand,void* theData=NULL);
    virtual void DoMenuCommand(MENU_TAG theMenuItem,
                               BOOLEAN isShiftkey,
                               BOOLEAN isControlKey);
    virtual void SizeWindow(int theWidth, int theHeigth);

  private:

      int BuildTheComponent( int standardAnswerForm );

      CNavigator   * itsNavigator;
      NButton      * okButton;
      NButton      * useParmsButton;
      NButton      * resetButton;
      NButton      * cancelButton;
      NEditControl * componentName;
      int            startPoint;

      Component    * theResultingComponent;
      Component    * theConditionComponent;
      Component    * theComponentBeingEdited;
      MSText       * keywordMSText;
      RWOrdered    * keywordPKeySet;
      int          * theFieldTypes;
      void        ** theAnswerPtrs;
      int            numberOfFieldTypes;
      char         * theKeywords;

      MyCScroller  * theScrollingFrame;
      SchByCond    * itsSchByCondWindow;

      CStringRW      itsInitialSettings;
      CStringRW      itsExtension;
};

#endif
