/********************************************************************

   File Name:    spfuncs.hpp
   Author:       nlc
   Date:         12/20/94

   Purpose:  These are regular functions used for various purposes

   Contents:

      BuildField
      BuildTheAnswer
      LoadRWOrdered
 *                                                                  *
 ********************************************************************/

#ifndef spfuncs_H
#define spfuncs_H

#include "spparms.hpp"
#include "variants.hpp"
#include "spglobal.hpp"

class CWindow;
class CSubview;
class MSText;
class RWOrdered;
class CStringRW;
class PKeyData;
class Component;

#include <float.h>

void * BuildField (CWindow * theParent, int theCommandNumber,
                   CSubview * theWindow, MSText * theMSText,
                   int * startPoint, int fieldNumber,
                   int * fieldType);

int BuildTheKeywords (MSText * theMSText, int * theFieldTypes,
                      void  ** theAnswerPtrs, int numberOfFields,
                      int typeOfAnswer, CStringRW & theAnswerString);

int LoadRWOrdered(const char * theString, RWOrdered * theList);

PKeyData * GetPKeyData(const char * stringOfMSText,
                       const char * stringOfPKey,
                       const char * stringOfATList =
                          (const char *) theSpGlobals->theVariants->
                                         GetSelectedVariant(),
                       int matchMethod = 0,
                       int postWarn    = 1);

MSText * GetMSTextPtr(const char * MSTextName = NULL);

void FreePKeyData(const char * stringOfMSText = NULL);

void ResetObjects (int * theFieldTypes = NULL, void ** theAnswerPtrs = NULL,
                   int numberOfFields = 0, int offSet = 0,
                   const char * theCParms = NULL);


void BuildCParms (int * theFieldTypes, void ** theAnswerPtrs,
                  int numberOfFields, CStringRW & theCParms);


char * AddToString(char * theAnswer,
                   const char * ptrChar,
                   unsigned int * usedCount,
                   unsigned int * sizeOfTheAnswer,
                   unsigned int stdToAdd = 1000);

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#endif
