/********************************************************************

   File Name:     group.cpp
   Author:        nlc
   Date:          05/30/95

   Purpose:       see group.hpp

*********************************************************************/

#include "PwrDef.h"
#include CStringRWC_i
#include <rw/ordcltn.h>
#include <rw/vstream.h>  // for debugging.
#include "group.hpp"
#include "compon.hpp"
#include "stand.hpp"
#include "runstr.hpp"
#include "spglobal.hpp"
#include "suppdefs.hpp"

RWDEFINE_COLLECTABLE(Group,GROUP)

Group::Group ( const char * name, int GNumber )
      :CStringRWC ( name )

{
   groupNumber     = GNumber;
   if (groupNumber >= theSpGlobals->runStream->groupNumbers)
                      theSpGlobals->runStream->groupNumbers = groupNumber+1;
   componentList   = (RWOrdered *) 0;
   standMembership = (RWOrdered *) 0;
   autoAddsDone    = 0;

}


Group::Group ( void )
      :CStringRWC ()
{
   groupNumber     = -1;
   componentList   = (RWOrdered *) 0;
   standMembership = (RWOrdered *) 0;
   autoAddsDone    = 0;
}



Group::Group ( Group * c)
      :CStringRWC ((CStringRWC &) *c)
{
   // Copy constructor.
   // There are NO (important) side effects.  
   // The group is duplicated and the standMembership list is
   // also duplicated.  But the component list is NOT duplicated nor
   // are the components added to the stands that are members of this
   // group; call CopyComponentsFromGroup to achieve that result.

   groupNumber = theSpGlobals->runStream->groupNumbers++;

   componentList = (RWOrdered *) 0;

   if (c->standMembership)
   {
      standMembership = new RWOrdered(c->standMembership->entries());
      RWOrderedIterator next(*c->standMembership);
      Stand * item;
      while (item = (Stand *) next()) standMembership->insert(item);
   }
   else standMembership = (RWOrdered *) 0;

   autoAddsDone = 0;
}



int Group::CopyComponentsFromGroup( Group * c)
{
   // Return code:
   // 0 = copy is ok.
   // 1 = there are already components in this group.
   // 2 = group c is not a group.
   //
   // *** WARNING *** This routine has side effects (in fact, they are
   // the main event!)
   //
   // 1) This group is added to the runstream
   // 2) The list of components from the source group are copied to this
   //    group's componentList.
   // 3) This group is added to the standMembership's list of
   //    components or groups.

   if (c->isA() != GROUP) return 2;

   theSpGlobals->runStream->AddGroup(this);

   if (componentList) return 1;

   if (!c->componentList) return 0;

   componentList = new RWOrdered(c->componentList->entries());
   RWOrderedIterator next(*c->componentList);
   Component * item;
   while (item = (Component *) next()) 
   {
      componentList->insert(item);
   }
   autoAddsDone = c->autoAddsDone;

   if (standMembership)
   {
      RWOrderedIterator nextStand(*standMembership);
      Stand * stand;
      while (stand = (Stand *) nextStand())
      {
         stand->AddComponentOrGroupToStand(this);
      }
   }
   return 0;
}




Group::~Group ( void )
{
   if (componentList) delete componentList;
   if (standMembership) delete standMembership;
}


RWBoolean Group::isEqual(const RWCollectable *c) const
{

   // Two groups are equal if they are at the same address OR
   // if their name is the same and their number is the same.

   return                   this               ==                 c ||
          ( (CStringRWC &) *this               == (CStringRWC &) *c &&
                ((Group *)  this)->groupNumber ==      ((Group *) c)->groupNumber);

}




