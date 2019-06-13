/********************************************************************

   File Name:     yearobj.cpp
   Author:        kfd & nlc
   Date:          08/25/94 & 04/05/95

********************************************************************/

#include <rw/rwdate.h>

#include "appdef.hpp"
#include "spfunc2.hpp"

#include "yearobj.hpp"
#include "spfuncs.hpp"
#include "spparms.hpp"
#include "suppdefs.hpp"


YearObj::YearObj()
{
   // YearObj constructor; initializes private variables

   RWDate rwdate;
   currentYear  = (int) rwdate.year();

   endYear      = SpNEGMAX;
   selectedYear = SpNEGMAX;
   maxInvYear   = SpNEGMAX;
   minInvYear   = SpPOSMAX;
   startYear    = maxInvYear;
   cycleLenSet  = 0;
   InitCycleLength();
   InitMaxCycles();
   SimLength();
}




int YearObj::GetCycleLength()
{
   return cycleLength;
}


int YearObj::SetCycleLength(int clen)
{
   if (clen == 0 || clen > 50) return cycleLength;
   cycleLenSet = 1;
   cycleLength = clen;
   return cycleLength;
}



void YearObj::InitCycleLength()
{
   if (!cycleLenSet)
   {
      PKeyData * cycleLenPKeyData = GetPKeyData("timing", "cycleLength");
      if (cycleLenPKeyData) sscanf (cycleLenPKeyData->pString.data(),
                                    "%d", &cycleLength);
      else cycleLength=10;
   }
}


int YearObj::SimLength()
{
   PKeyData * simLenPKeyData   = GetPKeyData("timing", "simLength");
   if (simLenPKeyData) sscanf (simLenPKeyData->pString.data(),
                               "%d", &simLength);
   else simLength = 100;
   return simLength;
}



int YearObj::InitMaxCycles()
{
   PKeyData * maxCyclesPKeyData = GetPKeyData("timing", "maxCycles");
   if (maxCyclesPKeyData) sscanf (maxCyclesPKeyData->pString.data(),
                                  "%d", &maxCycles);
   else maxCycles=40;
   return maxCycles;
}

int YearObj::GetMaxCycles()
{
   return maxCycles;
}


void YearObj::AdjustInvYearRange (int year)
{
   maxInvYear = max(year, maxInvYear);
   minInvYear = min(year, minInvYear);
   if (startYear < maxInvYear) startYear = maxInvYear;
   SetEndYear(startYear + SimLength());
}



int YearObj::SetEndYear(int proposedYear)
{
   if (startYear != SpNEGMAX)
   {
      if (proposedYear >= startYear &&
          proposedYear <= startYear+GetEndYear());
         endYear = proposedYear;
      return endYear;
   }
   else
   {
      endYear = proposedYear;
   }
   return endYear;

}



int YearObj::GetEndYear(void)
{
   // return ending year of simulation.

   if (endYear == SpNEGMAX)
   {
      if (maxInvYear != SpNEGMAX) return maxInvYear+simLength;
      else return currentYear+SimLength();
   }
   else return endYear;
}



void YearObj::SetSelectedYear(int year)
{
   selectedYear = year;
}



int YearObj::GetSelectedYear(void)
{
   if (selectedYear == SpNEGMAX)
   {
      if (maxInvYear != SpNEGMAX) return maxInvYear;
      else return currentYear;
   }
   else return selectedYear;
}



int YearObj::GetStartYear(void)
{
   if (startYear  != SpNEGMAX) return startYear;
   if (maxInvYear != SpNEGMAX) return maxInvYear;
   else return currentYear;
}




int YearObj::SetStartYear(int proposedYear)
{
   if (maxInvYear != SpNEGMAX)
   {
      if (proposedYear >= maxInvYear &&
          proposedYear <= maxInvYear+(GetCycleLength()*5))
          startYear = proposedYear;
      return startYear;
   }
   else
   {
      startYear = proposedYear;
      return proposedYear;
   }
}



int YearObj::GetInvYear(int which)
{
   if (which == 1 &&
       minInvYear != SpPOSMAX) return minInvYear;
   if (which == 2) return maxInvYear;
   return -1;
}



void YearObj::SetStartAndEndFromTopSection(const char * topKeywords)
{
   // scan the top keywords for the timing information and set
   // the values accordingly

   char  startString[]="Starting year for simulation is ";
   char    endString[]="Ending year for simulation is   ";
   char recentString[]="Min and Max inventory years are ";
   char  cycleLenStr[]="Common cycle length is          ";

   const char * startptr  = strstr(topKeywords,startString);
   const char * endptr    = strstr(topKeywords,endString);
   const char * recentptr = strstr(topKeywords,recentString);
   const char * cyclenptr = strstr(topKeywords,cycleLenStr);

   if (startptr)
   {
      startptr  += strlen(startString);
      sscanf(startptr,"%d",&startYear);
   }
   if (endptr)
   {
      endptr += strlen(endString);
      sscanf(endptr,"%d",&endYear);
   }
   if (recentptr)
   {
      recentptr += strlen(recentString);
      sscanf(recentptr,"%d%d", &minInvYear, &maxInvYear);
   }
   if (cyclenptr)
   {
      cycleLenSet = 1;
      cyclenptr += strlen(cycleLenStr);
      sscanf(cyclenptr,"%d",&cycleLength);
   }
}


void YearObj::VariantSelectionChanged()
{
   SimLength();
   InitCycleLength();
   InitMaxCycles();
}

