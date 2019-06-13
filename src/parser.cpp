/********************************************************************

  File Name:     parser.cpp
  Author:        nlc
  Date:          07/27/98

  see parser.hpp for notes.

 ********************************************************************/


#include "PwrDef.h"
#include "appdef.hpp"

#include <rw/bintree.h>
#include <rw/ordcltn.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include "reportd.hpp"
#include "spglobal.hpp"
#include "sendupdt.hpp"
#include "spfunc2.hpp"
#include "suppdefs.hpp"
#include "dirsept.hpp"
#include "readfvsw.hpp"
#include "parser.hpp"
#include "readfire.hpp"
#include "rdtreels.hpp"

FVSParser::FVSParser( void )
{
   itsOldCase = NULL;
   itsCaseDefineUsingKeywordTable    = new CaseDefineUsingKeywordTable();
   itsParseSummaryOutputTable        = new ParseSummaryOutputTable();
   itsParseComputeVariables          = new ParseComputeVariables();
   itsParseFirePotentialFlameReport  = new ParseFirePotentialFlameReport();
   itsParseFireAllFuelsReport        = new ParseFireAllFuelsReport();
   itsPPEOutputFlag = 0;
}




FVSParser::~FVSParser( void )
{
   delete itsCaseDefineUsingKeywordTable;
   delete itsParseSummaryOutputTable;
   delete itsParseComputeVariables;
   delete itsParseFirePotentialFlameReport;
   delete itsParseFireAllFuelsReport;
}



int FVSParser::ParseFVSOutputRecord(char       * theRecord,
                                    FILE       * theOpenedFilePtr,
                                    CStringRWC & theOpenedFileName)
{

   // we currently can not parse a PPE-generated output file.

   if (itsPPEOutputFlag) return 1;

   // if old case is NULL, we are still scaning for a PPE-generated file.

   if (!itsOldCase)
   {
      if (strstr(theRecord,"        PARALLEL PROCESSING EXTENSION --"))
      {
         itsPPEOutputFlag = 1;
         return 1;
      }
   }


   FVSCase * theCurrentCase = itsCaseDefineUsingKeywordTable->
     CaseDefine(theRecord,theOpenedFilePtr,theOpenedFileName);

   if (!itsOldCase) itsOldCase = theCurrentCase;

   if (itsOldCase &&
       theCurrentCase &&
       itsOldCase != theCurrentCase)
   {
      // notify all the parsers that the current case is closed (in case
      // they don't already realize it).

      ParseClose();
      itsOldCase = theCurrentCase;
   }

   // if itsOldCase has not been defined, then go on to the next record.

   if (!itsOldCase) return 0;

   int consumed = itsParseSummaryOutputTable->
                  ParseTable(theRecord, theOpenedFilePtr,
                             theOpenedFileName, itsOldCase);

   if (consumed) return 0;

   consumed = itsParseComputeVariables->
              ParseTable(theRecord, theOpenedFilePtr,
                         theOpenedFileName, itsOldCase);

   if (consumed) return 0;

   consumed = itsParseFirePotentialFlameReport->
              ParseTable(theRecord, theOpenedFilePtr,
                         theOpenedFileName, itsOldCase);

   if (consumed) return 0;

   consumed = itsParseFireAllFuelsReport->
              ParseTable(theRecord, theOpenedFilePtr,
                         theOpenedFileName, itsOldCase);

   return 0;
}



void FVSParser::ParseClose( void )
{
   itsOldCase = NULL;
   itsParseSummaryOutputTable        ->ParseClose();
   itsParseComputeVariables          ->ParseClose();
   itsParseFirePotentialFlameReport  ->ParseClose();
   itsParseFireAllFuelsReport        ->ParseClose();
}




CaseDefineUsingKeywordTable::CaseDefineUsingKeywordTable( void )
   :itsCurrentFVSCase(NULL)

{}



