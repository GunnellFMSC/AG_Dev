/*********************************************************************

   File Name:    dirsept.hpp
   Author:       nlc
   Date:         03/15/96

   Defines the character used to separate levels of file names.

 *********************************************************************/

#include "xvt.h"

#if XVTWS==WIN32WS
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif
