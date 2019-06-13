/*********************************************************************

   File Name:    editslf2.cpp
   Author:       nlc
   Date:         04/16/97

   see editslf.hpp for notes.  also note that editslf2.cpp is a direct
   extension of editslf.cpp source file.  It exists because editslf
   needed to be split to support Win16 link limitations.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include NText_i
#include NButton_i
#include NEditControl_i
#include CDesktop_i
#include CMenuBar_i
#include CDocument_i
#include CScroller_i
#include NLineText_i
#include CStringCollection_i
#include CRadioGroup_i
#include NListButton_i
#include CWindow_i
#include CStringRWC_i

#include "suppdefs.hpp"
#include "spprefer.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "spparms.hpp"
#include "dirsept.hpp"

#include "variants.hpp"

#include "spglobal.hpp"

#include "editslf.hpp"
#include "mylinetx.hpp"

#include "supphelp.hpp"  // for help #defines
extern XVT_HELP_INFO helpInfo;


void EditSLFWin::LoadCodes( void )
{
   CStringRW forVariant = itsVariantsNLineText->GetText();
   size_t blank = forVariant.first(' ');
   if (blank != RW_NPOS) (RWCString &) forVariant = forVariant.remove(blank);

   if (itsCurrentCodesVariant.length() &&
       itsCurrentCodesVariant == forVariant) return;

   itsLocMSTextPointer = LoadCodesHelper(&itsLocList,
                                         "Forests_",
                                         &forVariant,
                                         itsLocNListButton);
   itsHabMSTextPointer = LoadCodesHelper(&itsHabList,
                                         "HabPa_",
                                         &forVariant,
                                         itsHabNListButton);
   itsSpeciesMSTextPointer = LoadCodesHelper(&itsSpeciesList,
                                             "species_",
                                             &forVariant,
                                             itsSpeciesNListButton);
}


MSText *  EditSLFWin::LoadCodesHelper(RWOrdered   * theList,
                                      const char  * theName,
                                      CStringRW   * forVariant,
                                      NListButton * theListButton)
{

   theList->clearAndDestroy();

   CStringRWC neededMSText = theName;
   neededMSText += *forVariant;
   MSText * theMSText = (MSText *) theSpGlobals->
      theParmsData->find(&neededMSText);
   if (theMSText)
   {
      RWOrdered * thePKeys = theMSText->GetThePKeys();
      RWOrderedIterator pkeyIter(*thePKeys);
      PKeyData * aPkey;
      while (aPkey = (PKeyData *) pkeyIter())
      {
         CStringRWC * newCode = new CStringRWC((CStringRWC &) *aPkey);
         *newCode += " = ";
         *newCode += aPkey->pString;
         theList->insert(newCode);
      }
   }

   if (theList->entries())
   {
      theListButton->IListButton(*theList);
      theListButton->DoShow();
   }
   else theListButton->DoHide();

   return theMSText;
}



void EditSLFWin::ImportOldDirFile( const char * fileName )
{

/*********************************************************************

   The following is the description of the old dir file formats for type
   "A" and "C" Records.

     Item                  Columns    Data type    Format

   Record type               1         Alpha         A1
   Stand ID                  2-17      Alpha         A16
   Grouping Code            18-23      Alpha         A6
   Slope (percent)          24-26      Integer       I3
   Aspect (degrees)         27-29      Integer       I3
   Elevation (feet)         30-34      Integer       I5
   Site Species             35-36      Integer       A3
   Site Index               37-39      Integer       I3
   Inventory Year           40-43      Integer       I4
   Number of Plots          44-46      Integer       I3
   Number of NS Plots       47-49      Integer       I3
   Stockable Percent        50-52      Integer       I3
   Latitude                 53-54      Integer       I2
   Basal Area Factor        55-59      Real          F5.1
   Breakpoint DBH           60-64      Real          F5.1
   Maximum SDI              65-68      Integer       I4
   Maximum Basal Area       69-71      Integer       I3
   Stand Origin Date        72-75      Integer       I4
   Habitat Type or
      Plant Association     76-85      Alpha         A10
   Data Filename            86-115     Alpha         A30
   Stand Size (weight)      116-121    Real          F6.0
   Inv. of Fixed Plot Size  122-126    Real          F5.2
   Diameter Growth          127-128    Integer       I2
     Measurement Length
   Height growth            129-130    Integer       I2
     Measurement Length
   Mortality                131-132    Integer       I2
     Measurement Length

   "B" Record Format:

     Item                  Columns    Data type    Format

   Card Type                      1    Real          A1
   Stand ID                    2-17    Alpha         A16
   Keyword Filename           18-33    Alpha         A16

   Note these rules:

   Suppose considers these record type A and C records to be equivalent.

   All data are considered character data and leading and trailing blanks
   are striped for all fields.

   If a record is discovered, an attempt is made to match its data
   with an existing SLFrecord.  If non is found, then a new one is made.
   This rule applies to all record types (type A, B, and C).  Note that
   the file should not contain a record A and C with the same stand ID and
   only one B record should be present for any one A or C record.  But,
   a type B record could exist before a corresponding type A or C and
   the file need not be in sorted order.

   The keyword file name on the B records is considered an AddFile by
   Suppose.

*********************************************************************/


   itsSLFRecords.clearAndDestroy();
   itsCurrentRecord = 0;

   if (strlen(fileName))
   {
      FILE * DIRFile = fopen (fileName,"r");
      if (DIRFile)
      {
         char * record= new char[201];
         if (DIRFile != NULL)
         {
            while (fgets(record, 200, DIRFile))
            {
               if (strlen(record) <= 2) continue;

               CStringRW inRec = record;
               CStringRW sID   = inRec(1,16);
               sID = sID.strip(RWCString::both);

               RWOrderedIterator anSLFRecIter(itsSLFRecords);
               SLFRecord * aRec;
               while (aRec = (SLFRecord *) anSLFRecIter())
               {
                  if ((RWCString &) *aRec == (RWCString &) sID) break;
               }
               if (!aRec)
               {
                  if (itsTemplate)
                  {
                     aRec = new SLFRecord(itsTemplate);
                     (RWCString &) *aRec = (RWCString &) sID;
                  }
                  else aRec = new SLFRecord(sID);
                  itsSLFRecords.insert(aRec);
               }

               switch ((int) *record)
               {
                  case 'A':
                  case 'C':
                  {
                     if (aRec->groupCodes.isNull())
                        AssignValue(&aRec->groupCodes,inRec,18,23);
                     else
                     {
                        CStringRW tS;
                        AssignValue(&tS,inRec,18,23);
                        if (!tS.isNull() && !aRec->groupCodes.isNull())
                        {
                           tS+=" ";
                           aRec->groupCodes.prepend(tS);
                        }
                     }
                     AssignValue(&aRec->slope,inRec,24,26);
                     AssignValue(&aRec->aspect,inRec,27,29);
                     AssignValue(&aRec->elev,inRec,30,34);
                     AssignValue(&aRec->siteSpecies,inRec,35,36);
                     AssignValue(&aRec->siteIndex,inRec,37,39);
                     AssignValue(&aRec->invYear,inRec,40,43);
                     AssignValue(&aRec->nPlots,inRec,44,46);
                     AssignValue(&aRec->nNonStock,inRec,47,49);
                     AssignValue(&aRec->lat,inRec,53,54);
                     AssignValue(&aRec->baf,inRec,55,59);
                     AssignValue(&aRec->breakpoint,inRec,60,64);
                     AssignValue(&aRec->SDIMax,inRec,65,68);
                     AssignValue(&aRec->BAMax,inRec,69,71);
                     AssignValue(&aRec->originYear,inRec,72,75);
                     AssignValue(&aRec->hab,inRec,76,85);
                     AssignValue(&aRec->FVSTreeData,inRec,86,115);
                     AssignValue(&aRec->sampWt,inRec,116,121);
                     AssignValue(&aRec->fixed,inRec,122,126);
                     AssignValue(&aRec->DGperiod,inRec,127,128);
                     AssignValue(&aRec->HGperiod,inRec,129,130);
                     AssignValue(&aRec->mortPeriod,inRec,131,132);
                     break;
                  }
                  case 'B':
                  {
                     if (aRec->addFiles.isNull())
                        AssignValue(&aRec->addFiles,inRec,18,23);
                     else
                     {
                        CStringRW tS;
                        AssignValue(&tS,inRec,18,33);
                        if (!tS.isNull() && !aRec->addFiles.isNull())
                        {
                           tS+=" ";
                           aRec->addFiles.prepend(tS);
                        }
                     }
                     break;
                  }
                  default: break;
               }
            }
         }
         delete []record;
         fclose(DIRFile);
      }
      else xvt_dm_post_warning ("Suppose can not open file: %s",
                                fileName);
   }
}


