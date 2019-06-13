/********************************************************************

   File Name:     variants.cpp
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
#include "programs.hpp"
#include "spglobal.hpp"
#include "cmdline.hpp"
#include "spprefer.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"
#include "suppdefs.hpp"
#include "sendupdt.hpp"

#include "warn_dlg.hpp"

SpVariants::SpVariants( void )
{
   theSelectedVariant = -1;
   variantSelectionLocked = 0;
   listOfVariantsPKeyData = NULL;

   // suppress the sending of the update messages during construction.

   OKtoSendMsg = 0;

   isPPEBeingUsed = theSpGlobals->thePreferences->defaultUsePPE;
   PPEUseIsLocked =  0;

   // make a list of the possible variants... this list is taken from the
   // "variants" in the parameters file.

   CStringRWC variantString = "variants";
   variantsMSTextPointer = (MSText *) theSpGlobals->
      theParmsData->find(&variantString);

   // if variantsMSTextPointer is NULL, then Suppose could not find a list of
   // variants.  ... or, if the list is empty, then close the application.

   numberVariants = -1;
   if (variantsMSTextPointer) numberVariants = variantsMSTextPointer->
                                GetThePKeys()->entries();
   if (numberVariants < 0)
   {
      Warn("FATAL ERROR:\n"
           "Suppose could not find a list of FVS Variants\n"
           "in the parameters file.  Suppose will exit.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }

   listOfVariantsPKeyData = new ptrToPKeyData [numberVariants];

   LoadListOfLegalVariants();

   // If there is a specific variant on the command line
   // (not a preference, but a specific variant) then, set
   // it to be selected.  If the selection process
   // is successful, then lock the variant.

   if (!variantSelectionLocked)
   {
      if (SetSelectedVariant(theSpGlobals->theCommandLine->getTheVariant()))
         variantSelectionLocked = 1;
   }

   // if the variant is not locked, then set the variant to the "prefered" variant.

   if (!variantSelectionLocked) SetSelectedVariant(theSpGlobals->thePreferences->defaultVariant);

   // if there is still no variant selected, then pick the first one
   // in the list.

   if (theSelectedVariant == -1) theSelectedVariant = 0;

   OKtoSendMsg = 1;
}





int SpVariants::SetSelectedVariant( const char * theVariant )
{
   // return 1 if theVariant is set as selected, 0 if not.

   // if theVariant is NULL, return 0

   if (!theVariant) return 0;

   // if the desired variant is already selected, then we are done.

   if (theSelectedVariant > -1 &&
       strcmp(listOfVariantsPKeyData[theSelectedVariant]->data(),
              theVariant) == 0) return 1;

   if (variantSelectionLocked)
   {
      if (strcmp(listOfVariantsPKeyData[theSelectedVariant]->data(),
                 theVariant) != 0)
      {
         CStringRW msg = "You can not, at this point, change the variant";
         if (theSelectedVariant >= 0)
         {
            msg += "\nCurrent variant = ";
            msg += (CStringRWC &) *listOfVariantsPKeyData[theSelectedVariant];
         }
         msg += "\nRequested variant = ";
         msg += theVariant;
         Warn(msg.data());
         return 0;             // return with error condition.
      }
   }
   else
   {
      if (variantsMSTextPointer)
      {
         int theFoundVariant = FindLegalVariant( theVariant );
         if (theFoundVariant < 0)
         {
            CStringRW msg = "The variant you requested is not available.\n";
            if (theSelectedVariant >= 0)
            {
               msg += "\nCurrent variant = ";
               msg += (CStringRWC &)
                      *listOfVariantsPKeyData[theSelectedVariant];
            }
            msg += "\nRequested variant = ";
            Warn(msg.data());
            return 0;
         }
         else
         {
            if (theSelectedVariant != theFoundVariant)
            {
               theSelectedVariant = theFoundVariant;
               if (OKtoSendMsg) SendUpdateMessage (VARIANTSelectionChanged);
            }
         }
      }
   }
   return 1;                   // return "true", no error.
}





SpVariants::~SpVariants( void )
{
   if (listOfVariantsPKeyData) delete []listOfVariantsPKeyData;
}




int SpVariants::LoadListOfLegalVariants( void )
{

   // this routine is called from PickProgram and other places.  That is,
   // it is used to update the list as well as initially load it. If it
   // is called after a variant is already selected, it must reset the
   // index to the selected variant after it has loaded its list.

   const char * alreadySelectedVariant = NULL;
   if (theSelectedVariant > -1) alreadySelectedVariant =
      listOfVariantsPKeyData[theSelectedVariant]->data();

   // Invalidate the current selected variant.

   theSelectedVariant = -1;

   int theListCounter = 0;
   RWOrderedIterator nextVariant(*variantsMSTextPointer->GetThePKeys());
   PKeyData * oneVariant;
   while (oneVariant = (PKeyData *) nextVariant())
   {
      if (theSpGlobals->thePrograms->
          FindProgramThatContainsElement(oneVariant) > -1)
      {
         listOfVariantsPKeyData[theListCounter++] = oneVariant;
      }
   }
   numberVariants = theListCounter;

   if (numberVariants == 0)
      Warn ("There are no legal variants."
            "\nSpVariants::LoadListOfLegalVariants");

   if (theListCounter == 1)
   {
      variantSelectionLocked = 1;
      theSelectedVariant     = 0;
   }

   // if there was a previous variant selected, then we must reselect
   // it from the newly loaded list.

   if (alreadySelectedVariant) SetSelectedVariant(alreadySelectedVariant);

   return numberVariants;
}



const char * SpVariants::GetSelectedVariant( void )
{
   if (theSelectedVariant == -1) return NULL;
   return listOfVariantsPKeyData[theSelectedVariant]->data();
}




int SpVariants::LoadVariantCollection (RWOrdered * theVariantList)
{

   for (int i=0; i<numberVariants; i++)
   {
      theVariantList->insert(new CStringRWC(listOfVariantsPKeyData[i]->
                                            pString));
   }
   return theSelectedVariant;
}




int SpVariants::FindLegalVariant( const char * theVariant )
{

   if (variantSelectionLocked)
   {
      if (strcmp(listOfVariantsPKeyData[theSelectedVariant]->data(),
                 theVariant) == 0) return theSelectedVariant;
      else return -1;
   }
   else
   {
      for (int i=0; i<numberVariants; i++)
      {
         if (strcmp(listOfVariantsPKeyData[i]->data(),
                    theVariant) == 0) return i;
      }
      return -1;
   }
}





int SpVariants::NewStandVariantsInteraction( const char * theRequiredVariants )
{

   // the theRequiredVariants is a null pointer, then return 0

   if (!theRequiredVariants) return 0;

   // if theRequiredVariants is an empty string, then return 0

   int length = strlen (theRequiredVariants);

   if (length == 0) return 0;

   // make a copy of theRequiredVariants...

   char * theCopy = new char[length+1];
   strcpy (theCopy,theRequiredVariants);

   char * ptr;
   int returnValue = 0;

   // if the variantSelectionLocked, then the selected variant must be
   // in the list of theRequiredVariants... if yes, then set returnValue to 1,
   // otherwise leave it 0.

   if (variantSelectionLocked)
   {
      for (ptr = strtok(theCopy," @\n"); ptr != NULL;
           ptr = strtok(NULL," @\n"))
      {
         if (strcmp(listOfVariantsPKeyData[theSelectedVariant]->data(),
                    ptr) == 0)
         {
            returnValue = 1;
            break;
         }
      }
   }
   else
   {
      /*****
         If the variantSelectionLocked is not locked, then:
         1. Create a new list of legal variants equal in length to the
            current list.
         2. Load the new list with variants that are both in
            theRequiredVariants and the legal variants list.
         3. If the new list is empty, then delete the new list and
            return 0.
         4. Delete the old list of legal variants and point to
            the new one.
         5. If the new list only has one variant, then lock varaint
            selection point the the "one" and return 1.
         6. If the new list has two, "select" the first one in
            theRequiredVariants list and then return.
         7. If a the "selection" of a variant has changed from the last
            time in this routine, then signal CDocument of this fact.
       *****/

      ptrToPKeyData * newListOfVariantsPKeyData  =
         new ptrToPKeyData [numberVariants];

      int numberVariantsInNewList = 0;
      int i;

      const char * oldSelectedVariant = GetSelectedVariant();

      for (ptr = strtok(theCopy," @\n"); ptr != NULL;
           ptr = strtok(NULL," @\n"))
      {
         i = FindLegalVariant(ptr);
         if (i >= 0) newListOfVariantsPKeyData[numberVariantsInNewList++] =
                        listOfVariantsPKeyData[i];
      }

      if (numberVariantsInNewList > 0)
      {
         delete []listOfVariantsPKeyData;
         listOfVariantsPKeyData = newListOfVariantsPKeyData;
         numberVariants = numberVariantsInNewList;
         if (numberVariants == 1)
         {
            theSelectedVariant = 0;
            variantSelectionLocked = 1;
         }
         else
         {
            theSelectedVariant = FindLegalVariant(strtok(theCopy," @\n"));
         }

         // Ok, now repick the program, the command,
         // and available extensions.

         theSpGlobals->thePrograms->PickProgram(1);

         // return with the "ok" flag.

         returnValue = 1;
      }
      else
      {
         delete []newListOfVariantsPKeyData;
         returnValue = 0;
      }

      // if the variant selection has change, notify CDocument.

      if (oldSelectedVariant != GetSelectedVariant() ||
          strcmp(oldSelectedVariant,GetSelectedVariant()) != 0)
      {
         if (OKtoSendMsg) SendUpdateMessage (VARIANTSelectionChanged);
      }
   }
   delete theCopy;

   return returnValue;
}