int Group::AddStandToGroup ( Stand * stand )
{
   // Return codes:
   // 0 = All went well.
   // 1 = Stand was already in the group.
   // 2 = Adding the stand would imply adding a duplicate to the subset.
   // 3 = What your trying to add is not a stand.

   if (stand->isA() != STAND) return 3;

   // is the stand in the current group?

   if (standMembership)
   {
      if (standMembership->contains(stand)) return 1;

      // if the stand the group is in the current subset, we need to find out
      // if adding the stand would imply adding a duplicate to the subset.

      if (theSpGlobals->runStream->subsetGroups->contains(this))
      {
         if (theSpGlobals->runStream->subsetStands->contains(stand)) return 2;
         RWOrderedIterator subsetGroup(*theSpGlobals->runStream->subsetGroups);
         Group * group;
         while (group = (Group *) subsetGroup())
         {
            if (group->standMembership &&
                group->standMembership->contains(stand)) return 2;
         }
      }
   }
   else
   {
      standMembership = new RWOrdered(30);
   }

   // add the stand to the group's membership list.

   standMembership->insert(stand);

   // if the group has components, add the group to the stand's list
   // of componentsOrGroups.

   if (componentList) stand->AddComponentOrGroupToStand (this);

   // insure that the stand is in the list of all stands.

   theSpGlobals->runStream->AddStand(stand);

   return 0;
}





int Group::DelStandFromGroup ( Stand * stand )
{
   // Return codes:
   // 0 = all went well
   // 1 = stand is not a stand

   if (stand->isA() != STAND) return 1;

   // remove group from stand's list of components or groups.

   stand->DelComponentOrGroupFromStand(this);

   if (standMembership)
   {
      standMembership->remove(stand);

      if (standMembership->entries() == 0)
      {
         delete standMembership;
         standMembership = (RWOrdered *) 0;
      }
   }
   return 0;
}




void Group::ClearStandMembership( void )
{
   if (standMembership)
   {
      Stand * stand = (Stand *) standMembership->pop();
      if (stand)
      {
         stand->DelComponentOrGroupFromStand(this);
         if (standMembership && standMembership->entries() == 0)
         {
            delete standMembership;
            standMembership = (RWOrdered *) 0;
         }
         else ClearStandMembership();
      }
   }
}  





int Group::AddComponentToGroup ( Component * c,
                                 Component * place,
                                 int         after)
{
   // Return codes:
   // 0 = all went well
   // 1 = item is not a component

   if (c->isA() != COMPONENT) return 1;
   if (place && place->isA() != COMPONENT) return 1;

   if (componentList)
   {
      if (componentList->contains(c))
      {
         if (InvalidPlacement(componentList, c)) 
         {
            componentList->remove(c);
            AddComponentToGroup(c);
         }
      }
      else
      {
         Component * placeInList = place;
         if (!placeInList) placeInList = FindInsertPlace (componentList, 
                                                          c, &after);
         if (placeInList)
         {
            
            // if place is not in the list, don't add c.
            
            size_t location = componentList->index(placeInList);
            if (location != RW_NPOS) 
            {
               if (after)
               {
                  if (location < componentList->entries())
                  componentList->insertAt(++location, c);
                  else componentList->insert(c);
               }
               else componentList->insertAt(location, c);
            }
         }
         else componentList->insert(c);
      }
   }
   else
   {               // if the list is empty, then "place"
      if (!place)  // does not exist in the list.
      {
         componentList = new RWOrdered(10);
         componentList->insert(c);

         // if the group has stands, then add the group to the stand's
         // list of components or groups.

         if (standMembership)
         {
            RWOrderedIterator next(*standMembership);
            Stand * stand;
            while (stand = (Stand *) next())
            {
               stand->AddComponentOrGroupToStand (this, NULL);
            }
         }
      }
   }
   return 0;
}





int Group::DelComponentFromGroup ( Component * c )
{
   // Return codes:
   // 0 = all went well
   // 1 = item is not a c

   if (c->isA() != COMPONENT) return 1;

   if (componentList)
   {
      if (componentList->remove(c))
      {
         // if deleting the c results in deleting all
         // the components, then remove the group from every
         // stand's list of componentsOrGroups, and delete
         // the componentList.

         if (componentList->entries() == 0)
         {
            if (standMembership)
            {
               RWOrderedIterator next(*standMembership);
               Stand * stand;
               while (stand = (Stand *) next())
               {
                  stand->DelComponentOrGroupFromStand (this);
               }
            }
            delete componentList;
            componentList = (RWOrdered *) 0;
         }
      }
   }
   return 0;
}


int Group::IsComponentInGroup ( Component * c )
{
   // Return codes:
   // 0 = c is not in the group (or it is not a component).
   // 1 = c is in the group.

   if (c->isA() != COMPONENT ||
       !componentList)           return 0;

   if (componentList->contains(c)) return 1;
   return 0;
}
