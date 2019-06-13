 /********************************************************************

   File Name:     runstr.hpp
   Author:        nlc
   Date:          05/30/95 - 06/23/95, 10/11/2006

                             RunStream Data Structure

   This is a description of the data structure used by Suppose to keep track
   of Stands, Groups of stands, and Components.  These data are housed in a
   class called RunStream.

   Classes and definitions

      class Stand inherents CStringRWC, making it an RWCollectible.  It
   contains member data that defines it by a unique serial number, the
   sourceWindow name, the siteKeywords, the list of applicable variants,
   and a list of componentsOrGroups.  Stands can be cast CStringRW's.

      class Groups inherents CStringRWC and contains both a list of
   components and a list of standMembership.  It is known by the data
   associated with CStringRWC and a unique serial number.  Groups can be
   cast CStringRW's.

      class Component inherents CStringRWC and contains severial
   CStringRW members that define the keywords and so on associated with a
   component.  Components can be cast CStringRW.

      class RunStream is a base class, it contains lists of allStands,
   allGroups, allComponents, subsetGroups, subsetStands, and the topKeywords.

      To build a simulation file, RunStream::WriteRun is called, it
   follows this logic:

      write the top data (a few keywords and markers)

      iterate over allStands

         write siteKeywords, etc.

         iterate over componentsOrGroups

            if element is a component then write component.
            if element is a group then
               iterate over all components in the group
                  write component.
               end interative loop
            end if
         end iterative loop
      end iterative loop

      Note: all stands will be now defined and all group membership will
   also be defined, unless a group exists that contains no components.  To
   insure that those groups are defined and their stand membership is
   evident in the run file, we need to iterate over all groups, as follows:

      iterate over allGroups
         if group contains no components, then
            write the group identification
            iterate over standMembership
               write stand identification
            end interative loop
         end if
      end iterative loop

      Note: Now define the subset membership.  All groups and stands
   should be defined by this point.

      iterate over subsetGroups
         write the group identification
      end iterative loop

      iterate over subsetStands
         write the stand itendification
      end iterative loop

      write bottom information.

   The simulation file is read using RunStream::ReadRun.  The data in the
   structure is reloaded.

   All of the lists are RWOrdered, it might be an imporvement to make
   allGroups a binary tree.  Otherwise, having these as RWOrdered is just
   fine.  In the case of Stand::componentsOrGroups and Group::components,
   these two lists MUST remain RWOrdered.

   None of the lists OWN their elements except allGroups, allStands, and
   allComponents.  All of these lists are implemented as shallow lists of
   pointers to their respective members.  All can be cast to
   CStringCollectionRWC so any of the lists may be placed in an NListBox as
   a list of groups, stands, or components.

   Functions used to access this data structure are listed below.
   Data elements of RunStream are also listed below.  Data elements
   in other classes are listed in their respective class header files.

   Deleting RunStream results in all stands, groups, and components also
   being destroyed.  It is a safe way to reset Suppose.

   **************************************************************

   includedPostProcs
       a list of post processors included in the run.

   allGroups
       a list of all groups.  This class ownes the groups,
       deleting the class will delete the groups.

   allStands
       a list of all stands.  This class ownes the stands,
       deleting the class will delete the groups.

   allComponents
       a list of all components.  This class ownes the components,
       deleting the class will delete the components.

   subsetGroups
       a list of groups in the "current subset"...
       This class does NOT own the members of this shallow list.

   subsetStands
       a list of stands in the "current subset"...
       This class does NOT own the members of this shallow list.

   topPPEComponents
       a list of components that are written at the top of a PPE run,
       prior to the first ADDSTAND.

   botPPEComponents
       a list of components that are written to the end of a PPE run,
       after the last PROCESS and before the PROJECT.

   reportPPEComponents
       a list of components that are written to the end of a PPE run,
       after the PROJECT, prior to Stop

   topKeywords
       a string of "Comment" lines that define the temporal scope of
       the simulation.  They are at the top of the run, with or without
       the PPE.

   standNumbers
   groupNumbers
   componentNumbers
       serial numbers are generated from these values.
       values are initialized to zero when class is constructed.

   allSpGroups
       a list of all SpGroup components whose ojects are owned
       by allComponents

   **************************************************************
   *  Related Classes: Group, Stand, Component, PPEComponent    *
   *  See group.hpp, stand.hpp, and compon.hpp, respectively    *
   *  for descriptions of member data.  See below for           *
   *  delscriptions of member functions.                        *
   **************************************************************

   Related functions:

   RunStream::RunStream
       standard constructor.  see source to see which elements
       are built and which are left as null pointers.

   RunStream::~RunStream
       standard destructor.  This class owns all the groups,
       stands, and components...this destructor destroys all
       those objects.

   RunStream::WriteRun
       write all data needed in the run to the (opened) file
       pointed to by the argument.

   RunStream::ReadRun
       reads all data needed in the run from the (opened) file
       pointed to by the argument.

   RunStream::FindComponent (const char * name, const int number) const
   RunStream::FindGroup (const char * name, const int number) const
   RunStream::FindStand (const char * name, const int number) const
       given a name and a number, these routines return the a
       pointer to an existing component, group, or stand, if one
       exists in the corresponding list of allComponents,
       allGroups, or allStands.  If the item does not exist,
       a null pointer is returned.

   RunStream::AddComponent
       adds a component to the runstream and to each group
       and stand in the current subset, either before or after
       another specified component.

   RunStream::AddGroup
       adds a group to the runstream, no other action.

   RunStream::AddGroupToSubset
       adds a group to the subset unless adding it would
       result in a duplicate stand or group in the subset.

   RunStream::AddStand
       adds a stand to the run, takes no other action.

   RunStream::AddStandToSubset
       adds a stand to the subset unless adding it would
       result in a duplicate stand in the subest.

   RunStream::DelComponent
       deletes a component from all stands and all groups where
       it may be referenced, then deletes it form the runstream...
       if it is the component on the clip board it is delete but
       not destroyed.

   RunStream::BuildSubSetComponentList
       loads an RWOrdered list of components that make up those
       components attached to the current subset.  See runstr.cpp
       for more information as to what is included in the list.

   RunStream::PutComponentOnClipBoard
       sets the clipboard component.
   RunStream::GetComponentFromClipBoard
       gets the clipboard component.
   RunStream::ClearComponentClipBoard
       clears the clipboard component.
       (at the present, this is not the "Windows" clipboard.)

   RunStream::DelGroup
       deletes reference to the group in all stands in the groups
       membership list, then deletes the group from the runstream.

   RunStream::DelGroupFromSubset
       deletes a group from the subset.

   RunStream::GetCurrent
       returns the current group or stand.

   RunStream::DelStand
       deletes reference to the stand in all groups the stand may
       be a member of, then deletes the stand from the runstream.

   RunStream::DelStandFromSubset
       deletes a stand from the subset.

   RunStream::CreateSpGroups
       initializes allSpGroups by iterating allComponents and adding
       components of type SpGroup

   RunStream::AddComponentToSpGroups
       adds component to allSpGroups if component type is SpGroup

   RunStream::RemoveComponentFromSpGroups
       removed component from allSpGroups if component type is SpGroup

   RunStream::GetSpGroups
       returns an order list of common SpGroup names -- order list 
       but be freed by calling method

   Group::Group ( const char * name, int GNumber )
       standard constructor

   Group::Group ( void )
       constructor needed by RW when making copies using RW copy
       constructors (I assume).  I make all the copies, so this
       should never be called.

   Group::Group ( Group * g )
       group COPY constructor, see notes in group.cpp.  This is used
       with CopyComponentsFromGroup to build useful copies of groups.

   Group::~Group ( Group * g )
       does not own the members of it's list, so it just deletes the
       list.

   Group::AddComponentToGroup
       adds a component to a group.  If this is the first
       component in the group, then it adds the group to the
       componentsOrGroups list for all stands in the group's
       list of standMembership.  Components can be added before
       or after a specified component.

   Group::AddStandToGroup
       adds a stand to a groups list of stand membership.  if
       the group has components, thn it adds the group to the
       componentsOrGroups list for the stand.

   Group::CopyComponentsFromGroup
       copies the components from a group to a group.  If the
       group has standMembership, then the group is added to
       the componentsOrGroups list of the stand.  Also adds
       the group to the runstream.

   Group::DelComponentFromGroup
       removes a component from a group's list of components
       and if that action leaves the componentList empty, then
       the componentList is deleted.  If the componentList is
       empty, and the group has standMembership, then the
       group is removed from the stand's componentsOrGroups list.

   Group::DelStandFromGroup
       removes the stand from the group's standMembership and
       removes the group from the stand's componentsOrGroups.

   Stand::Stand ( const char * name, int SNumber )
       standard constructor

   Stand::Stand ( void )
       constructor needed by RW when making copies using RW copy
       constructors (I assume).  I make all the copies, so this
       should never be called.

   Stand::Stand ( Stand * g )
       stand COPY constructor, see notes in stand.cpp.  This is used
       with CopyComponentsOrGroupsFromStand to build useful copies.

   Stand::~Stand ( Stand * g )
       does not own the members of it's list, so it just deletes the
       list.

   Stand::WriteStand
       writes out the data that defines a stand, and the components
       that are attached directly (using WriteComponent) or via groups
       (using WriteGroupComponents)

   Stand::ReadStand
       reads the stand and all the associated components and groups.

   Stand::AddComponentOrGroupToStand
       Adds a component or a group to the componentsOrGroups.
       see AddComponentToGroup for more info.

   Stand::CopyComponentsOrGroupsFromStand
       copies the componentsOrGroups list from a stand to a
       stand.  for every item in the list, if the item is a
       group, then the stand is added to the group via
       AddStandToGroup.  Routine fails if the copy operation
       would result in a stand creating a duplicate in the
       subset.

   Stand::DelComponentOrGroupFromStand
       Removes an item from componentsOrGroup...if the item
       is a group, then calls DelStandFromGroup.


   **************************************************************

   Related pure functions:

   int ReadRunSegment (CStringRW ** element, FILE * filePtr, char * buff,
                       CStringRW * Tbuff)
       used buy the read routines, not otherwise useful.

   void WriteWindowParms (FILE * filePtr, CStringRW * windowParms)
       used to write the window parameters for any component.

   int ReadWindowParms (FILE * filePtr, char * buff,
                        CStringRW ** windowParmsPtr)
       used to read the window parameters for any component.

      Component * FindInsertPlace

   FindInsertPlace (RWOrdered * theList, Component * c, int * after)
       uses a set of rules to find a place to insert a component.
       It returns a Component*, not a subscript, and a int flag
       that indicates if the insert point is after or before the
       referenced component.

   InvalidPlacement (RWOrdered * theList, Component * c)
       uses a set of rules to determine if a component is placed in
       a list at a know, "invalid" place.  Calling this routine does
       not change the placement.
*
*********************************************************************/


