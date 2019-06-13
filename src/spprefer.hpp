/********************************************************************

   File Name:     spprefer.hpp
   Author:        nlc
   Date:          11/30/94

   Contents:      SpPreferences

   Purpose:       To process and house Suppose Preferences.


 ********************************************************************/

#ifndef __SpPrefer_HPP
#define __SpPrefer_HPP


#ifndef __SPPARMS_H
#include "spparms.hpp"
#define __SPPARMS_H
#endif

class SpPreferences
{
   public:

      SpPreferences( void );

      void upDateDefaultsFromParmsFile( void );
      void upDateDefaultsFromCommandLine( void );
      void upDateDefaultsFromPrefFile( void );
      void saveDefaultsToPrefFile( void );

      RWCString defaultVariant;
      RWCString defaultLocationsFileName;
      RWCString tutorialLocationsFileName;
      RWCString defaultDataBaseName;
      RWCString defaultParametersFileName;
      RWCString defaultEditor;
      int defaultUsePPE;
      int defaultSegmentCycle;
      int defaultProcessPlots;
      RWCString defaultWorkingDirectory;

};

#endif
