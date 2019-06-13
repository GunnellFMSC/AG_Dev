/********************************************************************

   File Name:     readfire.hpp
   Author:        nlc
   Date:          11/18/97

   Classes:       ParseFirePotentialFlameReport
                  ParseFireAllFuelsReport
                  ParseFireFuelConsumptionReport (not yet implimented)
                  ParseFireTreeMortalityReport   (not yet implimented)

   Purpose:  Parses the reports generated by the Fuels and Fire
             effects extension.
 *                                                                  *
 ********************************************************************/

#ifndef ParseFireReports_H
#define ParseFireReports_H

class ParseFirePotentialFlameReport
{
   public:

      ParseFirePotentialFlameReport( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      int parseCounter;

      FVSReport   * itsCurrentFVSReport;
      FVSVariable * flameLenSurSev;
      FVSVariable * flameLenSurMod;
      FVSVariable * flameLenTotSev;
      FVSVariable * flameLenTotMod;
      FVSVariable * fireTypeSev;
      FVSVariable * fireTypeMod;
      FVSVariable * pTorchSev;
      FVSVariable * pTorchMod;
      FVSVariable * torchIndex;
      FVSVariable * crownIndex;
      FVSVariable * crownBaseHeight;
      FVSVariable * crownBulkDensity;
      FVSVariable * potMortSev;
      FVSVariable * potMortMod;
      FVSVariable * potVolSev;
      FVSVariable * potVolMod;
      FVSVariable * potSmokeSev;
      FVSVariable * potSmokeMod;
      FVSVariable * fuelModel;
};

class ParseFireAllFuelsReport
{
   public:

      ParseFireAllFuelsReport( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      int parseCounter;

      FVSReport   * itsCurrentFVSReport;
      FVSVariable * surfDeadLitter;
      FVSVariable * surfDeadDuff;
      FVSVariable * surfDead0To3;
      FVSVariable * surfDeadGT3;
      FVSVariable * surfDead3to6;
      FVSVariable * surfDead6to12;
      FVSVariable * surfDeadGT12;
      FVSVariable * surfLiveHerb;
      FVSVariable * surfLiveShrub;
      FVSVariable * surfTotal;
      FVSVariable * standDead0To3;
      FVSVariable * standDeadGT3;
      FVSVariable * standLiveFol;
      FVSVariable * standLive0To3;
      FVSVariable * standLiveGT3;
      FVSVariable * standTotal;
      FVSVariable * totalFuels;
      FVSVariable * totalConsumption;
      FVSVariable * standingRemoved;
};


class ParseFireFuelConsumptionReport        // (not yet implimented)
{
   public:

      ParseFireFuelConsumptionReport( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      int parseCounter;

      FVSReport   * itsCurrentFVSReport;
      FVSVariable * percentMineralSoilExposure;
      FVSVariable * consumption0To3;
      FVSVariable * consumptionGT3;
      FVSVariable * consumption3to6;
      FVSVariable * consumption6to12;
      FVSVariable * consumptionGT12;
      FVSVariable * consumptionHerbAndShrub;
      FVSVariable * consumptionCrowns;
      FVSVariable * consumptionTotal;
      FVSVariable * percentConsumptionDuff;
      FVSVariable * percentConsumptionGT3;
      FVSVariable * percentCrowning;
      FVSVariable * smokeLT25;
      FVSVariable * smokeLT10;
};

class ParseFireTreeMortalityReport          // (not yet implimented)
{
   public:

      ParseFireTreeMortalityReport( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      int parseCounter;

      FVSReport   * itsCurrentFVSReport;

      FVSVariable * killed0To5;
      FVSVariable * before0To5;
      FVSVariable * killed5To10;
      FVSVariable * before5To10;
      FVSVariable * killed10To20;
      FVSVariable * before10To20;
      FVSVariable * killed20To30;
      FVSVariable * before20To30;
      FVSVariable * killed30To40;
      FVSVariable * before30To40;
      FVSVariable * killed40To50;
      FVSVariable * before40To50;
      FVSVariable * killedGe50;
      FVSVariable * beforeGe50;

};


#endif
