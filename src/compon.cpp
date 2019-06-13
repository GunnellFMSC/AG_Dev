/********************************************************************

   File Name:     compon.cpp
   Author:        nlc
   Date:          05/30/95

   Purpose:       see compon.hpp

*********************************************************************/

#include "PwrDef.h"
#include CStringRWC_i
#include <rw/ordcltn.h>
#include <rw/vstream.h>
#include "compon.hpp"
#include "group.hpp"
#include "stand.hpp"
#include "runstr.hpp"
#include "spglobal.hpp"
#include "extens.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"
#include "suppdefs.hpp"

RWDEFINE_COLLECTABLE(Component,COMPONENT)

Component::Component ( const char * name, int CNumber )
          :CStringRWC       (name),
           componentType    (-1),
           componentNumber  (CNumber),
           conditionSerialNum(-1),
           userEditedComment(0),
           comment          (NULL),
           sourceWindow     (NULL),
           windowParms      (NULL),
           keywords         (NULL),
           variant          (NULL),
           extension        (NULL),
           openedWindow     (NULL)


{
   if (theSpGlobals->runStream->componentNumbers <= CNumber)
       theSpGlobals->runStream->componentNumbers  = CNumber+1;
}


Component::Component ( void )
          :CStringRWC (),
           componentType    (-1),
           componentNumber  (-1),
           conditionSerialNum(-1),
           userEditedComment(0),
           comment          (NULL),
           sourceWindow     (NULL),
           windowParms      (NULL),
           keywords         (NULL),
           variant          (NULL),
           extension        (NULL),
           openedWindow     (NULL)
{}


Component::Component ( Component * c)
          :CStringRWC ((CStringRWC &) *c)
{

   //  Copy constructor.
   componentType   = c->componentType;

   componentNumber = theSpGlobals->runStream->componentNumbers++;
   conditionSerialNum = c->conditionSerialNum;
   userEditedComment  = c->userEditedComment;
   if (c->comment) comment = new CStringRW(*c->comment);
   else comment = (CStringRW *) 0;
   if (c->sourceWindow) sourceWindow = new CStringRW(*c->sourceWindow);
   else sourceWindow = (CStringRW *) 0;
   if (c->windowParms) windowParms = new CStringRW(*c->windowParms);
   else windowParms = (CStringRW *) 0;
   if (c->keywords) keywords = new CStringRW(*c->keywords);
   else keywords = (CStringRW *) 0;
   if (c->variant) variant = new CStringRW(*c->variant);
   else variant = (CStringRW *) 0;
   if (c->extension) extension = new CStringRW(*c->extension);
   else extension = (CStringRW *) 0;
   openedWindow   = (CWindow   *) 0;
}


Component::~Component ( void )
{
   if (comment)       delete comment;
   if (sourceWindow)  delete sourceWindow;
   if (windowParms)   delete windowParms;
   if (keywords)      delete keywords;
   if (variant)       delete variant;
   if (extension)     delete extension;
}



void Component::SetComponent ( Component * c, int all)
{

   (CStringRWC &) *this = (CStringRWC &) *c;

   if (all)
   {
      componentNumber    = c->componentNumber;
      conditionSerialNum = c->conditionSerialNum;
      openedWindow       = c->openedWindow;
   }

   // set this component to the values of c

   componentType      = c->componentType;
   userEditedComment  = c->userEditedComment;

   if (c->comment)
   {
      if (comment) *comment = *c->comment;
      else          comment = new CStringRW(*c->comment);
   }
   else
   {
      if (comment)
      {
         delete comment;
         comment=NULL;
      }
   }

   if (c->sourceWindow)
   {
      if (sourceWindow) *sourceWindow = *c->sourceWindow;
      else               sourceWindow = new CStringRW(*c->sourceWindow);
   }
   else
   {
      if (sourceWindow)
      {
         delete sourceWindow;
         sourceWindow=NULL;
      }
   }

   if (c->windowParms)
   {
      if (windowParms) *windowParms = *c->windowParms;
      else              windowParms = new CStringRW(*c->windowParms);
   }
   else
   {
      if (windowParms)
      {
         delete windowParms;
         windowParms=NULL;
      }
   }

   if (c->keywords)
   {
      if (keywords) *keywords = *c->keywords;
      else            keywords = new CStringRW(*c->keywords);
   }
   else
   {
      if (keywords)
      {
         delete keywords;
         keywords=NULL;
      }
   }

   if (c->variant)
   {
      if (variant) *variant = *c->variant;
      else            variant = new CStringRW(*c->variant);
   }
   else
   {
      if (variant)
      {
         delete variant;
         variant=NULL;
      }
   }

   if (c->extension)
   {
      if (extension) *extension = *c->extension;
      else            extension = new CStringRW(*c->extension);
   }
   else
   {
      if (extension)
      {
         delete extension;
         extension=NULL;
      }
   }
}



