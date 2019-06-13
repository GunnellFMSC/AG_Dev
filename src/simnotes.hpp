/********************************************************************

   File Name:    simnotes.hpp
   Author:       nlc
   Date:         02/27/97

   Purpose:  This class is used to enter and edit comments (called
             Simulation Notes, in Suppose) that are output in the 
             FVS keyword table.

   Contents:

     class SimNotesWindow
                                                                    *
 ********************************************************************/

#ifndef SimNotesWindow_H
#define SimNotesWindow_H

#include "cwwindow.hpp"
#include NScrollText_i

class NScrollText;
class NButton;
class CRadioGroup;
class NRadioButton;
class NListButton;
class Component;

class SimNotesWindow : public CWwindow
{
   public:

      SimNotesWindow(CDocument * theDocument);
      virtual void DoCommand(long theCommand,void* theData = NULL);
      virtual BOOLEAN Close( void );
      inline void ScrollColumnRuler(T_CPOS theOrgOffset)
         { itsColumnRuler->SetScrollPosition(0, theOrgOffset); }

   private:

      CRadioGroup  * itsNotesRadioGroup;
      long           itsNotesTop;
      long           itsNotesComponent;
      NRadioButton * itsNotesTopButton;
      NRadioButton * itsNotesComponentButton;
      NListButton  * itsComponentsListButton;

      CStringRW   * GetSimNotes (CStringRW * theNotes, int * isChanged);
      NButton     * itsOkButton;
      NButton     * itsSaveButton;
      NButton     * itsResetButton;
      NScrollText * itsColumnRuler;
      NScrollText * itsSimNotesEdit;
      Component   * itsComponent;

};

#endif
