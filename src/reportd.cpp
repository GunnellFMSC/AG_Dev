/********************************************************************

   File Name:     reportd.cpp
   Author:        nlc
   Date:          10/96 ... update 10/98

   Purpose:       see reportd.hpp
*******************************************************************/

#include "reportd.hpp"
#include "suppdefs.hpp"
#include <float.h>

AllFVSCases::AllFVSCases( size_t initialNumberOfCases )
            :RWOrdered(initialNumberOfCases),
             itsOBSOverCaseCase(NULL),
             itsNumOfCasesProcessed(0),
             itsCaseInUse(NULL)

{

   // these report types must correspond to the define statements
   // in the header file for ReportTypes ... they also happen to
   // correspond to the FVSDataTypes.  This happenstance is assumed
   // in the code, so beware!

   itsReportTypesList.insert(new CStringRWC ("List over years"));
   itsReportTypesList.insert(new CStringRWC ("List over tree records"));
   itsReportTypesList.insert(new CStringRWC ("List over cases"));
   itsReportTypesList.insert(new CStringRWC ("List over obs identifier"));

   // these graph types must correspond to the define statements
   // in the header file for GraphTypes ... they do not correpsond
   // to the FVSData types.

   itsGraphTypesList.insert(new CStringRWC("Line: Y over years"));
   itsGraphTypesList.insert(new CStringRWC("Scatter: Y over X"));
   itsGraphTypesList.insert(new CStringRWC("Bar: Y by obs identifier"));
   itsGraphTypesList.insert(new CStringRWC("Bar: Y by case"));
   itsGraphTypesList.insert(new CStringRWC("Phase: class level by year"));
}



AllFVSCases::~AllFVSCases( void )
{
   clearAndDestroy();
   itsReportTypesList.clearAndDestroy();
   itsGraphTypesList.clearAndDestroy();
   if (itsOBSOverCaseCase) delete itsOBSOverCaseCase;
}


size_t AllFVSCases::DeleteAllCases ( void )
{
   size_t numToDelete = entries();
   if (numToDelete)
   {
      clearAndDestroy();
      if (itsOBSOverCaseCase)
      {
         delete itsOBSOverCaseCase;
         itsOBSOverCaseCase = NULL;
         itsNumOfCasesProcessed = 0;
      }
   }
   return numToDelete;
}



FVSCase * AllFVSCases::DeleteCase (size_t i)
{
   FVSCase * toDelete = (FVSCase *) at(i);
   if (toDelete)
   {
      toDelete = (FVSCase *) remove(toDelete);
      if (toDelete)
      {
         delete toDelete;

         // if any case is removed, then all the OBSOverCaseCase
         // data is hereby destroyed (this is done because I don't
         // have time to write the code to find exactly what must be
         // destroyed and what can be kept).

         if (itsOBSOverCaseCase)
         {
            delete itsOBSOverCaseCase;
            itsOBSOverCaseCase = NULL;
            itsNumOfCasesProcessed = 0;
         }
      }
   }
   return toDelete;
}



void AllFVSCases::LoadOBSOverCaseCase()
{

   if (itsNumOfCasesProcessed == entries()) return;
   if (itsOBSOverCaseCase)
   {
      delete itsOBSOverCaseCase;
      itsNumOfCasesProcessed = 0;
   }
   itsOBSOverCaseCase = new FVSCase("OBSOverCaseCase", this);

   // iterate over the defined observations that are inside
   // reports of type FVSDataTypeOBSOverYears.

   RWOrdered reportsOverYears;
   RWOrderedIterator nextC(*this);
   FVSCase * aCase;
   while (aCase = (FVSCase *) nextC())
   {
      itsNumOfCasesProcessed++;
      aCase->LoadListWithReportsOfType(FVSDataTypeOBSOverYears,
                                       &reportsOverYears);

      CStringRW obsId = aCase->data();
      size_t colon = obsId.first(":");
      if (colon != RW_NPOS) obsId = obsId.remove(colon);

      RWOrderedIterator nextR(reportsOverYears);
      FVSReport * aReport;
      while (aReport = (FVSReport *) nextR())
      {
         RWOrderedIterator nextV(aReport->itsFVSVariables);
         FVSVariable * aVariable;
         while (aVariable = (FVSVariable *) nextV())
         {
            RWOrderedIterator nextO(aVariable->itsObservations);
            FVSObservation * anObservation;
            while (anObservation = (FVSObservation *) nextO())
            {
               CStringRW aReportID =
                  anObservation->itsObservationIdentification;
               size_t dollar = aReportID.first('$');
               if (dollar != RW_NPOS)
                  (RWCString&) aReportID = aReportID.remove(dollar);
               aReportID += ":";
               aReportID += *aReport;

               FVSReport * myReportOfSameName = itsOBSOverCaseCase->
                  GetReport(aReportID.data());
               if (!myReportOfSameName) myReportOfSameName =
                  new FVSReport(aReportID.data(),FVSDataTypeOBSOverCase,
                                NULL, 0, itsOBSOverCaseCase);

               FVSVariable * myVariableOfSameName =
                  myReportOfSameName->GetVariable(aVariable->data());
               if (!myVariableOfSameName)
               {
                  myVariableOfSameName =
                     new FVSVariable(aVariable->data(), myReportOfSameName,
                                     aVariable->itsUnitsTag);
                  myReportOfSameName->itsFVSVariables.
                     insert(myVariableOfSameName);
               }
               myVariableOfSameName->itsObservations.
                  insert(new FVSObservation (anObservation->data(),
                                             obsId.data(),
                                             myVariableOfSameName));
            }
         }
      }
   }
   if (itsOBSOverCaseCase->itsFVSReports.entries() > 1)
   {
      RWBinaryTree sort;
      RWOrderedIterator nextR1(itsOBSOverCaseCase->itsFVSReports);
      FVSReport * aReport;
      while (aReport = (FVSReport *) nextR1()) sort.insert(aReport);
      itsOBSOverCaseCase->itsFVSReports.clear();
      RWBinaryTreeIterator nextR2(sort);
      while (aReport = (FVSReport *) nextR2())
         itsOBSOverCaseCase->itsFVSReports.insert(aReport);
   }
}