FVSCase * CaseDefineUsingKeywordTable::
          CaseDefine(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName)
{

   char * ptr;
   char tmpStr[20];

   // if we are currently defining a case...

   if (itsCurrentFVSCase)
   {
      // look for the end of the case definition.  If we found it, we
      // finish the definition of the case.

      switch (itsParseState)
      {
         case 0:
         {
            if (strstr(theRecord,"        OPTIONS SELECTED BY DEFAULT"))
               itsParseState = 1;
            break;
         }
         case 1:
         {
            if (strstr(theRecord,"--------------")) itsParseState = 2;
            break;
         }
         case 2:
         {
            // Do we have the sampling weight?

            if (strstr(theRecord,"OF PLOTS=") &&
                (ptr = strstr(theRecord,"G WEIGHT=")) &&
                (ptr = ptr+10)) sscanf (ptr,"%f",&itsSamplingWeight);

            else if (strstr(theRecord,"--------------")) itsParseState = 3;
            break;
         }
         case 4:  // parsing the SPLABEL
         {
            // if the record is blank, then we are done.  Set the parse
            // state back to 0.

            RWCString tmp = theRecord;
            tmp = tmp.strip(RWCString::trailing,'\n');
            tmp = tmp.strip(RWCString::both,' ');
            if (tmp.length()) itsGroupCodes += tmp;
            else itsParseState = 0;
         }
         default: break;
      }

      if (itsParseState == 3)
      {
         if (itsStandID != NULLString)
         {
            *itsCurrentFVSCase += ":Stand=";
            *itsCurrentFVSCase += itsStandID;
         }
         if (itsMgmtID != NULLString)
         {
            *itsCurrentFVSCase += ":Mgmt=";
            *itsCurrentFVSCase += itsMgmtID;
         }
         if (itsGroupCodes != NULLString)
         {
            *itsCurrentFVSCase += ":Groups=(";
            *itsCurrentFVSCase += itsGroupCodes;
            *itsCurrentFVSCase += ")";
         }
         if (itsSamplingWeight != FLT_MIN)
            itsCurrentFVSCase->itsSamplingWeight=itsSamplingWeight;

         itsCurrentFVSCase = NULL;
      }
      else
      {
         // Do we have the Stand ID?

         if (ptr = strstr(theRecord,"STAND ID="))
         {
            // this is a likely stand id.

            ptr+=10;
            sscanf (ptr,"%s",tmpStr);
            itsStandID = tmpStr;
            ptr+=itsStandID.length();

            // now, check for the word " was ".  If it exists, then
            // use the next token as the stand id.

            if (*ptr)
            {
               if (strstr(ptr," was "))
               {
                  sscanf (ptr,"%s",tmpStr);
                  itsStandID = tmpStr;
               }

               // otherwise, find the token after the word " Stand "

               else if (ptr = strstr(ptr," Stand "))
               {
                  ptr+=6;
                  sscanf (ptr,"%s",tmpStr);
                  itsStandID = tmpStr;
               }
            }
         }

         // Do we have the Mgmt ID?

         else if ((ptr = strstr(theRecord,"    MANAGEMENT ID=")) &&
                  ptr+19)
         {
            sscanf (ptr+19,"%s",tmpStr);
            itsMgmtID = tmpStr;
         }

         // Do we have the group codes (SPLABEL)?

         else if (strstr(theRecord,"SPLABEL") &&
                  (ptr = strstr(theRecord,"SET:")))
         {
            ptr+=5;
            if (*ptr && *ptr != ' ')
            {
               itsGroupCodes = ptr;
               itsGroupCodes = itsGroupCodes.strip(RWCString::trailing,'\n');
               itsGroupCodes = itsGroupCodes.strip(RWCString::both,' ');
            }
            else itsParseState = 4;
         }
      }
   }
   else
   {
      // do we have a new case!

      if (strstr(theRecord,"      OPTIONS SELECTED BY INPUT"))
      {

         itsStandID        = NULLString;
         itsMgmtID         = NULLString;
         itsGroupCodes     = NULLString;
         itsSamplingWeight = FLT_MIN;
         itsParseState     = 0;


         int i = theSpGlobals->theReportData->GetNextCaseNumber();
         sprintf (tmpStr,"c%4.4d:",i);

         itsCurrentFVSCase = new FVSCase (tmpStr,
                                          theSpGlobals->theReportData);

         char sep[2] = SEPARATOR;
         size_t lastSep = theOpenedFileName.last(sep[0]);

         if (lastSep++)
         {
                 int strLen = theOpenedFileName.length()-lastSep;
            if (strLen > 0) *itsCurrentFVSCase +=
            theOpenedFileName(lastSep,strLen);
         }
         else *itsCurrentFVSCase += theOpenedFileName;

         theSpGlobals->theReportData->insert(itsCurrentFVSCase);
      }
   }
   return itsCurrentFVSCase;
}



