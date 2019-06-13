/********************************************************************

   File Name:     stand.hpp
   Author:        nlc
   Date:          05/30/95

   Purpose:       A Stand is a sampled unit of land.  A Stand class
                  holds the data necessary to build the stand simulation.

   Class membership notes:

   standNumber
       a unique serial number.  the name and the number of each
       stand must be unique.

   mgmtID
       the stand management ID.

   comment
       a comment string for the stand (if NULL, then there is none).

   sourceWindow
       the name of the source window (NULL for stands that come from a
       data base.

   windowParms
       the window parms for the sourceWindow, if any.

   siteKeywords
       base keyword set for site variables.

   treeKeywords
       keywords used to point to the tree data.

   timingKeywords
       TimeInt keywords, and etc.

   policyKeywords
       SpLabel keyword, if used.

   componentsOrGroups
       RWOrdered list of components or groups that make up the keyword
       set associated with this stand.  If a group is a member, then
       the components in that group are placed in order.
       This class does NOT own the members of this shallow list.
*
*********************************************************************/

#ifndef Stand_h
#define Stand_h

#include "PwrDef.h"
#include CStringRWC_i
#include "spglobal.hpp"
#include "runstr.hpp"

class RWOrdered;
class CStringRW;

class Stand : public CStringRWC
{

   RWDECLARE_COLLECTABLE(Stand)

   public:

      Stand ( const char * name, int SNumber = theSpGlobals->runStream->standNumbers++);
      Stand ( void );
      Stand ( Stand * c);   // copy constructor
      virtual ~Stand ( void );

      int WriteStand ( FILE * filePtr );
      int ReadStand (  FILE * filePtr, CStringRW * Tbuff, char * buff );

      int AddComponentOrGroupToStand (RWCollectable * c, RWCollectable * place = (RWCollectable *) 0, int after = 0);

      int DelComponentOrGroupFromStand (RWCollectable * c);

      int Stand::IsComponentInStand (const Component * c);

      int CopyComponentsOrGroupsFromStand (Stand * c);

      virtual RWBoolean isEqual(const RWCollectable *c) const;

      int           standNumber;
      CStringRW   * mgmtID;    // PWR_MANGLE(CStringRW)  * mgmtID
      CStringRW   * originID;
      CStringRW   * comment;
      CStringRW   * sourceWindow;
      CStringRW   * windowParms;
      CStringRW   * siteKeywords;
      CStringRW   * treeKeywords;
      CStringRW   * timingKeywords;
      CStringRW   * variants;
      CStringRW   * policyKeywords;
      RWOrdered   * componentsOrGroups;

};

#endif //Stand_h
