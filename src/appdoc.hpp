/********************************************************************
 *

   File Name:           appdoc.hpp
   Author:              kfd & nlc
   Date:                11/29/94

   Class:               CsupposeDoc
   Inheritance:         CDocument->CsupposeDoc


   Purpose:       This is the object which is the Suppose document.
                  It build the documents window and 
                  is the controller for bringing up other windows.

   Contents:      CsupposeDoc
                  CsupposeDoc::BuildWindow
                  CsupposeDoc::DoCommand
                  CsupposeDoc::DoMenuCommand
                  CsupposeDoc::DoNew
                  CsupposeDoc::DoOpen
                  CDocument  ::CloseAll
                  CsupposeDoc::UpdateMenus
                  CsupposeDoc::SpGetFilePointer
                  CsupposeDoc::SpManagementWindow
                  CsupposeDoc::SpModelModWindow
                  CsupposeDoc::SetTimingKeywordsForAllStands
                  CsupposeDoc::LoadSimFileUsingCommandLine
                  CsupposeDoc::RunSimulation
                  CsupposeDoc::PostProcessorWindow

 ********************************************************************/

#ifndef AppDoc_H
#define AppDoc_H

#include "PwrDef.h"

#include CTypeInfo_i
#include CDocument_i

#include <rw/ordcltn.h>

class SelectSimStandWin;
class GroupMembershipWindow;
class CMngWin;
class UseFVSKey;
class CsupposeWin;
class EditComponent;
class SelectVariant;
class PostProcWin;
class RunWizardWin;
class ReadFVSOutputWindow;
class DeleteStandWindow;
class DeleteGroupWindow;
class SimNotesWindow;
class EditLocationWin;
class EditSLFWin;
class EditFVSTreeDataWin;
class AverageFVSOutputWindow;
class ModelOutWin;
class CModelModWin;

class CsupposeDoc : public CDocument
{
   public:

      CsupposeDoc(CApplication *theApplication,int theId );

      virtual void BuildWindow(void);
      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual void DoMenuCommand(MENU_TAG theMenuItem,
                      BOOLEAN isShiftKey, BOOLEAN isControlKey);
      virtual BOOLEAN DoNew(void);
      virtual BOOLEAN DoOpen(void);
      virtual BOOLEAN DoSave(void);
      virtual BOOLEAN DoSaveAs(void);
      virtual void    UpdateMenus(CMenuBar* theMenuBar);
      virtual BOOLEAN DoClose(void) ;

      FILE_SPEC* SpGetFilePointer(void);
      void SpSelectSimStandWin(int autoPrompt);
      void SpManagementWindow(void);
      void SpModelModWindow(void);
      void LoadSimFileUsingCommandLine(void);
      void RunSimulation( void );
      void PostProcessorWindow( void );

      RWOrdered itsDynamicHelpWebLinks;

   private:

      CsupposeWin            * itsSelectionsWindow;
      SelectSimStandWin      * itsSelectSimStandWin;
      GroupMembershipWindow  * itsGroupMembershipWin;
      CMngWin                * itsManagementWin;
      UseFVSKey              * itsUseFVSKeyWin;
      EditComponent          * itsEditSimFileWin;
      SelectVariant          * itsSelectVariantWin;
      PostProcWin            * itsPostProcessorWin;
      RunWizardWin           * itsRunWizardWin;
      ReadFVSOutputWindow    * itsReadFVSOutputWindow;
      DeleteStandWindow      * itsDeleteStandWindow;
      DeleteGroupWindow      * itsDeleteGroupWindow;
      SimNotesWindow         * itsSimNotesWindow;
      EditLocationWin        * itsEditLocsWindow;
      EditSLFWin             * itsEditSLFWindow;
      EditFVSTreeDataWin     * itsEditFVSTreeDataWindow;
      AverageFVSOutputWindow * itsAverageFVSOutputWindow;
      ModelOutWin            * itsModelOutWindow;
      CModelModWin           * itsModelModWin;
      int                      itsFVSOutputNeedsScaning;

      void SetTimingKeywordsForAllStands( void );
      BOOLEAN ResetSuppose( int partial );
      BOOLEAN SetUpTutorialData( void );
      FILE_SPEC * locFilePointer;

      PWRClassInfo
};

#endif // AppDoc_H

