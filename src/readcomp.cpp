/*********************************************************************

   File Name:    readcomp.cpp
   Author:       nlc
   Date:         see readcomp.hpp

   see readcomp.hpp for notes.

 *********************************************************************/


#include "PwrDef.h"
#include "appdef.hpp"

#include "dirsept.hpp"
#include "runstr.hpp"
#include "extens.hpp"
#include "compon.hpp"
#include <fstream>

#include <rw/ordcltn.h>

#include "readcomp.hpp"

void ReadComponentsFromFile(FILE      * filePtr,
                            FILE_SPEC * fileSpec,
                            RWOrdered * componentsFromFile,
                            int         assumeComponentsAreUsed)
{
   CStringRW * Tbuff = new CStringRW ((RWSize_T) 500);
   Tbuff->maxWaste(500);
   Tbuff->remove(0);  // clear Tbuff
   char * buff  = new char [BUFFSIZE+1];
   char * ptr;
   char * name;
   char path[SZ_FNAME];
   Component * newComponent = NULL;

   class ComponentNumberMap
   {
      public:

         int oldComponentNumber;
         int newComponentNumber;
         ComponentNumberMap * chain;

         inline ComponentNumberMap (int oldNumber, int newNumber,
                                    ComponentNumberMap * lastMap = NULL)
               :newComponentNumber (newNumber),
                oldComponentNumber (oldNumber),
                chain              (lastMap)
         {}
   };

   ComponentNumberMap * lastMap = NULL;
   ComponentNumberMap * nextMap;
   int compNum;

   // make sure the list of components from a file are cleared.

   componentsFromFile->clearAndDestroy();

   if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;

   // if we have !!C we assume a real component
   // and we will load components until we don't
   // find any more. look for !!C...thereby picking up
   // PPEComponents !!CT !!CB !!CR

   if (strncmp (buff,"!!C",3) == 0)
   {
      while (strncmp (buff,"!!C",3) == 0)
      {
         ptr = strtok(buff," \n");
         name = strtok(NULL,"~\n");
         newComponent = new Component (name);

         // get the original component number
         ptr = strtok(NULL," \n");
         if (ptr) sscanf (ptr,"%d", &compNum);
         else compNum = -1;
         // store the old and new component number.
         if (compNum > -1) lastMap =
            new ComponentNumberMap (compNum, newComponent->componentNumber,
                                    lastMap);

         // get componentType

         ptr = strtok(NULL," \n");
         if (ptr) sscanf (ptr,"%d", &newComponent->componentType);
         else newComponent->componentType = -1;

         // if the component type is 4,5, or 6, the component is
         // actually a ppe component

         if (newComponent->componentType == 4 ||
             newComponent->componentType == 5 ||
             newComponent->componentType == 6)
         {
            Component * t = new PPEComponent(newComponent);
            delete newComponent;
            newComponent = t;
         }

         // get conditionSerialNum

         ptr = strtok(NULL," \n");
         if (ptr) sscanf (ptr,"%d", &newComponent->conditionSerialNum);
         else                        newComponent->conditionSerialNum = -1;
         if (lastMap && newComponent->conditionSerialNum > 0)
         {
            nextMap = lastMap;
            while (nextMap)
            {
               if (newComponent->conditionSerialNum ==
                   nextMap->oldComponentNumber)
               {
                  newComponent->conditionSerialNum =
                    nextMap->newComponentNumber;
                  break;
               }
               nextMap=nextMap->chain;
            }
         }

         // get variant data, if any.
         ptr = strtok(NULL," \n");
         if (ptr) sscanf (ptr,"%s", buff);
         else *buff = '@';
         if (*buff != '@')
         newComponent->variant = new CStringRW (buff, strlen(buff)+1);

         // get extension data, if any.
         ptr = strtok(NULL," \n");
         if (ptr) sscanf (ptr,"%s", buff);
         else *buff = '@';
         if (*buff != '@')
            newComponent->extension = new CStringRW (buff, strlen(buff)+1);

         // signal that an extension is in use.

         if (assumeComponentsAreUsed &&
             newComponent->extension) theSpGlobals->theExtensions->
             AddExtensionInUse(newComponent->extension->data());

         // skip over the group membership.

         // ready to read the contents
         // of the component.

         if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;

         if (strncmp (buff,"!!SW",4) == 0)
         {
            ptr = strtok(buff," \n");
            ptr = strtok(NULL," \n");
            newComponent->sourceWindow = new CStringRW (ptr, strlen(ptr));
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;
            if (strncmp (buff,"!!P ",4) == 0)
            if (ReadWindowParms(filePtr, buff,
                                &newComponent->windowParms)) goto ENDOFFILE;
         }

         Tbuff->remove(0); // clear Tbuff
         xvt_fsys_convert_dir_to_str(&fileSpec->dir, path, sizeof(path));
         *Tbuff="This component was inserted from file:\n";
         *Tbuff+=path;
         *Tbuff+=SEPARATOR;
         *Tbuff+=fileSpec->name;
         *Tbuff+="\n";

         // can't call ReadRunSegment because we're
         // looking for End not !!End

         // get the "userEditedComment" field, if it is present.

         if (strncmp (buff,"!!CE",4) == 0)
         {
            sscanf (buff+4,"%d", &newComponent->userEditedComment);
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;
         }

         if (strncmp (buff,"Comment",7) == 0)
         {

            if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;
            while (strncmp (buff,"End\n",4) != 0)
            {
               *Tbuff += buff;
               if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;
            }
            Tbuff->remove(Tbuff->length()-1);
            newComponent->comment = new CStringRW (Tbuff->data(),
                                                   Tbuff->length());
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ENDOFFILE;
         }
         newComponent->comment = new CStringRW (Tbuff->data(),
                                                Tbuff->length());

         if (strncmp (buff,"!!K\n",4) == 0)
         ReadRunSegment(&newComponent->keywords, filePtr, buff, Tbuff);

         componentsFromFile->insert(newComponent);
      }
   }

   // create an untitled component...use the
   // file name as the component name.

   else
   {
      *Tbuff  = "From file: ";
      *Tbuff += fileSpec->name;

      newComponent = new Component (Tbuff->data());
      newComponent->sourceWindow = new CStringRW ("FreeForm");

      *Tbuff=buff;
      while (fgets (buff, BUFFSIZE, filePtr))
      {
         // skip lines that start with !!
         // (we don't want Suppose control lines).

         if (strncmp (buff,"!!", 2) != 0) *Tbuff += buff;
      }
      *Tbuff = Tbuff->strip(RWCString::trailing,'\n');
      *Tbuff = Tbuff->strip(RWCString::trailing,' ');

      newComponent->keywords = new CStringRW (Tbuff->data(), Tbuff->length());

      Tbuff->remove(0); // clear Tbuff
      xvt_fsys_convert_dir_to_str(&fileSpec->dir, path, sizeof(path));
      *Tbuff="This component was inserted from:\n";
      *Tbuff+=path;
      *Tbuff+=SEPARATOR;
      *Tbuff+=fileSpec->name;
      *Tbuff+="\n";

      newComponent->comment = new CStringRW (Tbuff->data(), Tbuff->length());
      newComponent->componentType=2;

      componentsFromFile->insert(newComponent);

   }

   goto ITSCOOL;

   ENDOFFILE:
   delete newComponent;
   newComponent = NULL;

   ITSCOOL:
   delete Tbuff;
   delete []buff;
   if (lastMap)
   {
      nextMap = lastMap;
      while (nextMap)
      {
         nextMap=nextMap->chain;
         delete lastMap;
         lastMap=nextMap;
      }
   }
}



