/********************************************************************

   File Name:     extens.cpp
   Author:        nlc
   Date:          12/07/94; revised 11/18/2005 nlc

   see extens.hpp for notes.

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
#include "warn_dlg.hpp"

SpExtensions::SpExtensions( void )
{
   extensionSelectionLocked = 0;
   listOfExtensionsPKeyData     = NULL;
   isExtensionInSelectedProgram = NULL;
   listOfExtensionsInUse        = NULL;

   // make a list of the possible extensions... this list is taken from the
   // "extensions" in the parameters file.

   CStringRWC extensionString = "extensions";
   extensionsMSTextPointer = (MSText *) theSpGlobals->
      theParmsData->find(&extensionString);
   if (extensionsMSTextPointer) extensionsMSTextPointer->GetThePKeys();

   // if extensionsMSTextPointer is NULL, then Suppose could not find a
   // list of extensions.  ... or, if the list is empty, then close the
   // application.

   numberExtensions = -1;
   if (extensionsMSTextPointer) numberExtensions =
      extensionsMSTextPointer->thePKeys.entries();
   if (numberExtensions < 0)
   {
      Warn("FATAL ERROR:\n"
           "Suppose could not find a list of FVS Extensions\n"
           "in the parameters file.  Suppose will exit.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }

   listOfExtensionsPKeyData = new ptrToPKeyData [numberExtensions];
   isExtensionInSelectedProgram = new int       [numberExtensions];
   listOfExtensionsInUse        = new RWOrdered (numberExtensions);

   LoadListOfLegalExtensions();

   // specify that "base" is in use... always.

   listOfExtensionsInUse->insert(new CStringRWC("base"));

   if (numberExtensions == 0)
   {
      Warn ("FATAL ERROR:\nThere are no legal FVS Extensions.");
      delete this;
      theSpGlobals->theAppDocument->DoMenuCommand(M_FILE_QUIT,0,0);
   }


}





SpExtensions::~SpExtensions( void )
{
   if (listOfExtensionsPKeyData)     delete []listOfExtensionsPKeyData;
   if (isExtensionInSelectedProgram) delete []isExtensionInSelectedProgram;
   if (listOfExtensionsInUse)
   {
      listOfExtensionsInUse->clearAndDestroy();
      delete listOfExtensionsInUse;
   }
}





int SpExtensions::LoadListOfLegalExtensions( void )
{

   int theListCounter = 0;
   int addToList;

   // if the current list is loaded, then save a copy.  This will
   // be used to determine if an update message should be sent.

   ptrToPKeyData * copyOfExtensionsPKeyData = NULL;

   if (numberExtensions)
   {
      copyOfExtensionsPKeyData = new ptrToPKeyData [numberExtensions];
      for (int i=0; i<numberExtensions; i++)
         copyOfExtensionsPKeyData[i]=listOfExtensionsPKeyData[i];
   }

   RWOrderedIterator nextPKeyData(extensionsMSTextPointer->thePKeys);
   PKeyData * oneExtension;
   while (oneExtension = (PKeyData *) nextPKeyData())
   {
      addToList = 1;

      // find out if the extension is already been found legal

      if (theListCounter)
      {
         for (int i=0; i<theListCounter; i++)
         {
            if (listOfExtensionsPKeyData[i] == oneExtension)
            {
               addToList = 0;
               break;
            }
         }
      }

      // all programs contain the extension called "base".  So
      // if the extension is base, it is legal.

      if (addToList && strcmp(oneExtension->data(),"base") != 0)
      {

         // if there is a legal program that contains the extension and
         // the current variant, then the extension is legal.

         const char * currVar = NULL;
         if (theSpGlobals->theVariants->theSelectedVariant >= 0)
            currVar = theSpGlobals->theVariants->
               listOfVariantsPKeyData[theSpGlobals->theVariants->
                                      theSelectedVariant]->data();
         if (theSpGlobals->thePrograms->
             FindProgramThatContainsElement(
                oneExtension->data(),currVar) == -1) addToList = 0;
      }

      // if the extension is legal...then count it!

      if (addToList)
      {
         listOfExtensionsPKeyData[theListCounter] = oneExtension;

         // the extension is "picked" if it is part of the selected program

         isExtensionInSelectedProgram[theListCounter] =
            (strcmp(oneExtension->data(),"base") == 0) ||
            (theSpGlobals->thePrograms->
             SelectedProgramContainsElement(oneExtension->data()) >= 0);

         theListCounter++;
      }
   }

   // figure out if an update message should be sent...if the number
   // of extensions changed, then for sure!

   int sendMessage = 0;
   if (!copyOfExtensionsPKeyData || numberExtensions != theListCounter)
      sendMessage = 1;
   else
   {
      // if the list has changed in any respect, then YES!

      for (int i=0; i<numberExtensions; i++)
      {
         if (copyOfExtensionsPKeyData[i] != listOfExtensionsPKeyData[i])
         {
            sendMessage = 1;
            break;
         }
      }
   }
   if (copyOfExtensionsPKeyData) delete []copyOfExtensionsPKeyData;
   numberExtensions = theListCounter;
   if (sendMessage) SendUpdateMessage (EXTENSIONListChanged);
   return numberExtensions;
}






void SpExtensions::LoadExtensionCollection (RWOrdered * theExtensionList)
{

   LoadListOfLegalExtensions();

   theExtensionList->clearAndDestroy();

   for (int i=0; i<numberExtensions; i++)
   {
      theExtensionList->insert(new CStringRWC(
         listOfExtensionsPKeyData[i]->pString));
   }
}






int SpExtensions::FindLegalExtension( const char * theExtension )
{
   // if an ampersand "&" is in theExtension, process the extension
   // string as a set of extensions that all must be available together.

   if (strchr(theExtension,'&')) return FindMultiExtensions (theExtension);
   else
   {
      for (int i=0; i<numberExtensions; i++)
      {
         if (strcmp(listOfExtensionsPKeyData[i]->data(),
            theExtension) == 0) return i;
      }
   }
   return -1;
}



int SpExtensions::FindMultiExtensions( const char * theExtension )
{
   // if an ampersand "&" is in theExtension, process the extension
   // string as a set of extensions that all must be available together.

   RWOrdered theList;
   TokenizedExtensions(theExtension, &theList);
   int ret;
   ret=theSpGlobals->thePrograms->
            FindProgramThatContainsAllDesiredElements(theList,
            theSpGlobals->theVariants->GetSelectedVariant());
   theList.clearAndDestroy();
   return ret;
}


void SpExtensions::TokenizedExtensions( const char * theExtension,
                                        RWOrdered  * theList)
{
   // an ampersand "&" is in theExtension, process the extension
   // string as a set of extensions that all must be available together.

   char *list = new char[strlen(theExtension)+1];
   strcpy(list,theExtension);
   char *c,*t;
   t=list;
   for (c=list; *c; c++)
   {
      if (*c == '&')
      {
         *c=NULL;
         CStringRWC *s=new CStringRWC(t);
         theList->insert(s);
         t=c+1;
      }
   }
   CStringRWC *s=new CStringRWC(t);
   theList->insert(s);
}



int SpExtensions::AddExtensionInUse ( const char * theExtension,
                                      int OkToLockPgm)
{
   if (theExtension)
   {
      CStringRWC tmp = theExtension;
      return AddExtensionInUse ( tmp, OkToLockPgm);
   }
   else return 0;
}






int SpExtensions::AddExtensionInUse ( CStringRWC & theExtension,
                                      int OkToLockPgm)
{

   // return 0 if extension is already in the in use list.
   // return 1 if extension was added...

   if (theExtension.length() == 0 ||
       listOfExtensionsInUse->contains(&theExtension))  return 0;

   // if the extension being added is the PPE, then signal that it is
   // being used...and lock it.

   if (theExtension.RWCString::compareTo("ppe") == 0)
   {
         theSpGlobals->theVariants->isPPEBeingUsed = 1;
         theSpGlobals->theVariants->PPEUseIsLocked = 1;
   }

   listOfExtensionsInUse->insert(new CStringRWC(theExtension));

   // pick the program...this repicks the command, reloads the list
   // of valid extensions.

   theSpGlobals->thePrograms->PickProgram(OkToLockPgm);

   return 1;

}