int AllFVSCases::GetNextCaseNumber( void )
{
   int i = 1;
   if (entries())
   {
      FVSCase * lastCase = (FVSCase *) last();
      CStringRW tmpS = (CStringRW &) *lastCase;
      size_t iC = tmpS.first(':');
      tmpS = tmpS.remove(iC);
      tmpS = tmpS.strip(RWCString::leading,'c');
      tmpS = tmpS.strip(RWCString::leading,'a');
      tmpS = tmpS.strip(RWCString::leading,'t');
      sscanf (tmpS.data(),"%d",&i);
      i++;
   }
   return i;
}




FVSCase * AllFVSCases::AverageCases(RWOrdered * theCasesToAverage,
                                    const char * idString)
{

/*********************************************************************

The weighted averages are computed over the cases in
theCasesToAverage.  The weights are the sampling weights associated
with each case.  The output is itself a new case.

Each observation in the average case has the sampling basis.  This implies
that if there is an observation for a year (actually observation id)
that is not know for all the cases, then the observation is not
included in the average.  This can be thought of as a big set
intersection problem.  Each case, and every report in each case, and
every variable in each report, and every observation in every
variable, must be in all other variables, reports, and cases to be
included in the average.

The approach is to create a new case as a copy of the first one in the
list of theCasesToAverage.  Then, observations are added from the
other cases.  The search for observations is done so that the set
intersection is accomplished on the fly.  That is, if there is an
observation in the copy of the first case (here after called the
newCase) that is not in all the others, then it is deleted from
newCase.

Categorical variables, also called class-level variables are treated
in a special way.  Each "level" of the class is converted to an
observation identification and the value is the proportion of the
sampling weight over all cases where said level is found.  The report
is replicated for each year of the original observation.

For example, say that you create an Event Monitor variable name
SPCLASS@ that has the value 1 when a stand is classified as a
member of species class 1, the value 2 when classified as a member of
species class 2, and the value 3 when classified as a member of
species class 3. Here is some hypothetical data for 3 cases (each case
has a weighting factor of 1 for this illustration):

Year   c1:SPCLASS@   c2:SPCLASS@   c3:SPCLASS@
1995        1             3             3
2005        2             1             2
2015        2             1             3

When Suppose creates an "average" case, it replaces the variable name
SPCLASS@ with a set of three new variables named SPCLASS'1,
SPCLASS'2, and SPCLASS'3 and reports the proportion of the
total sampling weight at each class level for each time period.  So,
in the average, these data would be reported as:

Year   a4:SPCLASS'1   a4:SPCLASS'2   a4:SPCLASS'3
1995    0.333333          0            0.666667
2005    0.333333        0.666667          0
2015    0.333333        0.333333       0.333333

This is accomplished by creating a temporary collection of "reports"
called classConversionReports. Each report in this collection is
actually a class level variable.  Each variable in each report is a
class level.  So, in the case introduced above, the program will create
a temporary report called SPCLASS that will contain 3 variables named
1, 2, and 3.  The observations for each will be identified with the
identical observation identifications from the original cases but the
values will be the sum of the sampling weights.

Once all the cases are processed, the program converts the temporary
reports to variables in newCase by attaching them to the original report
from which they came and renaming them with the compound name of the
original name, an apostrophe, and the class level.  The sum of the
weights are each divided by the total weight to produce the
proportions.

An additional part of the process it to insure that the sampling basis
is constant for all categorical variables.  This is harder to do than
for the continuous variables, but the program does address this need.

*********************************************************************/

   // if there are no cases to average, then don't make one!

   if (!theCasesToAverage || theCasesToAverage->entries() < 2)
      return NULL;

   FVSCase * newCase = new FVSCase((FVSCase *) theCasesToAverage->first(),
                                   this);

   // create the identification of the newCase.

   char tmpStr[21];
   int i = GetNextCaseNumber();
   sprintf (tmpStr,"a%4.4d:",i);
   (CStringRWC &) *newCase = tmpStr;
   if (idString)
   {
      (CStringRWC &) *newCase += idString;
      (CStringRWC &) *newCase += ":";
   }

   // convert all the observations in the newCase to floating point...
   // and use just the floating point version in the following calculations.

   // classConversionReports is a temporary collection of reports used to
   // process class-type variables.

   RWOrdered classConversionReports;

   RWOrderedIterator nextReport(newCase->itsFVSReports);
   FVSReport * aReport;
   while (aReport = (FVSReport *) nextReport())
   {
      RWOrderedIterator nextVariable(aReport->itsFVSVariables);
      FVSVariable * aVariable;
      while (aVariable = (FVSVariable *) nextVariable())
      {
         // if aVariable is continious (not a class variable), then:
         // store the observations as floats by calling GetValueF

         if (aVariable->itsUnitsTag != FVSVariableUnitsClassVariable)
         {
            RWOrderedIterator nextObs(aVariable->itsObservations);
            FVSObservation * anObs;
            while (anObs = (FVSObservation *) nextObs())
            {
               anObs->itsValue = anObs->GetValueF() * newCase->itsSamplingWeight;
            }
         }
         else
         {
            // the variable is a class variable:

            // if the variable name is "StandID", then skip it!

            if (strcmp(aVariable->data(),"StandID") != 0)
            {

               // create a report that has the name equal to the variable
               // and insert it into the list of classConversionReports.

               FVSReport * varReport =
                     new FVSReport(aVariable->data(),
                                   aReport->itsFVSDataType,
                                   NULL, 0, NULL);
               classConversionReports.insert(varReport);

               // the variable names in varReport are the observation values
               // from the original.  The observation identifications are
               // identical to the original and the values are the weighting
               // factors from the original.

               RWOrderedIterator nextObs(aVariable->itsObservations);
               FVSObservation * anObs;
               FVSVariable * aVar;
               while (anObs = (FVSObservation *) nextObs())
               {
                  if (!varReport->GetVariable(anObs->data()))
                  {
                     aVar = new FVSVariable(anObs->data(), varReport,
                                            FVSVariableUnitsProportion,
                                            GraphScaleGroupProportion);
                     varReport->itsFVSVariables.insert(aVar);
                  }
               }

               // go back through the original observations and load the
               // variables.

               RWOrderedIterator nextVar(varReport->itsFVSVariables);
               while (aVar = (FVSVariable *) nextVar())
               {
                  nextObs.reset();
                  while (anObs = (FVSObservation *) nextObs())
                  {
                     // use the void constructor...define everything
                     // except leave the string portion empty.

                     FVSObservation * aNewObs = new FVSObservation();
                     aNewObs->itsFVSVariable = aVar;
                     aNewObs->itsObservationIdentification =
                           anObs->itsObservationIdentification;
                     if (strcmp(anObs->data(),aVar->data()) == 0)
                        aNewObs->itsValue = newCase->itsSamplingWeight;
                     else
                        aNewObs->itsValue = 0.0;
                     aVar->itsObservations.insert(aNewObs);
                  }
               }
            }
         }
      }
   }

   // iterate over the rest of the cases in theCasesToAverage.

   RWOrderedIterator nextCase(*theCasesToAverage);
   FVSCase * addCase = (FVSCase *) nextCase();

   // add the case id for the first case.

   size_t iC = addCase->first(':');
   if (iC != RW_NPOS && iC > 0) ((CStringRWC &) *newCase) +=
                                      ((CStringRWC &) *addCase) (0,iC);
   while (addCase = (FVSCase *) nextCase())
   {
      float addCaseSWT = addCase->itsSamplingWeight;

      // loop over the reports in newCase (this is not the case being
      // added, it is the case to which new cases are added).

      RWOrderedIterator nextReport(newCase->itsFVSReports);
      FVSReport * aReport;
      while (aReport = (FVSReport *) nextReport())
      {
         // check to see if the report is in the case to be added.  While
         // the logic seems backwards, it insures the set intersection.

         FVSReport * addReport = addCase->GetReport(aReport->data());

         // we now have matching reports.  combine the variables.

         if (addReport)
         {

            // loop over the variables in the report (this is the
            // report to which the addReport is being added).

            RWOrderedIterator nextVariable(aReport->itsFVSVariables);
            FVSVariable * aVariable;
            while (aVariable = (FVSVariable *) nextVariable())
            {
               // check to see if the variable is in the report to be
               // added.

               FVSVariable * addVariable =
                     addReport->GetVariable(aVariable->data());
               if (addVariable)
               {
                  // if aVariable is continious (not a class variable), then:

                  if (addVariable->itsUnitsTag != FVSVariableUnitsClassVariable)
                  {
                     RWOrderedIterator nextObs(aVariable->itsObservations);
                     FVSObservation * anObs;
                     while (anObs = (FVSObservation *) nextObs())
                     {
                        FVSObservation * addObs = addVariable->
                              GetObservation(anObs->itsObservationIdentification.
                                             data());
                        if (addObs)
                        {
                           // we now have matching observations.  sum them

                           anObs->itsValue += addObs->GetValueF()*addCaseSWT;
                        }
                        else
                        {
                           // if we don't have matching observations, delete
                           // this observation from aVariable.

                           aVariable->itsObservations.removeAndDestroy(anObs);
                        }
                     }
                     // if all the observations have been deleted, then
                     // also delete the variable from the report.

                     if (aVariable->itsObservations.entries() == 0)
                        aReport->itsFVSVariables.removeAndDestroy(aVariable);
                  }
                  else
                  {
                     // else the variable is a class level variable.

                     // find the variable name as a report in the
                     // classConversionReports

                     FVSReport * varReport = NULL;
                     RWOrderedIterator nextR(classConversionReports);
                     FVSReport * aR;
                     while (aR = (FVSReport *) nextR())
                     {
                        if (strcmp(aR->data(),addVariable->data()) == 0)
                        {
                           varReport = aR;
                           break;
                        }
                     }

                     if (varReport)
                     {
                        // loop over observations in each variable of varReport
                        // and delete any that are not in the list of observations for
                        // addVariable.  Conversely, observations that are in
                        // addVariable and not in observations carried by varReport
                        // will be skipped.  This insures that the set intersection
                        // is taken for categorical variables.

                        RWOrderedIterator nextVarInVarReport(varReport->itsFVSVariables);
                        FVSVariable * aVarInVarReport;
                        while (aVarInVarReport = (FVSVariable *) nextVarInVarReport())
                        {
                           RWOrderedIterator nextObsInVar(aVarInVarReport->itsObservations);
                           FVSObservation * aObsInVar;
                           while (aObsInVar = (FVSObservation *) nextObsInVar())
                           {
                              if (!addVariable->GetObservation(aObsInVar->
                                                itsObservationIdentification.data()))
                              {
                                 aVarInVarReport->itsObservations.removeAndDestroy(aObsInVar);
                              }
                           }

                           // if all the observations are gone for a variable, then
                           // delete the variable (in this case, remember that this
                           // is a level of the original categorical variable).

                           if (aVarInVarReport->itsObservations.isEmpty())
                           {
                              varReport->itsFVSVariables.removeAndDestroy(aVarInVarReport);
                           }
                        }

                        // if all the variables are gone (that is, all the class levels
                        // for the a categorical variable), then delete the report.

                        if (varReport->itsFVSVariables.isEmpty())
                        {
                           classConversionReports.removeAndDestroy(varReport);
                           varReport = NULL;
                        }
                     }

                     if (varReport)
                     {
                        RWOrderedIterator nextObs(addVariable->itsObservations);
                        FVSObservation * addObs;
                        while (addObs = (FVSObservation *) nextObs())
                        {
                           FVSVariable * theFVSVar = varReport->GetVariable(addObs->data());
                           if (theFVSVar)
                           {
                              FVSObservation * anObs = theFVSVar->
                                    GetObservation(addObs->itsObservationIdentification.
                                                   data());
                              if (anObs)
                              {
                                 // we now have matching observations.  add in the
                                 // sampling weight.

                                 anObs->itsValue += addCase->itsSamplingWeight;
                              }
                              else
                              {
                                 // if we don't have matching observations, delete
                                 // this observation from all the variables in the
                                 // varReport.  If any observation list becomes
                                 // empty, then delete varReport from classConversionReports
                                 // and delete aVariable form aReport.

                                 RWOrderedIterator nextInnerVariable(varReport->itsFVSVariables);
                                 FVSVariable * anInnerVariable;
                                 while (anInnerVariable = (FVSVariable *) nextInnerVariable())
                                 {
                                    FVSObservation * aDelObs = anInnerVariable->
                                          GetObservation(addObs->itsObservationIdentification.
                                                         data());
                                    anInnerVariable->itsObservations.removeAndDestroy(aDelObs);
                                    if (anInnerVariable->itsObservations.isEmpty())
                                    {
                                       classConversionReports.removeAndDestroy(varReport);
                                       aReport->itsFVSVariables.removeAndDestroy(aVariable);
                                    }
                                 }
                              }
                           }
                           else
                           {
                              // the variable has not been found (that is, the observation
                              // level has not yet been seen in the data).  Find out
                              // if the observation identification is present in the first
                              // variable (class level) of varReport...if it is, then
                              // add a new variable to the report giving it the observation
                              // value as the name. load the variables observation list with
                              // the current observation.  If it is not found, then simply
                              // skip the observation.

                              FVSVariable * firstVar = (FVSVariable *) varReport->
                                    itsFVSVariables.first();

                              FVSObservation * foundObs = firstVar->
                                    GetObservation(addObs->itsObservationIdentification.
                                                   data());
                              if (foundObs)
                              {
                                 theFVSVar = new FVSVariable(addObs->data(), varReport,
                                                             FVSVariableUnitsProportion,
                                                             GraphScaleGroupProportion);
                                 varReport->itsFVSVariables.insert(theFVSVar);
                                 RWOrderedIterator nextInnerObs(addVariable->itsObservations);
                                 FVSObservation * anInnerObs;
                                 int obsInitSwitch = 1;
                                 while (anInnerObs = (FVSObservation *) nextInnerObs())
                                 {

                                    // skip any observations that are not in the first.

                                    if (firstVar->GetObservation(anInnerObs->
                                                                 itsObservationIdentification.
                                                                 data()))
                                    {

                                       // use the void constructor...define everything
                                       // except leave the string portion empty.

                                       FVSObservation * aNewObs = new FVSObservation();
                                       aNewObs->itsFVSVariable = theFVSVar;
                                       aNewObs->itsObservationIdentification =
                                             anInnerObs->itsObservationIdentification;

                                       // initialize the sampling weight for all the
                                       // observations up to the current one...set the others
                                       // to zero.

                                       if (obsInitSwitch &&
                                           strcmp(anInnerObs->data(),theFVSVar->data()) == 0)
                                          aNewObs->itsValue = addCase->itsSamplingWeight;
                                       else
                                          aNewObs->itsValue = 0.0;

                                       if (anInnerObs == addObs) obsInitSwitch = 0;

                                       theFVSVar->itsObservations.insert(aNewObs);
                                    }
                                 }
                              }
                           }
                        }
                     }
                     else
                     {
                        // if the varReport is not present, then drop the variable.

                        aReport->itsFVSVariables.removeAndDestroy(aVariable);
                     }
                  }
               }
               else
               {
                  // if the addVariable is absent, then drop the variable.

                  aReport->itsFVSVariables.removeAndDestroy(aVariable);
               }
            }

            // if all the variables have been deleted, then
            // also delete the report from the case.

            if (aReport->itsFVSVariables.entries() == 0)
               newCase->itsFVSReports.removeAndDestroy(aReport);
         }
         else
         {
            // the addreport is absent, remove aReport.

            newCase->itsFVSReports.removeAndDestroy(aReport);
         }
      }

      // if all the reports have been deleted, then delete the newCase

      if (newCase->itsFVSReports.entries() == 0)
      {
         delete newCase;
         newCase = NULL;
      }
      else
      {

         // add the weighting factors for added case to newCase

         newCase->itsSamplingWeight += addCase->itsSamplingWeight;

         // add the case id to the list.

         if (newCase->length() > 100)
         {
            newCase->replace(97,50,"...",3);
         }
         else
         {
            (CStringRWC &) *newCase += ',';
            size_t iC = addCase->first(':');
            if (iC != RW_NPOS && iC > 0) ((CStringRWC &) *newCase) +=
                                         ((CStringRWC &) *addCase) (0,iC);
         }
      }
   }

   // if newCase "made it", then we need to divide all the continuous
   // variable's observations by the sampling weight.  Also "write" the
   // answer into the string version of the observation.

   if (newCase)
   {

      // if the total sampling weight is zero, then delete the case.

      if (newCase->itsSamplingWeight <  0.00001)
      {
         delete newCase;
         newCase = NULL;
      }
      else
      {
         RWOrdered tmpReportList(newCase->itsFVSReports.entries());
         RWOrderedIterator nReport(newCase->itsFVSReports);
         FVSReport * aReport;
         while (aReport = (FVSReport *) nReport())
         {
            tmpReportList.insert(aReport);
         }

         float inverseSamplingWT = 1./newCase->itsSamplingWeight;
         RWOrderedIterator nextReport(tmpReportList);
         while (aReport = (FVSReport *) nextReport())
         {
            RWOrdered classVars;
            RWOrderedIterator nextVariable(aReport->itsFVSVariables);
            FVSVariable * aVariable;
            while (aVariable = (FVSVariable *) nextVariable())
            {
               if (aVariable->itsUnitsTag == FVSVariableUnitsClassVariable)
               {
                  // save a list of class variables

                  classVars.insert(aVariable);
               }
               else
               {
                  // compute the average of continuous variables.

                  RWOrderedIterator nextObs(aVariable->itsObservations);
                  FVSObservation * anObs;
                  while (anObs = (FVSObservation *) nextObs())
                  {
                     float av = anObs->itsValue*inverseSamplingWT;
                     sprintf(tmpStr, "%g", av);
                     anObs->ChangeObservation(tmpStr,NULL);
                     anObs->GetValueF();
                  }
               }
            }
            if (classVars.entries())
            {
               RWOrderedIterator nextClassVar(classVars);
               FVSVariable * aClassVar;
               while (aClassVar = (FVSVariable *) nextClassVar())
               {
                  // process the class variables.
                  // save the name of the variable and then delete
                  // it from the report.

                  CStringRW vName = (CStringRW &) *aClassVar;
                  aReport->itsFVSVariables.removeAndDestroy(aClassVar);

                  // find the variable name as a report in the
                  // classConversionReports

                  FVSReport * toMove = NULL;
                  RWOrderedIterator nextR(classConversionReports);
                  FVSReport * aR;
                  while (aR = (FVSReport *) nextR())
                  {
                     if (strcmp(aR->data(),vName.data()) == 0)
                     {
                        toMove = aR;
                        break;
                     }
                  }

                  // modify the name so that it can serve as a prefix.

                  size_t iAt = vName.first('@');
                  if (iAt != RW_NPOS) vName = vName.remove(iAt);
                  vName += "'";

                  // iterate over all the variables, change the
                  // name by prepending vName.

                  if (toMove)
                  {
                     RWOrderedIterator nextVariableToMove(toMove->itsFVSVariables);
                     FVSVariable * aVarToMove;
                     while (aVarToMove = (FVSVariable *) nextVariableToMove())
                     {
                        aVarToMove->prepend(vName);
                        aReport->itsFVSVariables.insert(aVarToMove);

                        // change aVarToMove's report pointer to aReport.

                        aVarToMove->itsFVSReport = aReport;

                        // iterate over the observations to normalize the values

                        RWOrderedIterator nextObs(aVarToMove->itsObservations);
                        FVSObservation * anObs;
                        while (anObs = (FVSObservation *) nextObs())
                        {
                           float av = anObs->itsValue*inverseSamplingWT;
                           sprintf(tmpStr, "%g", av);
                           anObs->ChangeObservation(tmpStr,NULL);
                           anObs->GetValueF();
                        }
                     }

                     // once all the variables for the report have been moved,
                     // we can remove the report.

                     classConversionReports.remove(toMove);
                  }
               }

            }
         }

         // Go back through all the reports to copy the
         // variables to "FVSDataTypeOBSOverOther".
         // This will create more reports as needed.

         nextReport.reset();
                while (aReport = (FVSReport *) nextReport())
         {
            aReport->CopyObsToOBSOverOther();
         }
      }
   }
   return newCase;
}


