/********************************************************************

   File Name:     reportwn.hpp
   Author:        nlc
   Date:          10/25/96

   Class:         ReadFVSOutputWindow
   Inheritance:   CWindow->CWwindow

   Purpose:  This set of class provides for reading FVS-generated 
             output and loading the AllFVSCases data structure.
 *                                                                  *
 ********************************************************************/

#ifndef ReadFVSOutputWindow_H
#define ReadFVSOutputWindow_H

#include "PwrDef.h"
#include "cwwindow.hpp"

class NListBox;
class NButton;
class NListButton;
class CText;
class NEditControl;
class FVSParser;
class CaseDefineUsingKeywordTable;
class FVSReport;
class FVSVariable;
class ParseFirePotentialFlameReport;
class ParseFireAllFuelsReport;

class ReadFVSOutputWindow : public CWwindow
{
   public:

      ReadFVSOutputWindow( CDocument * theDocument );
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual BOOLEAN Close( void );
      void ReadFVSOutput( void );

   private:

      NListBox     * itsCaseListBox;
      NButton      * itsReadButton;
      NButton      * itsBrowseButton;
      NButton      * itsDeleteCasesButton;
      NButton      * itsAverageButton;
      NButton      * itsCloseButton;
      NButton      * itsStopButton;
      NListButton  * itsFileTypesListButton;
      NEditControl * itsCurrentFileEditControl;
      CText        * itsLinesReadCText;
      FILE_SPEC      itsFVSOutputFile; 
      int            itsStopFlag;
      CStringRWC     itsOpenedFileName;
      FILE         * itsOpenedFilePtr;
      FVSParser    * itsFVSOutputParser;

};


#endif

