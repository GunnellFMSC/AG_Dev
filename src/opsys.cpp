/********************************************************************

   File Name:     opsys.cpp
   Author:        nlc
   Date:          02/11/98

********************************************************************/

#ifdef AIXV4
#include <stdlib.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <iostream>
#include <fstream>

#include "opsys.hpp"

void OpSys (const char * theCommand)
{

#ifdef AIXV4

   system (theCommand);

#else

   WinExec(theCommand, SW_SHOWNORMAL);

#endif
}


void SpChangeDirectory (const char * theDirectory)
{

   if (!theDirectory) return;

#ifdef AIXV4

    if (chdir (theDirectory)) 
        cout << "Changing to " << theDirectory << " failed." << std::endl;

#else

    if (!SetCurrentDirectory	(theDirectory))
    {
        std::ofstream error("suppose.err");
        error	<< "Changing to	" << theDirectory << " failed."	<< std::endl;
        error	<< "Error code is "	<< GetLastError() << std::endl;
    }
      
#endif
}