RWDEFINE_COLLECTABLE(FVSCase,FVSCASE)

FVSCase::FVSCase(const char  * theCaseIdentification,
                 AllFVSCases * theAllFVSCases)
        :CStringRWC(theCaseIdentification),
         itsAllFVSCases(theAllFVSCases),
         itsSamplingWeight(1.0)
{}


FVSCase::FVSCase( void )
        :CStringRWC(),
         itsAllFVSCases(NULL)
{}


FVSCase::FVSCase ( FVSCase * c, AllFVSCases * a)
        :CStringRWC(*c),
         itsAllFVSCases(a),
         itsSamplingWeight(c->itsSamplingWeight)
{
   RWOrderedIterator nextReport(c->itsFVSReports);
   FVSReport * aReport;
   while (aReport = (FVSReport *) nextReport())
   {
      itsFVSReports.insert(new FVSReport(aReport,this));
   }
}


FVSCase::~FVSCase ( void )
{
    itsFVSReports.clearAndDestroy();
}




RWBoolean FVSCase::isEqual(const RWCollectable *t) const
{
   return (this == t);
}



FVSReport * FVSCase::GetReport(const char * name, int theFVSDataType)
{
   if (!name) return NULL;

   RWOrderedIterator nextReport(itsFVSReports);
   FVSReport * report;
   while (report = (FVSReport *) nextReport())
   {
      if (strcmp(report->data(),name) == 0)
      {
         if (theFVSDataType == -1 ||
             theFVSDataType == report->itsFVSDataType) return report;
      }
   }
   return NULL;
}



