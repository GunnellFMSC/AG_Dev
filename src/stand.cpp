/********************************************************************

   File Name:     stand.cpp
   Author:        nlc
   Date:          05/30/95

   Purpose:       see stand.hpp

*********************************************************************/

#include "PwrDef.h"
#include CStringRWC_i
#include <rw/ordcltn.h>
#include <rw/vstream.h>  // for debugging.
#include "suppdefs.hpp"
#include "stand.hpp"
#include "compon.hpp"
#include "group.hpp"
#include "runstr.hpp"
#include "variants.hpp"
#include "spglobal.hpp"
#include <stdio.h>

RWDEFINE_COLLECTABLE(Stand,STAND)

Stand::Stand ( const char * name, int SNumber )
      :CStringRWC ( name )

{
   standNumber        = SNumber;
   originID           = (CStringRW *) 0;
   mgmtID             = (CStringRW *) 0;
   comment            = (CStringRW *) 0;
   sourceWindow       = (CStringRW *) 0;
   windowParms        = (CStringRW *) 0;
   siteKeywords       = (CStringRW *) 0;
   treeKeywords       = (CStringRW *) 0;
   timingKeywords     = (CStringRW *) 0;
   variants           = (CStringRW *) 0;
   policyKeywords     = (CStringRW *) 0;
   componentsOrGroups = (RWOrdered *) 0;
}


Stand::Stand ( void )
      :CStringRWC ()
{
   standNumber        = -1;
   originID           = (CStringRW *) 0;
   mgmtID             = (CStringRW *) 0;
   comment            = (CStringRW *) 0;
   sourceWindow       = (CStringRW *) 0;
   windowParms        = (CStringRW *) 0;
   siteKeywords       = (CStringRW *) 0;
   treeKeywords       = (CStringRW *) 0;
   timingKeywords     = (CStringRW *) 0;
   variants           = (CStringRW *) 0;
   policyKeywords     = (CStringRW *) 0;
   componentsOrGroups = (RWOrdered *) 0;
}


Stand::Stand ( Stand * c )
      :CStringRWC ((CStringRWC &) *c)
{

   // Copy constructor.
   // This constructor creates a copy of the stand BUT, it does
   // NOT copy the list of componentsOrGroups NOR does it add the
   // stand to the runstream.  Call CopyComponentsFromStand to
   // accomplish that task.

   standNumber = theSpGlobals->runStream->standNumbers++;
   if (c->originID) originID = new CStringRW(*c->originID);
   else originID = (CStringRW *) 0;
   if (c->mgmtID) mgmtID = new CStringRW(*c->mgmtID);
   else mgmtID = (CStringRW *) 0;
   if (c->comment) comment = new CStringRW(*c->comment);
   else comment = (CStringRW *) 0;
   if (c->sourceWindow) sourceWindow = new CStringRW(*c->sourceWindow);
   else sourceWindow = (CStringRW *) 0;
   if (c->windowParms) windowParms = new CStringRW(*c->windowParms);
   else windowParms = (CStringRW *) 0;
   if (c->siteKeywords) siteKeywords = new CStringRW(*c->siteKeywords);
   else siteKeywords = (CStringRW *) 0;
   if (c->treeKeywords) treeKeywords = new CStringRW(*c->treeKeywords);
   else treeKeywords = (CStringRW *) 0;
   if (c->timingKeywords) timingKeywords = new CStringRW(*c->timingKeywords);
   else timingKeywords = (CStringRW *) 0;
   if (c->variants) variants = new CStringRW(*c->variants);
   else variants = (CStringRW *) 0;
   if (c->policyKeywords) policyKeywords = new CStringRW(*c->policyKeywords);
   else policyKeywords = (CStringRW *) 0;

   componentsOrGroups = (RWOrdered *) 0;
}


