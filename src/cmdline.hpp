/********************************************************************

   File Name:     cmdline.hpp
   Author:        nlc
   Date:          11/30/94

   Contents:      CommandLine
                  getCommandLineArgc
                  getCommandLineArgv
                  getSimFileName
                  getLocFileName
                  getParmsFileName

   Purpose:       To process and house the command line data.


 ********************************************************************/

#ifndef __CommandLine_H
#define __CommandLine_H

#include "PwrDef.h"

class CommandLine
{
   public:

                    CommandLine (int argc, char * argv[], char * envp[]);
      int           getCommandLineArgc( void );
      const char ** getCommandLineArgv( void );
      const char ** getCommandLineEnvp( void );
      const char *  getSimFileName( void );
      const char *  getLocFileName( void );
      const char *  getParmsFileName( void );
      const char *  getPreferencesFileName( void );
      const char *  getPPEYesNo( void );
      const char *  getDefVariant( void );
      const char *  getTheVariant( void );
      const char *  getDataBaseName( void );
      const char *  getEditor( void );
      const char *  getSegCycleYesNo( void );
	  const char *  getProcessPlotsYesNo( void );
	  const char *  getFVSBIN( void );
      const char *  getHelpFileName( void );
      const char *  getStartUpRect( void );

   private:

      int commandargc;
      const char ** commandargv;
      const char ** commandenvp;
      char fvsbinCopy[SZ_FNAME];
      const char * fvsbinPtr;
};



inline int CommandLine::getCommandLineArgc( void )
{
   return commandargc;
}



inline const char ** CommandLine::getCommandLineArgv( void )
{
   return commandargv;
}


inline const char ** CommandLine::getCommandLineEnvp( void )
{
   return commandenvp;
}


#endif