ParseSummaryOutputTable::ParseSummaryOutputTable( void )
                        :itsCurrentFVSReport(NULL),
                         itsStandID(NULLString)

{}


void ParseSummaryOutputTable::ParseClose( void )
{
  itsCurrentFVSReport = NULL;
}



int ParseSummaryOutputTable::ParseTable(char       * theRecord,
                                        FILE       * theOpenedFilePtr,
                                        CStringRWC & theOpenedFileName,
                                        FVSCase    * theCurrentCase)
{
   int consumed = 0;

   if (!theCurrentCase) return consumed;

   char tmpStr[20];

   // if we are currently parsing a report...

   if (itsCurrentFVSReport)
   {
      // if the parseCounter is less than 6, we have not found the
      // data yet.

      if (parseCounter < 6)
      {
         parseCounter++;

         // find out if forest type is a column in the file.  if just the
         // forest type is in the table, then set the flag to 1, if
         // the forest type and the size/stocking class info is present,
         // then set the flag to 2.

         if (parseCounter == 4)
         {
            if      (strstr(theRecord," MORT   MERCH FOREST")) itsHasForestType = 1;
            else if (strstr(theRecord," MORT   MERCH FOR SS")) itsHasForestType = 2;
         }

         consumed = 1;
         return consumed;
      }

      // look for the end of the report.  If we found it, are finished
      // with this report.

      RWCString sumRec = theRecord;
      sumRec = sumRec.strip(RWCString::trailing,'\n');
      sumRec = sumRec.strip(RWCString::both);

      if (sumRec == NULLString ||
          sumRec.first('F') != RW_NPOS ||
          sumRec.index("--")!= RW_NPOS) itsCurrentFVSReport = NULL;
      else
      {

         consumed = 1;

         // we have the first record of the report.  If the variables
         // are not yet built, then "get to it!".

         if (itsCurrentFVSReport->itsFVSVariables.entries() == 0)
         {
            AgePtr    = new FVSVariable ("Age",     itsCurrentFVSReport,
                                         FVSVariableUnitsYears);
            TpaPtr    = new FVSVariable ("Tpa",     itsCurrentFVSReport,
                                         FVSVariableUnitsTreesArea,
                                         GraphScaleGroupTreesArea);
            BAPtr     = new FVSVariable ("BA",      itsCurrentFVSReport,
                                         FVSVariableUnitsSqFtOrMeters);
            SDIPtr    = new FVSVariable ("SDI",     itsCurrentFVSReport);
            CCFPtr    = new FVSVariable ("CCF",     itsCurrentFVSReport);
            TopHtPtr  = new FVSVariable ("TopHt",   itsCurrentFVSReport,
                                         FVSVariableUnitsFtOrMeters,
                                         GraphScaleGroupFtOrMeters);
            QMDPtr    = new FVSVariable ("QMD",     itsCurrentFVSReport,
                                         FVSVariableUnitsInchesOrCM);
            TCuFtPtr  = new FVSVariable ("TCuFt",   itsCurrentFVSReport,
                                         FVSVariableUnitsTCuVol,
                                         GraphScaleGroupTCuVol);
            MCuFtPtr  = new FVSVariable ("MCuFt",   itsCurrentFVSReport,
                                         FVSVariableUnitsMCuVol,
                                         GraphScaleGroupMCuVol);
            BdFtPtr   = new FVSVariable ("BdFt",    itsCurrentFVSReport,
                                         FVSVariableUnitsBdft,
                                         GraphScaleGroupBdft);
            TpTCuFtPtr= new FVSVariable ("TpTCuFt", itsCurrentFVSReport,
                                         FVSVariableUnitsTCuVol,
                                         GraphScaleGroupTCuVol);
            TpMCuFtPtr= new FVSVariable ("TpMCuFt", itsCurrentFVSReport,
                                         FVSVariableUnitsMCuVol,
                                         GraphScaleGroupMCuVol);
            TpBdFtPtr = new FVSVariable ("TpBdFt",  itsCurrentFVSReport,
                                         FVSVariableUnitsBdft,
                                         GraphScaleGroupBdft);
            RTpaPtr   = new FVSVariable ("RTpa",    itsCurrentFVSReport,
                                         FVSVariableUnitsTreesArea);
            RTCuFtPtr = new FVSVariable ("RTCuFt",  itsCurrentFVSReport,
                                         FVSVariableUnitsTCuVol,
                                         GraphScaleGroupTCuVol);
            RMCuFtPtr = new FVSVariable ("RMCuFt",  itsCurrentFVSReport,
                                         FVSVariableUnitsMCuVol,
                                         GraphScaleGroupMCuVol);
            RBdFtPtr  = new FVSVariable ("RBdFt",   itsCurrentFVSReport,
                                         FVSVariableUnitsBdft,
                                         GraphScaleGroupBdft);
            AccPtr    = new FVSVariable ("Acc",     itsCurrentFVSReport,
                                         FVSVariableUnitsCuPerAcrePerYear,
                                         GraphScaleGroupGrowthMort);
            MortPtr   = new FVSVariable ("Mort",    itsCurrentFVSReport,
                                         FVSVariableUnitsCuPerAcrePerYear,
                                         GraphScaleGroupGrowthMort);
            MaiPtr    = new FVSVariable ("Mai",     itsCurrentFVSReport);

            if (itsHasForestType) ForestTypePtr =
                                        new FVSVariable ("ForType",
                                                         itsCurrentFVSReport,
                                                         FVSVariableUnitsClassVariable);
            else ForestTypePtr = NULL;

            if (itsHasForestType == 2)
            {
               SizClsPtr = new FVSVariable ("SizCls",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsClassVariable);
               StkClsPtr = new FVSVariable ("StkCls",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsClassVariable);
            }
            else
            {
               SizClsPtr = NULL;
               StkClsPtr = NULL;
            }

            StandIDPtr= new FVSVariable ("StandID", itsCurrentFVSReport,
                                         FVSVariableUnitsClassVariable);

            itsCurrentFVSReport->itsFVSVariables.insert(AgePtr);
            itsCurrentFVSReport->itsFVSVariables.insert(TpaPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(BAPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(SDIPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(CCFPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(TopHtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(QMDPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(TCuFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(MCuFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(BdFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(TpTCuFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(TpMCuFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(TpBdFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(RTpaPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(RTCuFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(RMCuFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(RBdFtPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(AccPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(MortPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(MaiPtr);
            if (ForestTypePtr) itsCurrentFVSReport->
                                     itsFVSVariables.insert(ForestTypePtr);
            if (SizClsPtr) itsCurrentFVSReport->
                                 itsFVSVariables.insert(SizClsPtr);
            if (StkClsPtr) itsCurrentFVSReport->
                                 itsFVSVariables.insert(StkClsPtr);
            itsCurrentFVSReport->itsFVSVariables.insert(StandIDPtr);
            itsTrTCuFt =0;
            itsTrMCuFt =0;
            itsTrBdFt  =0;
            itsStandID =NULLString;
         }

         // parse the record.  This is done in reverse... because we
         // don't know for sure if the first column is blank...we don't
         // know for sure if the year is 4 chars or not...we can't assume
         // that the record can be tokenized.  We have to do this the hard
         // way so that it will work.

         int len  = sumRec.length();
         int forc;
         switch (itsHasForestType)
         {
            case 0:
            {
               forc = len-123;
               break;
            }
            case 1:
            {
               forc = len-128;
               break;
            }
            case 2:
            {
               forc = len-130;
               break;
            }
            default: break;
         }

         int from = 0;
         RWCString year0 = sumRec(from,forc);
         year0 = year0.strip(RWCString::both);
         RWCString year1 = year0;
         year0 += "$0";
         year1 += "$1";

         RWCString tmp;

         if (StkClsPtr)
         {
            forc = 1;
            from = len-forc;
            tmp = sumRec(from,forc);
            StkClsPtr->itsObservations.
                  insert(new FVSObservation(tmp.data(),year1.data(),StkClsPtr));
            from -= forc;
            tmp  = sumRec(from,forc);
            SizClsPtr->itsObservations.
                  insert(new FVSObservation(tmp.data(),year1.data(),SizClsPtr));
         }
         if (ForestTypePtr)
         {
            if (StkClsPtr)
            {
               forc = 4;
               from -= forc;
            }
            else
            {
               forc = 5;
               from = len-forc;
            }
            tmp = sumRec(from,forc);
            tmp = tmp.strip(RWCString::both);
            ForestTypePtr->itsObservations.
                  insert(new FVSObservation(tmp.data(),year1.data(),
                                            ForestTypePtr));
         }
         forc = 8;
         if (ForestTypePtr) from -= forc;
         else               from = len-forc;
         tmp = sumRec(from,forc);
         tmp = tmp.strip(RWCString::both);
         MaiPtr->itsObservations.insert(new FVSObservation
                                        (tmp.data(),year1.data(),MaiPtr));

         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         MortPtr->itsObservations.insert(new FVSObservation
                                         (tmp.data(),year1.data(),MortPtr));
         forc = 5;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         AccPtr->itsObservations.insert(new FVSObservation
                                        (tmp.data(),year1.data(),AccPtr));
         forc = 8;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString periodLen = tmp;

         forc = 5;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString ResQmd = tmp;

         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString ATopHt = tmp;

         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString ACCF = tmp;

         forc = 5;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString ASDI = tmp;

         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString ABA = tmp;

         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString RBdFt = tmp;
         RBdFtPtr->itsObservations.insert(new FVSObservation
                                          (tmp.data(),year1.data(),RBdFtPtr));
         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString RMCuFt = tmp;
         RMCuFtPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year1.data(),RMCuFtPtr));
         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString RTCuFt = tmp;
         RTCuFtPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year1.data(),RTCuFtPtr));

         itsTrTCuFt += ConvertToLong(RTCuFt);
         itsTrMCuFt += ConvertToLong(RMCuFt);
         itsTrBdFt  += ConvertToLong(RBdFt);

         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         RWCString RTpa = tmp;
         RTpaPtr->itsObservations.insert(new FVSObservation
                                         (tmp.data(),year1.data(),BdFtPtr));

         // was there a harvest?

         int itCut = (tmp != "0");

         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);

         if (itCut)
         {
            BdFtPtr->itsObservations.insert(new FVSObservation
                                            (tmp.data(),year0.data(),BdFtPtr));
            tmp = ConvertToString(ConvertToLong(tmp)-ConvertToLong(RBdFt));
         }
         BdFtPtr->itsObservations.insert(new FVSObservation
                                         (tmp.data(),year1.data(),BdFtPtr));
         TpBdFtPtr->itsObservations.insert(new FVSObservation
                    (ConvertToString(ConvertToLong(tmp)+itsTrBdFt).data(),
                     year1.data(),TpBdFtPtr));

         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);

         if (itCut)
         {
            MCuFtPtr->itsObservations.insert(new FVSObservation
                                             (tmp.data(),year0.data(),MCuFtPtr));
            tmp = ConvertToString(ConvertToLong(tmp)-ConvertToLong(RMCuFt));
         }
         MCuFtPtr->itsObservations.insert(new FVSObservation
                                          (tmp.data(),year1.data(),MCuFtPtr));
         TpMCuFtPtr->itsObservations.insert(new FVSObservation
                    (ConvertToString(ConvertToLong(tmp)+itsTrMCuFt).data(),
                     year1.data(),TpMCuFtPtr));

         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            TCuFtPtr ->itsObservations.insert(new FVSObservation
                                              (tmp.data(),year0.data(),TCuFtPtr));
            tmp = ConvertToString(ConvertToLong(tmp)-ConvertToLong(RTCuFt));
         }
         TCuFtPtr->itsObservations.insert(new FVSObservation
                                          (tmp.data(),year1.data(),TCuFtPtr));
         TpTCuFtPtr->itsObservations.insert(new FVSObservation
                     (ConvertToString(ConvertToLong(tmp)+itsTrTCuFt).data(),
                      year1.data(),TpTCuFtPtr));

         forc = 5;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            QMDPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year0.data(),QMDPtr));
            QMDPtr->itsObservations.insert(new FVSObservation
                                           (ResQmd.data(),year1.data(),QMDPtr));
         }
         else QMDPtr->itsObservations.insert(new FVSObservation
                                             (tmp.data(),year1.data(),QMDPtr));

         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            TopHtPtr->itsObservations.insert(new FVSObservation
                                             (tmp.data(),year0.data(),TopHtPtr));
            TopHtPtr->itsObservations.insert(new FVSObservation
                                             (ATopHt.data(),year1.data(),TopHtPtr));
         }
         else TopHtPtr->itsObservations.insert(new FVSObservation
                                               (tmp.data(),year1.data(),TopHtPtr));

         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            CCFPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year0.data(),CCFPtr));
            CCFPtr ->itsObservations.insert(new FVSObservation
                                            (ACCF.data(),year1.data(),CCFPtr));
         }
         else CCFPtr->itsObservations.insert(new FVSObservation
                                             (tmp.data(),year1.data(),CCFPtr));
         forc = 5;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            SDIPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year0.data(),SDIPtr));
            SDIPtr->itsObservations.insert(new FVSObservation
                                           (ASDI.data(),year1.data(),SDIPtr));
         }
         else SDIPtr->itsObservations.insert(new FVSObservation
                                             (tmp.data(),year1.data(),SDIPtr));
         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            BAPtr->itsObservations.insert(new FVSObservation
                                          (tmp.data(),year0.data(),BAPtr));
            BAPtr->itsObservations.insert(new FVSObservation
                                          (ABA.data(),year1.data(),BAPtr));
         }
         else BAPtr->itsObservations.insert(new FVSObservation
                                            (tmp.data(),year1.data(),BAPtr));
         forc = 6;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         if (itCut)
         {
            TpaPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year0.data(),TpaPtr));
            tmp = ConvertToString(ConvertToLong(tmp)-ConvertToLong(RTpa));
            TpaPtr->itsObservations.insert(new FVSObservation
                                           (tmp.data(),year1.data(),TpaPtr));
         }
         else TpaPtr->itsObservations.insert(new FVSObservation
                                             (tmp.data(),year1.data(),TpaPtr));

         forc = 4;
         from -= forc;
         tmp  = sumRec(from,forc);
         tmp  = tmp.strip(RWCString::both);
         AgePtr->itsObservations.insert(new FVSObservation
                                        (tmp.data(),year1.data(), AgePtr));

         if (itsStandID.length() == 0)
         {
            tmp  = theCurrentCase->data();
            size_t pos = tmp.first('=');
            if (pos != RW_NPOS)
            {
               tmp = tmp.remove(0,pos+1);
               pos = tmp.first(':');
               if (pos != RW_NPOS) tmp.resize(pos);
               itsStandID = tmp;
            }
         }
         StandIDPtr->itsObservations.insert(new FVSObservation
                                            (itsStandID.data(),
                                             year1.data(), StandIDPtr));

      }
   }
   else
   {
      // we have a new report...see if the current case already has
      // this report...if not, make a new one.

      if (strstr(theRecord,"      SUMMARY STATISTICS (PER ACRE "))
      {
         const char * name = "Summary Statistics";
         parseCounter = 0;
         itsHasForestType = 0;
         itsCurrentFVSReport = theCurrentCase->GetReport(name);
         if (!itsCurrentFVSReport)
            itsCurrentFVSReport = new FVSReport (name,
                                                 FVSDataTypeOBSOverYears,
                                                 NULL, -1, theCurrentCase);
         consumed = 1;
      }
   }
   return consumed;
}


