/********************************************************************

   File Name:     opsys.hpp
   Author:        nlc
   Date:          09/02/98

   provides a very simple interface to the operating system. 

********************************************************************/

#ifndef __OPSYS_H
#define __OPSYS_H

#include <fstream>

void OpSys (const char * theCommand);
void SpChangeDirectory (const char * theDirectory);

#endif
