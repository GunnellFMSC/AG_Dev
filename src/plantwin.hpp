/********************************************************************

   File Name:     plantwin.hpp
   Author:        nlc
   Date:          01/15/98

   Class:         PlantNaturalWin
   Inheritance:   CWwindow -> CMgtClassWindow -> PlantNaturalWin



   Contents:      PlantNaturalWin()
                  DoCommand
                  InitializeWithParms
                  BuildCParmSettingString
                  ResetTheComponentSettings
                  BuildKeywordString

*******************************************************************/

#ifndef PlantWin_h
#define PlantWin_h

#include "PwrDef.h"
#include "appdef.hpp"
#include "cmgtclas.hpp"

class NButton;
class CRadioGroup;

class NEditControl;
class CText;
class NListButton;
class MyNLineText;

class PKeyData;
class Component;
class MSText;
class MyCScrolleres;

class PlantNaturalSubview;

class PlantNaturalWin : public CMgtClassWindow
{
   public:

      PlantNaturalWin(const CStringRW&                   = NULLString,
                      int        fullEstab               = -1,
                      Component * theComponent           = NULL,
                      Component * theConditionComponent  = NULL);

      virtual void    DoCommand(long theCommand,void* theData = NULL);
      virtual BOOLEAN Close ( void );
      void            InitializeWithParms(void);
      CStringRW       BuildCParmSettingString(void);
      void            ResetTheComponentSettings(void);
      CStringRW       BuildKeywordString(void);
      void            SetFocusOnNextElement(void * theData);

   private:

      MSText           * itsMSText;

      NButton          * itsAnotherPlantNaturalButton;

      int                itsTopOfFreeSpace;
      int                itsFullEstabInUse;
      int                itsHasSproutingSpecies;
      int                itsInGrowthOnDefault;

      CRadioGroup      * itsSproutingRadioGroup;
      long               itsSproutingOn;
      long               itsSproutingOff;
      NRadioButton     * itsSproutingOnButton;
      NRadioButton     * itsSproutingOffButton;

      CRadioGroup      * itsInGrowthRadioGroup;
      long               itsInGrowthOn;
      long               itsInGrowthOff;
      NRadioButton     * itsInGrowthOnButton;
      NRadioButton     * itsInGrowthOffButton;

      CRadioGroup      * itsStockAdjRadioGroup;
      long               itsStockAdjZero;
      long               itsStockAdjSet;
      NRadioButton     * itsStockAdjZeroButton;
      NRadioButton     * itsStockAdjSetButton;
      MyNLineText      * itsStockAdjLineText;

      MyNLineText      * itsPrepYearsLineText;
      MyNLineText      * itsBurnPrepLineText;
      MyNLineText      * itsMechPrepLineText;

      MyNLineText      * itsPlantYearsLineText;

      RWOrdered          itsPlantNaturalSubviews;
      RWOrdered          itsSpeciesList;
      RWOrdered          itsShadeCodeList;

      CStringRW          itsInitialSettings;

   friend class PlantNaturalSubview;

};


class PlantNaturalSubview : public CSubview
{
   public:

      PlantNaturalSubview(PlantNaturalWin * thePlantNaturalWin,
                          MyCScroller     * theScrollingFrame,
                          const CRect     & theRegion);
      virtual void DoCommand(long theCommand,void* theData = NULL);
      void         Init(const char * theParms);
      float        NumberTrees( void );
      CStringRW *  GetParms( void );
      void         GetKeyword(CStringRW * theKeyword);

   private:

      CRadioGroup      * itsPlantNaturalRadioGroup;
      long               itsPlant;
      long               itsNatural;
      CStringRW          itsParms;
      NRadioButton     * itsPlantButton;
      NRadioButton     * itsNaturalButton;
      NListButton      * itsSpeciesListButton;
      MyNLineText      * itsTreesAcreNLineText;
      MyNLineText      * itsSurvivalNLineText;
      MyNLineText      * itsAveAgeNLineText;
      MyNLineText      * itsAveHeightNLineText;
      NListButton      * itsShadeCodeListButton;
      PlantNaturalWin  * itsPlantNaturalWin;

   friend class PlantNaturalWin;

};

#endif
