/********************************************************************

   File Name:     selectsi.hpp
   Author:        kfd & nlc
   Date:          10/18/94 and 04/96 and 02/97

   Class:         SelectSimStandWin
   Inheritance:   CWindow -> SelectSimStandWin

   Purpose:       Allow the user to select simulation stands from
                  available stands and add them to the simulation.

*******************************************************************/

#ifndef Selectsim_h
#define Selectsim_h

#include "PwrDef.h"
#include "appdef.hpp"
#include CWindow_i

class NCheckBox;
class NButton;
class CRadioGroup;
class NRadioButton;
class NGroupBox;
class NListBox;
class NLineText;
class NText;
class RWOrdered;
class CStringRW;
class AllLocationData;
class RWCRegexp;
class CNavigator;
class MyNLineText;

class SelectSimStandWin : public CWindow
{
   public:

      SelectSimStandWin(CDocument * theDocument,
                        FILE_SPEC * theLocFilePointer,
                        FILE      * locFile);

      virtual BOOLEAN Close(void);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      void SetCountString( void );

   private:

      void BareGround( void );
      void SpFillGroupList( void );
      void SpFillLocationList( void );
      void SpFillStandList( void );
      void SpAcceptStandList( void );
      void SpResetAddStandButton( void );
      void SortRWOrdered(RWOrdered * list);
      int  FindInRWOrderedSetOfCStringRWC(RWOrdered * list,
                                          const char * c = NULL,
                                          const RWCRegexp * re = NULL,
                                          size_t start = 0);

      CNavigator   * itsNavigator;
      CRadioGroup  * itsSelectionRadioGroup;
      long           itsSelectionByLocation;
      long           itsSelectionByGroup;
      NRadioButton * itsSelectionByLocationButton;
      NRadioButton * itsSelectionByGroupButton;

      NListBox     * itsLocationListBox;
      NListBox     * itsGroupListBox;
      NListBox     * itsStandListBox;

      NText        * itsAvailableNText;

      NText        * itsCountNText;
      CStringRW    * itsCountTextString;

      NGroupBox    * itsStandSelectionGroupBox;
      CRadioGroup  * itsStandSelectionRadioGroup;
      long           itsSelInAny;
      long           itsSelInEvery;
      NRadioButton * itsSelInAnyButton;
      NRadioButton * itsSelInEveryButton;

      NGroupBox    * itsAddFileGroupBox;
      CRadioGroup  * itsAddFileRadioGroup;
      long           itsAddFileYes;
      long           itsAddFileNo;
      NRadioButton * itsAddFileYesButton;
      NRadioButton * itsAddFileNoButton;

      NCheckBox    * itsAddFileCheckBox;

	  MyNLineText  * itsStandIDText;
      NButton      * itsCloseButton;
      NButton      * itsDeleteSButton;
      NButton      * itsBareGroundButton;
      NButton      * itsAddStandsButton;
      NButton      * itsAllStandsButton;

      RWOrdered    * itsLocList;

	  RWOrdered    * itsGroupList;
      RWOrdered    * itsStandList;
	  //ALS
      //RWSortedVector    * itsStandList;


      FILE_SPEC    * itsLocFilePointer;

      AllLocationData * itsLocationData;

};

#endif