Stand::~Stand ( void )
{
   if (comment)            delete comment;
   if (originID)           delete originID;
   if (mgmtID)             delete mgmtID;
   if (sourceWindow)       delete sourceWindow;
   if (windowParms)        delete windowParms;
   if (siteKeywords)       delete siteKeywords;
   if (treeKeywords)       delete treeKeywords;
   if (timingKeywords)     delete timingKeywords;
   if (variants)           delete variants;
   if (policyKeywords)     delete policyKeywords;
   if (componentsOrGroups) delete componentsOrGroups;
}


RWBoolean Stand::isEqual(const RWCollectable *c) const
{

   // two stands are equal if they are at the same address OR
   // if their name is the same and their number is the same.

   return                   this               ==                 c ||
          ( (CStringRWC &) *this               == (CStringRWC &) *c &&
                ((Stand *)  this)->standNumber ==      ((Stand *) c)->standNumber);

}




int Stand::WriteStand ( FILE * filePtr )
{
   if (!filePtr) return 1;

   fprintf (filePtr,"!!Stand %s %d\n", data(), standNumber);

   if (originID)
   {
      if (originID->index(*this, this->length(),
                          0, RWCString::exact) == RW_NPOS)
      {
         originID->prepend(" was ");
         originID->prepend(*this);
      }
      fprintf (filePtr,"StdIdent\n%-8.26s %s\n", data(), originID->data());
   }
   else
      fprintf (filePtr,"StdIdent\n%-8.26s", data());

   if (mgmtID)
      fprintf (filePtr,"MgmtID\n%-4.4s\n", mgmtID->data());
   
   if (comment)
   {
      *comment = comment->strip(both,'\n');
      *comment = comment->strip(both,' ');
      fprintf (filePtr,"Comment\n%s\nEnd\n", comment->data());
   }
   if (sourceWindow)
   {
      fprintf (filePtr,"!!SW %s\n", sourceWindow->data());
      if (windowParms)
         WriteWindowParms (filePtr, windowParms);
   }
   if (siteKeywords)
   {
      fprintf (filePtr,"!!SK");
      if (variants) fprintf (filePtr," %s", variants->data());
      else          fprintf (filePtr," @");
      *siteKeywords = siteKeywords->strip(trailing,(char) '\n');
      fprintf (filePtr,"\n%s\n!!End\n",siteKeywords->data()); 
   }
   if (timingKeywords)
      fprintf (filePtr,"!!TK\n%s\n!!End\n", timingKeywords->data());

   // loop over all the componentsOrGroups

   Component * lastComponent = (Component *) 0;
   if (componentsOrGroups)
   {
      RWOrderedIterator next(*componentsOrGroups);
      RWCollectable * item;
      while (item = (RWCollectable *) next())
      {
         if (item->isA() == COMPONENT) 
         {
            ((Component *) item)->WriteComponent(lastComponent, 
                                                 NULL, filePtr);
            lastComponent = (Component *) item;
         }
         if (item->isA() == GROUP)
         {
            if (((Group *) item)->componentList)
            {
               RWOrderedIterator nextComponent(*((Group *) item)->componentList);
               Component * component;
               while (component = (Component *) nextComponent())
               {
                  component->WriteComponent (lastComponent, 
                                             (Group *) item, filePtr);
                  lastComponent = (Component *) component;
               }
            }
         }
      }

      // done writting components.  

      // if the lastComponent is realted to an extension, then write an End.

      if (lastComponent && lastComponent->extension &&
                    strncmp(lastComponent->extension->data(),"base",4) != 0)
         fprintf (filePtr,"End\n");

      // if the lastComponent component type is 1 or 3,
      // then write an EndIf line.

      if (lastComponent && (lastComponent->componentType == 1 ||
                            lastComponent->componentType == 3))
      fprintf (filePtr,"EndIf\n");
   }

   // write the treeKeywords
   
   if (treeKeywords)
      fprintf (filePtr,"!!TR\n%s\n!!End\n", treeKeywords->data());

   // Create the SPLabel keywords...loop over all groups.  Add
   // a group id if the stand is a member of the group.
   // Always start with a fresh set.

   if (policyKeywords)
   {
      delete policyKeywords;
      policyKeywords=NULL;
   }


   RWOrderedIterator nextGroup(*theSpGlobals->runStream->allGroups);
   Group * group;
   while (group = (Group *) nextGroup())
   {
      if (group->standMembership &&
          group->standMembership->contains(this)) 
      {
         if (!policyKeywords)
         {
            policyKeywords  = new CStringRW("SPLabel\n   ");
            *policyKeywords += ((CStringRWC *) group)->data();
         }
         else 
         {
            *policyKeywords += ", &\n   ";
            *policyKeywords += ((CStringRWC *) group)->data();
         } 
      }
   }

   if (policyKeywords)
      fprintf (filePtr,"%s\n", policyKeywords->data());

   fprintf (filePtr,"Process\n!!EndStand\n");
   return 0;
}