ParseComputeVariables::ParseComputeVariables( void )
                      :itsCurrentFVSReport(NULL)
{}


void ParseComputeVariables::ParseClose( void )
{

   // when the parser is closed, check for variables that can be
   // copied to FVSDataTypeOBSOverOther.

   if (itsCurrentFVSReport) itsCurrentFVSReport->CopyObsToOBSOverOther();
   itsCurrentFVSReport = NULL;
}



int ParseComputeVariables::ParseTable(char       * theRecord,
                                      FILE       * theOpenedFilePtr,
                                      CStringRWC & theOpenedFileName,
                                      FVSCase    * theCurrentCase)
{

   int consumed = 0;

   if (!theCurrentCase) return consumed;

   const char * cmpu   = "             CMPU     ";
   const char * doneIn = "  DONE IN ";

   // do we have a compute variable?

   if (strstr(theRecord,doneIn) &&
       strstr(theRecord,cmpu))
   {

      // signal that the record is consumed

      consumed = 1;

      // get the variable

      char * cPtr = strstr(theRecord,cmpu)+strlen(cmpu);
      char tmpC[11];
      sscanf (cPtr,"%s",tmpC);
      RWCString var = tmpC;
      var = var.strip(RWCString::both);

      // get the year.

      cPtr = strstr(theRecord,doneIn)+strlen(doneIn);
      sscanf (cPtr,"%s",tmpC);
      RWCString year = tmpC;
      year = year.strip(RWCString::both);

      // get the before/after thin variable flag (if it exists) and
      // change the year if necessary.

      int yearChanged = 0;
      size_t varLen = var.length();
      if (varLen > 2)
      {
         if (strcmp(var(varLen-2,varLen-1).data(),"$0") == 0)
         {
            var = var.remove(varLen-2);
            yearChanged=1;
            year += "$0";
         }
         else if (strcmp(var(varLen-2,varLen-1).data(),"$1") == 0)
         {
            var = var.remove(varLen-2);
            yearChanged=1;
            year += "$1";
         }
      }
      if (!yearChanged) year += "$1";

      // get the class variable flag which also carries the max number
      // of levels number.

      int classLevels = -1;
      size_t atSign = var.first('@');
      if (atSign != RW_NPOS)
      {
         classLevels = 0;
         if (++atSign < var.length())
         {
            const char * ptr = var.data()+atSign;
            if (isdigit((int) *ptr)) sscanf(ptr,"%d",&classLevels);
         }
      }

      // get the value.

      cPtr += 6;
      sscanf (cPtr,"%s",tmpC);
      RWCString val = tmpC;
      val = val.strip(RWCString::both);

      // if the last three chars are ".00", then remove them.

      size_t len = val.length();
      if (len > 3 &&
          strcmp(val(len-3,3).data(),".00") == 0) val = val.remove(len-3);

      // set a pointer to the report.

      if (!itsCurrentFVSReport)
      {

         // if the case already has this report, just find it.
         // otherwise make a new one.

         const char * name = "Compute Variables";
         itsCurrentFVSReport = theCurrentCase->GetReport(name);
         if (!itsCurrentFVSReport) itsCurrentFVSReport =
               new FVSReport (name, FVSDataTypeOBSOverYears,
                              NULL, -1, theCurrentCase);
      }

      // get a pointer to the variable in the current report.

      RWOrderedIterator nextVar(itsCurrentFVSReport->itsFVSVariables);
      FVSVariable * aVar;
      FVSVariable * theVar = NULL;
      while (aVar = (FVSVariable *) nextVar())
      {
         // get a pointer to the variable.
         if (*aVar == var)
         {
            theVar = aVar;
            break;
         }
      }

      // if the variable does not yet exist, then create it, and add it
      // to the list of the reports variables.

      if (!theVar)
      {
         int fvsVType;
         if (classLevels == -1) fvsVType = FVSVariableUnitsUnknown;
         else                   fvsVType = FVSVariableUnitsClassVariable;

         theVar = new FVSVariable(var.data(), itsCurrentFVSReport,
                                  fvsVType,
                                  GraphScaleGroupUndefined,
                                  classLevels);
         itsCurrentFVSReport->itsFVSVariables.insert(theVar);
      }

      // variable exists now! find an observation that has the same
      // identification...if you find one, simply change its value.
      // otherwise, add it!

      RWOrderedIterator nextObs(theVar->itsObservations);
      FVSObservation * aObs;
      FVSObservation * theObs = NULL;
      while (aObs = (FVSObservation *) nextObs())
      {
         // get a pointer to the observation.
         if (aObs->itsObservationIdentification == year)
         {
            theObs = aObs;
            break;
         }
      }
      if (theObs) theObs->ChangeObservation(val.data(),NULL);
      else theVar->itsObservations.
                 insert(new FVSObservation(val.data(),year.data(),
                                           theVar));
   }
   return consumed;
}


