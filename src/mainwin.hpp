/********************************************************************

   File Name:           mainwin.hpp
   Author:              kfd
   Date:                11/29/94

   Class:               CsupposeWin
   Inheritance:         CWindow->CsupposeWin
   Helper(s):

   Purpose:       This object is the selections window which is the driver
                  for the Suppose system.  It creates the control panel and
                  set command values.
                  SpUpdateFileName changes the text in the filename control.

   Contents:      CsupposeWin
                  CsupposeWin::IsupposeWin
                  CsupposeWin::DoCommand
                  CsupposeWin::DoMenuCommand
                  CsupposeWin::SpUpdateFileName
                  CsupposeWin::Key
                  ...
                                                                    *
 ********************************************************************/

#ifndef MainWin_H
#define MainWin_H

#include "PwrDef.h"
#include "suppdefs.hpp"

#include CTreeItem_i
#include CTreeView_i
#include CTypeInfo_i
#include CWindow_i

class CImage;
class CNavigator;
class CText;
class CTreeView;
class NButton;
class NGroupBox;
class NListBox;
class NText;
class RWOrdered;
class Component;
class Group;
class Stand;


#define StandListSelectionCmd 1
#define EditSelectionCmd 2
#define CutCmd 3
#define CopyCmd 4
#define PasteCmd 5
#define DeleteCmd 6
#define InsertFromFileCmd 7
#define WriteToFileCmd 8
#define AppendCmd 9


class MyCTreeSorter : public CTreeStringSorter
{
   public:

      MyCTreeSorter( void );
      virtual CTreeSorter::Comparison Compare (const CTreeItem* theFirstItem,
                                               const CTreeItem* theSecondItem) const;
};

class CsupposeWin : public CWindow
{
   public:

      CsupposeWin(CDocument *theDocument, const CRect& theRegion,
                const CStringRW& theTitle  = NULLString,
                long theWindowAttributes = WSF_NONE,
                WIN_TYPE theWindowType   = W_DOC,
                int theMenuBarId         = TASK_MENUBAR);

      virtual void DoCommand(long theCommand, void* theData=NULL);
      virtual void DoMenuCommand(MENU_TAG theMenuItem, BOOLEAN isShiftKey, BOOLEAN isControlKey);
      virtual void Key(const CKey& theKey);

      virtual BOOLEAN Close(void);
      void SpUpdateFileName(void);
      void SpUpdateStandCount(void);
      void LaunchComponentWindow(void);

  protected:

    class MyCTreeSource : public CTreeSource
    {
       public:
         MyCTreeSource(CsupposeWin* theEnclosure);
         virtual ~MyCTreeSource();

         virtual RWGVector(CTreeItemInfo)* GetTreeData(CTreeItem *theParent) const;

       private:
         CsupposeWin* itsEnclosure;
    };

   private:

      int AddPPEList(CStringRWC strPpeName, RWOrdered & ppeList);
      void ResetWindow(RWCollectable * selectedElement = NULL, int clearSelection = 0);
      void BuildTreeView(void);
      void BuildStandList(void);
      void ReadComponent(FILE * filePtr, FILE_SPEC * fileSpec);
      void DefinePlacementArgs(const char * what, Component ** placeArg, int * after);
      void ClearChildren(CTreeNodeItem * aNode);
      void SortAllChildren(CTreeItem * aNode, CTreeSorter * itsSorter);
      void LoadSelectedList(CTreeItem* aNode, RWCollectable * selectedElement, RWOrdered & selList);
      CTreeItem * FindFirst(CTreeItem* aNode, RWCollectable * selectedElement);
      CTreeNodeItem * AddStand(CTreeNodeItem * root, Stand * aStand);
      CTreeNodeItem * AddGroup(CTreeNodeItem * root, Group * aGroup);
      CTreeNodeItem * AddComponent(CTreeNodeItem * root, Component * aComponent);
      void ExpandMostChildren(CTreeItem* aNode);
      Stand * StandToExpand(void);

      CNavigator   * itsNavigator;

      NGroupBox    * itsSimPrepGrpBox;
      NButton      * itsSelectSimStand;
      NButton      * itsSetTime;
      NButton      * itsSelectModelOut;
      NButton      * itsManagement;
      NButton      * itsSpecifyMod;
      NButton      * itsUseFVSKeywords;
      NButton      * itsInsertFromFile;
      NButton      * itsSelectPostProc;
      NButton      * itsRunSimulation;
      NText        * itsStandText;
      NText        * itsStandCountText;
      NText        * itsGroupText;
      NText        * itsGroupCountText;

      CImage       * itsExpandedImage;
      CImage       * itsCollapsedImage;
      CImage       * itsTermnodeImage;
      CTreeView    * itsComponentTreeView;
      NText        * itsAffectedStandText;
      NListBox     * itsStandListBox;

      NGroupBox    * itsEditSimContGrpBox;
      NButton      * itsEditButton;
      NButton      * itsDeleteButton;
      NButton      * itsWriteButton;
      NButton      * itsAppendButton;
      NButton      * itsCopyButton;
      NButton      * itsCutButton;
      NButton      * itsChangeGroupMembership;
      NButton      * itsPasteButton;

      NGroupBox    * itsAfterSimGrpBox;
      NButton      * readFVSOutputButton;
      NButton      * itsGenerateGraph;  
      NButton      * itsGenerateReport;
      NButton      * itsExitButton;

      int            itsTreeSequence;

      MyCTreeSource * itsSource;
      MyCTreeSorter  itsSorter;

      Component    * itsCurrentComponent;
      Group        * itsCurrentGroup;
      Stand        * itsCurrentStand;

      RWOrdered      itsStandList;
      RWOrdered      itsComponentsFromFile;

      PWRClassInfo
};

class MyUserData : public RWCollectable
{
   RWDECLARE_COLLECTABLE(MyUserData)

   public:

      MyUserData(RWCollectable * a, int n);
      MyUserData(MyUserData * c);
      MyUserData(void);
      virtual ~MyUserData(void);

      RWCollectable * itsItem;
      int             itsSequence;
      RWClassID       itsKind;
};

#endif// MainWin_H