void FVSCase::LoadListWithReportsOfType(int theFVSDataType,
                                        RWOrdered * theList,
                                        BOOLEAN clearFirst)
{
   if (theList)
   {
      if (clearFirst) theList->clear();
      RWOrderedIterator nextReport(itsFVSReports);
      FVSReport * report;
      while (report = (FVSReport *) nextReport())
      {
         if (report->itsFVSDataType == theFVSDataType)
            theList->insert(report);
      }
   }
}




RWDEFINE_COLLECTABLE(FVSReport,FVSREPORT)

FVSReport::FVSReport (const char * theReportName,
                      int          theFVSDataType,
                      const char * theFileName,
                      long         theFilePosition,
                      FVSCase    * theFVSCase)
          :CStringRWC (theReportName),
           itsFVSDataType(theFVSDataType),
           itsFilePosition(theFilePosition),
           itsFVSCase (theFVSCase)
{
   if (theFileName) itsFileName=theFileName;
   if (itsFVSCase)  itsFVSCase->itsFVSReports.insert(this);
}



FVSReport::FVSReport ( FVSReport * r, FVSCase * c )
          :CStringRWC (*r),
           itsFVSDataType(r->itsFVSDataType),
           itsFileName(r->itsFileName),
           itsFilePosition(r->itsFilePosition),
           itsFVSCase (c)
{
   RWOrderedIterator nextVar(r->itsFVSVariables);
   FVSVariable * aVar;
   while (aVar = (FVSVariable *) nextVar())
      itsFVSVariables.insert(new FVSVariable(aVar,this));
}



