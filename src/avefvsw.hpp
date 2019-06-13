/********************************************************************

   File Name:     avefvsw.hpp
   Author:        nlc
   Date:          07/27/98

   Class:         AverageFVSOutputWindow
   Inheritance:   CWindow->CWwindow

   Purpose:  This set of class provides for averaging FVS-generated 
             output and loading the averages into AllFVSCases.
 *                                                                  *
 ********************************************************************/

#ifndef AverageFVSOutputWindow_H
#define AverageFVSOutputWindow_H

#include "PwrDef.h"
#include "cwwindow.hpp"

class NListBox;
class NButton;

class AverageFVSOutputWindow : public CWwindow
{
   public:

      AverageFVSOutputWindow( CDocument * theDocument );
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual BOOLEAN Close( void );
      void LoadGroupList( void );

   private:

      NListBox     * itsCaseListBox;
      NListBox     * itsGroupListBox;
      RWOrdered      itsGroupList;
      NButton      * itsAverageCasesButton;
      NButton      * itsAverageGroupsButton;
      NButton      * itsReadFVSOutputButton;
      NButton      * itsResetButton;
      NButton      * itsCloseButton;

};


#endif

