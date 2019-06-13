/*********************************************************************

   File Name:    editslf.cpp
   Author:       nlc
   Date:         04/16/97

   see editslf.hpp for notes.  also note that editslf2.cpp is a direct
   extension of this source file.  It exists because editslf needed to
   be split to support Win16 link limitations.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include NText_i
#include NButton_i
#include NEditControl_i
#include CDesktop_i
#include CMenuBar_i
#include CDocument_i
#include NScrollBar_i
#include NLineText_i
#include CNavigator_i
#include CStringCollection_i
#include CRadioGroup_i
#include NListButton_i
#include CWindow_i
#include CStringRWC_i

#include "mycscrol.hpp"
#include "sendupdt.hpp"
#include "suppdefs.hpp"
#include "spprefer.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "spparms.hpp"
#include "dirsept.hpp"

#include "variants.hpp"

#include "spglobal.hpp"

#include "editslf.hpp"
#include "mylinetx.hpp"

#include "supphelp.hpp"  // for help #defines
extern XVT_HELP_INFO helpInfo;

EditSLFWin::EditSLFWin( CDocument * theDocument )
           :CWwindow(theDocument,
                     CenterTopWinPlacement(CRect(5,24,571,462)),
                     "Edit Stand List File",
                     WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                     W_DOC, TASK_MENUBAR),
            itsCurrentRecord(0),
            itsTemplate(NULL)
{
   xvt_scr_set_busy_cursor();

   // create and clear the navigator

   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   new NText(this, CRect(4,8,126,32), "Stand List File:", 0L);

   itsSLFNameNLineText = new MyNLineText ( this,
                                           CPoint(130,6),
                                           344, 1, 0, SZ_FNAME);

   itsBrowseButton = new NButton(this, CRect(482,4,562,36), "Browse");
   itsBrowseButton->SetCommand(2);

   new NText(this, CRect(4,44,63,68), "Record", 0L);
   itsRecordNumberNLineText = new MyNLineText ( this, CPoint(66,40),
                                                30, 9);
   itsRecordCountNText = new NText(this, CRect(107,44,170,68),
                                   NULLString, 0L);

   itsPreviousButton = new NButton(this, CRect(174,40,236,72),"<<Prev");
   itsPreviousButton->SetCommand(3);
   itsNextButton = new NButton(this, CRect(240,40,302,72), "Next>>");
   itsNextButton->SetCommand(4);
   itsNewButton = new NButton(this, CRect(306,40,390,72), "New");
   itsNewButton->SetCommand(5);
   itsDeleteButton = new NButton(this, CRect(394,40,478,72), "Delete");
   itsDeleteButton->SetCommand(6);
   itsDuplicateButton = new NButton(this, CRect(482,40,562,72),
                                    "Duplicate");
   itsDuplicateButton->SetCommand(7);

   new NText(this, CRect(4,84,110,108), "Stand ID:", 0L);
   itsCurrStandIDNLineText = new MyNLineText ( this, CPoint(104,80),
                                               193, 34, 0, 26);

   itsFindButton = new NButton(this, CRect(306,80,375,112), "Find:");
   itsFindButton->SetCommand(8);
   itsFindNLineText = new MyNLineText (this, CPoint(379,80), 183, 8, 0, 26);

   itsScrollingWindow = new MyCScroller(this, CRect(4,120,562,398), 558, 278);
   itsScrollingWindow->IScroller(FALSE, TRUE, TRUE, 24, 278);
   itsScrollingWindow->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                               RIGHTSTICKY  | LEFTSTICKY);

   itsOkButton = new NButton(this, CRect(4,402,88,434), "Ok");
   itsOkButton->SetCommand(10);
   itsOkButton->SetGlue(BOTTOMSTICKY);

   itsSaveButton = new NButton(this, CRect(100,402,184,434), "Save");
   itsSaveButton->SetCommand(11);
   itsSaveButton->SetGlue(BOTTOMSTICKY);

   itsSaveAsButton = new NButton(this, CRect(196,402,280,434), "Save As");
   itsSaveAsButton->SetCommand(12);
   itsSaveAsButton->SetGlue(BOTTOMSTICKY);

   itsTemplateButton = new NButton(this, CRect(291,402,375,434), "Template");
   itsTemplateButton->SetCommand(33);
   itsTemplateButton->SetGlue(BOTTOMSTICKY);

   itsImportButton = new NButton(this, CRect(387,402,471,434), "Import");
   itsImportButton->SetCommand(13);
   itsImportButton->SetGlue(BOTTOMSTICKY);

   itsCancelButton = new NButton(this, CRect(482,402,562,434), "Cancel");
   itsCancelButton->SetCommand(14);
   itsCancelButton->SetGlue(BOTTOMSTICKY);

   int top = 4;
   int end = 558-5-NScrollBar::NativeWidth();

   // Record type A

   new NText(itsScrollingWindow, CRect(4,top,110,top+20),
             "FVS tree file:", 0L);
   itsFVSTreeFileNLineText = new MyNLineText (itsScrollingWindow,
                                              CPoint(114,top),
                                              end-84-114, 34, 0, 500, 1);
   itsFVSTreeFileBrowseButton = new NButton(itsScrollingWindow,
                                            CRect(end-80,top,end,top+32),
                                            "Browse");
   itsFVSTreeFileBrowseButton->SetCommand(15);
   top += 36;

   itsPointDataRadioGroup = new CRadioGroup(itsScrollingWindow,
                                            CPoint(4,top), FALSE);
   itsWithPointData = itsPointDataRadioGroup->AddButton(CPoint(0,0),
      "FVS tree data contains plot-level site data.", 0);
   itsNoPointData = itsPointDataRadioGroup->AddButton(CPoint(0,22),
      "FVS tree data contains NO plot-level site data.", 0);

   top += 50;
   int left = 4;
   int len  = ((end-left)/2);
   int right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "FVS Variant code(s):", 0L);
   left = right+4; right=len;
   itsVariantsNLineText = new MyNLineText (itsScrollingWindow,
                                           CPoint(left,top),
                                           len-174, 34, 0, 40, 1);
   itsVariantsNLineText->SetCommand(21);
   CStringRWC variantsString = "variants";
   itsVariantsMSTextPointer =  (MSText *) theSpGlobals->
         theParmsData->find(&variantsString);
   if (itsVariantsMSTextPointer)
   {
      RWOrderedIterator nextVariant(*itsVariantsMSTextPointer->GetThePKeys());
      PKeyData * oneVariant;
      while (oneVariant = (PKeyData *) nextVariant())
      {
         CStringRWC * aVar = new CStringRWC(*oneVariant);
         *aVar += " = ";
         *aVar += oneVariant->pString;
         itsVariantsList.insert(aVar);
      }
   }

   left = right+8;
   itsVariantsNListButton = new NListButton(itsScrollingWindow,
                                            CRect(left,top,end,top+300),
                                            itsVariantsList, 0L);
   itsVariantsNListButton->SetCommand(22);

   // Record type B

   top += 36;
   left = 4;
   len  = ((end-left)/3);
   right= left+106;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Inventory year:", 0L);
   left = right+4; right=len;
   itsInvYearNLineText = new MyNLineText (itsScrollingWindow,
                                          CPoint(left,top), len-114, 34);
   left = right+4; right=left+110;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "State:", CTL_FLAG_RIGHT_JUST);
   left = right+4; right=len+len;
   itsStateNLineText = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len-114, 34);
   left = right+4; right=left+110;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "County:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsCountyNLineText = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len-114, 34);

   top += 36;
   left = 4;
   len  = ((end-left)/3);
   right= left+106;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Model type:", 0L);
   left = right+4; right=len;
   itsModelTypeNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-114, 34);
   left = right+4; right=left+110;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Physio. region:", CTL_FLAG_RIGHT_JUST);
   left = right+4; right=len+len;
   itsPhysRegionNLineText = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len-114, 34);
   left = right+4; right=left+110;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Forest type:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsForestTypeNLineText = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len-114, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Latitude:", 0L);
   left = right+4; right=len;
   itsLatNLineText = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Longitude:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsLongitudeNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-174, 34);


   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Location code:", 0L);
   left = right+4; right=len;
   itsLocNLineText = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len-174, 23);

   left = right+8;
   itsLocNListButton = new NListButton(itsScrollingWindow,
                                       CRect(left,top,end,top+300),
                                       itsLocList, 0L);
   itsLocNListButton->SetCommand(24);

   top += 36;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Hab type/Plant Assoc:", 0L);
   left = right+4; right=len;
   itsHabNLineText = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len-174, 25);
   left = right+8;
   itsHabNListButton = new NListButton(itsScrollingWindow,
                                       CRect(left,top,end,top+300),
                                       itsHabList, 0L);
   itsHabNListButton->SetCommand(26);

   top += 36;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "PV reference code:", 0L);
   left = right+4;
   itsPVRefCodeNLineText = new MyNLineText (itsScrollingWindow,
                                           CPoint(left,top), len-174, 34);

   top += 36;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Stand year of origin:", 0L);
   left = right+4; right=len;
   itsOriginYearNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Aspect (degrees):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsAspectNLineText = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Slope (percent):", 0L);
   left = right+4; right=len;
   itsSlopeNLineText = new MyNLineText (itsScrollingWindow,
                                        CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Elevation:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsElevNLineText = new MyNLineText (itsScrollingWindow,
                                       CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Basal area factor:", 0L);
   left = right+4; right=len;
   itsBAFNLineText = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Inverse of fixed plot size:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsFixedNLineText = new MyNLineText (itsScrollingWindow,
                                        CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Break point diameter:", 0L);
   left = right+4; right=len;
   itsBreakpointNLineText = new MyNLineText (itsScrollingWindow,
                                             CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Number of plots:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsNPlotsNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Number non-stockable:", 0L);
   left = right+4; right=len;
   itsNNonStockNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Stand sampling weight:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsSampWtNLineText = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= len;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Percent stockable:", 0L);
   left = right+8;
   itsPNonStNLineText = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len-154, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+150;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "DG translation code:", 0L);
   left = right+4; right=len-40;
   itsDGTransCodeNLineText = new MyNLineText (itsScrollingWindow,
                                              CPoint(left,top), len-194, 27);
   itsTransList.insert(new CStringRWC(
                       "0 = Measured directly, end of period"));
   itsTransList.insert(new CStringRWC(
                       "1 = Subtract \"growth\" from \"current\" value"));
   itsTransList.insert(new CStringRWC(
                       "2 = Measured directly, start of period"));
   itsTransList.insert(new CStringRWC(
                       "3 = Subtract \"current\" value from \"growth\""));
   left = right+8;
   itsDGTransCodeNListButton = new NListButton(itsScrollingWindow,
                                               CRect(left,top,end,top+100),
                                               itsTransList, 0L);
   itsDGTransCodeNListButton->SetCommand(28);

   top += 36;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "DG measurement period:", 0L);
   left = right+4;
   itsDGperiodNLineText = new MyNLineText (itsScrollingWindow,
                                           CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+150;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "HG translation code:", 0L);
   left = right+4; right=len-40;
   itsHGTransCodeNLineText = new MyNLineText (itsScrollingWindow,
                                              CPoint(left,top), len-194, 29);
   left = right+8;
   itsHGTransCodeNListButton = new NListButton(itsScrollingWindow,
                                               CRect(left,top,end,top+100),
                                               itsTransList, 0L);
   itsHGTransCodeNListButton->SetCommand(30);

   top += 36;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "HG measurement period:", 0L);
   left = right+4;
   itsHGperiodNLineText = new MyNLineText (itsScrollingWindow,
                                           CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= len;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Mortality measurement period:", 0L);
   left = right+8;
   itsMortperiodNLineText = new MyNLineText (itsScrollingWindow,
                                             CPoint(left,top), len-154, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Maximum basal area:", 0L);
   left = right+4; right=len;
   itsBAMaxNLineText = new MyNLineText (itsScrollingWindow,
                                        CPoint(left,top), len-174, 34);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Maximum SDI:", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsSDIMaxNLineText = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len-174, 34);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(4,top,110,top+20),
             "Site species:", 0L);
   left = right+4; right=len;
   itsSiteSpeciesNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-174, 31);
   left = right+8;
   itsSpeciesNListButton = new NListButton(itsScrollingWindow,
                                           CRect(left,top,end,top+300),
                                           itsSpeciesList, 0L);
   itsSpeciesNListButton->SetCommand(32);

   top += 32;
   left = 4;
   len  = ((end-left)/3);
   right= left+106;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Site index:", 0L);
   left = right+4;
   itsSiteIndexNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(left,top), len-114, 34);
   top += 32;

   // Record type C

   new NText(itsScrollingWindow, CRect(4,top,110,top+20), "Group codes:", 0L);
   itsGroupCodeNLineText = new MyNLineText (itsScrollingWindow,
                                            CPoint(114,top),
                                            end-114, 34, 0, 200, 1);
   top += 32;

   // Record type D

   new NText(itsScrollingWindow, CRect(4,top,110,top+20), "Add File(s):", 0L);

   itsAddFileNLineText = new MyNLineText (itsScrollingWindow,
                                          CPoint(114,top), end-84-114,
                                          34, 0, 500, 1);
   itsAddFileBrowseButton = new NButton(itsScrollingWindow,
                                        CRect(end-80,top,end,top+32),
                                        "Browse");
   itsAddFileBrowseButton->SetCommand(20);
   top += 36;

   // Record type E

   new NText(itsScrollingWindow, CRect(4,top,110,top+20),
       "StandCN:", 0L);

   itsStandCNNLineText = new MyNLineText (itsScrollingWindow,
                                          CPoint(114,top),
                                          end-114, 34, 0, 200, 1);
   top += 32;



   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), EDITSLFWindow, 0L);

   DisplayRecord();

   aNavigator->AppendTabStop(new CTabStop(itsSLFNameNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsBrowseButton));
   aNavigator->AppendTabStop(new CTabStop(itsRecordNumberNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsPreviousButton));
   aNavigator->AppendTabStop(new CTabStop(itsNextButton));
   aNavigator->AppendTabStop(new CTabStop(itsNewButton));
   aNavigator->AppendTabStop(new CTabStop(itsDeleteButton));
   itsScrollingWindow->AppendToNavigator(aNavigator);
   aNavigator->AppendTabStop(new CTabStop(itsDuplicateButton));
   aNavigator->AppendTabStop(new CTabStop(itsOkButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveAsButton));
   aNavigator->AppendTabStop(new CTabStop(itsTemplateButton));
   aNavigator->AppendTabStop(new CTabStop(itsImportButton));
   aNavigator->AppendTabStop(new CTabStop(itsCancelButton));
   itsScrollingWindow->ShrinkToFit();
   AdjustScrollingWinSize(itsScrollingWindow, this);

   Show();
   aNavigator->InitFocus();
}






EditSLFWin::~EditSLFWin( void )
{
   itsVariantsList.clearAndDestroy();
   itsLocList.clearAndDestroy();
   itsHabList.clearAndDestroy();
   itsSpeciesList.clearAndDestroy();
   itsTransList.clearAndDestroy();
   itsSLFRecords.clearAndDestroy();
   if (itsTemplate) delete itsTemplate;
}




void EditSLFWin::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                         // File name CR command
      {
         ReadFile();
         DisplayRecord();
         break;
      }
      case 2:                         // File name Browse button
      {
         FILE_SPEC fileSpec;
         strcpy(fileSpec.type,"slf");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec, "Stand List File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            itsSLFNameNLineText->SetText(path);
            ReadFile();
            DisplayRecord();
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 3:                         // Previous
      {
         StoreCurrent();
         if (itsSLFRecords.entries())
         {
            if (itsCurrentRecord == 0) itsCurrentRecord =
                                         itsSLFRecords.entries()-1;
            else itsCurrentRecord--;
         }
         else itsCurrentRecord = 0;
         DisplayRecord();
         break;
      }
      case 4:                         // Next
      {
         StoreCurrent();
         itsCurrentRecord++;
         if (itsCurrentRecord >= itsSLFRecords.entries())
            itsCurrentRecord = 0;
         DisplayRecord();
         break;
      }
      case 5:                         // New
      {
         StoreCurrent();
         SLFRecord * aRec;
         if (itsTemplate) aRec = new SLFRecord(itsTemplate);
         else             aRec = new SLFRecord();
         if (itsSLFRecords.entries() == itsCurrentRecord++)
              itsSLFRecords.insert(aRec);
         else itsSLFRecords.insertAt(itsCurrentRecord,aRec);
         DisplayRecord();

         // activate the itsCurrStandIDNLineText object only if
         // another object has caused the call to DoCommand.

         if (theData) itsCurrStandIDNLineText->Activate();
         break;
      }
      case 6:                         // Delete
      {
         if (itsSLFRecords.entries())
            itsSLFRecords.removeAndDestroy(itsSLFRecords.
                                        at(itsCurrentRecord));
         DisplayRecord();
         break;
      }
      case 7:                         // Duplicate
      {
         StoreCurrent();
         if (itsSLFRecords.entries())
         {
            itsSLFRecords.insertAt(itsCurrentRecord,
               new SLFRecord((SLFRecord *)
                             itsSLFRecords.at(itsCurrentRecord++)));
         }
         else xvt_dm_post_error("Nothing to duplicate.");
         DisplayRecord();
         break;
      }
      case 8:                         // Find Button and Find Text CR command
      {
         if (itsSLFRecords.entries() == 1) break;

         // if nothing to look for, then set the find string to the
         // current stand and break.

         CStringRW tmp = itsFindNLineText->GetText();
         tmp = tmp.strip(RWCString::both);
         if (tmp.length() == 0)
         {
            itsFindNLineText->SetText((CStringRWC &)
                                      *itsSLFRecords.at(itsCurrentRecord));
            break;
         }

         // use a circular search logic.

         tmp = itsFindNLineText->GetText();
         tmp = tmp.strip(RWCString::both);
         size_t i;
         int found = -1;
         for (i=itsCurrentRecord; i < itsSLFRecords.entries(); i++)
         {
            if (strcmp(tmp.data(),
                       ((CStringRWC *) itsSLFRecords.at(i))->data()) == 0)
            {
               found=i;
               break;
            }
         }
         if (found == -1)
         {
            for (i=0; i < itsCurrentRecord; i++)
            {
               if (strcmp(tmp.data(),
                          ((CStringRWC *) itsSLFRecords.at(i))->data()) == 0)
               {
                  found=i;
                  break;
               }
            }
         }
         if (found > -1)
         {
            itsCurrentRecord = found;
            DisplayRecord();
         }
         break;
      }
      case 9:                         // Record N of N text CR command
      {
         StoreCurrent();
         CStringRW tmp = itsRecordNumberNLineText->GetText();
         tmp = tmp.strip(RWCString::both);
         long i = ConvertToLong(tmp.data())-1;
         if (i < 0) itsCurrentRecord = 0;
         else itsCurrentRecord = i;
         DisplayRecord();
         break;
      }
      case 10:                        // Ok button pushed
      {
         StoreCurrent();
         if (WriteFile()) Close();
         break;
      }
      case 11:                        // Save button
      {
         WriteFile();
         break;
      }
      case 12:                        // Save As button
      {

         FILE_SPEC fileSpec;
         fileSpec.name[0]=0;
         strcpy(fileSpec.type,"slf");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_save(&fileSpec, "Stand List File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            itsSLFNameNLineText->SetText(path);
            WriteFile();
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 13:                        // Import button pushed (maybe!)
      {
         FILE_SPEC fileSpec;
         strcpy(fileSpec.type,"dir");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec,
                                   "Old Submittal System .dir file:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            CStringRW newName = path;
            size_t suffixLoc = newName.index(".dir",0,RWCString::ignoreCase);
            if (suffixLoc != RW_NPOS)
               newName = newName.replace(suffixLoc,4,".slf",4);
            itsSLFNameNLineText->SetText(newName);
            ImportOldDirFile(path);
            DisplayRecord();
         }
         xvt_fsys_restore_dir();
         break;
         break;
      }
      case 14:                        // Cancel button pushed
      {
         Close();
         break;
      }

      case 15:                        // FVS tree file Browse button
      {
         FILE_SPEC fileSpec;
         fileSpec.type[0] = NULL;
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec, "FVS tree file:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);

            CStringRW tmp = itsFVSTreeFileNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            if (tmp.length())
            {
               tmp += "+";
               tmp += path;
            }
            else tmp = path;
            itsFVSTreeFileNLineText->SetText(tmp);
         }
         xvt_fsys_restore_dir();
         itsFVSTreeFileNLineText->Activate();
         break;
      }

      case 20:                        // Add File Browse button
      {
         FILE_SPEC fileSpec;
         strcpy(fileSpec.type,"kcp");
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec, "Add File:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);

            CStringRW tmp = itsAddFileNLineText->GetText();
            tmp = tmp.strip(RWCString::both);
            if (tmp.length())
            {
               tmp += " ";
               tmp += path;
            }
            else tmp = path;
            itsAddFileNLineText->SetText(tmp);
         }
         xvt_fsys_restore_dir();
         itsAddFileNLineText->Activate();
         break;
      }
      case 21:                        // Variant text
      {
         CStringRWC tmpCS = itsVariantsNLineText->GetText();
         tmpCS = tmpCS.strip(RWCString::both);
         size_t theOne = RW_NPOS;
         if (itsVariantsMSTextPointer)
         {
            size_t blank = tmpCS.first(' ');
            if (blank)
            {
               if (blank != RW_NPOS) (RWCString &) tmpCS =
                                        tmpCS.remove(blank);
               theOne = itsVariantsMSTextPointer->
                  GetThePKeys()->index(&tmpCS);
            }
         }
         if (theOne != RW_NPOS) itsVariantsNListButton->SelectItem(theOne);
         else itsVariantsNListButton->DeselectAll();
         StoreCurrent();
         DisplayRecord();
         FocusOnNextObject(itsVariantsNLineText);
         break;
      }
      case 22:                        // Variant button
      {
         if (itsVariantsNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsVariantsNListButton->GetFirstSelectedItem();
            tmp = tmp.strip(RWCString::both);
            size_t blank = tmp.first(' ');
            if (blank != RW_NPOS) tmp = tmp.remove(blank);
            itsVariantsNLineText->SetText(tmp);
            StoreCurrent();
            DisplayRecord();
            itsVariantsNLineText->Activate();
         }
         break;
      }
      case 23:                        // Loc Text CR
      {
         CStringRWC tmpCS = itsLocNLineText->GetText();
         tmpCS = tmpCS.strip(RWCString::both);
         size_t theOne = RW_NPOS;
         if (itsLocMSTextPointer)
         {
            size_t blank = tmpCS.first(' ');
            if (blank)
            {
               if (blank != RW_NPOS) (RWCString &) tmpCS =
                                        tmpCS.remove(blank);
               theOne = itsLocMSTextPointer->
                  GetThePKeys()->index(&tmpCS);
            }
         }
         if (theOne != RW_NPOS &&
             itsLocList.entries()) itsLocNListButton->SelectItem(theOne);
         else itsLocNListButton->DeselectAll();
         FocusOnNextObject(itsLocNLineText);
         break;
      }
      case 24:                        // Loc list button
      {
         if (itsLocNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsLocNListButton->GetFirstSelectedItem();
            tmp = tmp.strip(RWCString::both);
            size_t blank = tmp.first(' ');
            if (blank != RW_NPOS) tmp = tmp.remove(blank);
            itsLocNLineText->SetText(tmp);
            itsLocNLineText->Activate();
         }
         break;
      }
      case 25:                        // Hab/Pa Text CR
      {
         CStringRWC tmpCS = itsHabNLineText->GetText();
         tmpCS = tmpCS.strip(RWCString::both);
         size_t theOne = RW_NPOS;
         if (itsHabMSTextPointer)
         {
            size_t blank = tmpCS.first(' ');
            if (blank)
            {
               if (blank != RW_NPOS) (RWCString &) tmpCS =
                                        tmpCS.remove(blank);
               theOne = itsHabMSTextPointer->
                  GetThePKeys()->index(&tmpCS);
            }
         }
         if (theOne != RW_NPOS &&
             itsHabList.entries()) itsHabNListButton->SelectItem(theOne);
         else itsHabNListButton->DeselectAll();
         FocusOnNextObject(itsHabNLineText);
         break;
      }
      case 26:                        // Hab/Pa list button
      {
         if (itsHabNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsHabNListButton->GetFirstSelectedItem();
            tmp = tmp.strip(RWCString::both);
            size_t blank = tmp.first(' ');
            if (blank != RW_NPOS) tmp = tmp.remove(blank);
            itsHabNLineText->SetText(tmp);
            itsHabNLineText->Activate();
            break;
         }
      }
      case 27:                        // DG Translation Code Text CR
      {
         int i = 0;
         sscanf (itsDGTransCodeNLineText->GetText().data(),
                 "%d",&i);
         if (i >= 0 && i < 4)
            itsDGTransCodeNListButton->SelectItem(i);
         else
            itsDGTransCodeNListButton->DeselectAll();
         FocusOnNextObject(itsDGTransCodeNLineText);
         break;
      }
      case 28:                        // DG Translation Code button
      {
         if (itsDGTransCodeNListButton->GetSelectPosition() > -1)
         {
            itsDGTransCodeNLineText->SetText(ConvertToString(
               itsDGTransCodeNListButton->GetSelectPosition()));
            itsDGTransCodeNLineText->Activate();
         }
         break;
      }
      case 29:                        // HG Translation Code Text CR
      {
         int i = 0;
         sscanf (itsHGTransCodeNLineText->GetText().data(),
                 "%d",&i);
         if (i >= 0 && i < 4)
            itsHGTransCodeNListButton->SelectItem(i);
         else
            itsHGTransCodeNListButton->DeselectAll();
         FocusOnNextObject(itsHGTransCodeNLineText);
         break;
      }
      case 30:                        // HG Translation Code button
      {
         if (itsHGTransCodeNListButton->GetSelectPosition() > -1)
         {
            itsHGTransCodeNLineText->SetText(ConvertToString(
               itsHGTransCodeNListButton->GetSelectPosition()));
            itsHGTransCodeNLineText->Activate();
         }
         break;
      }
      case 31:                        // Site species text CR
      {
         CStringRWC tmpCS = itsSiteSpeciesNLineText->GetText();
         tmpCS = tmpCS.strip(RWCString::both);
         size_t theOne = RW_NPOS;
         if (itsSpeciesMSTextPointer)
         {
            size_t blank = tmpCS.first(' ');
            if (blank)
            {
               if (blank != RW_NPOS) (RWCString &) tmpCS =
                                        tmpCS.remove(blank);
               theOne = itsSpeciesMSTextPointer->
                  GetThePKeys()->index(&tmpCS);
            }
         }
         if (theOne != RW_NPOS &&
             itsSpeciesList.entries()) itsSpeciesNListButton->
                                       SelectItem(theOne);
         else itsSpeciesNListButton->DeselectAll();
         FocusOnNextObject(itsSiteSpeciesNLineText);
         break;
      }
      case 32:                        // Site species list button
      {
         if (itsSpeciesNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsSpeciesNListButton->GetFirstSelectedItem();
            tmp = tmp.strip(RWCString::both);
            size_t blank = tmp.first(' ');
            if (blank != RW_NPOS) tmp = tmp.remove(blank);
            itsSiteSpeciesNLineText->SetText(tmp);
            itsSiteSpeciesNLineText->Activate();
         }
         break;
      }
      case 33:                        // Template button pushed
      {
         if (itsTemplate)
         {
            delete itsTemplate;
            itsTemplate = NULL;
            xvt_dm_post_message("Template record is unset.");
         }
         else
         {
            StoreCurrent();
            itsTemplate = new SLFRecord((SLFRecord *) itsSLFRecords.
                                        at(itsCurrentRecord));
            xvt_dm_post_message("Current record is set as template.");
         }
         break;
      }
      case 34:                        // General MyNlineText CR.
      {
         FocusOnNextObject(theData);
         break;
      }
      case EXDataUpdated:             // Another window updated the data.
      {
         if (theData)
         {
            // if theData is not null, it contains the file name of the
            // file that was updated.

            if (strstr((const char *) theData,
                       itsSLFNameNLineText->GetText().data()) ||
                strstr(itsSLFNameNLineText->GetText().data(),
                       (const char *) theData))
            {
               CWindow * curFront = G->GetDesktop()->GetFrontWindow();
               if (curFront != this) G->GetDesktop()->SetFrontWindow((CWindow*)
                                                                     this);
               if (xvt_dm_post_ask("Reload","Ignore",NULL,
                                   "File %s has changed.", theData)
                   == RESP_DEFAULT)
               {
                  ReadFile();
                  DisplayRecord();
               }
               if (curFront != this) G->GetDesktop()->SetFrontWindow(curFront);
            }
         }
         break;
      }

      default:
      {
         CWwindow::DoCommand(theCommand,theData);
         break;
      }
   }
}


void EditSLFWin::DisplayRecord( void )
{
   CStringRW newTitle ("of ");
   if (itsSLFRecords.entries())
   {
      if (itsCurrentRecord >= itsSLFRecords.entries())
         itsCurrentRecord = itsSLFRecords.entries()-1;

      SLFRecord * display = (SLFRecord *) itsSLFRecords.
         at(itsCurrentRecord);

      newTitle += ConvertToString((int) itsSLFRecords.entries());
      itsRecordCountNText->SetTitle(newTitle);

      itsRecordNumberNLineText->
         SetText(ConvertToString((int) itsCurrentRecord+1));

      itsCurrStandIDNLineText->SetText((CStringRWC &) *display);

      itsFVSTreeFileNLineText->SetText(Disp(display->FVSTreeData));
      if ((strcmp("WithPointData",
                  display->samplePointDataFlag.data()) == 0) ||
          (strcmp("1",
                  display->samplePointDataFlag.data()) == 0))
           itsPointDataRadioGroup->SetSelectedButton(itsWithPointData);
      else itsPointDataRadioGroup->SetSelectedButton(itsNoPointData);

      CStringRWC tmpCS = Disp(display->variants);
      itsVariantsNLineText->SetText(tmpCS);
      size_t theOne = RW_NPOS;
      if (itsVariantsMSTextPointer)
      {
         size_t blank = tmpCS.first(' ');
         if (blank)
         {
            if (blank != RW_NPOS) (RWCString &) tmpCS = tmpCS.remove(blank);
            theOne = itsVariantsMSTextPointer->GetThePKeys()->index(&tmpCS);
         }
      }
      if (theOne != RW_NPOS &&
          itsVariantsList.entries()) itsVariantsNListButton->
                                        SelectItem(theOne);
      else itsVariantsNListButton->DeselectAll();

      LoadCodes();

      itsInvYearNLineText      ->SetText(Disp(display->invYear));
      itsStateNLineText        ->SetText(Disp(display->state));
      itsCountyNLineText       ->SetText(Disp(display->county));
      itsModelTypeNLineText    ->SetText(Disp(display->modelType));
      itsPhysRegionNLineText   ->SetText(Disp(display->physRegion));
      itsForestTypeNLineText   ->SetText(Disp(display->forestType));
      itsPVRefCodeNLineText   ->SetText(Disp(display->pvRefCode));
      itsLatNLineText          ->SetText(Disp(display->lat));
      itsLongitudeNLineText    ->SetText(Disp(display->longitude));

      tmpCS = Disp(display->loc);
      itsLocNLineText->SetText(tmpCS);
      theOne = RW_NPOS;
      if (itsLocMSTextPointer)
      {
         size_t blank = tmpCS.first(' ');
         if (blank)
         {
            if (blank != RW_NPOS) (RWCString &) tmpCS = tmpCS.remove(blank);
            theOne = itsLocMSTextPointer->GetThePKeys()->index(&tmpCS);
         }
      }
      if (theOne != RW_NPOS &&
          itsLocList.entries()) itsLocNListButton->SelectItem(theOne);
      else itsLocNListButton->DeselectAll();

      tmpCS = Disp(display->hab);
      itsHabNLineText->SetText(tmpCS);
      theOne = RW_NPOS;
      if (itsHabMSTextPointer)
      {
         size_t blank = tmpCS.first(' ');
         if (blank)
         {
            if (blank != RW_NPOS) (RWCString &) tmpCS = tmpCS.remove(blank);
            theOne = itsHabMSTextPointer->GetThePKeys()->index(&tmpCS);
         }
      }
      if (theOne != RW_NPOS &&
          itsHabList.entries()) itsHabNListButton->SelectItem(theOne);
      else itsHabNListButton->DeselectAll();

      itsOriginYearNLineText   ->SetText(Disp(display->originYear));
      itsAspectNLineText       ->SetText(Disp(display->aspect));
      itsSlopeNLineText        ->SetText(Disp(display->slope));
      itsElevNLineText         ->SetText(Disp(display->elev));
      itsBAFNLineText          ->SetText(Disp(display->baf));
      itsFixedNLineText        ->SetText(Disp(display->fixed));
      itsBreakpointNLineText   ->SetText(Disp(display->breakpoint));
      itsNPlotsNLineText       ->SetText(Disp(display->nPlots));
      itsNNonStockNLineText    ->SetText(Disp(display->nNonStock));
      itsSampWtNLineText       ->SetText(Disp(display->sampWt));
      itsPNonStNLineText       ->SetText(Disp(display->pNonSt));

      tmpCS = Disp(display->DGTransCode);
      itsDGTransCodeNLineText  ->SetText(tmpCS);
      int i = 0;
      sscanf (tmpCS.data(),"%d",&i);
      if (i >= 0 && i < 4)
         itsDGTransCodeNListButton->SelectItem(i);
      else
         itsDGTransCodeNListButton->DeselectAll();

      itsDGperiodNLineText     ->SetText(Disp(display->DGperiod));

      tmpCS = Disp(display->HGTransCode);
      itsHGTransCodeNLineText  ->SetText(tmpCS);
      i = 0;
      sscanf (tmpCS.data(),"%d",&i);
      if (i >= 0 && i < 4)
         itsHGTransCodeNListButton->SelectItem(i);
      else
         itsHGTransCodeNListButton->DeselectAll();

      itsHGperiodNLineText     ->SetText(Disp(display->HGperiod));
      itsMortperiodNLineText   ->SetText(Disp(display->mortPeriod));
      itsBAMaxNLineText        ->SetText(Disp(display->BAMax));
      itsSDIMaxNLineText       ->SetText(Disp(display->SDIMax));

      tmpCS = Disp(display->siteSpecies);
      itsSiteSpeciesNLineText->SetText(tmpCS);
      theOne = RW_NPOS;
      if (itsSpeciesMSTextPointer)
      {
         size_t blank = tmpCS.first(' ');
         if (blank)
         {
            if (blank != RW_NPOS) (RWCString &) tmpCS = tmpCS.remove(blank);
            theOne = itsSpeciesMSTextPointer->GetThePKeys()->index(&tmpCS);
         }
      }
      if (theOne != RW_NPOS &&
          itsSpeciesList.entries()) itsSpeciesNListButton->SelectItem(theOne);
      else itsSpeciesNListButton->DeselectAll();

      itsSiteIndexNLineText->SetText(Disp(display->siteIndex));

      itsGroupCodeNLineText->SetText(Disp(display->groupCodes));
      itsAddFileNLineText  ->SetText(Disp(display->addFiles));
      itsStandCNNLineText  ->SetText(Disp(display->standCN));

   }
   else
   {
      itsCurrentRecord=0;
      itsCurrStandIDNLineText->Clear();
      DoCommand (5,NULL);
   }
}



void EditSLFWin::StoreCurrent( void )
{
   if (itsSLFRecords.entries())
   {
      SLFRecord * display = (SLFRecord *) itsSLFRecords.
         at(itsCurrentRecord);

      (CStringRWC &) *display = itsCurrStandIDNLineText->GetText();

      display->FVSTreeData = itsFVSTreeFileNLineText->GetText();

      if (itsPointDataRadioGroup->GetSelectedButton() == itsWithPointData)
           display->samplePointDataFlag = "WithPointData";
      else display->samplePointDataFlag = "NoPointData";

      display->variants      = itsVariantsNLineText     ->GetText();
      display->invYear       = itsInvYearNLineText      ->GetText();
      display->state         = itsStateNLineText        ->GetText();
      display->county        = itsCountyNLineText       ->GetText();
      display->lat           = itsLatNLineText          ->GetText();
      display->modelType     = itsModelTypeNLineText    ->GetText();
      display->physRegion    = itsPhysRegionNLineText   ->GetText();
      display->forestType    = itsForestTypeNLineText   ->GetText();
      display->longitude     = itsLongitudeNLineText    ->GetText();
      display->loc           = itsLocNLineText          ->GetText();
      display->hab           = itsHabNLineText          ->GetText();
      display->originYear    = itsOriginYearNLineText   ->GetText();
      display->aspect        = itsAspectNLineText       ->GetText();
      display->slope         = itsSlopeNLineText        ->GetText();
      display->elev          = itsElevNLineText         ->GetText();
      display->baf           = itsBAFNLineText          ->GetText();
      display->fixed         = itsFixedNLineText        ->GetText();
      display->breakpoint    = itsBreakpointNLineText   ->GetText();
      display->nPlots        = itsNPlotsNLineText       ->GetText();
      display->nNonStock     = itsNNonStockNLineText    ->GetText();
      display->sampWt        = itsSampWtNLineText       ->GetText();
      display->pNonSt        = itsPNonStNLineText       ->GetText();
      display->DGTransCode   = itsDGTransCodeNLineText  ->GetText();
      display->DGperiod      = itsDGperiodNLineText     ->GetText();
      display->HGTransCode   = itsHGTransCodeNLineText  ->GetText();
      display->HGperiod      = itsHGperiodNLineText     ->GetText();
      display->mortPeriod    = itsMortperiodNLineText   ->GetText();
      display->BAMax         = itsBAMaxNLineText        ->GetText();
      display->SDIMax        = itsSDIMaxNLineText       ->GetText();
      display->siteSpecies   = itsSiteSpeciesNLineText  ->GetText();
      display->siteIndex     = itsSiteIndexNLineText    ->GetText();
      display->groupCodes    = itsGroupCodeNLineText    ->GetText();
      display->addFiles      = itsAddFileNLineText      ->GetText();
      display->standCN       = itsStandCNNLineText      ->GetText();
      display->pvRefCode       = itsPVRefCodeNLineText      ->GetText(); 
   }
}




int EditSLFWin::WriteFile( void )
{

   StoreCurrent();

   if (itsSLFRecords.entries() &&
       itsSLFNameNLineText->GetText().length())
   {
      FILE * SLFFile = fopen (itsSLFNameNLineText->
                              GetText().data(),"w");
      if (SLFFile)
      {
         int nRec = 0;
         RWOrderedIterator anSLFRecIter(itsSLFRecords);
         SLFRecord * anSLFRec;
         while (anSLFRec = (SLFRecord *) anSLFRecIter())
         {
            if (anSLFRec->length())
            {
               anSLFRec->WriteRecord(SLFFile);
               nRec++;
            }
         }
         fclose (SLFFile);
         xvt_dm_post_message("%d stands output.", nRec);
         SendUpdateMessage(EXDataUpdated,
                           (void *) itsSLFNameNLineText->GetText().data(),
                           this);
         return 1;
      }
      else xvt_dm_post_warning("File %s could not be opened.",
                               itsSLFNameNLineText->
                               GetText().data());
   }
   return 0;
}


void EditSLFWin::ReadFile( void )
{
   itsSLFRecords.clearAndDestroy();
   itsCurrentRecord = 0;

   if (itsSLFNameNLineText->GetText().length())
   {
      FILE * SLFFile = fopen (itsSLFNameNLineText->
                              GetText().data(),"r");
      if (SLFFile)
      {
         char * record= new char[2001];
         char * ptr;
         if (SLFFile != NULL)
         {
            while (fgets(record, 2000, SLFFile))
            {
               if (strlen(record) <= 2) continue;
               ptr = strtok (record," \n");
               switch ((int) *ptr)
               {
                  case 'A':
                  {
                     ptr = strtok(NULL," \n");
                     if (ptr && *ptr != '@')
                     {
                        SLFRecord * aRec;
                        if (itsTemplate)
                        {
                           aRec = new SLFRecord(itsTemplate);
                           (RWCString &) *aRec = ptr;
                        }
                        else aRec = new SLFRecord(new SLFRecord(ptr));
                        itsSLFRecords.insert(aRec);
                        AssignValue(&aRec->FVSTreeData);
                        AssignValue(&aRec->samplePointDataFlag);
                        if (ptr = strtok(NULL,"@\n")) aRec->variants = ptr;
                     }
                     break;
                  }

                  case 'B':
                  {
                     ptr = strtok(NULL," \n");
                     if (ptr && *ptr != '@' &&
                         strcmp(ptr,((CStringRWC *)
                                     itsSLFRecords.last())->data()) == 0)
                     {
                        SLFRecord * aRec = (SLFRecord *) itsSLFRecords.last();
                        if (!aRec) break;
                        AssignValue (&aRec->invYear);
                        AssignValue (&aRec->lat);
                        AssignValue (&aRec->longitude);
                        AssignValue (&aRec->loc);
                        AssignValue (&aRec->hab);
                        AssignValue (&aRec->originYear);
                        AssignValue (&aRec->aspect);
                        AssignValue (&aRec->slope);
                        AssignValue (&aRec->elev);
                        AssignValue (&aRec->baf);
                        AssignValue (&aRec->fixed);
                        AssignValue (&aRec->breakpoint);
                        AssignValue (&aRec->nPlots);
                        AssignValue (&aRec->nNonStock);
                        AssignValue (&aRec->sampWt);
                        AssignValue (&aRec->pNonSt);
                        AssignValue (&aRec->DGTransCode);
                        AssignValue (&aRec->DGperiod);
                        AssignValue (&aRec->HGTransCode);
                        AssignValue (&aRec->HGperiod);
                        AssignValue (&aRec->mortPeriod);
                        AssignValue (&aRec->BAMax);
                        AssignValue (&aRec->SDIMax);
                        AssignValue (&aRec->siteSpecies);
                        AssignValue (&aRec->siteIndex);
                        AssignValue (&aRec->modelType);
                        AssignValue (&aRec->physRegion);
                        AssignValue (&aRec->forestType);
                        AssignValue (&aRec->state);
                        AssignValue (&aRec->county);
                        AssignValue (&aRec->pvRefCode);
                     }
                     break;
                  }
                  case 'C':
                  {
                     SLFRecord * aRec = (SLFRecord *) itsSLFRecords.last();
                     if (!aRec) break;
                     ptr = strtok(NULL," \n");
                     if (ptr && *ptr != '@' &&
                         strcmp(ptr,((CStringRWC *)
                                     itsSLFRecords.last())->data()) == 0)
                     {
                        if (ptr = strtok(NULL,"@\n"))
                        {
                           if (aRec->groupCodes.isNull()) aRec->groupCodes=ptr;
                           else
                           {
                              aRec->groupCodes+=" ";
                              aRec->groupCodes+=ptr;
                           }
                        }
                     }
                     break;
                  }
                  case 'D':
                  {
                     SLFRecord * aRec = (SLFRecord *) itsSLFRecords.last();
                     if (!aRec) break;
                     ptr = strtok(NULL," \n");
                     if (ptr && *ptr != '@' &&
                         strcmp(ptr,((CStringRWC *)
                                     itsSLFRecords.last())->data()) == 0)
                     {
                        if (ptr = strtok(NULL,"@\n"))
                        {
                           if (aRec->addFiles.isNull()) aRec->addFiles=ptr;
                           else
                           {
                              aRec->addFiles+=" ";
                              aRec->addFiles+=ptr;
                           }
                        }
                     }
                     break;
                  }
                  case 'E':
                  {
                     SLFRecord * aRec = (SLFRecord *) itsSLFRecords.last();
                     if (!aRec) break;
                     ptr = strtok(NULL," \n");
                     if (ptr && *ptr != '@' &&
                         strcmp(ptr,((CStringRWC *)
                                     itsSLFRecords.last())->data()) == 0)
                     {
                        if (ptr = strtok(NULL,"@\n"))
                        {
                           if (aRec->standCN.isNull()) aRec->standCN=ptr;
                           else
                           {
                              aRec->standCN+=" ";
                              aRec->standCN+=ptr;
                           }
                        }
                     }
                     break;
                  }
                  default: break;
               }
            }
         }
         delete []record;
         fclose(SLFFile);
      }
      else xvt_dm_post_warning ("Suppose can not open Stand List File %s",
                                itsSLFNameNLineText->GetText().data());
   }
}


CStringRW & EditSLFWin::Disp(CStringRW & c)
{
   if (c.length() == 0 || c.first('@') != RW_NPOS) return NULLString;
   else return c;
}




