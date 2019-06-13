/********************************************************************

   File Name:     programs.cpp
   Author:        nlc
   Date:          12/07/94; revised 11/18/2005 nlc

   see programs.hpp for notes.

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
#include "programs.hpp"
#include "extens.hpp"
#include "spglobal.hpp"
#include "commands.hpp"
#include "cmdline.hpp"
#include "spprefer.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"
#include "sendupdt.hpp"
#include "dirsept.hpp"

#include "warn_dlg.hpp"


SpPrograms::SpPrograms( void )
{
   theSelectedProgram = -1;
   programSelectionLocked = 0;
   listOfProgramsPKeyData = NULL;

   // make a list of the possible programs... this list is taken from the
   // "programs" in the parameters file.

   CStringRWC programString = "programs";
   programsMSTextPointer = (MSText *) theSpGlobals->
      theParmsData->find(&programString);

   // if programsMSTextPointer is NULL, then Suppose could not find a
   // list of programs.  ... or, if the list is empty, then close the
   // application.

   numberPrograms = -1;
   if (programsMSTextPointer) numberPrograms =
       programsMSTextPointer->GetThePKeys()->entries();
   if (numberPrograms <= 0)
   {
      Warn("FATAL ERROR:\n"
           "Suppose could not find a list of FVS Programs\n"
           "in the parameters file.  Suppose will exit.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }

   listOfProgramsPKeyData = new ptrToPKeyData [numberPrograms];

   LoadListOfLegalPrograms(1);

   if (numberPrograms == 0)
   {
      RWCString warn = "WARNING:\nSuppose found no FVS Programs.\n"
                       "Directory searched: ";
      warn += theSpGlobals->theCommandLine->getFVSBIN();
      warn += "\n\nSuppose will continue running as if all FVS\n"
              "programs are installed on your computer.";
      Warn (warn.data());
      LoadListOfLegalPrograms(0);
   }

   if (numberPrograms == 0)
   {
      Warn ("FATAL ERROR: No legal FVS Programs.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }
}





SpPrograms::~SpPrograms( void )
{
   if (listOfProgramsPKeyData) delete []listOfProgramsPKeyData;
}





int SpPrograms::LoadListOfLegalPrograms(int pgmChecking)
{
/****

   To be initially legal, a program must contain the PPE if the PPE is being
   used or NOT contain the PPE if the PPE is not being used, and it must
   contain the designated variant from the command line.  NO assumptions as
   to the use of other extensions is made because the list of extensions has
   not been established yet.

 ****/

   int theListCounter = 0;
   int isLegal;

   const char * fvsbin = NULL;
   if (pgmChecking) fvsbin = theSpGlobals->theCommandLine->getFVSBIN();
   CStringRWC pgmFileName;

   if (fvsbin)
   {
      pgmFileName=fvsbin;
      pgmFileName+=SEPARATOR;
   }

   CStringRWC searchPPE = "ppe";
   CStringRWC search;
   RWOrderedIterator nextProgram(*programsMSTextPointer->GetThePKeys());
   PKeyData * oneProgram;
   while (oneProgram = (PKeyData *) nextProgram())
   {
      isLegal = 1;

      // if the program contains the PPE, a command that contains the PPE
      // must also be legal.

      if (oneProgram->aTList.contains(&searchPPE))
      {
         if (!(theSpGlobals->theCommands->
               FindCommandThatContainsElement(&searchPPE) >= 0))
            isLegal = 0;
      }
      else

      // if the program does not contain the PPE, a command that does not
      // contain the PPE must also be legal.

      {
         if (!(theSpGlobals->theCommands->
               FindCommandThatDoesNotContainElement(&searchPPE) >= 0))
            isLegal = 0;
      }

      // if the variant specified on the command line, the program must
      // contain the variant.

      if (isLegal)
      {
         if (theSpGlobals->theCommandLine->getTheVariant())
         {
            search = theSpGlobals->theCommandLine->getTheVariant();
            if (!(oneProgram->aTList.contains(&search))) isLegal = 0;
         }
      }

      // check to see if the program is on the users system.

      if (isLegal && fvsbin)
      {
         FILE_SPEC aFS;

#if XVTWS==WIN32WS
         search = *oneProgram;
         search += ".exe";
#else
         search = "FVS";
         search += *oneProgram;
#endif

         xvt_str_copy(aFS.name, search);
         xvt_str_copy(aFS.type, "");  // extension not needed in this case...
         xvt_fsys_convert_str_to_dir((char *)fvsbin, &aFS.dir);

         if (!xvt_fsys_get_file_attr(&aFS, XVT_FILE_ATTR_EXIST))
            isLegal=0;

      }

      // if the program isLegal...then count it!

      if (isLegal)
      {
         listOfProgramsPKeyData[theListCounter] = oneProgram;
         theListCounter++;
      }
   }
   numberPrograms = theListCounter;
   if (theListCounter == 1)
   {
      programSelectionLocked = 1;
      theSelectedProgram = 0;
      SendUpdateMessage (PROGRAMSelectionChanged);
   }
   return numberPrograms;
}






