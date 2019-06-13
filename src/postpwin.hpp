/********************************************************************

   File Name:     postpwin.hpp
   Author:        nlc
   Date:          11/03/95

   Class:         PostProcWin
   Inheritance:   CWindow -> PostProcWin

   Purpose:       Allow the user to select post processors

   Contents:      PostProcWin
                  ~PostProcWin

*******************************************************************/

#ifndef postpwin_h
#define postpwin_h

#include "PwrDef.h"
#include "appdef.hpp"
#include CWindow_i
#include <rw/ordcltn.h>

class NButton;
class NListBox;
class RWOrdered;
class NScrollText;
class PostProc;

//------------------------------------------------------------------

class PostProcWin : public CWindow
{
  public:

      PostProcWin (CDocument *theDocument);
      virtual ~PostProcWin(void);
      virtual void DoCommand(long theCommand,void* theData=NULL);

  private:

      void LoadAvailableList( void );
      void DisplayDescription(PostProc * selectedPostProc);

      NButton           * closeButton;
      NButton           * includeButton;
      NButton           * deleteButton;
      NListBox          * availableListBox;
      NListBox          * includedListBox;
      RWOrdered           availableList;
      NScrollText       * descriptionTextEdit;
};

#endif
