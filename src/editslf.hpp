/********************************************************************

   File Name:    editslf.hpp
   Author:       nlc
   Date:         04/16/97

   Purpose:  This class is used to display, edit, or both, the
             records in a stand list file.

   Contents:

     class EditSLFWin
                                                                    *
 ********************************************************************/

#ifndef EditSLFWin_H
#define EditSLFWin_H

#include "cwwindow.hpp"
#include <rw/ordcltn.h>

class NButton;
class NListButton;
class NText;
class NLineText;
class CDocument;
class MyNLineText;
class RWOrdered;
class MyCScroller;
class CRadioGroup;
class MSText;
class SLFRecord;

class EditSLFWin : public CWwindow
{
   public:

      EditSLFWin ( CDocument * theDocument );
      virtual ~EditSLFWin ( void );
      virtual void DoCommand(long theCommand, void* theData = NULL);

   private:

      MyCScroller  * itsScrollingWindow;
      NButton      * itsOkButton;
      NButton      * itsCancelButton;
      NButton      * itsImportButton;
      NButton      * itsTemplateButton;
      NButton      * itsSaveButton;
      NButton      * itsSaveAsButton;
      SLFRecord    * itsTemplate;

      MyNLineText  * itsSLFNameNLineText;
      NButton      * itsBrowseButton;

      NText        * itsRecordCountNText;
      MyNLineText  * itsRecordNumberNLineText;
      NButton      * itsPreviousButton;
      NButton      * itsNextButton;
      NButton      * itsNewButton;
      NButton      * itsDeleteButton;
      NButton      * itsDuplicateButton;
      NButton      * itsFindButton;
      MyNLineText  * itsFindNLineText;
      MyNLineText  * itsCurrStandIDNLineText;
      size_t         itsCurrentRecord;
      RWOrdered      itsSLFRecords;

      MyNLineText  * itsFVSTreeFileNLineText;
      NButton      * itsFVSTreeFileBrowseButton;

      CRadioGroup  * itsPointDataRadioGroup;
      long           itsWithPointData;
      long           itsNoPointData;
      RWOrdered      itsVariantsList;
      NListButton  * itsVariantsNListButton;
      MyNLineText  * itsVariantsNLineText;
      MSText       * itsVariantsMSTextPointer;
      MyNLineText  * itsInvYearNLineText;
      MyNLineText  * itsLatNLineText;
      MyNLineText  * itsLongitudeNLineText;
      MyNLineText  * itsStateNLineText;
      MyNLineText  * itsCountyNLineText;
      MyNLineText  * itsModelTypeNLineText;
      MyNLineText  * itsPhysRegionNLineText;
      MyNLineText  * itsForestTypeNLineText;
      RWOrdered      itsLocList;
      NListButton  * itsLocNListButton;
      MyNLineText  * itsLocNLineText;
      MSText       * itsLocMSTextPointer;
      RWOrdered      itsHabList;
      NListButton  * itsHabNListButton;
      MyNLineText  * itsHabNLineText;
      MSText       * itsHabMSTextPointer;
      MyNLineText  * itsOriginYearNLineText;
      MyNLineText  * itsAspectNLineText;
      MyNLineText  * itsSlopeNLineText;
      MyNLineText  * itsElevNLineText;
      MyNLineText  * itsBAFNLineText;
      MyNLineText  * itsFixedNLineText;
      MyNLineText  * itsBreakpointNLineText;
      MyNLineText  * itsNPlotsNLineText;
      MyNLineText  * itsNNonStockNLineText;
      MyNLineText  * itsSampWtNLineText;
      MyNLineText  * itsPNonStNLineText;
      RWOrdered      itsTransList;
      NListButton  * itsDGTransCodeNListButton;
      MyNLineText  * itsDGTransCodeNLineText;
      MyNLineText  * itsDGperiodNLineText;
      NListButton  * itsHGTransCodeNListButton;
      MyNLineText  * itsHGTransCodeNLineText;
      MyNLineText  * itsHGperiodNLineText;
      MyNLineText  * itsMortperiodNLineText;
      MyNLineText  * itsBAMaxNLineText;
      MyNLineText  * itsSDIMaxNLineText;
      RWOrdered      itsSpeciesList;
      NListButton  * itsSpeciesNListButton;
      MSText       * itsSpeciesMSTextPointer;
      MyNLineText  * itsSiteSpeciesNLineText;
      MyNLineText  * itsSiteIndexNLineText;

      MyNLineText  * itsGroupCodeNLineText;
      MyNLineText  * itsAddFileNLineText;
      NButton      * itsAddFileBrowseButton;
      MyNLineText  * itsStandCNNLineText;
	  MyNLineText  * itsPVRefCodeNLineText;

      CStringRW      itsCurrentCodesVariant;

      void StoreCurrent( void );
      int  WriteFile( void );
      void ReadFile( void );
      void DisplayRecord( void );
      CStringRW & Disp(CStringRW & c);
      void AssignValue(CStringRW * c);
      void AssignValue(CStringRW * c, CStringRW & inRec, int n1, int n2);
      void LoadCodes( void);
      MSText * LoadCodesHelper(RWOrdered   * theList,
                               const char  * theName,
                               CStringRW   * forVariant,
                               NListButton * theListButton);

      void ImportOldDirFile( const char * fileName );
      void FocusOnNextObject( void * curObj = NULL);
};



class SLFRecord : public CStringRWC
{

   RWDECLARE_COLLECTABLE(SLFRecord)

   public:

      SLFRecord  ( char * record);
      SLFRecord  ( CStringRW & name);
      SLFRecord  ( SLFRecord * c);   // copy constructor
      SLFRecord  ( void );
      void WriteRecord (FILE * SLFFile);
      virtual RWBoolean isEqual(const RWCollectable * t) const;

   private:

      // Record type A

      CStringRW FVSTreeData;
      CStringRW samplePointDataFlag;
      CStringRW variants;

      // Record type B

      CStringRW invYear;
      CStringRW lat;
      CStringRW longitude;
      CStringRW loc;
      CStringRW hab;
      CStringRW originYear;
      CStringRW aspect;
      CStringRW slope;
      CStringRW elev;
      CStringRW baf;
      CStringRW fixed;
      CStringRW breakpoint;
      CStringRW nPlots;
      CStringRW nNonStock;
      CStringRW sampWt;
      CStringRW pNonSt;
      CStringRW DGTransCode;
      CStringRW DGperiod;
      CStringRW HGTransCode;
      CStringRW HGperiod;
      CStringRW mortPeriod;
      CStringRW BAMax;
      CStringRW SDIMax;
      CStringRW siteSpecies;
      CStringRW siteIndex;
      CStringRW modelType;
      CStringRW physRegion;
      CStringRW forestType;
      CStringRW state;
      CStringRW county;
      CStringRW pvRefCode;

      // Record type C

      CStringRW groupCodes;

      // Record type D

      CStringRW addFiles;

      // Record type E

      CStringRW standCN;

      const char * OutAt (CStringRW& theString);

   friend EditSLFWin;

};


#endif