void SpPrograms::UpdateListOfLegalPrograms( void )
{
/*
 *  The update process...to be in the "new" list, the program
 *  1. Must already be legal.
 *  2. Must be based on a legal variant (or the only variant if
 *     variant is locked).
 *  3. If the PPE's use is locked and if the PPE is being used,
 *     then program must contain it.
 *  4. If the PPE's use is locked and if the PPE is NOT being used,
 *     then program must NOT contain it.
 *  5. The program must contain all extensions that are being used.
 */

   // if programSelectionLocked, then the list is already legal and *done*.

   if (programSelectionLocked) return;

   // allocate a new list...open a loop over all in the existing list of
   // legal programs.

   ptrToPKeyData * newListOfProgramsPKeyData =
      new ptrToPKeyData [numberPrograms];

   CStringRWC ppeSearch = "ppe";
   int numberProgramsInNewList = 0;
   CStringRWC * var;

   for (int i=0; i<numberPrograms; i++)
   {
      int isLegal = 1;

      // if the variants are locked, then the program must contain the
      // locked variant.

      if (theSpGlobals->theVariants->variantSelectionLocked)
      {
         var = theSpGlobals->theVariants->
            listOfVariantsPKeyData[theSpGlobals->theVariants->
                                  theSelectedVariant];
         isLegal = (int) listOfProgramsPKeyData[i]->aTList.contains(var);
      }
      else
      {

         // if the variants are not locked, then the program must contain
         // a legal variant.

         for (int variant = 0;
                  variant < theSpGlobals->theVariants->numberVariants;
                  variant++)
         {
            var = theSpGlobals->theVariants->listOfVariantsPKeyData[variant];
            isLegal = (int) listOfProgramsPKeyData[i]->aTList.contains(var);
            if (isLegal) break;
         }
      }

      // if the ppe is being used, does the program must contain it.

      if (isLegal && theSpGlobals->theVariants->PPEUseIsLocked)
      {
         if (theSpGlobals->theVariants->isPPEBeingUsed == 1)
            if (!listOfProgramsPKeyData[i]->aTList.contains(&ppeSearch))
               isLegal = 0;
      }

      // if the ppe is NOT being used, the program must NOT contain it?

      if (isLegal && theSpGlobals->theVariants->PPEUseIsLocked)
      {
         if (theSpGlobals->theVariants->isPPEBeingUsed == 0)
            if (listOfProgramsPKeyData[i]->aTList.contains(&ppeSearch))
               isLegal = 0;
      }

      // if a listOfExtensionsInUse has elements, then every legal program
      // must contain *ALL* of the elements in the listOfExtensionsInUse.

      if (isLegal &&
          theSpGlobals->theExtensions->listOfExtensionsInUse->entries())
      {
         RWOrderedIterator nextExtension(*theSpGlobals->theExtensions->
                                         listOfExtensionsInUse);
         CStringRWC * anInUseExtension;
         while (anInUseExtension = (CStringRWC *) nextExtension())
         {
            // if the extension is "base", assume it is legal.

            if (strcmp(anInUseExtension->data(),"base") != 0)
            {
               // the extension string is really a set of extensions, all must be present.
               if (strchr(anInUseExtension->data(),'&'))
               {
                  RWOrdered theList;
                  theSpGlobals->theExtensions->TokenizedExtensions(
                                               anInUseExtension->data(), &theList);
                  RWOrderedIterator nextEx(theList);
                  CStringRWC * anInUseEx;
                  while (anInUseEx = (CStringRWC *) nextEx())
                  {
                     if (strcmp(anInUseEx->data(),"base") != 0)
                     {
                        isLegal = (int) listOfProgramsPKeyData[i]->
                                        aTList.contains(anInUseEx);
                        if (!isLegal) break;
                     }
                  }
                  theList.clearAndDestroy();
               }
               else
               {
                  isLegal = (int) listOfProgramsPKeyData[i]->
                                  aTList.contains(anInUseExtension);
               }
            }

            // if any *ONE* extension is not in the program,
            // then reject the program.

            if (!isLegal) break;
         }
      }

      // if the program is legal, then add it to the list.

      if (isLegal)
         newListOfProgramsPKeyData[numberProgramsInNewList++]=
            listOfProgramsPKeyData[i];
   }
   if (numberProgramsInNewList == 0)
   {
      Warn ("FATAL ERROR in SpPrograms::UpdateListOfLegalPrograms:\n"
            "Suppose can not find the program required to run\n"
            "your projection and will now terminate.");
      delete []newListOfProgramsPKeyData;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }


   delete []listOfProgramsPKeyData;
   listOfProgramsPKeyData = newListOfProgramsPKeyData;
   numberPrograms = numberProgramsInNewList;

   // if the use of the PPE is not locked, see if any of the "legal" program
   // contain the PPE.  If none do, then lock out the use of the PPE.
   // Note that the PPE will be in the list of programs if it is in the
   // list of extensions in use, do to the logic above.

   if (!theSpGlobals->theVariants->PPEUseIsLocked)
      if (!FindProgramThatContainsElement(&ppeSearch))
         theSpGlobals->theVariants->PPEUseIsLocked = 1;

   if (numberPrograms == 1)
   {
      theSelectedProgram = 0;
      programSelectionLocked = 1;
   }
   else theSelectedProgram = -1;
   SendUpdateMessage (PROGRAMSelectionChanged);

}