#ifndef RunStream_h
#define RunStream_h

#define BUFFSIZE  1000 // size of the input char buffer.

#include "PwrDef.h"
#include CStringRWC_i

class RWOrdered;
class Component;
class Group;
class Stand;
class RWCollectable;
class MSText;

class RunStream
{

   public:

      RunStream ( void );
     ~RunStream ( void );

      int WriteRun (FILE * filePtr);
      int ReadRun  (FILE * filePtr);

      int AddComponent(Component * c,
                       Component * place = (Component *) 0,
                       int         after = 0);

      void AddToCollection(RWOrdered * theList, Component * c,
                           Component * placeInList = NULL, int after = 0);

      int AddGroup    (Group     * g);
      int AddStand    (Stand     * s);

      int DelComponent(Component * c);
      int DelGroup    (Group     * g);
      int DelStand    (Stand     * s);

      void BuildSubSetComponentList (RWOrdered  * theList,
                                     int          theType      = -1,
                                     const char * theExtension = NULL,
                                     const char * theVariant   = NULL,
                                     const Component * curComponent = NULL);

      Component * PPEChecking(MSText * keywordMSText,
                              Component * theResultingComponent);
      void PutComponentOnClipBoard (Component * c);
      Component * GetComponentFromClipBoard (void);
      void ClearComponentClipBoard (void);

