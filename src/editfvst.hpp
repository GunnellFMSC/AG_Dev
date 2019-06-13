/********************************************************************

   File Name:    editfvst.hpp
   Author:       nlc
   Date:         08/25/97

   Purpose:  This class is used to display, edit, or both, the
             records in a stand list file.

   Contents:

     class EditFVSTreeDataWin
                                                                    *
 ********************************************************************/

#ifndef EditFVSTreeDataWin_H
#define EditFVSTreeDataWin_H

#include "cwwindow.hpp"
#include <rw/ordcltn.h>

class NButton;
class NListButton;
class CText;
class NText;
class NLineText;
class CDocument;
class MyNLineText;
class RWOrdered;
class MyCScroller;
class CRadioGroup;
class MSText;

class EditFVSTreeDataWin : public CWwindow
{
   public:

      EditFVSTreeDataWin ( CDocument * theDocument );
      virtual ~EditFVSTreeDataWin ( void );
      virtual void DoCommand(long theCommand, void* theData = NULL);

   private:

      MyCScroller  * itsScrollingWindow;
      NButton      * itsOkButton;
      NButton      * itsCancelButton;
      NButton      * itsImportButton;
      NButton      * itsTemplateButton;
      NButton      * itsSaveButton;
      NButton      * itsSaveAsButton;

      MyNLineText  * itsFVSTDFNameNLineText;
      NButton      * itsBrowseButton;

      NText        * itsRecordCountNText;
      MyNLineText  * itsRecordNumberNLineText;
      NButton      * itsPreviousButton;
      NButton      * itsNextButton;
      NButton      * itsNewButton;
      NButton      * itsDeleteButton;
      NButton      * itsDuplicateButton;

      CText        * itsColumnRuler;
      CText        * itsCurrentRecord;
      size_t         itsCurrentRecordNumber;
      RWOrdered      itsFVSTDRecords;
      CStringRW    * itsTemplate;

      RWOrdered      itsVariantsList;
      NListButton  * itsVariantsNListButton;
      MSText       * itsVariantsMSTextPointer;

      MyNLineText  * itsPlotID;
      MyNLineText  * itsTreeID;
      MyNLineText  * itsTreeCount;

      RWOrdered      itsTreeHistoryCodesList;
      NListButton  * itsTreeHistoryCodesNListButton;
      MyNLineText  * itsTreeHistory;

      RWOrdered      itsSpeciesList;
      NListButton  * itsSpeciesNListButton;
      MyNLineText  * itsSpecies;

      MyNLineText  * itsDBH;
      MyNLineText  * itsDBHGrowth;
      MyNLineText  * itsHeight;
      MyNLineText  * itsHeightToTopkill;
      MyNLineText  * itsHeightGrowth;
      MyNLineText  * itsCrownRatio;
      NButton      * itsHelpButton;
      MyNLineText  * itsDamageCode1;
      MyNLineText  * itsDamageSeverity1;
      MyNLineText  * itsDamageCode2;
      MyNLineText  * itsDamageSeverity2;
      MyNLineText  * itsDamageCode3;
      MyNLineText  * itsDamageSeverity3;
      MyNLineText  * itsTreeValueClass;
      MyNLineText  * itsCutOrLeave;

      MyNLineText  * itsSlope;
      MyNLineText  * itsAspect;

      RWOrdered      itsHabCodeList;
      NListButton  * itsHabCodeNListButton;
      MyNLineText  * itsHabCode;

      MyNLineText  * itsTopoCode;
      MyNLineText  * itsSitePrepCode;
      MyNLineText  * itsTreeAge;

      void StoreCurrent( void );
      int  WriteFile( void );
      void ReadFile( void );
      void DisplayRecord( void );
      void FocusOnNextObject( void * curObj = NULL);
      void LoadCodes(RWOrdered   * theList,
                     const char  * theName,
                     PKeyData    * theVariant,
                     NListButton * theListButton);

      void StashGetString (MyNLineText * obj, CStringRW & rec,
                           size_t start, size_t len, int left=0);
};

#endif









