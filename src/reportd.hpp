/********************************************************************

   File Name:     reportd.hpp
   Author:        nlc
   Date:          10/96

   Purpose:       Holds the data that allows users to create reports
                  based on FVS output.

   Class:         AllFVSCases
   Inheritance:   RWOrdered

                  Holds all the cases.

   Class:         FVSCase
   Inheritance:   CStringRWC

                  Holds one case and a list of reports for the case.

   Class:         FVSReport
   Inheritance:   CStringRWC

                  Holds one report and a list of variables
                  for the report.

   Class:         FVSVariable
   Inheritance:   CStringRWC

                  Holds one variable and a list of observations for the
                  variable.

   Class:         FVSObservation
   Inheritance:   CStringRWC

                  Holds one observation, for one variable, for one
                  report, for one case, of the set of cases.

*******************************************************************/

#ifndef FVSReportData_H
#define FVSReportData_H

#include "PwrDef.h"
#include CStringRWC_i

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ordcltn.h>


class FVSCase;
class FVSReport;
class FVSVariable;
class FVSObservation;

class AllFVSCases : public RWOrdered
{
   public:

      AllFVSCases( size_t initialNumberOfCases );
      virtual ~AllFVSCases( void );
      FVSCase     * AverageCases(RWOrdered * theCasesToAverage = NULL,
                                 const char * idString = NULL);
      int           itsNumOfCasesProcessed;
      void          LoadOBSOverCaseCase( void );
      int           GetNextCaseNumber( void );
      size_t        DeleteAllCases ( void );
      FVSCase     * DeleteCase ( size_t i );
      RWOrdered     itsReportTypesList;
      RWOrdered     itsGraphTypesList;
      FVSCase     * itsOBSOverCaseCase;
      FVSCase     * itsCaseInUse;
};

// Report data types are:

#define FVSDataTypeUnknown            0
#define FVSDataTypeOBSOverYears       1
#define FVSDataTypeOBSOverTreeIDS     2
#define FVSDataTypeOBSOverCase        3
#define FVSDataTypeOBSOverOther       4


// Units of measure types ... these are used to tag the units of measure
// in the variable picker. type 0, unknown, results in no units being
// displayed.

#define FVSVariableUnitsUnknown           0
#define FVSVariableUnitsSampleWt          1
#define FVSVariableUnitsTCuVol            2
#define FVSVariableUnitsMCuVol            3
#define FVSVariableUnitsBdft              4
#define FVSVariableUnitsFtOrMeters        5
#define FVSVariableUnitsInchesOrCM        6
#define FVSVariableUnitsSqFtOrMeters      7
#define FVSVariableUnitsYears             8
#define FVSVariableUnitsTreesArea         9
#define FVSVariableUnitsCuPerAcrePerYear  10
#define FVSVariableUnitsPercent           11
#define FVSVariableUnitsProportion        12
#define FVSVariableUnitsTonsPerAcre       13
#define FVSVariableUnitsMilesPerHour      14
#define FVSVariableUnitsClassVariable     15


// Define scaling groups.  Variables that are members of the same
// scaling group are globally scaled on graphs... AND
// define the number of scaling types (used in graphics routines).

#define GraphScaleGroupUndefined         0
#define GraphScaleGroupSampleWt          1
#define GraphScaleGroupTreesArea         2
#define GraphScaleGroupTCuVol            3
#define GraphScaleGroupMCuVol            4
#define GraphScaleGroupBdft              5
#define GraphScaleGroupGrowthMort        6
#define GraphScaleGroupFtOrMeters        7
#define GraphScaleGroupPercent           8
#define GraphScaleGroupProportion        9
#define GraphScaleGroupMilesPerHour      10
#define GraphScaleGroupTonsPerAcre       11
#define NumberGraphScaleGroups           GraphScaleGroupTonsPerAcre + 1

// Report types are:
// NOTE that the FVSDataTypes and the ReportTypes have a 1 to 1
// correspondence.

#define ReportTypeUnknown            0
#define ReportTypeOBSOverYears       1
#define ReportTypeOBSOverTreeIDS     2
#define ReportTypeOBSOverCase        3
#define ReportTypeOBSOtherID         4

// Graph types are:

