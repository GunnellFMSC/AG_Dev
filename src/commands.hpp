/********************************************************************

   File Name:     commands.hpp
   Author:        nlc
   Date:          04/20/94

 see variants.hpp for notes.
 
 ********************************************************************/

#ifndef __SpCommands_HPP
#define __SpCommands_HPP

#include "PwrDef.h"

class PKeyData;
class MSText;
class RWOrdered;

class SpCommands
{
   public:

      SpCommands ( void );
     ~SpCommands ( void );

      int            LoadCommandCollection (RWOrdered * theCommandList);
      int            LoadListOfLegalCommands( void );
      int            FindLegalCommand( const char * theCommand );
      int            FindCommandThatContainsElement( CStringRWC * theElement );
      int            FindCommandThatDoesNotContainElement( CStringRWC * theElement );
      void           PickCommand( int OkToLock = 0 );
      const char *   GetCommand( void );

      int            theSelectedCommand;
      int            numberCommands;
      MSText       * commandsMSTextPointer;
      int            commandSelectionLocked;

      typedef PKeyData  * ptrToPKeyData;
      ptrToPKeyData * listOfCommandsPKeyData;

};


#endif