FVSReport::FVSReport ( void )
          :CStringRWC (),
           itsFVSDataType(-1),
           itsFileName (),
           itsFilePosition(0),
           itsFVSCase (NULL)
{}



FVSReport::~FVSReport ( void )
{
  itsFVSVariables.clearAndDestroy();
}


RWBoolean FVSReport::isEqual(const RWCollectable *t) const
{
   return (this == t);
}


int FVSReport::compareTo(const RWCollectable* a) const
{
   if (this == a) return 0;
   if (strcmp(((CStringRWC *) this)->data(),
              ((CStringRWC *) a)->data()) == 0) return 0;

   size_t colon1 = ((CStringRWC *) this)->first(':');
   if (colon1 == RW_NPOS) return strcmp(((CStringRWC *) this)->data(),
                                        ((CStringRWC *) a)->data());

   size_t colon2 = ((CStringRWC *) a)->first(':');
   if (colon2 == RW_NPOS) return strcmp(((CStringRWC *) this)->data(),
                                        ((CStringRWC *) a)->data());

   CStringRWC part2This = ((CStringRWC &) *this)
      (++colon1,((CStringRWC *) this)->length()-colon1+1);
   CStringRWC part2A =    ((CStringRWC &) *a)
      (++colon2,((CStringRWC *)    a)->length()-colon2+1);

   int part2Compare = strcmp(part2This.data(),part2A.data());

   if (part2Compare == 0) return strcmp(((CStringRWC *) this)->data(),
                                        ((CStringRWC *) a)->data());
   return part2Compare;
}


