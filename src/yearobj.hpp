/********************************************************************

   File Name:     yearobj.h
   Author:        kfd & nlc
   Date:          08/25/94 & 04/05/95 & 02/08/96

   Class:         YearObj
   Inheritance:   none

   Purpose:       An instance of this object is used to initialize and store
                  default beginning simulation year, simulation interval
                  and selected year, and related data.

********************************************************************/

#ifndef YearObj_H
#define YearObj_H

#include "PwrDef.h"

#include CStringRW_i


class CStringRW;
class PKeyData;

class YearObj
{
   public:

           YearObj();
      int  GetStartYear(void);
      int  SetStartYear(int proposedYear);
      int  GetSelectedYear(void);
      void SetSelectedYear(int year);
      int  GetEndYear(void);
      int  SetEndYear(int proposedYear);
      void AdjustInvYearRange (int year);
      void SetStartAndEndFromTopSection(const char * Comment);
      int  GetInvYear(int which);
      int  currentYear;
      int  GetCycleLength(void);
      int  SetCycleLength(int clen);
      void InitCycleLength(void);
      int  SimLength(void);
      int  InitMaxCycles(void);
      int  GetMaxCycles(void);
      void VariantSelectionChanged();

   private:
      int endYear;
      int startYear;
      int yearInterval;
      int selectedYear;
      int cycleLength;
      int simLength;
      int maxInvYear;
      int minInvYear;
      int cycleLenSet;
      int maxCycles;

};


#endif