int SpPrograms::LoadProgramCollection (RWOrdered * theProgramList)
{

   for (int i=0; i<numberPrograms; i++)
   {
      CStringRWC * pgm = new CStringRWC(*listOfProgramsPKeyData[i]);
      size_t nL = pgm->first('\n');
      if (nL != RW_NPOS) pgm->remove(nL);
      theProgramList->insert(pgm);
   }
   return theSelectedProgram;
}




int SpPrograms::FindProgramThatContainsElement(CStringRWC * theElement,
                                               const char * theVariant)
{
   if (programSelectionLocked)
   {
      if (listOfProgramsPKeyData[theSelectedProgram]->
          aTList.contains(theElement))
      {
         if (theVariant)
         {
            CStringRWC variant = theVariant;
            if (listOfProgramsPKeyData[theSelectedProgram]->
                aTList.contains(&variant)) return theSelectedProgram;
         }
         else return theSelectedProgram;
      }
   }
   else
   {
      for (int i=0; i<numberPrograms; i++)
      {
         if (listOfProgramsPKeyData[i]->aTList.contains(theElement))
         {
            if (theVariant)
            {
               CStringRWC variant = theVariant;
               if (listOfProgramsPKeyData[i]->
                   aTList.contains(&variant))  return i;
            }
            else return i;
         }
      }
   }
   return -1;
}




int SpPrograms::FindProgramThatContainsElement(const char * theElement,
                                               const char * theVariant)
{

   CStringRWC search = theElement;
   return FindProgramThatContainsElement(&search,theVariant);
}





int SpPrograms::SelectedProgramContainsElement(CStringRWC * theElement)
{
   if (strcmp(theElement->data(),"base") == 0) return theSelectedProgram;
   if (theSelectedProgram > -1 &&
       listOfProgramsPKeyData[theSelectedProgram]->
       aTList.contains(theElement)) return theSelectedProgram;
   else return -1;
}




int SpPrograms::SelectedProgramContainsElement(const char * theElement)
{

   CStringRWC search = theElement;
   return SelectedProgramContainsElement(&search);
}


