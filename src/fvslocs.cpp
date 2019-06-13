/********************************************************************

   File Name:     fvslocs.cpp
   Author:        nlc
   Date:          12/06/94 & 01/31/94

   See fvslocs.hpp for information.

********************************************************************/

#include "spparms.hpp"
#include "fvslocs.hpp"

SpSpecies::SpSpecies()
{
   forVariant = NULL;
   numberSpecies = -1;
}




void SpSpecies::ISpSpecies( void )
{

   if (forVariant) delete []forVariant;
   forVariant = NULL;
   if (theSpGlobals->theVariants->GetSelectedVariant())
   {
      forVariant = new char[strlen(
         theSpGlobals->theVariants->GetSelectedVariant())+1];
      strcpy(forVariant,theSpGlobals->theVariants->GetSelectedVariant());
   }
                             // if we are resetting the object, then
   if (numberSpecies >= 0)   // delete the old data.
   {
      for (int i=0; i<numberSpecies; i++)
      {
         delete []alphaCodes[i];
         delete []numericCodes[i];
         delete []commonNames[i];
      }
      delete []alphaCodes;
      delete []numericCodes;
      delete []commonNames;
   }
   numberSpecies = -1;

   if (!forVariant) return;

   int spIndex = 0;

   // set up a search for the section needed.

   CStringRWC neededMSText = "species_";
   neededMSText += forVariant;
   MSText * neededMSTextPointer = (MSText *) theSpGlobals->
      theParmsData->find(&neededMSText);

   if (neededMSTextPointer)
   {
      RWOrdered * thePKeys = neededMSTextPointer->GetThePKeys();
      numberSpecies = thePKeys->entries();

      if (numberSpecies)
      {
         alphaCodes    = new char * [numberSpecies];
         numericCodes  = new char * [numberSpecies];
         commonNames   = new char * [numberSpecies];

         RWOrderedIterator pkeyIter(*thePKeys);
         PKeyData * aPkey;
         while (aPkey = (PKeyData *) pkeyIter())
         {
            char * newstring;

            // PKey is the alphacode.

            newstring = new char[aPkey->length()+1];
            strcpy(newstring,aPkey->data());
            alphaCodes[spIndex] = newstring;

            // The first member of the ATList is the numeric code.

            numericCodes[spIndex] = NULL;

            if (aPkey->aTList.entries())
            {
               CStringRWC * atListFirstElement =
                  (CStringRWC *) aPkey->aTList.first();
               newstring = new char[atListFirstElement->length()+1];
               strcpy(newstring,atListFirstElement->data());
               numericCodes[spIndex] = newstring;
            }

            // PString is the common name.

            newstring = new char[aPkey->pString.length()+1];
            strcpy(newstring,aPkey->pString.data());
            commonNames[spIndex] = newstring;

            spIndex++;
         }
         numberSpecies = spIndex;
      }
      else numberSpecies = -1;
   }
}




SpSpecies::~SpSpecies( void )
{
   if (numberSpecies > -1)
   {
      for (int i=0; i<numberSpecies; i++)
      {
         delete []alphaCodes[i];
         delete []numericCodes[i];
         delete []commonNames[i];
      }
      delete []alphaCodes;
      delete []numericCodes;
      delete []commonNames;
   }
}


//**************************************************************************


HabPa::HabPa()
{
   forVariant = NULL;
   numberItems = -1;
}




void HabPa::IHabPa( void )
{

   delete []forVariant;
   forVariant = NULL;
   if (theSpGlobals->theVariants->GetSelectedVariant())
   {
      forVariant = new char[strlen(
         theSpGlobals->theVariants->GetSelectedVariant())+1];
      strcpy(forVariant,theSpGlobals->theVariants->GetSelectedVariant());
   }
                             // if we are resetting the object, then
   if (numberItems >= 0)   // delete the old data.
   {
      for (int i=0; i<numberItems; i++)
      {
         delete []numericCodes[i];
         delete []names[i];
      }
      delete []numericCodes;
      delete []names;
   }
   numberItems = -1;

   if (!forVariant) return;

   int index;

   // set up a search for the section needed.

   CStringRWC neededMSText = "HabPa_";
   neededMSText += forVariant;
   MSText * neededMSTextPointer = (MSText *) theSpGlobals->
      theParmsData->find(&neededMSText);

   if (neededMSTextPointer)
   {
      RWOrdered * thePKeys = neededMSTextPointer->GetThePKeys();
      numberItems = (int) thePKeys->entries();

      if (numberItems)
      {
         numericCodes = new char * [numberItems];
         names        = new char * [numberItems];

         char * newstring;
         index = 0;

         RWOrderedIterator pkeyIter(*thePKeys);
         PKeyData * aPkey;
         while (aPkey = (PKeyData *) pkeyIter())
         {
            // PKey is the numeric code.

            newstring = new char[aPkey->length()+1];
            strcpy(newstring,aPkey->data());
            numericCodes[index] = newstring;

            // PString is the name.

            newstring = new char[aPkey->pString.length()+1];
            strcpy(newstring,aPkey->pString.data());
            names[index] = newstring;
            index++;
         }
         numberItems = index;
      }
      else numberItems = -1;
   }
}




HabPa::~HabPa( void )
{
   if (numberItems > -1)
   {
      for (int i=0; i<numberItems; i++)
      {
         delete []numericCodes[i];
         delete []names[i];
      }
      delete []numericCodes;
      delete []names;
   }
}


//**************************************************************************


Forests::Forests()
{
   forVariant = NULL;
   numberItems = -1;
}




void Forests::IForests( void )
{

   delete []forVariant;
   forVariant = NULL;
   if (theSpGlobals->theVariants->GetSelectedVariant())
   {
      forVariant = new char[strlen(
         theSpGlobals->theVariants->GetSelectedVariant())+1];
      strcpy(forVariant,theSpGlobals->theVariants->GetSelectedVariant());
   }

   // if we are resetting the object, then delete the old data.

   if (numberItems >= 0)
   {
      for (int i=0; i<numberItems; i++)
      {
         delete []numericCodes[i];
         delete []names[i];
      }
      delete []numericCodes;
      delete []names;
   }
   numberItems = -1;

   if (!forVariant) return;

   int index;

   // set up a search for the section needed.

   CStringRWC neededMSText = "Forests_";
   neededMSText += forVariant;

   MSText * neededMSTextPointer = (MSText *) theSpGlobals->
      theParmsData->find(&neededMSText);

   if (neededMSTextPointer)
   {
      RWOrdered * thePKeys = neededMSTextPointer->GetThePKeys();
      numberItems = (int) thePKeys->entries();

      if (numberItems)
      {
         numericCodes = new char * [numberItems];
         names        = new char * [numberItems];
         char * newstring;
         index = 0;

         RWOrderedIterator pkeyIter(*thePKeys);
         PKeyData * aPkey;
         while (aPkey = (PKeyData *) pkeyIter())
         {

            // PKey is the numeric code.

            newstring = new char[aPkey->length()+1];
            strcpy(newstring,aPkey->data());
            numericCodes[index] = newstring;

            // PString is the name.

            newstring = new char[aPkey->pString.length()+1];
            strcpy(newstring,aPkey->pString.data());
            names[index] = newstring;

            index++;
         }
         numberItems = index;
      }
      else numberItems = -1;
   }
}




Forests::~Forests( void )
{
   if (numberItems > -1)
   {
      for (int i=0; i<numberItems; i++)
      {
         delete []numericCodes[i];
         delete []names[i];
      }
      delete []numericCodes;
      delete []names;
   }
}

