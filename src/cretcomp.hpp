/********************************************************************

   File Name:     cretcomp.hpp
   Author:        kfd & nlc
   Date:          01/30/95

*******************************************************************/

#ifndef CretComp_H
#define CretComp_H

#include "spglobal.hpp"
#include "variants.hpp"

class Component;

Component * CreateComponent(const char* itsName,
                    const char* itsScreen   = 0,
                    const char* itsKeywords = 0,
                    const char* itsCParms   = 0,
                    const char* itsComment  = 0,
                    const char* itsExtension= 0,
                    int         itsCType    = 0,
                    const char* itsVariant  =
                        theSpGlobals->theVariants->GetSelectedVariant());

#endif //CretComp_H
