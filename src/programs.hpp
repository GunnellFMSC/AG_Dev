/********************************************************************

   File Name:     programs.hpp
   Author:        nlc
   Date:          04/20/94

   see variants.hpp for notes.

 ********************************************************************/

#ifndef __SpPrograms_HPP
#define __SpPrograms_HPP

#include "PwrDef.h"

class PKeyData;
class MSText;
class RWOrdered;

class SpPrograms
{
   public:

      SpPrograms ( void );
     ~SpPrograms ( void );

      int          LoadProgramCollection (RWOrdered * theProgramList);
      int          LoadListOfLegalPrograms(int pgmChecking=1);
      int          FindLegalProgram( const char * theProgram );
      int          LockLegalProgram( int pgmToLock );
      int          FindProgramThatContainsElement( const char * theElement,
                                                   const char * theVariant = NULL);
      int          FindProgramThatContainsElement( CStringRWC * theElement,
                                                   const char * theVariant = NULL);
      int          SpPrograms::FindProgramThatContainsAllDesiredElements
                               (RWOrdered  & theDesiredElements,
                                const char * theVariant);
      int          SelectedProgramContainsElement( const char * theElement );
      int          SelectedProgramContainsElement( CStringRWC * theElement );
      int          SelectThisProgram(const char * pgm, int OkToLock = 0);
      int          SelectThisProgram(int pgm, int OkToLock);
      void         UpdateListOfLegalPrograms( void );
      void         PickProgram(int OkToLock = 0);
      const char * GetProgram( void );

      int          theSelectedProgram;
      int          numberPrograms;
      MSText     * programsMSTextPointer;
      int          programSelectionLocked;

      typedef PKeyData  * ptrToPKeyData;
      ptrToPKeyData     * listOfProgramsPKeyData;

};


#endif