int SpPrograms::SelectThisProgram(int pgm, int OkToLock)
{
/****

   This routine sets theSelectedProgram to the desired one.  The routine
   assumes that the caller is picking a program from the current list of
   legal programs.  It does not load the list if it is empty.

   If OkToLock is true, then it locks the selection.

   return values: 0 if selection did not occur as requested
                  1 otherwise
 ****/

   if (theSelectedProgram == pgm) return 1;

   if (programSelectionLocked) return 0;

   if (numberPrograms == 0 || pgm > numberPrograms) return 0;

   CStringRWC * var = (CStringRWC *)
      listOfProgramsPKeyData[pgm]->aTList.at(0);
   if (theSpGlobals->theVariants->SetSelectedVariant(var->data()))
   {
      theSelectedProgram = pgm;
      if (OkToLock) programSelectionLocked = 1;
      theSpGlobals->theExtensions->LoadListOfLegalExtensions();
      SendUpdateMessage (PROGRAMSelectionChanged);
      return 1;
   }
   return 0;
}




int SpPrograms::SelectThisProgram(const char * pgm, int OkToLock)
{
/****

   This routine is used to set a program as the selected program.  It first
   checks the current legal list...if it finds the list empty then the
   list is loaded via a call to UpdateListOfLegalPrograms.  The requested
   program must be in that list to be selected.

   If it finds the program, then it sets it as the selected program.

   If OkToLock is true, then it locks the selection.

   return values: 0 if selection did not occur as requested
                  1 otherwise
 ****/

   // if the current program is the requested program, then return true.

   if (theSelectedProgram > -1 &&
       strcmp(listOfProgramsPKeyData[theSelectedProgram]->data(),pgm) == 0)
       return 1;

   // if the program selection is already locked, then return false.

   if (programSelectionLocked) return 0;

   // if the number of legal programs is empty, then load it.

   if (!numberPrograms) UpdateListOfLegalPrograms();

   if (!numberPrograms) return 0;

   // set a temporary pgmFound to -1, signaling that the pgm is not found yet.

   int pgmFound = -1;

   // search within the legal program list.

   for (int i=0; i<numberPrograms; i++)
   {
      if (strcmp(listOfProgramsPKeyData[i]->data(),pgm) == 0)
      {
         pgmFound = i;
         break;
      }
   }

   if (pgmFound == -1) return 0;

   // if the program is found, then set it selected and then set the
   // variant it contains as the current variant.  The variant it contains
   // is the first member of the programs's aTList.

   CStringRWC * var = (CStringRWC *) listOfProgramsPKeyData[pgmFound]->
      aTList.at(0);
   if (theSpGlobals->theVariants->SetSelectedVariant(var->data()))
   {
      // set the selected program and return.

      theSelectedProgram = pgmFound;
      if (OkToLock && numberPrograms == 1) programSelectionLocked = 1;
      SendUpdateMessage (PROGRAMSelectionChanged);
      return 1;
   }
   return 0;
}



int SpPrograms::FindLegalProgram( const char * theProgram )
{
   for (int i=0; i<numberPrograms; i++)
   {
      if (strcmp(listOfProgramsPKeyData[i]->data(),
                 theProgram) == 0)
         return i;
   }
   return -1;
}



int SpPrograms::LockLegalProgram( int pgmToLock )
{

   if (pgmToLock >= 0 && pgmToLock < numberPrograms)
   {
      theSelectedProgram = pgmToLock;
      programSelectionLocked = 1;

      theSpGlobals->theCommands  ->PickCommand(1);
      theSpGlobals->theExtensions->LoadListOfLegalExtensions();
      theSpGlobals->theVariants  ->LoadListOfLegalVariants();

      SendUpdateMessage (PROGRAMSelectionChanged);

      return theSelectedProgram;

   }
   return -1;
}



