/****************************************************************

   File Name:     dlstdgrp.hpp
   Author:        nlc
   Date:          10/96

   Class:         DeleteStandWindow
   Inheritance:   DeleteStandWindow->CWindow


   Purpose:       Allow users to delete a stand.

*******************************************************************/

#ifndef DeleteStandGroupWindows_H
#define DeleteStandGroupWindows_H

#include "cwwindow.hpp"

class NButton;
class NListButton;
class NListBox;
class NText;
class NTextEdit;
class RWOrdered;
class Stand;
class Group;

class DeleteStandWindow: public CWwindow
{
  public:

      DeleteStandWindow( CDocument * theDocument, void * theData);
      virtual void DoCommand(long theCommand, void* theData = NULL);
      virtual void SizeWindow(int theWidth, int theHeight);

  private:

      Stand        * itsPreSetStand;
      NButton      * itsDeleteStandButton;
      NButton      * itsCloseButton;
      NListBox     * itsTargetStandListBox;
      NListBox     * itsAttachedComponentsListBox;
      RWOrdered    * itsAttachedComponentsList;
      NText        * itsComponentsText;
      NTextEdit    * itsUserNoteText;

};



class DeleteGroupWindow: public CWwindow
{
  public:

      DeleteGroupWindow( CDocument * theDocument, void * theData);
      virtual void DoCommand(long theCommand, void* theData = NULL);
      virtual void SizeWindow(int theWidth, int theHeight);

  private:

      Group        * itsPreSetGroup;
      NButton      * itsDeleteGroupButton;
      NButton      * itsCloseButton;
      NListBox     * itsTargetGroupListBox;
      NListBox     * itsAttachedComponentsListBox;
      RWOrdered    * itsAttachedComponentsList;
      NText        * itsComponentsText;
      NTextEdit    * itsUserNoteText;

};


#endif //DeleteStandGroupWindows_H
