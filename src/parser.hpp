/********************************************************************

   File Name:     parser.hpp
   Author:        nlc
   Date:          07/27/98


   Purpose:  This set of class provides for reading FVS-generated
             output and loading the AllFVSCases data structure.
 *                                                                  *
 ********************************************************************/

#ifndef FVSOutputParser_H
#define FVSOutputParser_H

class FVSReport;
class FVSVariable;
class ParseFirePotentialFlameReport;
class ParseFireAllFuelsReport;

class CaseDefineUsingKeywordTable
{
   public:

      CaseDefineUsingKeywordTable( void );
      FVSCase * CaseDefine(char       * theRecord,
                           FILE       * theOpenedFilePtr,
                           CStringRWC & theOpenedFileName);

   private:

      RWCString      itsStandID;
      RWCString      itsMgmtID;
      RWCString      itsGroupCodes;
      FVSCase      * itsCurrentFVSCase;
      float          itsSamplingWeight;
      int            itsParseState;

};



class ParseSummaryOutputTable
{
   public:

      ParseSummaryOutputTable( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      int parseCounter;
      int itsHasForestType;

      FVSReport   * itsCurrentFVSReport;
      RWCString     itsStandID;
      FVSVariable * AgePtr;
      FVSVariable * TpaPtr;
      FVSVariable * BAPtr;
      FVSVariable * SDIPtr;
      FVSVariable * CCFPtr;
      FVSVariable * TopHtPtr;
      FVSVariable * QMDPtr;
      FVSVariable * TCuFtPtr;
      FVSVariable * MCuFtPtr;
      FVSVariable * BdFtPtr;
      FVSVariable * TpTCuFtPtr;
      FVSVariable * TpMCuFtPtr;
      FVSVariable * TpBdFtPtr;
      FVSVariable * RTpaPtr;
      FVSVariable * RTCuFtPtr;
      FVSVariable * RMCuFtPtr;
      FVSVariable * RBdFtPtr;
      FVSVariable * AccPtr;
      FVSVariable * MortPtr;
      FVSVariable * MaiPtr;
      FVSVariable * StandIDPtr;
      FVSVariable * ForestTypePtr;
      FVSVariable * SizClsPtr;
      FVSVariable * StkClsPtr;
      long          itsTrTCuFt;
      long          itsTrMCuFt;
      long          itsTrBdFt;
};


class ParseComputeVariables
{
   public:

      ParseComputeVariables( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      FVSReport * itsCurrentFVSReport;
      RWOrdered   itsComputeVariables;
};


class FVSParser
{

   public:

      FVSParser( void );
     ~FVSParser( void );
      int ParseFVSOutputRecord(char       * theRecord,
                               FILE       * theOpenedFilePtr,
                               CStringRWC & theOpenedFileName);
      void ParseClose( void );

      int itsPPEOutputFlag;

   private:

      FVSCase                        * itsOldCase;
      CaseDefineUsingKeywordTable    * itsCaseDefineUsingKeywordTable;
      ParseSummaryOutputTable        * itsParseSummaryOutputTable;
      ParseComputeVariables          * itsParseComputeVariables;
      ParseFirePotentialFlameReport  * itsParseFirePotentialFlameReport;
      ParseFireAllFuelsReport        * itsParseFireAllFuelsReport;
};


#endif

