/********************************************************************

File Name:     cprefswin.hpp
Author:        pmg
Date:          10/24/06

Contents:      CPrefsWin

Purpose:       To display and allow for edit of Suppose Preferences.

********************************************************************/

#ifndef __CPrefsWin_HPP
#define __CPrefsWin_HPP

#include "spprefer.hpp"
#include "spglobal.hpp"

#include "PwrDef.h"
#include CWindow_i
#include NButton_i
#include NEditControl_i

class CPrefsWin : public CWindow
{
public:
   CPrefsWin(CDocument *theDocument);

   virtual void DoCommand(long theCommand, void* theData = NULL);

private:
   NEditControl *itsDefaultVariant;
   NEditControl *itsDefaultLocationsFileName;
   NEditControl *itsDefaultParametersFileName;
   NEditControl *itsDefaultDataBaseName;
   NEditControl *itsDefaultEditor;
   NEditControl *itsDefaultUsePPE;
   NEditControl *itsDefaultSegmentCycle;
   NEditControl *itsDefaultWorkingDirectory;
   RWCString databaseAnswers[5];

   void  *itsDefaultProcessPlots;

   NButton *itsApply;
   NButton *itsClose;
   NButton *itsSetDir;
   BOOLEAN itsModified;
};

#endif //__CPrefsWin_HPP