FVSVariable * FVSReport::GetVariable(const char * name,
                                     int theUnitsTag)
{
      RWOrderedIterator nextV(itsFVSVariables);
      FVSVariable * aV;
      while (aV = (FVSVariable *) nextV())
      {
         if (strcmp(aV->data(),name) == 0)
         {
            if (theUnitsTag == -1 ||
                theUnitsTag == aV->itsUnitsTag) return aV;
         }
      }
      return NULL;
}


void FVSReport::LoadListWithVariablesOfUnit(RWOrdered * theList,
                                            int variableUnits,
                                            BOOLEAN includeLogic,
                                            BOOLEAN clearFirst)
{
   if (theList)
   {
      if (clearFirst) theList->clear();
      RWOrderedIterator nextVariable(itsFVSVariables);
      FVSVariable * variable;
      while (variable = (FVSVariable *) nextVariable())
      {
         if (variableUnits == -1) theList->insert(variable);
         else
         {
            if (includeLogic)
            {
               if (variable->itsUnitsTag == variableUnits)
                  theList->insert(variable);
            }
            else
            {
               if (variable->itsUnitsTag != variableUnits)
                  theList->insert(variable);
            }
         }
      }
   }
}


void FVSReport::CopyObsToOBSOverOther(FVSVariable * aVarToProcess)
{

   // when a variable has a name that includes an apostrophe, a set
   // of reports is created (using the current report name with the
   // observation identification for the variables prepended for the
   // name).  The new variables for the report is the variable name
   // with the apostrophe and subsequent chars removed.  The
   // observation identification for the new variables is made from
   // the chars that follow the apostrophe.

   // if the value of aVarToProcess is NULL, then process all
   // the variables in the Report.  Otherwise just process the
   // variable pass, if it exists.

   RWOrderedIterator nextVar(itsFVSVariables);
   FVSVariable * aVar;
   while (aVar = (FVSVariable *) nextVar())
   {
      if (!aVarToProcess ||
          aVarToProcess == aVar)
      {
         // if the variable name has an apostrophe ('), then we can build
         // a report for the variable.

         size_t apostrophe = aVar->first('\'');
         size_t dollarInVarName = aVar->first('$');

         if (apostrophe != RW_NPOS && apostrophe)
         {
            CStringRW aVarName = *aVar;
            aVarName = aVarName.remove(apostrophe);

            CStringRW newObsID = *aVar;
            apostrophe++;
            size_t nChars = newObsID.length()-apostrophe;
            newObsID = newObsID(apostrophe,nChars);

            RWOrderedIterator nextObs(aVar->itsObservations);
            FVSObservation * anObs;
            while (anObs = (FVSObservation *) nextObs())
            {
               CStringRW reportName = anObs->itsObservationIdentification;
               if (dollarInVarName == RW_NPOS)
               {
                  size_t dolObs = anObs->itsObservationIdentification.
                        first('$');
                  if (dolObs != RW_NPOS) reportName = reportName.remove(dolObs);
               }
               reportName += ":";
               reportName += *this;

               FVSReport * theFVSReport =
                     itsFVSCase->GetReport(reportName.data(),
                                           FVSDataTypeOBSOverOther);
               if (!theFVSReport) theFVSReport =
                     new FVSReport (reportName.data(), FVSDataTypeOBSOverOther,
                                    NULL, -1, itsFVSCase);

               FVSVariable * theFVSVar = theFVSReport->
                     GetVariable(aVarName.data(), aVar->itsUnitsTag);

               if (!theFVSVar)
               {
                  theFVSVar = new FVSVariable (aVarName.data(),theFVSReport,
                                               aVar->itsUnitsTag,
                                               aVar->itsScaleGroup,
                                               aVar->itsClassLevels);
                  theFVSReport->itsFVSVariables.insert(theFVSVar);
               }

               FVSObservation * existingObs = theFVSVar->GetObservation(newObsID.data());
               if (!existingObs) theFVSVar->itsObservations.
                     insert(new FVSObservation(anObs->data(),newObsID.data(),theFVSVar));
            }
         }
      }
   }
}