void SpPrograms::PickProgram(int OkToLock)
{

   if (programSelectionLocked)
   {
      theSpGlobals->theCommands->PickCommand(OkToLock);
      return;
   }

   UpdateListOfLegalPrograms();

   theSpGlobals->theExtensions->LoadListOfLegalExtensions();
   theSpGlobals->theVariants->LoadListOfLegalVariants();

   if (programSelectionLocked)
   {
      theSpGlobals->theCommands->PickCommand(OkToLock);
      return;
   }

   int willWork;
   int numberProgramsThatWillWork = 0;
   int oneProgramThatWillWork = -1;

   CStringRWC search;
   CStringRWC ppeSearch = "ppe";

   for (int i=0; i<numberPrograms; i++)
   {
      willWork = 1;

      // does the program contain theSelectedVariant?

      if (theSpGlobals->theVariants->theSelectedVariant >= 0)
      {
         search = *theSpGlobals->theVariants->listOfVariantsPKeyData[
                   theSpGlobals->theVariants->theSelectedVariant];

         if (!listOfProgramsPKeyData[i]->aTList.contains(&search))
            willWork = 0;
      }

      // if the ppe is being used, does the program contain it?

      if (willWork)
      {
         if (theSpGlobals->theVariants->isPPEBeingUsed == 1)
            if (!listOfProgramsPKeyData[i]->aTList.contains(&ppeSearch))
               willWork = 0;
      }

      // if the ppe is NOT being used, the program must NOT contain it?

      if (willWork)
      {
         if (theSpGlobals->theVariants->isPPEBeingUsed == 0)
            if (listOfProgramsPKeyData[i]->aTList.contains(&ppeSearch))
               willWork = 0;
      }

      // count the programs that will work...point to the first one that will.

      if (willWork)
      {
         numberProgramsThatWillWork++;
         if (oneProgramThatWillWork < 0) oneProgramThatWillWork = i;
      }
   }

   // the variant selection takes priority over the use/nonuse of PPE...
   // according to the following code.

   if (oneProgramThatWillWork < 0 &&
       theSpGlobals->theVariants->PPEUseIsLocked == 0 &&
       theSpGlobals->theVariants->isPPEBeingUsed == 1)
   {
      theSpGlobals->theVariants->isPPEBeingUsed = 0;
      PickProgram();
   }

   if (numberProgramsThatWillWork > 0)
      theSelectedProgram = oneProgramThatWillWork;


        if (OkToLock                        &&
            numberProgramsThatWillWork == 1 &&
            theSpGlobals->theVariants->variantSelectionLocked)
        {
           theSpGlobals->theVariants->variantSelectionLocked = 1;
           theSpGlobals->theExtensions->extensionSelectionLocked = 1;
           theSpGlobals->theVariants->PPEUseIsLocked = 1;
           programSelectionLocked = 1;
        }

        SendUpdateMessage (PROGRAMSelectionChanged);

   // pick a command, given the program and extensions data.

   theSpGlobals->theCommands->PickCommand(OkToLock);
}




const char * SpPrograms::GetProgram( void )
{
   PickProgram();
   if (theSelectedProgram > -1)
      return listOfProgramsPKeyData[theSelectedProgram]->data();
   else return NULL;
}





int SpPrograms::FindProgramThatContainsAllDesiredElements
                   (RWOrdered  & theDesiredElements,
                    const char * theVariant)
{
   // This routine attempts to find a program that contains all the
   // elements in theDesiredElements list.

   // return -1, if no program can be found.
   // return <a program number> if a program can be found.

   // Only programs from the list of legal programs are checked.

   // if programSelectionLocked, then only check theSelectedProgram

   if (programSelectionLocked)
   {
      RWOrderedIterator nextDesiredElement(theDesiredElements);
      CStringRWC * oneElement;
      while (oneElement = (CStringRWC *) nextDesiredElement())
      {
         // all pgms have base as an element.

         if (strcmp(oneElement->data(),"base") == 0) continue;
         if (!listOfProgramsPKeyData[theSelectedProgram]->
             aTList.contains(oneElement)) return -1;
         else
         {
            if (theVariant)
            {
               CStringRWC variant = theVariant;
               if (!listOfProgramsPKeyData[theSelectedProgram]->
                   aTList.contains(&variant))  return -1;
            }
         }
      }
      return theSelectedProgram;
   }
   else
   {
      for (int i=0; i<numberPrograms; i++)
      {
         int elementMissing = 0;
         RWOrderedIterator nextDesiredElement(theDesiredElements);
         CStringRWC * oneElement;
         while (oneElement = (CStringRWC *) nextDesiredElement())
         {
            // all pgms have base as an element.

            if (strcmp(oneElement->data(),"base") == 0) continue;
            if (!listOfProgramsPKeyData[i]->aTList.contains(oneElement))
            {
               elementMissing = 1;
               break;
            }
         }
         if (elementMissing) continue;

         if (theVariant)
         {
            CStringRWC variant = theVariant;
            if (!listOfProgramsPKeyData[i]->
                aTList.contains(&variant))  elementMissing = 1;
         }
         if (!elementMissing) return i;
      }
   }
   return -1;
}




