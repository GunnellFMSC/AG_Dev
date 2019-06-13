/*********************************************************************

   File Name:    slfrec.cpp
   Author:       nlc
   Date:         04/29/97

   This file uses the editslf.hpp header file for its definitions.

 *********************************************************************/

#include "PwrDef.h"

#include "spparms.hpp"
#include "spglobal.hpp"
#include "variants.hpp"
#include "editslf.hpp"


RWDEFINE_COLLECTABLE(SLFRecord,EDITSTAND)


SLFRecord::SLFRecord(char * record)
          :CStringRWC(record)
{
   variants=theSpGlobals->theVariants->GetSelectedVariant();
}


SLFRecord::SLFRecord(SLFRecord * c)
          :CStringRWC ((CStringRWC &) * c)
{
   // copy constructor

   FVSTreeData          = c->FVSTreeData;
   samplePointDataFlag  = c->samplePointDataFlag;
   variants             = c->variants;
   invYear              = c->invYear;
   lat                  = c->lat;
   longitude            = c->longitude;
   loc                  = c->loc;
   hab                  = c->hab;
   originYear           = c->originYear;
   aspect               = c->aspect;
   slope                = c->slope;
   elev                 = c->elev;
   baf                  = c->baf;
   fixed                = c->fixed;
   breakpoint           = c->breakpoint;
   nPlots               = c->nPlots;
   nNonStock            = c->nNonStock;
   sampWt               = c->sampWt;
   pNonSt               = c->pNonSt;
   DGTransCode          = c->DGTransCode;
   DGperiod             = c->DGperiod;
   HGTransCode          = c->HGTransCode;
   HGperiod             = c->HGperiod;
   mortPeriod           = c->mortPeriod;
   BAMax                = c->BAMax;
   SDIMax               = c->SDIMax;
   siteSpecies          = c->siteSpecies;
   siteIndex            = c->siteIndex;
   modelType            = c->modelType;
   physRegion           = c->physRegion;
   forestType           = c->forestType;
   state                = c->state;
   county               = c->county;
   pvRefCode            = c->pvRefCode;

   groupCodes           = c->groupCodes;
   addFiles             = c->addFiles;
   standCN              = c->standCN;
}



SLFRecord::SLFRecord( void )
          :CStringRWC ( )
{
   variants=theSpGlobals->theVariants->GetSelectedVariant();
}



SLFRecord::SLFRecord (CStringRW & name)
          :CStringRWC (name)
{
   variants=theSpGlobals->theVariants->GetSelectedVariant();
}



RWBoolean SLFRecord::isEqual(const RWCollectable * t) const
{
   return this == t;
}


void SLFRecord::WriteRecord (FILE * SLFFile)
{

   if (!SLFFile) return;

   // Record type A

   variants = variants.strip(RWCString::both);
   fprintf(SLFFile,"A %s %s %s %s @\n",data(),OutAt(FVSTreeData),
           OutAt(samplePointDataFlag),variants.data());

   // Record type B

   fprintf(SLFFile,"B %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s "
           "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
           data(),OutAt(invYear),OutAt(lat),
           OutAt(longitude),OutAt(loc),OutAt(hab),OutAt(originYear),
           OutAt(aspect),OutAt(slope),OutAt(elev),OutAt(baf),
           OutAt(fixed),OutAt(breakpoint),OutAt(nPlots),
           OutAt(nNonStock),OutAt(sampWt),OutAt(pNonSt),
           OutAt(DGTransCode),OutAt(DGperiod),OutAt(HGTransCode),
           OutAt(HGperiod),OutAt(mortPeriod),OutAt(BAMax),
           OutAt(SDIMax),OutAt(siteSpecies),OutAt(siteIndex),
           OutAt(modelType),OutAt(physRegion),OutAt(forestType),
           OutAt(state),OutAt(county),OutAt(pvRefCode));

   // Record type C

   groupCodes = groupCodes.strip(RWCString::both);
   if (groupCodes.length() &&
       groupCodes.first('@') != 0) fprintf(SLFFile,"C %s %s @\n",
                                           data(),groupCodes.data());

   // Record type D

   addFiles = addFiles.strip(RWCString::both);
   if (addFiles.length() &&
       addFiles.first('@') != 0) fprintf(SLFFile,"D %s %s @\n",
                                         data(),addFiles.data());
   // Record type E

   standCN = standCN.strip(RWCString::both);
   if (standCN.length() &&
       standCN.first('@') != 0) fprintf(SLFFile,"E %s %s\n",
                                         data(),standCN.data());
}


const char * SLFRecord::OutAt (CStringRW& theString)
{
   theString = theString.strip(RWCString::both);
   if (theString.isNull()) return "@";
   else
   {
      size_t i=0;
      while (i<theString.length())
      {
         if (iscntrl(theString(i))) theString.remove(i);
         i++;
      }
   }
   if (theString.isNull()) return "@";
   else return theString.data();
}