#define GraphTypeUnknown             0
#define GraphTypeLineOverYears       1
#define GraphTypeScatterYOverX       2
#define GraphTypeBarYOverObsID       3
#define GraphTypeBarYOverCase        4
#define GraphTypePhaseOverYears      5

class FVSCase : public CStringRWC
{

   RWDECLARE_COLLECTABLE(FVSCase)

   public:

      FVSCase (const char  * theCaseIdentification,
               AllFVSCases * theAllFVSCases);
      FVSCase ( void );
      FVSCase ( FVSCase * c, AllFVSCases * a);
      virtual ~FVSCase ( void );
      virtual RWBoolean isEqual(const RWCollectable *t) const;
      FVSReport * GetReport(const char * name,
                            int theFVSDataType = -1);
      void LoadListWithReportsOfType(int theFVSDataType,
                                     RWOrdered * theList,
                                     BOOLEAN clearFirst = TRUE);

      float itsSamplingWeight;
      AllFVSCases * itsAllFVSCases;
      RWOrdered     itsFVSReports;
};


class FVSReport : public CStringRWC
{

   RWDECLARE_COLLECTABLE(FVSReport)

   public:

      FVSReport (const char * theReportName,
                 int          theFVSDataType,
                 const char * theFileName,
                 long         theFilePosition,
                 FVSCase    * theFVSCase);
      FVSReport ( void );
      FVSReport ( FVSReport * r, FVSCase * c );
      virtual ~FVSReport ( void );
      int  GetFVSDataType( void );
      void LoadListWithVariablesOfUnit(RWOrdered * theList,
                                       int variableUnits = -1,
                                       BOOLEAN includeLogic = TRUE,
                                       BOOLEAN clearFirst = TRUE);
      FVSVariable * GetVariable(const char * name,
                                int theUnitsTag = -1);
      void CopyObsToOBSOverOther(FVSVariable * aVarToProcess = NULL);
      virtual RWBoolean isEqual(const RWCollectable *t) const;
      virtual int       compareTo(const RWCollectable* a) const;
      RWOrdered   itsFVSVariables;
      FVSCase   * itsFVSCase;
      RWCString   itsFileName;
      long        itsFilePosition;
      int         itsFVSDataType;

};


class FVSVariable : public CStringRWC
{

   RWDECLARE_COLLECTABLE (FVSVariable)

   public:

      FVSVariable (const char * theVariableName,
                   FVSReport  * theFVSReport,
                   int          theUnitsTag    = FVSVariableUnitsUnknown,
                   int          theScaleGroup  = GraphScaleGroupUndefined,
                   int          theClassLevels = -1);
      FVSVariable ( void );
      FVSVariable ( FVSVariable * v, FVSReport * r );
      virtual ~FVSVariable ( void );
      virtual RWBoolean isEqual(const RWCollectable *t) const;
      FVSVariable * GetNextVariable (const FVSVariable * currVariable,
                                     const BOOLEAN       nextCase);
      FVSObservation * GetObservation(const char * id);

      CStringRW & BuildNameString(CStringRW & theString);
      const char * GetUnitsDescription( void ) const;

      RWOrdered   itsObservations;
      FVSReport * itsFVSReport;
      int         itsUnitsTag;
      int         itsScaleGroup;
      int         itsClassLevels;
      float       GetMaxF( void );
      float       GetMinF( void );
      size_t      GetMaxIndex( void );
      size_t      GetMinIndex( void );

   private:

      float itsMax;
      float itsMin;
      size_t itsMaxIndex;
      size_t itsMinIndex;

      void FindMaxMin( void );

   friend class FVSObservation;

};


class FVSObservation : public CStringRWC

{
   RWDECLARE_COLLECTABLE (FVSObservation)

   public:

      FVSObservation (const char  * theValue,
                      const char  * theObservationIdentification,
                      FVSVariable * theFVSVariable);

      void ChangeObservation (const char  * theValue,
                              const char  * theObservationIdentification);
      FVSObservation ( void );
      FVSObservation ( FVSObservation * o, FVSVariable * v );
      virtual RWBoolean isEqual(const RWCollectable *t) const;
      float         GetValueF( void );
      int           GetValueI( void );
      float         GetObsIDF( void );
      int           GetObsIDI( void );

      FVSVariable * itsFVSVariable;
      CStringRWC    itsObservationIdentification;
      float         itsValue; // be careful...used in averaging code.
};

#endif




