/********************************************************************

   File Name:    schcond.hpp
   Author:       nlc
   Date:         12/23/94

   Purpose:  This class provides for support of FVS conditions that
             are generated using the suppose parameters file.

   Contents:

     class SchByCond
          ~SchByCond
           DoCommand
           DoMenuCommand
           Close
           ISchByCond        (private)
           ClearScroller     (private)
           LoadScroller      (private)
           BuildTheComponent (private)

 ********************************************************************/

#ifndef SchByCond_H
#define SchByCond_H


class Component;
class NButton;
class NEditControl;
class NListButton;
class NListBox;
class CText;
class MyCScroller;
class PKeyData;
class RWOrdered;
class MSText;
class CWindow;

#include "cwwindow.hpp"
#include CStringRW_i

class SchByCond : public CWwindow
{
  public:

    SchByCond(CWindow * theParent, int callBackNumber,
              const char * defaultCondition,
              const CStringRW & thetitle,
              Component * currentComponent = NULL,
              Component * dependentComponent = NULL);


    virtual ~SchByCond( void );
    virtual BOOLEAN Close( void );
    virtual void DoCommand(long theCommand,void* theData=NULL);
    virtual void DoMenuCommand(MENU_TAG theMenuItem,
                              BOOLEAN isShiftkey,
                              BOOLEAN isControlKey);

  private:

      NButton      * okButton;
      NButton      * resetButton;
      NButton      * cancelButton;
      NButton      * useExistingButton;
      NButton      * freeFormButton;
      NEditControl * componentName;
      NEditControl * waitingPeriod;
      CText        * waitingPeriodText;
      NListButton  * libTypeListButton;
      NListButton  * libraryListButton;
      MyCScroller  * theScrollingWindow;

      CStringRW      itsInitialSettings;
      CStringRW      itsOriginalTitle;

      RWOrdered      parmsLibList;
      RWOrdered      usrLibList;
      RWOrdered      libTypeList;



      RWOrdered    * existingConditionList;
      NListBox     * existingConditionListBox;
      CText        * existingConditionText;
      int            existingConditionInUse;

      int            startPoint;

      Component    * theResultingComponent;
      Component    * theComponentBeingEdited;
      Component    * theDependentComponent;
      MSText       * conditionMSTextPointer;
      RWOrdered    * conditionPKeyDataCollection;
      int            numberConditionPKeys;
      int          * theFieldTypes;
      void        ** theAnswerPtrs;
      int            numberOfFieldTypes;

      typedef PKeyData  * ptrToPKeyData;

      ptrToPKeyData * listOfConditionsPKeyData;
      int             numberConditionsInPtrList;

      const char   * theDefaultCondition;
      int            IamBeingClosedCallBack;
      CWindow      * myParentIs;
      BOOLEAN        closeCalledByCloseCancel;

      void ISchByCond(const char * defaultCondition);
      void ClearScroller( void );
      void LoadScroller( void );
      void BuildTheComponent(int freeForm=FALSE );
      void BuildIfKeyword (CStringRW * keywords);
      void LoadUsrLibList( void );

      void HideCondition( void );
      void ShowCondition( void );
      void HideExisting ( void );
      void ShowExisting ( void );

};

#endif
