/********************************************************************

   File Name:    readcomp.hpp
   Author:       nlc
   Date:         03/27/96

   Purpose:  This pure function reads a file of components and saves
             them in an RWOrdered set.

 ********************************************************************/

#ifndef ReadComponentsFromFile_H
#define ReadComponentsFromFile_H

class RWOrdered;

void ReadComponentsFromFile (FILE      * filePtr,
                             FILE_SPEC * fileSpec,
                             RWOrdered * componentsFromFile,
                             int         assumeComponentsAreUsed = 1);

#endif