RWBoolean Component::isEqual(const RWCollectable *c) const
{

   // two components are equal if they are at the same address OR
   // if their name is the same and their number is the same.

   return                   this               ==                 c ||
          ( (CStringRWC &) *this               == (CStringRWC &) *c &&
            ((Component *)  this)->
                               componentNumber ==  ((Component *) c)->componentNumber);

}

int Component::WriteComponent (Component * lastComponent,
                               Group * group, FILE * filePtr,
                               int specialWriteAppend, const char * tag)
{

   if (!filePtr) return 1;

   // define a flag, isWritten, to be 1 if the component has already
   // been written, at some place, in the runstream.  if isWritten, some
   // of the output data is not repeated.

   int isWritten = 0;
   if (!specialWriteAppend)
   {
      isWritten = theSpGlobals->runStream->writtenComponents->
                                               contains(this);
      if (!isWritten) theSpGlobals->runStream->writtenComponents->
                                               insert(this);
   }

   // before this component is written, we have to deal with the end
   // of the last component.

   BOOLEAN lastComponentIsNOTaBaseComponent =
           (lastComponent && lastComponent->extension &&
            lastComponent->extension->index("base",0,RWCString::ignoreCase) ==
            RW_NPOS);

   if (lastComponent && lastComponent->isA() == PPECOMPONENT)
   {
      lastComponentIsNOTaBaseComponent =
            (lastComponent->extension &&
             lastComponent->extension->index("ppe",0,RWCString::ignoreCase) ==
             RW_NPOS);
   }

   // rule 1:  if the current component type is 1 or 2, and the lastComponent
   //          type is 1 or 3, then write an EndIf line.

   if (componentType == 1 || componentType == 2)
   {
      if (lastComponent && (lastComponent->componentType == 1 ||
                            lastComponent->componentType == 3))
      {
         // an EndIf is not part of any extensions except the base.
         // so, if the lastComponent is part of an extension, first
         // write an End, then the EndIf, and then set the lastComponent
         // to NULL.

         if (lastComponentIsNOTaBaseComponent) fprintf (filePtr,"End\n");

         // if a group is defined, and the component is "inside" and IF group,
         // then write the activity group policy label.

         if (!specialWriteAppend && group)
         {
            fprintf (filePtr,"\nAGPLabel\n  %s\n",group->data());
         }

         fprintf (filePtr,"EndIf\n");
         if (lastComponentIsNOTaBaseComponent)
         {
            lastComponent=NULL;
            lastComponentIsNOTaBaseComponent=FALSE;
         }
      }
   }

   // rule 2:  if the current compoenent is marked part of a different
   //          extension than the lastComponent, then write an End line.
   //          A NULL pointer for extension is the same as having the
   //          extension point to the string "base".

   if (lastComponentIsNOTaBaseComponent)
   {
      // the last component is NOT part of the base.

      if (!extension ||
          (extension &&
          strcmp(extension->data(),lastComponent->extension->data()) != 0))

         fprintf (filePtr,"End\n");
   }

   // rule 3:  if the current component is part of the an extension
   //          and the last component is not part of the same extension
   //          then write the correct "header" record for the component.

   // subrule 3: if specialWriteAppend is true (the component is being
   //            written to something other than a runstream), then
   //            do not write the header record.

   char exten[10];

   if (!specialWriteAppend && extension &&
       strncmp(extension->data(),"base",4) != 0)
   {
      if (!lastComponent ||
          (lastComponent && !lastComponent->extension) ||
          (lastComponent &&  lastComponent->extension  &&
          strcmp(lastComponent->extension->data(),extension->data()) != 0))
      {

         strncpy(exten,extension->data(),9);
         strtok(exten," ");
         char * cp;
         for (cp=exten; *cp != NULL; cp++)
            *cp = (char) tolower((int) *cp);

         PKeyData * pkey = GetPKeyData("extensPrefixes",exten);

         if (pkey)
         {
            if (keywords &&
                keywords->index(pkey->pString) != 0)
               fprintf (filePtr,"%s\n",pkey->pString.data());
         }
      }
   }

   const char * t = "!!C";
   if (tag) t=tag;
   fprintf (filePtr,"%s %s~%d %d %d ", t, data(), componentNumber, componentType,
                                                conditionSerialNum);
   if (variant) fprintf (filePtr,"%s ", variant->data());
   else         fprintf (filePtr,"@ ");
   if (extension) fprintf (filePtr,"%s ", extension->data());
   else           fprintf (filePtr,"@ ");
   if (group)
      fprintf (filePtr,"%s %d\n", group->data(), group->groupNumber);
   else
      fprintf (filePtr,"@\n");

   if (sourceWindow && !isWritten)
   {
      fprintf (filePtr,"!!SW %s\n", sourceWindow->data());
      if (windowParms)
         WriteWindowParms (filePtr, windowParms);
   }

   // write the comment, if present

   if (comment && comment->length())
   {
      if (!isWritten)
      {
         *comment = comment->strip(both,'\n');
         *comment = comment->strip(both,' ');
      }
      fprintf (filePtr,"!!CE %d\nComment\n%s\nEnd\n",
               userEditedComment, comment->data());
   }

   // write the keywords, if present (they should be!)

   if (keywords)
   {
      *keywords = keywords->strip(both,'\n');
      *keywords = keywords->strip(both,' ');
      fprintf (filePtr,"!!K\n%s\n!!End\n", keywords->data());
   }

   return 0;
}