int Stand::ReadStand ( FILE * filePtr, CStringRW * Tbuff, char * buff )
{
   char * ptr;
   char * name;
   int number;
   Component * component;
   
   if (!filePtr) return 1;

   if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;

   if (strncmp (buff,"StdIdent",8) == 0) 
   {
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      ptr = strtok(buff," ");
      ptr = strtok(NULL,"\n");
      while (*ptr == ' ') ptr++;
      if (ptr) originID = new CStringRW(ptr);
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }

   if (strncmp (buff,"MgmtID",6) == 0) 
   {
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      if (buff) mgmtID = new CStringRW(buff);
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }

                                          // can't call ReadRunSegment
   if (strncmp (buff,"Comment",7) == 0)   // because we're looking of End
   {                                      // not !!End
      Tbuff->remove(0);  // clear Tbuff
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      while (strncmp (buff,"End\n",4) != 0)
      {
         *Tbuff += buff;
         if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      }
      Tbuff->remove(Tbuff->length()-1);
      comment = new CStringRW (Tbuff->data(), Tbuff->length());
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }
      
   if (strncmp (buff,"!!SW",4) == 0)
   {
      ptr = strtok(buff," \n");
      name = strtok(NULL," \n");
      sourceWindow = new CStringRW (name, strlen(name));
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      if (strncmp (buff,"!!P ",4) == 0)
         if (ReadWindowParms(filePtr, buff, &windowParms)) return EOF;
   }

   if (strncmp (buff,"!!SK",4) == 0)
   {
      name = buff+5;
      if (*name != '@')
      {
         *(name+strlen(name)-1) = 0;  // remove the line feed.
         variants = new CStringRW (name, strlen(name));
         theSpGlobals->theVariants->NewStandVariantsInteraction( name );
      }
      if (ReadRunSegment(&siteKeywords, filePtr, buff, Tbuff)) return EOF;
   }
      
   if (strncmp (buff,"!!TK",4) == 0)
      if (ReadRunSegment(&timingKeywords, filePtr, buff, Tbuff)) return EOF;
            
   if (strncmp (buff,"!!C ",4) == 0)
   {
      while (strncmp (buff,"!!C ",4) == 0)
      {
         // parse the name and number, if the component does not
         // already exist then create it.  Call the reader on
         // the component passing the entire string.
      
         ptr = strtok(buff," \n");
         name = strtok(NULL,"~\n");
         ptr = strtok(NULL," \n");
         sscanf (ptr,"%d", &number);
         component = theSpGlobals->runStream->FindComponent(name,number);
         if (!component) 
         {
            component = new Component(name,number);
            theSpGlobals->runStream->allComponents->insert(component);
         }
         number = component->ReadComponent (this, filePtr, Tbuff, buff);
         if (number) return number;

         // skip to the next Suppose control line, marked by !!.

         while (strncmp (buff,"!!",2) != 0)
            if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      }
   }

   if (strncmp (buff,"!!TR",4) == 0)
      if (ReadRunSegment(&treeKeywords, filePtr, buff, Tbuff)) return EOF;

   // through away any more keywords (the policy keywords, for example).

   while (strncmp (buff,"!!EndStand",10) != 0)
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   return 0;
}



