/********************************************************************

   File Name:     group.cpp
   Author:        nlc
   Date:          05/30/95

   Purpose:       Stands can be in groups and groups hold a list of 
                  components.  This class defines and provides functionality
                  to groups.

   groupNumber
       a unique serial number.  the string name and the number of each
       group must be unique.

   componentList
       RWOrdered list of components which are attached to the group.
       This class does NOT own the members of this shallow list.

   standMembership
       RWOrdered list of stands which are attached to the group.
       This class does NOT own the members of this shallow list.
*
*********************************************************************/


#ifndef Group_h
#define Group_h

#include "PwrDef.h"
#include CStringRWC_i
#include "spglobal.hpp"
#include "runstr.hpp"

class RWOrdered;
class Stand;
class Component;

class Group : public CStringRWC
{

   RWDECLARE_COLLECTABLE(Group)

   public:
    
      Group ( const char * name,
              int GNumber = theSpGlobals->runStream->groupNumbers++);
      Group ( Group * c);   // copy constructor
      Group ( void );
      virtual ~Group ( void );

      int AddStandToGroup ( Stand * stand );
      int DelStandFromGroup ( Stand * stand );

      int AddComponentToGroup ( Component * c,
                                Component * place = (Component *) 0,
                                int         after = 0);
      int DelComponentFromGroup ( Component * c );

      int IsComponentInGroup ( Component * c );

      int CopyComponentsFromGroup( Group * c);

      void ClearStandMembership( void );

      virtual RWBoolean Group::isEqual(const RWCollectable *c) const;

      int           groupNumber;
      RWOrdered   * componentList;
      RWOrdered   * standMembership;
      int           autoAddsDone;

};


#endif //Group_h
