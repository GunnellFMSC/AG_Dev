/********************************************************************

   File Name:     extens.hpp
   Author:        nlc
   Date:          04/20/94; revised 11/18/2005 nlc

   see variants.hpp for notes.

 ********************************************************************/

#ifndef __SpExtensions_HPP
#define __SpExtensions_HPP

#include "PwrDef.h"

class PKeyData;
class MSText;
class RWOrdered;
class CStringRWC;

class SpExtensions
{
   public:

      SpExtensions ( void );
      virtual ~SpExtensions ( void );

      void           LoadExtensionCollection (RWOrdered * theExtensionList);
      int            LoadListOfLegalExtensions( void );
      int            FindLegalExtension( const char * theExtension );
      void           PickExtension(int OkToLock = 0);
      int            AddExtensionInUse ( const char * theExtensionn,
                                         int OkToLockPgm = 1);
      int            AddExtensionInUse ( CStringRWC & theExtensionn,
                                         int OkToLockPgm = 1);

      int            numberExtensions;
      int            extensionSelectionLocked;
      int          * isExtensionInSelectedProgram;
      MSText       * extensionsMSTextPointer;
      RWOrdered    * listOfExtensionsInUse;
      void           TokenizedExtensions( const char * theExtension,
                                          RWOrdered  * theList);

      typedef PKeyData  * ptrToPKeyData;
      ptrToPKeyData     * listOfExtensionsPKeyData;

   private:

      int            FindMultiExtensions( const char * theExtension );

};


#endif