int Stand::AddComponentOrGroupToStand (RWCollectable * c,
                                       RWCollectable * place,
                                       int             after)
{
   // Return code:
   // 0 = all went well
   // 1 = item is not a component or group.

   if (c->isA() != COMPONENT && c->isA() != GROUP) return 1;
   if (place && 
      (place->isA() != COMPONENT && place->isA() != GROUP)) return 1;

   // if componentsOrGroups is NULL then create it

   if (!componentsOrGroups) 
   {
      componentsOrGroups = new RWOrdered(10);
      componentsOrGroups->insert(c);
   }
   else
   {
      // if item is already in the list, don't add it.

      if (componentsOrGroups->contains(c))
      {
         if (InvalidPlacement(componentsOrGroups, (Component *) c)) 
         {
            componentsOrGroups->remove(c);
            AddComponentOrGroupToStand(c);
         }
      }
      else
      {
         RWCollectable * placeInList = place;
         if (!placeInList && c->isA() == COMPONENT) placeInList = 
            (RWCollectable *) FindInsertPlace (componentsOrGroups,
                                               (Component *) c, &after); 
         if (placeInList)
         {
            // if place is not in the list, don't add component.
            
            size_t location = componentsOrGroups->index(placeInList);
            if (location != RW_NPOS) 
            {
               if (after)
               {
                  if (location < componentsOrGroups->entries())
                  componentsOrGroups->insertAt(location+1, c);
                  else componentsOrGroups->insert(c);
               }
               else componentsOrGroups->insertAt(location, c);
            }
         }
         else componentsOrGroups->insert(c);
      }
   }
   return 0;
}




int Stand::CopyComponentsOrGroupsFromStand (Stand * c)
{
   // Return code:
   // 0 = all went well
   // 1 = there are already componentsOrGroups in the stand.
   // 2 = making the copy would result in the stand being duplicated
   //     in the current subset.
   // 3 = c is not a stand.
   //
   // *** WARNING *** This routine has side effects (in fact, they are
   // the main event!)
   //
   // 1) This stand is added to the runstream
   // 2) The list of componentsOrGroups from the source stand are copied 
   //    to this stands's componentsOrGroups list.
   // 3) If the item copied is a group, then this stand is added to the 
   //    standMembership list of the group.
  
   if (c->isA() != STAND) return 3;

   // if componentsOrGroups already has elements, return.

   if (componentsOrGroups) return 1;

   if (!c->componentsOrGroups) 
   {
      theSpGlobals->runStream->AddStand(this);
      return 0;
   }

   componentsOrGroups = new RWOrdered(c->componentsOrGroups->entries());
   RWOrderedIterator next(*c->componentsOrGroups);
   RWCollectable * item;
   while (item = (RWCollectable *) next())
   {
      componentsOrGroups->insert(item);
      int returnCode = 0;
      if (item->isA() == GROUP) 
         returnCode = ((Group *) item)->AddStandToGroup(this);
      if (returnCode == 2)
      {
         componentsOrGroups->clear();
         delete componentsOrGroups;
         componentsOrGroups = (RWOrdered *) 0;
         return 2;
      }
   }

   theSpGlobals->runStream->AddStand(this);

   return 0;
}




int Stand::DelComponentOrGroupFromStand (RWCollectable *c)
{
   // Return code:
   // 0 = all went well
   // 1 = what you want to delete is not a component or group.

   if (c->isA() != COMPONENT && c->isA() != GROUP) return 1;

   if (componentsOrGroups) 
   {
      if (componentsOrGroups->remove(c))
      {
         if (componentsOrGroups->entries() == 0)
         {
            delete componentsOrGroups;
            componentsOrGroups = (RWOrdered *) 0;
         }
      }
   }
   return 0;
}



int Stand::IsComponentInStand (const Component * c)
{
   // Return code:
   // 0 = c is not in the stand (or it is not a component).
   // 1 = c is in the stand.

   if (c->isA() != COMPONENT ||
       !componentsOrGroups)     return 0;

   if (componentsOrGroups->contains(c)) return 1;
   return 0;
}
