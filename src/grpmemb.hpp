/********************************************************************

   File Name:     grpmemb.hpp
   Author:        nlc
   Date:          05/96

   Class:         GroupMembershipWindow
   Inheritance:   CWindow->CWwindow

   Class:         GroupMembershipSubview
   Inheritance:   GroupMembershipSubview->CSubview

   Class:         StandMgmtSubview
   Inheritance:   StandMgmtSubview->CSubview

   Class:         SetMgmtIDWindow
   Inheritance:   SetMgmtIDWindow->CWindow

   Class:         SetGroupMembershipWindow
   Inheritance:   SetGroupMembershipWindow->CWindow

   Purpose:       Allow users to create and change group names
                  and membership.  It is also used to set the
                  management ids for stands.


*******************************************************************/

#ifndef GroupMembershipWindow_H
#define GroupMembershipWindow_H

#include "cwwindow.hpp"

class NButton;
class NScrollBar;
class NListButton;
class SetMgmtIDWindow;
class SetGroupMembershipWindow;
class Group;
class CText;
class Stand;
class MyNLineText;
class NEditControl;
class CRadioGroup;


class GroupMembershipWindow: public CWwindow
{
  public:

      GroupMembershipWindow(CDocument *theDocument);

      virtual void SizeWindow(int theWidth, int theHeigth);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      void BuildElements(int theWidth, int theHeigth);
      void LoadStandsAndGroups( void );
      void ResetWindow( void );
      virtual void HScroll(SCROLL_CONTROL theEventType, UNITS thePosition);
      virtual void VScroll(SCROLL_CONTROL theEventType, UNITS thePosition);

      RWOrdered    itsListOfStandSubviews;
      RWOrdered    itsListOfGroupSubviews;
      size_t       numberOfRows;
      size_t       numberOfColumns;

  private:

      NButton    * itsCloseButton;
      NButton    * itsSetMembershipButton;
      NButton    * itsSetMgmtIDsButton;
      NScrollBar * itsVerticalScrollBar;
      NScrollBar * itsHorizontalScrollBar;

      SetGroupMembershipWindow * 
                   itsSetGroupMembershipWindow;

      SetMgmtIDWindow * 
                   itsSetMgmtIDWindow;

      int          heightOfNLineText;

};




class StandMgmtSubview: public CSubview
{
   public:

      StandMgmtSubview(CSubview * theEnclosure,
                       const CRect& theRegion);

      virtual void DoCommand(long theCommand,void* theData=NULL);

      void IStandMgmtSubview(Stand * theStand = NULL,
                             int theSequenceNumber = 0);

      CText       * itsSequenceNumber;
      MyNLineText * itsStandID;
      MyNLineText * itsMgmtID;
      Stand       * itsStand;
      int           itsStandHasChanged;

};      



class GroupMembershipSubview: public CSubview
{
   public:

      GroupMembershipSubview(CSubview * theEnclosure,
                             const CRect& theRegion);

      virtual void DoCommand(long theCommand,void* theData=NULL);

      void IGroupMembershipSubview(Group * theGroup = NULL);

      MyNLineText * itsGroupID;
      int           itsGroupHasChanged;
      Group       * itsGroup;
      RWOrdered     itsListOfCheckBoxes;
};



class SetMgmtIDWindow: public CWindow
{
   public:

      SetMgmtIDWindow(GroupMembershipWindow * groupMembership);
      virtual void DoCommand(long theCommand,void* theData=NULL);

   private:

      NButton       * itsApplyButton;
      NButton       * itsCloseButton;
      NEditControl   * itsNewMgmtID;
      NListButton   * itsGroupListButton;

      GroupMembershipWindow * 
                      itsGroupMembershipWindow;


};
   

   
class SetGroupMembershipWindow: public CWindow
{
   public:

      SetGroupMembershipWindow(GroupMembershipWindow * groupMembership);
      virtual void DoCommand(long theCommand,void* theData=NULL);

   private:

      NButton       * itsApplyButton;
      NButton       * itsCloseButton;
      NListButton   * itsTargetGroupListButton;
      NListButton   * itsSource1GroupListButton;
      NListButton   * itsSource2GroupListButton;
      CRadioGroup   * itsLogicalOperatorRadioGroup;
      long            itsAndRButton;
      long            itsOrRButton;
      long            itsNotRButton;      

      GroupMembershipWindow * 
                      itsGroupMembershipWindow;


};
      

#endif //GroupMembershipWindow_H
