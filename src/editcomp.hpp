/********************************************************************

   File Name:    editcomp.hpp
   Author:       nlc
   Date:         01/27/95 and 02/96
                 09/03/99 added CTreeView processing

   Purpose:  This class is used to list existing components and
             them to be edited.  Editing includes cut, copy, paste
             and delete using the edit menu, and it include editing
             the component by launching the screen used to create
             the keyword originally.

   Contents:

     class EditComponent
     class MyUserData
     class MyCTreeSorter

 ********************************************************************/

#ifndef EditComponent_H
#define EditComponent_H

#include "XVTPwr.h"

#include CTreeItem_i
#include CTreeView_i

#include "mainwin.hpp"

class NButton;
class NListBox;
class NText;
class RWOrdered;
class CRadioGroup;
class NRadioButton;
class NGroupBox;
class CTreeView;
class CImage;
class Component;
class Group;
class Stand;


//class MyCTreeSorter : public CTreeStringSorter
//{
//   public:
//
//      MyCTreeSorter( void );
//      virtual CTreeSorter::Comparison Compare (const CTreeItem* theFirstItem,
//                                               const CTreeItem* theSecondItem) const;
//};


#include "cwwindow.hpp"

class EditComponent : public CWwindow
{
  public:

    EditComponent(CDocument *theDocument);

    virtual void DoCommand(long theCommand,void* theData = NULL);
    virtual void DoMenuCommand(MENU_TAG theMenuItem,
                               BOOLEAN isShiftkey,
                               BOOLEAN isControlKey);
    virtual void Key(const CKey& theKey);

  protected:

    class MyCTreeSource : public CTreeSource
    {
       public:
         MyCTreeSource(EditComponent* theEnclosure);
         virtual ~MyCTreeSource();

         virtual RWGVector(CTreeItemInfo)* GetTreeData(CTreeItem *theParent) const;

       private:
         EditComponent* itsEnclosure;
    };

  private:

    int AddPPEList( CStringRWC strPpeName, RWOrdered & ppeList );
    void ResetWindow(RWCollectable * selectedElement = NULL, int clearSelection = 0);
    void BuildTreeView( void );
    void BuildStandList( void );
    void LaunchComponentWindow( void );
    void ReadComponent(FILE * filePtr, FILE_SPEC * fileSpec);
    void DefinePlacementArgs(const char * what,
                             Component ** placeArg,
                             int        * after);
    void ClearChildren(CTreeNodeItem * aNode);
    void SortAllChildren(CTreeItem * aNode, CTreeSorter * itsSorter);
    void LoadSelectedList(CTreeItem* aNode,
                          RWCollectable * selectedElement,
                          RWOrdered & selList);
    CTreeItem * FindFirst(CTreeItem* aNode, RWCollectable * selectedElement);
    CTreeNodeItem * AddStand(CTreeNodeItem * root, Stand * aStand);
    CTreeNodeItem * AddGroup(CTreeNodeItem * root, Group * aGroup);
    CTreeNodeItem * AddComponent(CTreeNodeItem * root, Component * aComponent);
    void ExpandMostChildren(CTreeItem* aNode);
    Stand * StandToExpand( void );

    NGroupBox    * addToSimGroupBox;
    NGroupBox    * editSimGroupBox;
    NGroupBox    * runOptionsGroupBox;
    NGroupBox    * afterRunGroupBox;
    NButton      * selStandsButton;
    NButton      * useKeywordsButton;
    NButton      * selMgmtActionsButton;
    NButton      * selReportsButton;
    NButton      * readButton;
    //NButton      * closeButton;
    NButton      * editButton;
    NButton      * pasteButton;
    NButton      * copyButton;
    NButton      * cutButton;
    NButton      * deleteButton;
    NButton      * writeButton;
    NButton      * appendButton;
    NButton      * timeScaleButton;
    NButton      * changeGroupsButton;
    NButton      * postProcsButtion;
    NButton      * runSimButton;
    //NButton      * exitButton;
    NButton      * readFVSOutputButton;
    NButton      * genGraphsButton;
    NButton      * genReportsButton;
    NText        * standCountText;
    NText        * groupCountText;
    CRadioGroup  * selectionRadioGroup;
    CTreeView    * componentTreeView;
    CImage       * expandedImage;
    CImage       * collapsedImage;
    CImage       * termnodeImage;
    NListBox     * standListBox;
    Component    * itsCurrentComponent;
    Group        * itsCurrentGroup;
    Stand        * itsCurrentStand;

    MyCTreeSource * itsSource;
    MyCTreeSorter  itsSorter;

    RWOrdered      componentsFromFile;
    RWOrdered      standList;

    int            treeSequence;

};

//class MyUserData : public RWCollectable
//{
//   RWDECLARE_COLLECTABLE(MyUserData)
//
//   public:
//
//      MyUserData(RWCollectable * a, int n);
//      MyUserData(MyUserData * c);
//      MyUserData( void );
//      virtual ~MyUserData( void );
//
//      RWCollectable * itsItem;
//      int             itsSequence;
//      RWClassID       itsKind;
//};



#endif
