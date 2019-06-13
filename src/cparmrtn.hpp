/********************************************************************

   File Name:     cparmrtn.hpp
   Author:        kfd
   Date:          02/95


   Contents:

     StuffCParm   returns: nothing
                  input:   CStringRW & theCParmString -- the growing CParms
                           const char * theString     -- the string to stuff

     GetCParm     returns: CStringRW             -- the desired set of CParms
                  input:   const char * theCParm -- the CParm string
                           int theIndex          -- index of desired CParm set

********************************************************************/
#ifndef CParmRtn_H
#define CParmRtn_H

   CStringRW GetCParm(const char * theCParm, int theIndex);
   void StuffCParm(CStringRW & theCParmString, const char * theString);

#endif // CParmRtn_H