int FVSReport::GetFVSDataType( void )
{
   return itsFVSDataType;
}


RWDEFINE_COLLECTABLE(FVSVariable,FVSVARIABLE)


FVSVariable::FVSVariable (const char * theVariableName,
                          FVSReport  * theFVSReport,
                          int          theUnitsTag,
                          int          theScaleGroup,
                          int          theClassLevels)
            :CStringRWC(theVariableName),
             itsFVSReport(theFVSReport),
             itsUnitsTag(theUnitsTag),
             itsScaleGroup(theScaleGroup),
             itsClassLevels(theClassLevels),
             itsMaxIndex(RW_NPOS),
             itsMinIndex(RW_NPOS)
{}

FVSVariable::FVSVariable ( void )
            :CStringRWC(),
             itsFVSReport(NULL),
             itsUnitsTag(FVSVariableUnitsUnknown),
             itsScaleGroup(GraphScaleGroupUndefined),
             itsClassLevels(-1),
             itsMaxIndex(RW_NPOS),
             itsMinIndex(RW_NPOS)
{}


FVSVariable::FVSVariable ( FVSVariable * v, FVSReport * r )
            :CStringRWC(*v),
             itsFVSReport(r),
             itsUnitsTag(v->itsUnitsTag),
             itsScaleGroup(v->itsScaleGroup),
             itsClassLevels(v->itsClassLevels),
             itsMaxIndex(RW_NPOS),
             itsMinIndex(RW_NPOS)
{
   RWOrderedIterator nextObs(v->itsObservations);
   FVSObservation * anObs;
   while (anObs = (FVSObservation *) nextObs())
      itsObservations.insert(new FVSObservation(anObs,this));
}

FVSVariable::~FVSVariable ( void )
{
   itsObservations.clearAndDestroy();
}

float FVSVariable::GetMaxF( void )
{
   if (itsMaxIndex == RW_NPOS) FindMaxMin();
   if (itsClassLevels > 0 &&
       float(itsClassLevels) > itsMax) return float(itsClassLevels);
   else return itsMax;
}
float FVSVariable::GetMinF( void )
{
   if (itsMaxIndex == RW_NPOS) FindMaxMin();
   if (itsClassLevels > 0  && 0. < itsMin) return 0.;
   else return itsMin;
}
size_t FVSVariable::GetMaxIndex( void )
{
   if (itsMaxIndex == RW_NPOS) FindMaxMin();
   return itsMaxIndex;
}
size_t FVSVariable::GetMinIndex( void )
{
   if (itsMinIndex == RW_NPOS) FindMaxMin();
   return itsMinIndex;
}
void FVSVariable::FindMaxMin( void )
{
   itsMax = -FLT_MAX;
   itsMin =  FLT_MAX;

   if (itsObservations.entries())
   {
      for (size_t obsIndex=0; obsIndex<itsObservations.entries(); obsIndex++)
      {
         FVSObservation * anObs = (FVSObservation *)
                                   itsObservations.at(obsIndex);
         if (anObs->length())
         {
            float v = anObs->GetValueF();
            if (v > itsMax)
            {
               itsMaxIndex = obsIndex;
               itsMax=v;
            }
            if (v < itsMin)
            {
               itsMinIndex = obsIndex;
               itsMin=v;
            }
         }
      }
   }
}


const char * FVSVariable::GetUnitsDescription( void ) const
{
   switch (itsUnitsTag)
   {
      case FVSVariableUnitsUnknown:
      {
         return "";
         break;
      }
      case FVSVariableUnitsSampleWt:
      {
         return "Sampling weight (acres, hectares, or other)";
         break;
      }
      case FVSVariableUnitsTCuVol:
      {
         return "Total cubic volume per acre or hectare";
         break;
      }
      case FVSVariableUnitsMCuVol:
      {
         return "Merchantable cubic volume/acre or hectare";
         break;
      }
      case FVSVariableUnitsBdft:
      {
         return "Board feet/acre";
         break;
      }
      case FVSVariableUnitsFtOrMeters:
      {
         return "Feet or meters";
         break;
      }
      case FVSVariableUnitsInchesOrCM:
      {
         return "Inches or cm.";
         break;
      }
      case FVSVariableUnitsSqFtOrMeters:
      {
         return "Square feet/acre or meters/hectare";
         break;
      }
      case FVSVariableUnitsYears:
      {
         return "Years";
         break;
      }
      case FVSVariableUnitsTreesArea:
      {
         return "Trees/acre or hectare";
         break;
      }
      case FVSVariableUnitsCuPerAcrePerYear:
      {
         return "Cubic volume/acre/year or /hectare/year";
         break;
      }
      case FVSVariableUnitsPercent:
      {
         return "Percent";
         break;
      }
      case FVSVariableUnitsProportion:
      {
         return "Proporiton";
         break;
      }
      case FVSVariableUnitsTonsPerAcre:
      {
         return "Tons/acre or Metric tons/hectare";
         break;
      }
      case FVSVariableUnitsMilesPerHour:
      {
         return "Miles or km/hour";
         break;
      }
      case FVSVariableUnitsClassVariable:
      {
         return "Class level";
         break;
      }
      default: return "";
   }
   return "";
}


