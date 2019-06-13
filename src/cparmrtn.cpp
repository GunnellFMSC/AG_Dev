/********************************************************************

   File Name:     cparmrtn.cpp
   Author:        kfd, nlc 10/2006
   Date:          02/95


   Contents:

     StuffCParm   returns: nothing
                  input:   CStringRW & theCParmString -- the growing CParms
                           const char * theString     -- the string to stuff

     GetCParm     returns: CStringRW             -- the desired set of CParms
                  input:   const char * theCParm -- the CParm string
                           int theIndex          -- index of desired CParm set

********************************************************************/
#include "appdef.hpp"

#include CStringRW_i
#include <rw/ctoken.h>
#include "cparmrtn.hpp"


CStringRW GetCParm(const char * theCParm, int theIndex)
{
   CStringRW theString;
   RWCTokenizer next(theCParm);
   int i=0;
   while (!(theString=next("~")).isNull())
   {
      if (i == theIndex) return theString;
      i++;
   }
   return NULLString;
}


void StuffCParm(CStringRW & theCParmString, const char * theString)
{

   theCParmString.append(theString);
   theCParmString.append('~');       // this is the end of parm set marker
   return;
}