void EditSLFWin::AssignValue(CStringRW * c)
{
   char * ptr;
   if ((ptr = strtok(NULL," \n")) && *ptr != '@') *c = ptr;
}


void EditSLFWin::AssignValue(CStringRW * c, CStringRW & inRec,
                             int n1, int n2)
{
   if (c && inRec.length() >= n1 && n2 >= n1)
   {
      CStringRW toAssign = inRec(n1-1,n2-n1+1);
      toAssign = toAssign.strip(RWCString::both);
      size_t i=0;
      while (i<toAssign.length())
      {
         if (iscntrl(toAssign(i))) toAssign.remove(i);
         i++;
      }
      if (!toAssign.isNull()) *c = toAssign;
   }
}



void EditSLFWin::FocusOnNextObject(void * curObj)
{
   if (curObj == NULL) return;
   else if (curObj == itsCurrStandIDNLineText)
                      itsFVSTreeFileNLineText->Activate();
   else if (curObj == itsFVSTreeFileNLineText)
                      itsVariantsNLineText->Activate();
   else if (curObj == itsVariantsNLineText)
                      itsInvYearNLineText->Activate();
   else if (curObj == itsInvYearNLineText)
                      itsStateNLineText->Activate();
   else if (curObj == itsStateNLineText)
                      itsCountyNLineText->Activate();
   else if (curObj == itsCountyNLineText)
                      itsModelTypeNLineText->Activate();
   else if (curObj == itsModelTypeNLineText)
                      itsPhysRegionNLineText->Activate();
   else if (curObj == itsPhysRegionNLineText)
                      itsForestTypeNLineText->Activate();
   else if (curObj == itsForestTypeNLineText)
                      itsLatNLineText->Activate();
   else if (curObj == itsLatNLineText)
                      itsLongitudeNLineText->Activate();
   else if (curObj == itsLongitudeNLineText)
                      itsLocNLineText->Activate();
   else if (curObj == itsLocNLineText)
                      itsHabNLineText->Activate();
   else if (curObj == itsHabNLineText)
                      itsOriginYearNLineText->Activate();
   else if (curObj == itsOriginYearNLineText)
                      itsAspectNLineText->Activate();
   else if (curObj == itsAspectNLineText)
                      itsSlopeNLineText->Activate();
   else if (curObj == itsSlopeNLineText)
                      itsElevNLineText->Activate();
   else if (curObj == itsElevNLineText)
                      itsBAFNLineText->Activate();
   else if (curObj == itsBAFNLineText)
                      itsFixedNLineText->Activate();
   else if (curObj == itsFixedNLineText)
                      itsBreakpointNLineText->Activate();
   else if (curObj == itsBreakpointNLineText)
                      itsNPlotsNLineText->Activate();
   else if (curObj == itsNPlotsNLineText)
                      itsNNonStockNLineText->Activate();
   else if (curObj == itsNNonStockNLineText)
                      itsSampWtNLineText->Activate();
   else if (curObj == itsSampWtNLineText)
                      itsPNonStNLineText->Activate();
   else if (curObj == itsPNonStNLineText)
                      itsDGTransCodeNLineText->Activate();
   else if (curObj == itsDGTransCodeNLineText)
                      itsDGperiodNLineText->Activate();
   else if (curObj == itsDGperiodNLineText)
                      itsHGTransCodeNLineText->Activate();
   else if (curObj == itsHGTransCodeNLineText)
                      itsHGperiodNLineText->Activate();
   else if (curObj == itsHGperiodNLineText)
                      itsMortperiodNLineText->Activate();
   else if (curObj == itsMortperiodNLineText)
                      itsBAMaxNLineText->Activate();
   else if (curObj == itsBAMaxNLineText)
                      itsSDIMaxNLineText->Activate();
   else if (curObj == itsSDIMaxNLineText)
                      itsSiteSpeciesNLineText->Activate();
   else if (curObj == itsSiteSpeciesNLineText)
                      itsSiteIndexNLineText->Activate();
   else if (curObj == itsSiteIndexNLineText)
                      itsGroupCodeNLineText->Activate();
   else if (curObj == itsGroupCodeNLineText)
                      itsAddFileNLineText->Activate();
   else if (curObj == itsAddFileNLineText)
                      itsNewButton->Activate();
}