int Component::ReadComponent ( Stand * stand, FILE * filePtr,
                               CStringRW * Tbuff, char * buff )
{
   char * ptr;
   int number;

   // get componentType

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%d", &componentType);
   else return 1;

   // get conditionSerialNum

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%d", &conditionSerialNum);
   else return 1;

   // get variant data, if any.

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%s", buff);
   else return 1;
   if (*buff != '@' && !variant) variant =
                                 new CStringRW (buff, strlen(buff)+1);

   // get extension data, if any.

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%s", buff);
   else return 1;
   if (*buff != '@' && !extension) extension =
                                   new CStringRW (buff, strlen(buff)+1);

   // signal that an extension is in use.

   if (extension) theSpGlobals->theExtensions->
      AddExtensionInUse(extension->data());

   // get group

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%s", buff);
   else return 1;
   Group * group = (Group *) 0;

   // the group "name" is @, then the component is not part of a group,
   // just part of the stand.

   if (*buff == '@')
   {
      // add the component pointer to the stand's list of
      // componentsOrGroups.

      stand->AddComponentOrGroupToStand(this);

   }
   else
   {
      // get the group name and number.  find out if it already exists.

      ptr = strtok(NULL," \n");
      if (ptr) sscanf (ptr,"%d", &number);
      group = theSpGlobals->runStream->FindGroup (buff,number);

      // if the group does not yet exist, create it, add it to the run.

      if (!group)
      {
         group = new Group (buff, number);
         theSpGlobals->runStream->AddGroup(group);
      }

      // Then add the component to the group (if it is already there, it
      // will not be added, so there is no need to check), and add the
      // stand to the group's standMembership.

      group->AddComponentToGroup(this);
      group->AddStandToGroup(stand);
   }

   // ready to read the contents of the component.

   if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;

   if (strncmp (buff,"!!SW",4) == 0)
   {
      ptr = strtok(buff," \n");
      ptr = strtok(NULL," \n");
      if (sourceWindow) *sourceWindow = ptr;
      else sourceWindow = new CStringRW (ptr, strlen(ptr));
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      if (strncmp (buff,"!!P ",4) == 0)
      {
        if (ReadWindowParms(filePtr, buff, &windowParms)) return EOF;
      }
   }

   // get the "userEditedComment" field, if it is present.

   if (strncmp (buff,"!!CE",4) == 0)
   {
      sscanf (buff+4,"%d", &userEditedComment);
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }

   // can't call ReadRunSegment because we're looking
   // for End not !!End

   if (strncmp (buff,"Comment",7) == 0)
   {
      Tbuff->remove(0);  // clear Tbuff
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      while (strncmp (buff,"End\n",4) != 0)
      {
         if (!comment) *Tbuff += buff;  // if already defined, don't save
         if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      }
      if (!comment) // if already defined, don't redefine.
      {
         Tbuff->remove(Tbuff->length()-1);
         comment = new CStringRW (Tbuff->data(), Tbuff->length());
      }
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }

   // always insure defined components have a NULL comment.

   else comment = NULL;

   if (strncmp (buff,"!!K\n",4) == 0)
      return ReadRunSegment(&keywords, filePtr, buff, Tbuff);
   return 0;
}


