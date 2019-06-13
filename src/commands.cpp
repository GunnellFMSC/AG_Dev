/********************************************************************

   File Name:     commands.cpp
   Author:        nlc
   Date:          12/07/94

 see variants.hpp for notes.

 ********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include CStringRWC_i

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>


#include "appdoc.hpp"
#include "variants.hpp"
#include "commands.hpp"
#include "spglobal.hpp"
#include "programs.hpp"
#include "spprefer.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"

#include "warn_dlg.hpp"

SpCommands::SpCommands( void )
{
   theSelectedCommand = -1;
   commandSelectionLocked = 0;
   listOfCommandsPKeyData = NULL;

   // make a list of the possible commands... this list is taken from the
   // "commands" in the parameters file.

   CStringRWC commandString = "commands";
   commandsMSTextPointer = NULL;
   if (theSpGlobals->theParmsData->entries() != NULL)
   {
      commandsMSTextPointer = (MSText *) theSpGlobals->
         theParmsData->find(&commandString);
      if (commandsMSTextPointer) commandsMSTextPointer->GetThePKeys();
   }

   // if commandsMSTextPointer is NULL, then Suppose could not find a list of
   // commands.  ... or, if the list is empty, then close the application.

   numberCommands = -1;
   if (commandsMSTextPointer) numberCommands = 
      commandsMSTextPointer->GetThePKeys()->entries();
   if (numberCommands < 0)
   {
      Warn("FATAL ERROR:\n"
           "Suppose could not find a list of Run Commands\n"
           "in the parameters file.  Suppose will exit.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }

   listOfCommandsPKeyData = new ptrToPKeyData [numberCommands];
   
   LoadListOfLegalCommands();

   if (numberCommands == 0)
   {
      Warn ("FATAL ERROR:\nThere are no legal Run Commands.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }


}





SpCommands::~SpCommands( void )
{
   if (listOfCommandsPKeyData) delete []listOfCommandsPKeyData;
}




int SpCommands::LoadListOfLegalCommands( void )
{

   // a command is legal if it contains a reference to the OS in its
   // ATList... the reference MUST be present.  We can not use 
   // FindPKeyData because the list will also contain other elements.

   int theListCounter = 0;
   CStringRWC searchElement = theSpGlobals->OS;
   
   RWOrderedIterator nextPKeyData(commandsMSTextPointer->thePKeys);
   PKeyData * onePKeyData;
   while (onePKeyData = (PKeyData *) nextPKeyData())
   {
      if (onePKeyData->aTList.contains(&searchElement))
         listOfCommandsPKeyData[theListCounter++] = onePKeyData;
   }
   numberCommands = theListCounter;
   if (numberCommands == 1)
   {
      commandSelectionLocked = 1;
      theSelectedCommand = 0;
   }
   return numberCommands;
}






int SpCommands::LoadCommandCollection (RWOrdered * theCommandList)
{

   for (int i=0; i<numberCommands; i++)
   {
      CStringRWC * cmd = new CStringRWC(listOfCommandsPKeyData[i]->pString);
      size_t nL = cmd->first('\n');
      if (nL != RW_NPOS) cmd->remove(nL); 
      theCommandList->insert(cmd);
   }
   return theSelectedCommand;
}






int SpCommands::FindCommandThatContainsElement(CStringRWC * theElement)
{

   for (int i=0; i<numberCommands; i++)
   {
      if (listOfCommandsPKeyData[i]->aTList.contains(theElement))
         return i;
   }
   return -1;
}






int SpCommands::FindCommandThatDoesNotContainElement(CStringRWC * theElement)
{

   for (int i=0; i<numberCommands; i++)
   {
      if (!listOfCommandsPKeyData[i]->aTList.contains(theElement))
         return i;
   }
   return -1;
}






int SpCommands::FindLegalCommand( const char * theCommand )
{
   for (int i=0; i<numberCommands; i++)
   {
      // note that for a PKeyData, data() is the PKey.

      if (strcmp(listOfCommandsPKeyData[i]->data(), theCommand) == 0)
         return i;
   }
   return -1;
}






void SpCommands::PickCommand( int OkToLock )
{

   if (commandSelectionLocked) return;

   int willWork;
   int numberCommandsThatWillWork = 0;
   int oneCommandThatWillWork = -1;
   CStringRWC searchPPE = "ppe";

   for (int i=0; i<numberCommands; i++)
   {
      willWork = 1;

      // does the command support theSelectedProgram?

      if (theSpGlobals->thePrograms->theSelectedProgram >= 0)
      {

         PKeyData * theProgramBeingConsidered = 
                   theSpGlobals->thePrograms->listOfProgramsPKeyData[
                   theSpGlobals->thePrograms->theSelectedProgram];
         if (theProgramBeingConsidered->aTList.contains(&searchPPE))
         {
            if (!listOfCommandsPKeyData[i]->aTList.contains(&searchPPE))
               willWork = 0;
         }
         else
         {
            if (listOfCommandsPKeyData[i]->aTList.contains(&searchPPE)) 
               willWork = 0;
         }
      }

      // count the programs that will work...point to the first one that will.

      if (willWork)
      {
         numberCommandsThatWillWork++;
         if (oneCommandThatWillWork < 0) oneCommandThatWillWork = i;
      }
   }

   if (numberCommandsThatWillWork > 0) theSelectedCommand = oneCommandThatWillWork;
   if (OkToLock                        &&
       numberCommandsThatWillWork == 1 &&
       theSpGlobals->thePrograms->programSelectionLocked) commandSelectionLocked = 1;
}




const char * SpCommands::GetCommand( void )
{
   PickCommand();
   if (theSelectedCommand > -1)
      return listOfCommandsPKeyData[theSelectedCommand]->pString.data();
   else return NULL;

}