      int AddGroupToSubset (Group * g);
      int AddStandToSubset (Stand * g);

      int DelGroupFromSubset (Group * g);
      int DelStandFromSubset (Stand * g);
      RWCollectable * GetCurrent(void);

      Component * FindComponent (const char * name, const int number) const;
      Group     * FindGroup     (const char * name, const int number) const;
      Stand     * FindStand     (const char * name, const int number) const;
      void WritePPEComponents(RWOrdered * theList, FILE * filePtr,
                              const char * tag);
      int  ReadPPEComponents(RWOrdered ** theList, FILE * filePtr,
                             CStringRW * Tbuff, char * buff,
                             const char * tag);
      void ProcessPPEComponents(RWOrdered * theList);

      void CreateSpGroups (void);
      void AddComponentToSpGroups (Component * c);
      void RemoveComponentFromSpGroups (Component * c);
      RWOrdered * GetSpGroups (void);

      RWOrdered   * includedPostProcs;
      RWOrdered   * allGroups;
      RWOrdered   * allStands;
      RWOrdered   * allComponents;
      RWOrdered   * subsetGroups;
      RWOrdered   * subsetStands;
      RWOrdered   * writtenComponents;
      RWOrdered   * topPPEComponents;
      RWOrdered   * botPPEComponents;
      RWOrdered   * reportPPEComponents;
      int           standNumbers;
      int           groupNumbers;
      int           componentNumbers;
      CStringRW   * topSimNotes;
      CStringRW   * topKeywords;
      Component   * componentClipBoard;
      RWOrdered   * allSpGroups;


};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Global funcitons associated with RunStream, Group, Stand, & Component
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


int ReadRunSegment (CStringRW ** element, FILE * filePtr,
                    char * buff, CStringRW * Tbuff);

void WriteWindowParms (FILE * filePtr, CStringRW * windowParms);

int ReadWindowParms (FILE * filePtr, char * buff,
                     CStringRW ** windowParms);

Component * FindInsertPlace (RWOrdered * theList,
                             Component * c, int * after);

int InvalidPlacement (RWOrdered * theList, Component * c);



#endif //RunStream_h