RWDEFINE_COLLECTABLE(PPEComponent,PPECOMPONENT)


PPEComponent::PPEComponent ( const char * name, int CNumber)
             :Component (name,CNumber)
{}

PPEComponent::PPEComponent ( void )
             :Component ()
{}

PPEComponent::PPEComponent ( PPEComponent * c)
             :Component (c)
{}

PPEComponent::PPEComponent ( Component * c)
             :Component (c)
{}

PPEComponent::~PPEComponent ( void )
{}




int PPEComponent::WriteComponent (Component * lastComponent,
                                  Group * group, FILE * filePtr,
                                  int specialWriteAppend, const char * tag)
{
   return Component::WriteComponent(lastComponent,group,
                                    filePtr,specialWriteAppend,tag);
}


int PPEComponent::ReadComponent ( FILE * filePtr,
                                  CStringRW * Tbuff, char * buff)
{

   char * ptr;
   int number;

   // get componentType

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%d", &componentType);
   else return 1;

   // get conditionSerialNum

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%d", &conditionSerialNum);
   else return 1;

   // get variant data, if any.

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%s", buff);
   else return 1;
   if (*buff != '@' && !variant) variant =
                                 new CStringRW (buff, strlen(buff)+1);

   // get extension data, if any.

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%s", buff);
   else return 1;
   if (*buff != '@' && !extension) extension =
                                   new CStringRW (buff, strlen(buff)+1);

   // signal that an extension is in use.

   if (extension) theSpGlobals->theExtensions->
      AddExtensionInUse(extension->data());

   // get group...is will always be equal to @ for ppeComponents.

   ptr = strtok(NULL," \n");
   if (ptr) sscanf (ptr,"%s", buff);
   else return 1;

   // ready to read the contents of the component.

   if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;

   if (strncmp (buff,"!!SW",4) == 0)
   {
      ptr = strtok(buff," \n");
      ptr = strtok(NULL," \n");
      if (sourceWindow) *sourceWindow = ptr;
      else sourceWindow = new CStringRW (ptr, strlen(ptr));
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      if (strncmp (buff,"!!P ",4) == 0)
      {
        if (ReadWindowParms(filePtr, buff, &windowParms)) return EOF;
      }
   }

   // get the "userEditedComment" field, if it is present.

   if (strncmp (buff,"!!CE",4) == 0)
   {
      sscanf (buff+4,"%d", &userEditedComment);
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }

   // can't call ReadRunSegment because we're looking
   // for End not !!End

   if (strncmp (buff,"Comment",7) == 0)
   {
      Tbuff->remove(0);  // clear Tbuff
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      while (strncmp (buff,"End\n",4) != 0)
      {
         if (!comment) *Tbuff += buff;  // if already defined, don't save
         if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      }
      if (!comment) // if already defined, don't redefine.
      {
         Tbuff->remove(Tbuff->length()-1);
         comment = new CStringRW (Tbuff->data(), Tbuff->length());
      }
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }

   // always insure defined components have a NULL comment.

   else comment = NULL;

   if (strncmp (buff,"!!K\n",4) == 0)
      return ReadRunSegment(&keywords, filePtr, buff, Tbuff);
   return 0;
}
