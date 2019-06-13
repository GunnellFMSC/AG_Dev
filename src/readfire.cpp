/********************************************************************

  File Name:     readfire.cpp
  Author:        nlc
  Date:          10/25/96

  see readfire.hpp for notes.

 ********************************************************************/

#include "reportd.hpp"
#include "readfire.hpp"
#include <rw/ctoken.h>

ParseFirePotentialFlameReport::ParseFirePotentialFlameReport( void )
                              :itsCurrentFVSReport(NULL)
{}


void ParseFirePotentialFlameReport::ParseClose( void )
{
  itsCurrentFVSReport = NULL;
}



int ParseFirePotentialFlameReport::ParseTable(char       * theRecord,
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
      // if the parseCounter is less than 5, we have not found the
      // data yet.

      if (parseCounter < 4)
      {
             parseCounter++;
             consumed = 1;
             return consumed;
      }

      // look for the end of the report.  If we found it, are finished
      // with this report.

      RWCString sumRec = theRecord;
      sumRec = sumRec.strip(RWCString::trailing,'\n');
      sumRec = sumRec.strip(RWCString::both);

      if (sumRec == NULLString ||
               sumRec.index("FOREST VEGETATION") != RW_NPOS ||
               sumRec.index("FIRE MODEL") != RW_NPOS ||
               sumRec.index("----------") != RW_NPOS) itsCurrentFVSReport = NULL;
      else
      {
             consumed = 1;

         // we have the first record of the report.  If the variables
         // are not yet built, then "get to it!".

         if (itsCurrentFVSReport->itsFVSVariables.entries() == 0)
         {
            flameLenSurSev = new FVSVariable ("FLSurfSev",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsFtOrMeters,
                                            GraphScaleGroupFtOrMeters);
            flameLenSurMod = new FVSVariable ("FLSurfMod",
                                                  itsCurrentFVSReport,
                                                  FVSVariableUnitsFtOrMeters,
                                                  GraphScaleGroupFtOrMeters);
            flameLenTotSev = new FVSVariable ("FLTotSev",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsFtOrMeters,
                                            GraphScaleGroupFtOrMeters);
            flameLenTotMod = new FVSVariable ("FLTotMod",
                                                  itsCurrentFVSReport,
                                                  FVSVariableUnitsFtOrMeters,
                                                  GraphScaleGroupFtOrMeters);
            fireTypeSev = new FVSVariable ("FTypeSev",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsClassVariable);
            fireTypeMod = new FVSVariable ("FTypeMod",
                                                  itsCurrentFVSReport,
                                                  FVSVariableUnitsClassVariable);
            pTorchSev = new FVSVariable ("PTorchSev",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsProportion,
                                          GraphScaleGroupProportion);
            pTorchMod = new FVSVariable ("PTorchMod",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsProportion,
                                          GraphScaleGroupProportion);
            torchIndex = new FVSVariable ("TorchIndx",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsMilesPerHour,
                                          GraphScaleGroupMilesPerHour);
            crownIndex = new FVSVariable ("CrownIndx",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsMilesPerHour,
                                          GraphScaleGroupMilesPerHour);
            crownBaseHeight = new FVSVariable ("CrBaseHt",
                                               itsCurrentFVSReport,
                                               FVSVariableUnitsFtOrMeters,
                                               GraphScaleGroupFtOrMeters);
            crownBulkDensity = new FVSVariable ("CrBulkDen",
                                                itsCurrentFVSReport);
            potMortSev = new FVSVariable ("PotMortSev",
                                           itsCurrentFVSReport,
                                           FVSVariableUnitsPercent,
                                           GraphScaleGroupPercent);
            potMortMod = new FVSVariable ("PotMortMod",
                                                 itsCurrentFVSReport,
                                                 FVSVariableUnitsPercent,
                                                 GraphScaleGroupPercent);
            potVolSev = new FVSVariable ("PotVolSev",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsTCuVol,
                                          GraphScaleGroupTCuVol);
            potVolMod = new FVSVariable ("PotVolMod",
                                                itsCurrentFVSReport,
                                                FVSVariableUnitsTCuVol,
                                                GraphScaleGroupTCuVol);
            potSmokeSev = new FVSVariable ("PotSmokeSev",
                                            itsCurrentFVSReport);
            potSmokeMod = new FVSVariable ("PotSmokeMod",
                                                  itsCurrentFVSReport);
            fuelModel = new FVSVariable ("FuelModel",
                                         itsCurrentFVSReport,
                                         FVSVariableUnitsClassVariable);

            itsCurrentFVSReport->itsFVSVariables.insert(flameLenSurSev);
            itsCurrentFVSReport->itsFVSVariables.insert(flameLenSurMod);
            itsCurrentFVSReport->itsFVSVariables.insert(flameLenTotSev);
            itsCurrentFVSReport->itsFVSVariables.insert(flameLenTotMod);
            itsCurrentFVSReport->itsFVSVariables.insert(fireTypeSev);
            itsCurrentFVSReport->itsFVSVariables.insert(fireTypeMod);
            itsCurrentFVSReport->itsFVSVariables.insert(pTorchSev);
            itsCurrentFVSReport->itsFVSVariables.insert(pTorchMod);
            itsCurrentFVSReport->itsFVSVariables.insert(torchIndex);
            itsCurrentFVSReport->itsFVSVariables.insert(crownIndex);
            itsCurrentFVSReport->itsFVSVariables.insert(crownBaseHeight);
            itsCurrentFVSReport->itsFVSVariables.insert(crownBulkDensity);
            itsCurrentFVSReport->itsFVSVariables.insert(potMortSev);
            itsCurrentFVSReport->itsFVSVariables.insert(potMortMod);
            itsCurrentFVSReport->itsFVSVariables.insert(potVolSev);
            itsCurrentFVSReport->itsFVSVariables.insert(potVolMod);
            itsCurrentFVSReport->itsFVSVariables.insert(potSmokeSev);
            itsCurrentFVSReport->itsFVSVariables.insert(potSmokeMod);
            itsCurrentFVSReport->itsFVSVariables.insert(fuelModel);
         }

         // parse the record and store the tokens.

         RWCTokenizer nextToken(sumRec);
         RWCString year = nextToken();
         year += "$1";
         RWCString aToken;

         aToken = nextToken();
         flameLenSurSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),flameLenSurSev));
         aToken = nextToken();
         flameLenSurMod->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),flameLenSurMod));
         aToken = nextToken();
         flameLenTotSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),flameLenTotSev));
         aToken = nextToken();
         flameLenTotMod->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),flameLenTotMod));
         aToken = nextToken();
         fireTypeSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),fireTypeSev));
         aToken = nextToken();
         fireTypeMod->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),fireTypeMod));
         aToken = nextToken();
         pTorchSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),pTorchSev));
         aToken = nextToken();
         pTorchMod->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),pTorchMod));
         aToken = nextToken();
         torchIndex->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),torchIndex));
         aToken = nextToken();
         crownIndex->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),crownIndex));
         aToken = nextToken();
         crownBaseHeight->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),crownBaseHeight));
         aToken = nextToken();
         crownBulkDensity->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),crownBulkDensity));
         aToken = nextToken();
         potMortSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),potMortSev));
         aToken = nextToken();
         potMortMod ->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),potMortMod));
         aToken = nextToken();
         potVolSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),potVolSev));
         aToken = nextToken();
         potVolMod ->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),potVolMod));
         aToken = nextToken();
         potSmokeSev->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),potSmokeSev));
         aToken = nextToken();
         potSmokeMod ->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),potSmokeMod));
         aToken = nextToken();
         fuelModel->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),fuelModel));
      }
   }
   else
   {
      // we have a new report...see if the current case already has
      // this report...if not, make a new one.

      if (strstr(theRecord,"  FLAME LENGTH (FT)  FIRE PROB OF "))
      {
         const char * name = "Potential Fire Report";
         parseCounter = 0;
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



ParseFireAllFuelsReport::ParseFireAllFuelsReport( void )
                        :itsCurrentFVSReport(NULL)
{}


void ParseFireAllFuelsReport::ParseClose( void )
{
  itsCurrentFVSReport = NULL;
}



int ParseFireAllFuelsReport::ParseTable(char       * theRecord,
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
      // if the parseCounter is less than 5, we have not found the
      // data yet.

      if (parseCounter < 6)
      {
         parseCounter++;
         consumed = 1;
         return consumed;
      }

      // look for the end of the report.  If we found it, are finished
      // with this report.

      RWCString sumRec = theRecord;
      sumRec = sumRec.strip(RWCString::trailing,'\n');
      sumRec = sumRec.strip(RWCString::both);

      if (sumRec == NULLString ||
          sumRec.index("FOREST VEGETATION") != RW_NPOS ||
          sumRec.first('F') != RW_NPOS ||
          sumRec.index("--")!= RW_NPOS) itsCurrentFVSReport = NULL;
      else
      {
         consumed = 1;

         // we have the first record of the report.  If the variables
         // are not yet built, then "get to it!".

         if (itsCurrentFVSReport->itsFVSVariables.entries() == 0)
         {
            surfDeadLitter = new FVSVariable ("SrfDLitt",
                                              itsCurrentFVSReport,
                                              FVSVariableUnitsTonsPerAcre,
                                              GraphScaleGroupTonsPerAcre);
            surfDeadDuff = new FVSVariable ("SrfDDuff",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            surfDead0To3 = new FVSVariable ("SrfD0To3",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            surfDeadGT3 = new FVSVariable ("SrfDGT3",
                                           itsCurrentFVSReport,
                                           FVSVariableUnitsTonsPerAcre,
                                           GraphScaleGroupTonsPerAcre);
            surfDead3to6 = new FVSVariable ("SrfD3To6",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            surfDead6to12 = new FVSVariable ("SrfD6To12",
                                             itsCurrentFVSReport,
                                             FVSVariableUnitsTonsPerAcre,
                                             GraphScaleGroupTonsPerAcre);
            surfDeadGT12 = new FVSVariable ("SrfDGT12",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            surfLiveHerb = new FVSVariable ("SrfHerb",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            surfLiveShrub = new FVSVariable ("SrfShrub",
                                             itsCurrentFVSReport,
                                             FVSVariableUnitsTonsPerAcre,
                                             GraphScaleGroupTonsPerAcre);
            surfTotal = new FVSVariable ("SurfTot",
                                         itsCurrentFVSReport,
                                         FVSVariableUnitsTonsPerAcre,
                                         GraphScaleGroupTonsPerAcre);
            standDead0To3 = new FVSVariable ("StdD0To3",
                                             itsCurrentFVSReport,
                                             FVSVariableUnitsTonsPerAcre,
                                             GraphScaleGroupTonsPerAcre);
            standDeadGT3 = new FVSVariable ("StdDGT3",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            standLiveFol = new FVSVariable ("StdFol",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            standLive0To3 = new FVSVariable ("StdL0To3",
                                             itsCurrentFVSReport,
                                             FVSVariableUnitsTonsPerAcre,
                                             GraphScaleGroupTonsPerAcre);
            standLiveGT3 = new FVSVariable ("StdLGT3",
                                            itsCurrentFVSReport,
                                            FVSVariableUnitsTonsPerAcre,
                                            GraphScaleGroupTonsPerAcre);
            standTotal = new FVSVariable ("StdTot",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsTonsPerAcre,
                                          GraphScaleGroupTonsPerAcre);
            totalFuels = new FVSVariable ("AllTot",
                                          itsCurrentFVSReport,
                                          FVSVariableUnitsTonsPerAcre,
                                          GraphScaleGroupTonsPerAcre);
            totalConsumption = new FVSVariable ("Consumption",
                                                itsCurrentFVSReport,
                                                FVSVariableUnitsTonsPerAcre,
                                                GraphScaleGroupTonsPerAcre);
            standingRemoved = new FVSVariable ("StndgRemoved",
                                                itsCurrentFVSReport,
                                                FVSVariableUnitsTonsPerAcre,
                                                GraphScaleGroupTonsPerAcre);

            itsCurrentFVSReport->itsFVSVariables.insert(surfDeadLitter);
            itsCurrentFVSReport->itsFVSVariables.insert(surfDeadDuff);
            itsCurrentFVSReport->itsFVSVariables.insert(surfDead0To3);
            itsCurrentFVSReport->itsFVSVariables.insert(surfDeadGT3);
            itsCurrentFVSReport->itsFVSVariables.insert(surfDead3to6);
            itsCurrentFVSReport->itsFVSVariables.insert(surfDead6to12);
            itsCurrentFVSReport->itsFVSVariables.insert(surfDeadGT12);
            itsCurrentFVSReport->itsFVSVariables.insert(surfLiveHerb);
            itsCurrentFVSReport->itsFVSVariables.insert(surfLiveShrub);
            itsCurrentFVSReport->itsFVSVariables.insert(surfTotal);
            itsCurrentFVSReport->itsFVSVariables.insert(standDead0To3);
            itsCurrentFVSReport->itsFVSVariables.insert(standDeadGT3);
            itsCurrentFVSReport->itsFVSVariables.insert(standLiveFol);
            itsCurrentFVSReport->itsFVSVariables.insert(standLive0To3);
            itsCurrentFVSReport->itsFVSVariables.insert(standLiveGT3);
            itsCurrentFVSReport->itsFVSVariables.insert(standTotal);
            itsCurrentFVSReport->itsFVSVariables.insert(totalFuels);
            itsCurrentFVSReport->itsFVSVariables.insert(totalConsumption);
            itsCurrentFVSReport->itsFVSVariables.insert(standingRemoved);
 }

         // parse the record and store the tokens.

         RWCTokenizer nextToken(sumRec);
         RWCString year = nextToken();
         year += "$1";
         RWCString aToken;

         aToken = nextToken();
         surfDeadLitter->itsObservations.insert(new FVSObservation
                         (aToken.data(),year.data(),surfDeadLitter));
         aToken = nextToken();
         surfDeadDuff->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfDeadDuff));
         aToken = nextToken();
         surfDead0To3->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfDead0To3));
         aToken = nextToken();
         surfDeadGT3->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfDeadGT3));
         aToken = nextToken();
         surfDead3to6->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfDead3to6));
         aToken = nextToken();
         surfDead6to12->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfDead6to12));
         aToken = nextToken();
         surfDeadGT12->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfDeadGT12));
         aToken = nextToken();
         surfLiveHerb->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfLiveHerb));
         aToken = nextToken();
         surfLiveShrub->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfLiveShrub));
         aToken = nextToken();
         surfTotal->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),surfTotal));
         aToken = nextToken();
         standDead0To3->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standDead0To3));
         aToken = nextToken();
         standDeadGT3->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standDeadGT3));
         aToken = nextToken();
         standLiveFol->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standLiveFol));
         aToken = nextToken();
         standLive0To3->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standLive0To3));
         aToken = nextToken();
         standLiveGT3->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standLiveGT3));
         aToken = nextToken();
         standTotal->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standTotal));
         aToken = nextToken();
         totalFuels->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),totalFuels));
         aToken = nextToken();
         totalConsumption->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),totalConsumption));
         aToken = nextToken();
         standingRemoved->itsObservations.insert(new FVSObservation
                       (aToken.data(),year.data(),standingRemoved));
      }
   }
   else
   {
      // we have a new report...see if the current case already has
      // this report...if not, make a new one.

      if (strstr(theRecord,"                 ESTIMATED FUEL LOADINGS"))
      {
         const char * name = "All fuels report";
         parseCounter = 0;
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


