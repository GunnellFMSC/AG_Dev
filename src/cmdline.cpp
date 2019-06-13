/********************************************************************

   File Name:     cmdline.cpp
   Author:        nlc
   Date:          11/30/94

   See cmdline.hpp for information.

********************************************************************/

#include "cmdline.hpp"
#include <string.h>
#include "warn_dlg.hpp"
#include "dirsept.hpp"

#if XVTWS==WIN32WS
#include <stdlib.h>
#endif

CommandLine::CommandLine (int argc, char * argv[], char * envp[])
            :commandargc(argc),
             fvsbinPtr(NULL)
{
   commandargv = (const char **) argv;	  // command argument vector
   commandenvp = (const char **) envp;
}




const char * CommandLine::getSimFileName( void )
{                                         // simfile name is the last arg.
   if (commandargc > 1 &&
      *commandargv[commandargc-1] != '-') return commandargv[commandargc-1];
   return (const char *) 0;
}



const char * CommandLine::getLocFileName( void )
{

   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"locsfile=",9) == 0 &&
             isgraph(*(here+9))) return here+9;
      }
   }
   return (char *) 0;
}



const char * CommandLine::getParmsFileName( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"parmsfile=",10) == 0 && isgraph(*(here+10))) 
			 return here+10;
      }
   }
   return (char *) 0;
}



const char * CommandLine::getPreferencesFileName( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"prefsfile=",10) == 0 &&
             isgraph(*(here+10))) return here+10;
      }
   }
   return (char *) 0;
}



const char * CommandLine::getPPEYesNo( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"PPE=",4) == 0 &&
             isgraph(*(here+4))) return here+4;
      }
   }
   return (char *) 0;
}

const char * CommandLine::getSegCycleYesNo( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"segcycle=",9) == 0 &&
             isgraph(*(here+9))) return here+9;
      }
   }
   return (char *) 0;
}

const char * CommandLine::getProcessPlotsYesNo( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"processplots=",13) == 0 &&
             isgraph(*(here+13))) return here+13;
      }
   }
   return (char *) 0;
}

const char * CommandLine::getDefVariant( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"defvariant=",11) == 0 &&
             isgraph(*(here+11))) return here+11;
      }
   }
   return (char *) 0;
}




const char * CommandLine::getTheVariant( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"variant=",8) == 0 &&
             isgraph(*(here+8))) return here+8;
      }
   }
   return (char *) 0;
}




const char * CommandLine::getDataBaseName( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"database=",10) == 0 &&
             isgraph(*(here+10))) return here+10;
      }
   }
   return (char *) 0;
}




const char * CommandLine::getEditor( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"editor=",7) == 0 &&
             isgraph(*(here+7))) return here+7;
      }
   }
   return (char *) 0;
}



const char * CommandLine::getFVSBIN( void )
{

   if (fvsbinPtr) return fvsbinPtr;

   // 1. check the command line for -fvsbin=

   if (!fvsbinPtr)
   {
      for (int i=1; i<commandargc; i++)
      {
         if (*commandargv[i] == '-')
         {
            const char * here = commandargv[i]+1;
            if (strncmp(here,"fvsbin=",7) == 0 &&
                isgraph(*(here+7)))
            {
               fvsbinPtr = here+7;
               return fvsbinPtr;
            }
         }
      }
   }

#if XVTWS!=WIN32WS

   // 2. check for an environment variable FVSBIN=

   if (!fvsbinPtr)
   {
      const char * here;
      for (int i=0; here = commandenvp[i]; i++)
      {
         if (strncmp(here,"FVSBIN=",7) == 0 &&
             isgraph(*(here+7)))
         {
            fvsbinPtr = here+7;
            return fvsbinPtr;
         }
      }
   }

   // 3. check for an environment variable FVSHOME=

   if (!fvsbinPtr)
   {
      const char * here;
      for (int i=0; here = commandenvp[i]; i++)
      {
         if (strncmp(here,"FVSHOME=",8) == 0 &&
             isgraph(*(here+8)))
         {
            fvsbinPtr = here+8;
            return fvsbinPtr;
         }
      }
   }

#else

   // 2. WIN32 check for an environment variable FVSBIN=

   if (!fvsbinPtr) fvsbinPtr = getenv("FVSBIN=");

   // 3. WIN32 check for an environment variable FVSHOME=

   if (!fvsbinPtr)fvsbinPtr = getenv("FVSHOME=");

   // 4. if WIN32 (only), then path name of the program (argv[0]).

   if (!fvsbinPtr)
   {
      strcpy(fvsbinCopy,commandargv[0]);
      char * sepPtr = SEPARATOR;
      char * lastBackShash = strrchr(fvsbinCopy,(int) *sepPtr);
      *lastBackShash = 0;
      fvsbinPtr = fvsbinCopy;
      return fvsbinPtr;
   }

#endif

   return fvsbinPtr;
}




const char * CommandLine::getHelpFileName( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"helpfile=",9) == 0 &&
             isgraph(*(here+9))) return here+9;
      }
   }
   return (char *) 0;
}


const char * CommandLine::getStartUpRect( void )
{
   for (int i=1; i<commandargc; i++)
   {
      if (*commandargv[i] == '-')
      {
         const char * here = commandargv[i]+1;
         if (strncmp(here,"startrect=",10) == 0 &&
             isgraph(*(here+10))) return here+10;
      }
   }
   return (char *) 0;
}