FVSVariable * FVSVariable::GetNextVariable (const FVSVariable * currVariable,
                                            const BOOLEAN       nextCase)
{
   // if the currVariable is null, then return NULL.  Also insure that
   // the variable contains a pointer to the report, and the report
   // contains a pointer to the case.

   if (!currVariable) return NULL;
   if (!currVariable->itsFVSReport) return NULL;
   if (!currVariable->itsFVSReport->itsFVSCase) return NULL;

   // if the nextCase is true, then find the current case and then
   // look for the next.

   if (nextCase)
   {
      if (currVariable->itsFVSReport->itsFVSDataType ==
          FVSDataTypeOBSOverCase) return (FVSVariable *) currVariable;

      FVSReport * currReport = currVariable->itsFVSReport;
      FVSCase   * currCase   = currReport->itsFVSCase;
      size_t item = currCase->itsAllFVSCases->index(currCase);
      if (item == RW_NPOS) return NULL;
      FVSCase * nextCase;
      item++;
      if (item < currCase->itsAllFVSCases->entries())
           nextCase = (FVSCase *) currCase->itsAllFVSCases->at(item);
      else nextCase = (FVSCase *) currCase->itsAllFVSCases->first();

      // now that the case has been defined, find a report that
      // has the same name as the current report.  If none is found
      // just return the same variable.

      RWOrderedIterator nextR(nextCase->itsFVSReports);
      FVSReport * aR;
      while (aR = (FVSReport *) nextR())
         if ((CStringRWC &) *aR == (CStringRWC &) *currReport) break;
      if (!aR) return (FVSVariable *) currVariable;

      RWOrderedIterator nextV(aR->itsFVSVariables);
      FVSVariable * aV;
      while (aV = (FVSVariable *) nextV())
         if ((CStringRWC &) *aV == (CStringRWC &) *currVariable) return aV;
      if (!aV) return (FVSVariable *) currVariable;
   }
   else
   {
      // just access the report that corresponds to the current
      // variable and get the next variable.

      size_t item = currVariable->itsFVSReport->
         itsFVSVariables.index(currVariable);
      if (item == RW_NPOS) return NULL;
      if (++item >= currVariable->itsFVSReport->itsFVSVariables.entries())
           return (FVSVariable *)
              currVariable->itsFVSReport->itsFVSVariables.first();
      else return (FVSVariable *)
              currVariable->itsFVSReport->itsFVSVariables.at(item);
   }
   return NULL;  // this statement should never be reached.
}




FVSObservation * FVSVariable::GetObservation(const char * id)
{
   RWOrderedIterator nextObs(itsObservations);
   FVSObservation * anObs;
   while (anObs = (FVSObservation *) nextObs())
      if (strcmp(anObs->itsObservationIdentification.data(),id) == 0)
         return anObs;
   return NULL;
}


CStringRW & FVSVariable::BuildNameString(CStringRW & theString)
{
   switch (itsFVSReport->itsFVSDataType)
   {
      case FVSDataTypeOBSOverYears:
      {
         theString = (CStringRW &) *itsFVSReport->itsFVSCase;
         size_t colon = theString.first(':');
         if (colon != RW_NPOS) theString = theString.remove(++colon);
         else                  theString = NULLString;
         break;
      }
      case FVSDataTypeOBSOverOther:
      case FVSDataTypeOBSOverTreeIDS:
      {
         theString = (CStringRW &) *itsFVSReport->itsFVSCase;
         size_t colon = theString.first(':');
         if (colon != RW_NPOS) theString = theString.remove(++colon);
         else                  theString = NULLString;

         colon = itsFVSReport->first(':');
         if (colon != RW_NPOS)
         {
            colon += theString.length()+1;
            theString += (CStringRW &) *itsFVSReport;
            theString = theString.remove(colon);
         }
         break;
      }
      case FVSDataTypeOBSOverCase:
      {
         theString = (CStringRW &) *itsFVSReport;
         size_t colon = theString.first(':');
         if (colon != RW_NPOS) theString = theString.remove(++colon);
         else                  theString = NULLString;
         break;
      }
      default: break;
   }
   theString += ((CStringRW &) *this);
   return theString;
}



RWBoolean FVSVariable::isEqual(const RWCollectable *t) const
{
   return (this == t);
}



RWDEFINE_COLLECTABLE(FVSObservation,FVSOBSERVATION)

FVSObservation::FVSObservation (const char  * theValue,
                                const char  * theObservationIdentification,
                                FVSVariable * theFVSVariable)
               :CStringRWC(theValue),
                itsObservationIdentification(theObservationIdentification),
                itsFVSVariable(theFVSVariable)

{
   (CStringRWC &) *this = strip(RWCString::both);
   itsObservationIdentification = itsObservationIdentification.strip(RWCString::both);
   itsFVSVariable->itsMaxIndex = RW_NPOS;
}


FVSObservation::FVSObservation ( void )
               :CStringRWC(),
                itsObservationIdentification(),
                itsFVSVariable(NULL)

{}


FVSObservation::FVSObservation ( FVSObservation * o, FVSVariable * v)
               :CStringRWC(*o),
                itsObservationIdentification(o->itsObservationIdentification),
                itsFVSVariable(v)
{}

void FVSObservation::ChangeObservation(const char * theValue,
                                       const char * theObservationIdentification)
{
   if (theValue)
   {
      replace(0,length(),theValue);
      strip(RWCString::both);
   }
   if (theObservationIdentification)
   {
      itsObservationIdentification = theObservationIdentification;
      itsObservationIdentification =
            itsObservationIdentification.strip(RWCString::both);
   }
   itsFVSVariable->itsMaxIndex = RW_NPOS;
   itsFVSVariable->itsMinIndex = RW_NPOS;
}


RWBoolean FVSObservation::isEqual(const RWCollectable *t) const
{
   return (this == t);
}


float FVSObservation::GetValueF( void )
{
   itsValue = 0.;
   if (length()) sscanf (data(),"%f",&itsValue);
   return itsValue;
}
int FVSObservation::GetValueI( void )
{
   if (length())
   {
      int x;
      sscanf (data(),"%d",&x);
      return x;
   }
   else return 0;
}
float FVSObservation::GetObsIDF( void )
{
   if (itsObservationIdentification.length())
   {
      float x;
      sscanf (itsObservationIdentification.data(),"%f",&x);
      return x;
   }
   else return 0.;
}
int FVSObservation::GetObsIDI( void )
{
   if (itsObservationIdentification.length())
   {
      int x;
      sscanf (itsObservationIdentification.data(),"%d",&x);
      return x;
   }
   else return 0;
}

