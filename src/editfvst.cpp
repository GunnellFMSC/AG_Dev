/*********************************************************************

   File Name:    editfvst.cpp
   Author:       nlc
   Date:         08/25/97

   see editfvst.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include NText_i
#include NScrollBar_i
#include CText_i
#include NButton_i
#include NEditControl_i
#include CDesktop_i
#include CMenuBar_i
#include CDocument_i
#include NLineText_i
#include CStringCollection_i
#include CRadioGroup_i
#include NListButton_i
#include CWindow_i
#include CStringRWC_i
#include CNavigator_i

#include <iostream>
#include <fstream>

#include "mycscrol.hpp"
#include "suppdefs.hpp"
#include "spprefer.hpp"
#include "spglobal.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "spparms.hpp"
#include "dirsept.hpp"

#include "variants.hpp"

#include "spglobal.hpp"

#include "editfvst.hpp"
#include "mylinetx.hpp"

#include "supphelp.hpp"  // for help #defines

extern XVT_HELP_INFO helpInfo;

EditFVSTreeDataWin::EditFVSTreeDataWin( CDocument * theDocument )
                   :CWwindow(theDocument,
                             CenterTopWinPlacement(CRect(5,24,571,462)),
                             "Edit FVS Tree Data File",
                             WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                             W_DOC, TASK_MENUBAR),
                             itsCurrentRecordNumber(0),
                             itsTemplate(NULL)
{
   xvt_scr_set_busy_cursor();

   // create and clear the navigator

   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   int left =4;
   int top  =4;
   int right=left+152;
   int bot  =top +24;
   const int winWidth=GetFrame().Right();
   new NText(this, CRect(left,top,right,bot), "FVS Tree Data File:", 0L);

   left  =right+4;
   top   =6;
   int width=318;
   itsFVSTDFNameNLineText = new MyNLineText ( this,
                                             CPoint(left,top),
                                             width, 1, 0, SZ_FNAME);
   left  =left+width+4;
   right =GetFrame().Right()-4;
   bot   =top +32;
   itsBrowseButton = new NButton(this, CRect(left,top,right,bot), "Browse");
   itsBrowseButton->SetCommand(2);

   left  =4;
   top   =bot+4;
   right =left+152;
   bot   =top+24;

   new NText(this, CRect(left,top,right,bot),"Codes for variant:", 0L);

   CStringRWC variantsString = "variants";
   const char * curVar = theSpGlobals->theVariants->GetSelectedVariant();
   int curVarNum = -1;
   itsVariantsMSTextPointer =  (MSText *) theSpGlobals->
         theParmsData->find(&variantsString);
   if (itsVariantsMSTextPointer)
   {
      int i=0;
      RWOrderedIterator nextVariant(*itsVariantsMSTextPointer->GetThePKeys());
      PKeyData * oneVariant;
      while (oneVariant = (PKeyData *) nextVariant())
      {
         CStringRWC * aVar = new CStringRWC(oneVariant->pString);
         if (strcmp(curVar,oneVariant->data()) == 0) curVarNum = i;
         itsVariantsList.insert(aVar);
         i++;
      }
   }

   left = right+4;
   right= winWidth-4;
   itsVariantsNListButton = new NListButton(this,
                                            CRect(left,top,right,top+300),
                                            itsVariantsList, 0L);
   if (curVarNum > -1) itsVariantsNListButton->SelectItem(curVarNum);

   itsVariantsNListButton->SetCommand(3);

   left =4;
   top  =bot+8;
   right=63;
   bot  =top+32;
   new NText(this, CRect(left,top,right,bot), "Record", 0L);
   left =right+4;
   itsRecordNumberNLineText = new MyNLineText ( this, CPoint(left,top+2),
                                                40, 9);
   left +=44;
   right =left+63;
   itsRecordCountNText = new NText(this, CRect(left,top,right,bot),
                                   NULLString, 0L);
   left  =right+4;
   right =left+62;

   itsPreviousButton = new NButton(this, CRect(left,top,right,bot),"<<Prev");
   itsPreviousButton->SetCommand(4);
   left  =right+4;
   right =left+62;
   itsNextButton = new NButton(this, CRect(left,top,right,bot), "Next>>");
   itsNextButton->SetCommand(5);
   left  =right+4;
   right =left+80;
   itsNewButton = new NButton(this, CRect(left,top,right,bot), "New");
   itsNewButton->SetCommand(6);
   left  =right+4;
   right =left+80;
   itsDeleteButton = new NButton(this, CRect(left,top,right,bot), "Delete");
   itsDeleteButton->SetCommand(7);
   left  =right+4;
   right =left+84;
   itsDuplicateButton = new NButton(this, CRect(left,top,right,bot),
                                    "Duplicate");
   itsDuplicateButton->SetCommand(8);

   left =8;
   top  =bot+4;
   right=winWidth-4;
   bot  =top+24;

   itsColumnRuler = new CText(this, CPoint(left,top),
                              "....+....1....+....2....+....3....+....4"
                              "....+....5....+....6....+");
   #ifdef AIXV4
      itsColumnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
   #else
      itsColumnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
   #endif

   right = xvt_dwin_get_text_width(itsColumnRuler->
                                   GetCWindow()->GetXVTWindow(),
                                   (char *) itsColumnRuler->GetTitle().data(),
                                   -1)+left;
   int leadingp, ascentp, descent;
   xvt_dwin_get_font_metrics(itsColumnRuler->GetCWindow()->GetXVTWindow(),
               &leadingp, &ascentp, &descent);

   bot = top+leadingp+ascentp+descent;
   itsColumnRuler->Size(CRect(left,top,right,bot));

   top = bot+2;
   bot = top+leadingp+ascentp+descent;

   itsCurrentRecord = new CText(this, CRect(left,top,right,bot),
                                NULLString);

   #ifdef AIXV4
      itsCurrentRecord->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
   #else
      itsCurrentRecord->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
   #endif

   left =4;
   top  =bot+6;
   right=winWidth-4;
   bot  =GetFrame().Bottom()-40;

   itsScrollingWindow = new MyCScroller(this, CRect(left,top,right,bot),
                                      558, 278);
   itsScrollingWindow->IScroller(FALSE, TRUE, TRUE, 24, 278);
   itsScrollingWindow->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                               RIGHTSTICKY  | LEFTSTICKY);
   left =4;
   top  =bot+4;
   int len=((winWidth-4)/6)-4;
   right=len;
   bot  =top+32;

   itsOkButton = new NButton(this, CRect(left,top,right,bot), "Ok");
   itsOkButton->SetCommand(10);
   itsOkButton->SetGlue(BOTTOMSTICKY);

   left =right+4;
   right=left+len;
   itsSaveButton = new NButton(this, CRect(left,top,right,bot), "Save");
   itsSaveButton->SetCommand(11);
   itsSaveButton->SetGlue(BOTTOMSTICKY);

   left =right+4;
   right=left+len;
   itsSaveAsButton = new NButton(this, CRect(left,top,right,bot), "Save As");
   itsSaveAsButton->SetCommand(12);
   itsSaveAsButton->SetGlue(BOTTOMSTICKY);

   left =right+4;
   right=left+len;
   itsTemplateButton = new NButton(this, CRect(left,top,right,bot), "Template");
   itsTemplateButton->SetCommand(13);
   itsTemplateButton->SetGlue(BOTTOMSTICKY);

   left =right+4;
   right=left+len;
   itsImportButton = new NButton(this, CRect(left,top,right,bot), "Import");
   itsImportButton->SetCommand(14);
   itsImportButton->SetGlue(BOTTOMSTICKY);

   left =right+4;
   right=left+len;
   itsCancelButton = new NButton(this, CRect(left,top,right,bot), "Cancel");
   itsCancelButton->SetCommand(15);
   itsCancelButton->SetGlue(BOTTOMSTICKY);

   int end = winWidth-8-5-NScrollBar::NativeWidth();
   left =4;
   len  = ((end-left)/3);
   top  =4;
   right= left+106;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Plot ID (1-4):", 0L);
   left = right+4; right=left+40;
   itsPlotID = new MyNLineText (itsScrollingWindow,
                                CPoint(left,top), 40, 20);
   left = right+4; right=left+110;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Tree ID (5-7):", CTL_FLAG_RIGHT_JUST);
   left = right+4; right=left+40;
   itsTreeID = new MyNLineText (itsScrollingWindow,
                                CPoint(left,top), 40, 20);
   left = right+4; right=left+150;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Tree count (8-13):", CTL_FLAG_RIGHT_JUST);
   left = right+4; right=end-4;
   itsTreeCount = new MyNLineText (itsScrollingWindow,
                                   CPoint(left,top), right-left, 20);
   left = 4;
   top += 32;
   right= left+150;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Tree history (14):", 0L);
   left = right+4; right=left+40;
   itsTreeHistory = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), right-left, 21);
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "0 = live tree"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "1 = live tree"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "2 = live tree"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "3 = live tree"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "4 = live tree"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "5 = live tree"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "6 = died during mortality observation period"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "7 = died during mortality observation period"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "8 = died prior to mortality observation period"));
   itsTreeHistoryCodesList.insert(new CStringRWC(
                "9 = died prior to mortality observation period"));
   left = right+4; right=end-4;
   itsTreeHistoryCodesNListButton =
      new NListButton(itsScrollingWindow,
                      CRect(left,top,end,top+200),
                      itsTreeHistoryCodesList, 0L);
   itsTreeHistoryCodesNListButton->SetCommand(22);

   top += 32;
   left = 4;
   right= left+150;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Species (15-17):", 0L);
   left = right+4; right=left+40;
   itsSpecies = new MyNLineText (itsScrollingWindow,
                                 CPoint(left,top), right-left, 23);
   left = right+4; right=end-4;
   itsSpeciesNListButton = new NListButton(itsScrollingWindow,
                                           CRect(left,top,end,top+300),
                                           itsSpeciesList, 0L);
   itsSpeciesNListButton->SetCommand(24);

   top += 36;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "DBH (18-21):", 0L);
   left = right+4; right=len;
   itsDBH = new MyNLineText (itsScrollingWindow,
                             CPoint(left,top), 40, 20);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "DBH Growth (22-24):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsDBHGrowth = new MyNLineText (itsScrollingWindow,
                                   CPoint(left,top), 40, 20);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Height (25-27):", 0L);
   left = right+4; right=len;
   itsHeight = new MyNLineText (itsScrollingWindow,
                                CPoint(left,top), 40, 20);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Height to topkill (28-30):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   itsHeightToTopkill = new MyNLineText (itsScrollingWindow,
                             CPoint(left,top), 40, 20);

   top += 32;
   left = 4;
   len  = ((end-left)/2);
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Ht Growth (31-34):", 0L);
   left = right+4; right=len;
   itsHeightGrowth = new MyNLineText (itsScrollingWindow,
                                   CPoint(left,top), 40, 20);
   left = right+4; right=left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Crown ratio (35):", CTL_FLAG_RIGHT_JUST);
   left = right+4; right=end-4;
   itsCrownRatio = new MyNLineText (itsScrollingWindow,
                                CPoint(left,top), 30, 20);

   top += 32;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Damage code 1 (36-37):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsDamageCode1 = new MyNLineText (itsScrollingWindow,
                                     CPoint(left,top), len, 20);
   left = right+4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Severity 1 (38-39):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   right= left+len;
   itsDamageSeverity1 = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len, 20);
   left = right+8;
   right= left+80;
   itsHelpButton = new NButton(itsScrollingWindow,
                               CRect(left,top,right,top+30), "Help");
   itsHelpButton->SetCommand(16);

   top += 32;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Damage code 2 (40-41):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsDamageCode2 = new MyNLineText (itsScrollingWindow,
                                     CPoint(left,top), len, 20);
   left = right+4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Severity 2 (42-43):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   right= left+len;
   itsDamageSeverity2 = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len, 20);
   left = right+8;
   right= left+80;
   itsHelpButton = new NButton(itsScrollingWindow,
                               CRect(left,top,right,top+30), "Help");
   itsHelpButton->SetCommand(16);

   top += 32;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Damage code 3 (44-45):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsDamageCode3 = new MyNLineText (itsScrollingWindow,
                                     CPoint(left,top), len, 20);
   left = right+4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Severity 3 (46-47):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   right= left+len;
   itsDamageSeverity3 = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len, 20);
   left = right+8;
   right= left+80;
   itsHelpButton = new NButton(itsScrollingWindow,
                               CRect(left,top,right,top+30), "Help");
   itsHelpButton->SetCommand(16);


   top += 32;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Tree value class (48):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsTreeValueClass = new MyNLineText (itsScrollingWindow,
                                     CPoint(left,top), len, 20);
   left = right+4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Prescription code (49):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   right= left+len;
   itsCutOrLeave = new MyNLineText (itsScrollingWindow,
                                         CPoint(left,top), len, 20);
   left = right+8;
   right= left+80;
   itsHelpButton = new NButton(itsScrollingWindow,
                               CRect(left,top,right,top+30), "Help");
   itsHelpButton->SetCommand(16);

   top += 32;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Plot slope (50-51):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsSlope = new MyNLineText (itsScrollingWindow,
                               CPoint(left,top), len, 20);
   left = right+4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Plot aspect (52-54):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   right= left+len;
   itsAspect = new MyNLineText (itsScrollingWindow,
                                CPoint(left,top), len, 20);
   left = right+8;
   right= left+80;
   itsHelpButton = new NButton(itsScrollingWindow,
                               CRect(left,top,right,top+30), "Help");
   itsHelpButton->SetCommand(16);

   top += 32;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Plot habitat/PA (55-57):", 0L);
   left = right+4; right=left+40;
   itsHabCode = new MyNLineText (itsScrollingWindow,
                                 CPoint(left,top), right-left, 25);
   left = right+4; right=end-4;
   itsHabCodeNListButton = new NListButton(itsScrollingWindow,
                                           CRect(left,top,end,top+200),
                                           itsHabCodeList, 0L);
   itsHabCodeNListButton->SetCommand(26);

   top += 36;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Plot topo code (58):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsTopoCode = new MyNLineText (itsScrollingWindow,
                               CPoint(left,top), len, 20);
   left = right+4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Plot site prep (59):", CTL_FLAG_RIGHT_JUST);
   left = right+4;
   right= left+len;
   itsSitePrepCode = new MyNLineText (itsScrollingWindow,
                                      CPoint(left,top), len, 20);
   left = right+8;
   right= left+80;
   itsHelpButton = new NButton(itsScrollingWindow,
                               CRect(left,top,right,top+30), "Help");
   itsHelpButton->SetCommand(16);

   top += 36;
   left = 4;
   right= left+170;
   new NText(itsScrollingWindow, CRect(left,top,right,top+20),
             "Tree age (60-62):", 0L);
   left = right+4;
   len  = 40;
   right= left+len;
   itsTreeAge = new MyNLineText (itsScrollingWindow,
                               CPoint(left,top), len, 20);



   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), EDITFVSTWindow, 0L);

   // Load the codes lists and display the record.

   DoCommand (3,NULL);

   aNavigator->AppendTabStop(new CTabStop(itsFVSTDFNameNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsBrowseButton));
   aNavigator->AppendTabStop(new CTabStop(itsVariantsNListButton));
   aNavigator->AppendTabStop(new CTabStop(itsRecordNumberNLineText));
   aNavigator->AppendTabStop(new CTabStop(itsPreviousButton));
   aNavigator->AppendTabStop(new CTabStop(itsNextButton));
   aNavigator->AppendTabStop(new CTabStop(itsNewButton));
   aNavigator->AppendTabStop(new CTabStop(itsDeleteButton));
   aNavigator->AppendTabStop(new CTabStop(itsDuplicateButton));
   itsScrollingWindow->AppendToNavigator(aNavigator);
   aNavigator->AppendTabStop(new CTabStop(itsOkButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveButton));
   aNavigator->AppendTabStop(new CTabStop(itsSaveAsButton));
   aNavigator->AppendTabStop(new CTabStop(itsTemplateButton));
   aNavigator->AppendTabStop(new CTabStop(itsCancelButton));
   itsScrollingWindow->ShrinkToFit();
   AdjustScrollingWinSize(itsScrollingWindow, this);

   Show();
   aNavigator->InitFocus();
}






EditFVSTreeDataWin::~EditFVSTreeDataWin( void )
{
   itsVariantsList.clearAndDestroy();
   itsTreeHistoryCodesList.clearAndDestroy();
   itsHabCodeList.clearAndDestroy();
   itsSpeciesList.clearAndDestroy();
   itsFVSTDRecords.clearAndDestroy();
   if (itsTemplate) delete itsTemplate;
}




void EditFVSTreeDataWin::DoCommand(long theCommand,void* theData)
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
            itsFVSTDFNameNLineText->SetText(path);
            ReadFile();
            DisplayRecord();
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 3:                         // Variant list button
      {
         int curVarNum = itsVariantsNListButton->GetSelectPosition();
         if (itsVariantsMSTextPointer && curVarNum > -1)
         {
            RWOrdered * theVariants = itsVariantsMSTextPointer->GetThePKeys();
            PKeyData  * theVariant  = (PKeyData *) theVariants->
                                         at((size_t) curVarNum);

            LoadCodes(&itsSpeciesList,"species_",
                      theVariant,itsSpeciesNListButton);
            LoadCodes(&itsHabCodeList,"HabPa_",
                      theVariant,itsHabCodeNListButton);
         }
         DisplayRecord();
         break;
      }
      case 4:                         // Previous
      {
         StoreCurrent();
         if (itsFVSTDRecords.entries())
         {
            if (itsCurrentRecordNumber == 0) itsCurrentRecordNumber =
                                         itsFVSTDRecords.entries()-1;
            else itsCurrentRecordNumber--;
         }
         else itsCurrentRecordNumber = 0;
         DisplayRecord();
         break;
      }
      case 5:                         // Next
      {
         StoreCurrent();
         itsCurrentRecordNumber++;
         if (itsCurrentRecordNumber >= itsFVSTDRecords.entries())
            itsCurrentRecordNumber = 0;
         DisplayRecord();
         FocusOnNextObject();
         break;
      }
      case 6:                         // New
      {
         StoreCurrent();
         CStringRWC * aRec;
         if (itsTemplate) aRec = new CStringRWC(*itsTemplate);
         else             aRec = new CStringRWC();
         if (itsFVSTDRecords.entries() == itsCurrentRecordNumber++)
              itsFVSTDRecords.insert(aRec);
         else itsFVSTDRecords.insertAt(itsCurrentRecordNumber,aRec);
         DisplayRecord();
         break;
      }
      case 7:                         // Delete
      {
         if (itsFVSTDRecords.entries())
            itsFVSTDRecords.removeAndDestroy(itsFVSTDRecords.
                                        at(itsCurrentRecordNumber));
         DisplayRecord();
         break;
      }
      case 8:                         // Duplicate
      {
         StoreCurrent();
         if (itsFVSTDRecords.entries())
         {
            CStringRWC * tmp = (CStringRWC *) itsFVSTDRecords.
               at(itsCurrentRecordNumber);
            tmp = new CStringRWC(*tmp);
            itsFVSTDRecords.insertAt(itsCurrentRecordNumber++,tmp);
         }
         else xvt_dm_post_error("Nothing to duplicate.");
         DisplayRecord();
         break;
      }
      case 9:                         // Record N of N text CR command
      {
         StoreCurrent();
         CStringRW tmp = itsRecordNumberNLineText->GetText();
         tmp = tmp.strip(RWCString::both);
         long i = ConvertToLong(tmp.data())-1;
         if (i < 0) itsCurrentRecordNumber = 0;
         else itsCurrentRecordNumber = i;
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
         fileSpec.type[0]=0;
         xvt_fsys_get_dir(&fileSpec.dir);
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_save(&fileSpec, "FVS tree file:")
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR,
                     fileSpec.name);
            itsFVSTDFNameNLineText->SetText(path);
            WriteFile();
         }
         xvt_fsys_restore_dir();
         break;
      }
      case 13:                        // Template button pushed
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
            CStringRWC * tmp = (CStringRWC *) itsFVSTDRecords.
               at(itsCurrentRecordNumber);
            itsTemplate = new CStringRW(*tmp);
            xvt_dm_post_message("Current record is set as template.");
         }
         break;
      }

      case 14:                        // Import button pushed (maybe!)
      {
         xvt_dm_post_message("Import function is not yet available.");
         break;
      }
      case 15:                        // Cancel button pushed
      {
         Close();
         break;
      }
      case 16:                        // Help button for codes
      {
         xvt_help_display_topic(helpInfo, FVSTREEDATA);
         break;
      }
      case 20:                        // CR command in MyNLineTexts
      {
         StoreCurrent();
         FocusOnNextObject(theData);
         break;
      }
      case 21:                        // Tree History Code MyNLineText
      {
         int i;
         sscanf (itsTreeHistory->GetText().data(),"%d", &i);
         if (i > -1 && i < itsTreeHistoryCodesList.entries())
              itsTreeHistoryCodesNListButton->SelectItem(i);
         else itsTreeHistoryCodesNListButton->DeselectAll();
         StoreCurrent();
         FocusOnNextObject(itsTreeHistory);
         break;
      }
      case 22:                        // Tree History Code list button
      {
         if (itsTreeHistoryCodesNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsTreeHistoryCodesNListButton->
                  GetFirstSelectedItem();
            tmp.resize(tmp.first(' '));
            itsTreeHistory->SetText(tmp);
            StoreCurrent();
            FocusOnNextObject(itsTreeHistory);
         }
         break;
      }
      case 23:                        // Species Code MyNLineText
      {
         CStringRW tmp = itsSpecies->GetText();
         tmp.toUpper();
         itsSpecies->SetText(tmp);
         itsSpeciesNListButton->DeselectAll();
         RWOrderedIterator anIter(itsSpeciesList);
         CStringRWC * aSpecies;
         int i=0;
         int l2=tmp.length();
         while (aSpecies = (CStringRWC *) anIter())
         {
            int len = aSpecies->first(' ');
            if (strncmp(aSpecies->data(),tmp.data(),max(l2,len)) == 0)
            {
               itsSpeciesNListButton->SelectItem(i);
               break;
            }
            else i++;
         }
         StoreCurrent();
         FocusOnNextObject(itsSpecies);
         break;
      }
      case 24:                        // Species Code list button
      {
         if (itsSpeciesNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsSpeciesNListButton->GetFirstSelectedItem();
            tmp.resize(tmp.first(' '));
            itsSpecies->SetText(tmp);
            StoreCurrent();
            FocusOnNextObject(itsSpecies);
         }
         break;
      }
      case 25:                        // Plot Hab/PA Code MyNLineText
      {
         CStringRW tmp = itsHabCode->GetText();
         tmp.toUpper();
         itsHabCode->SetText(tmp);

         RWOrderedIterator anIter(itsHabCodeList);
         CStringRWC * aHab;
         int i=0;
         int l2=tmp.length();
         itsHabCodeNListButton->DeselectAll();
         while (aHab= (CStringRWC *) anIter())
         {
            int len = aHab->first(' ');
            if (strncmp(aHab->data(),
                        itsHabCode->GetText().data(),max(l2,len)) == 0)
            {
               itsHabCodeNListButton->SelectItem(i);
               break;
            }
            else i++;
         }
         StoreCurrent();
         FocusOnNextObject(itsHabCode);
         break;
      }
      case 26:                        // Plot Hab/PA list button
      {
         if (itsHabCodeNListButton->GetSelectPosition() > -1)
         {
            CStringRW tmp = itsHabCodeNListButton->GetFirstSelectedItem();
            tmp.resize(tmp.first(' '));
            itsHabCode->SetText(tmp);
            StoreCurrent();
            FocusOnNextObject(itsHabCode);
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


inline CStringRW StashSetString (CStringRW & rec, size_t start, size_t len)
{
   CStringRW tmp = rec(start,len);
   tmp = tmp.strip(RWCString::both);
   return tmp;
}


void EditFVSTreeDataWin::DisplayRecord( void )
{
   CStringRW newTitle ("of ");
   if (itsFVSTDRecords.entries())
   {
      if (itsCurrentRecordNumber >= itsFVSTDRecords.entries())
          itsCurrentRecordNumber  = itsFVSTDRecords.entries()-1;

      newTitle += ConvertToString((int) itsFVSTDRecords.entries());
      itsRecordCountNText->SetTitle(newTitle);

      itsRecordNumberNLineText->
         SetText(ConvertToString((int) itsCurrentRecordNumber+1));

      CStringRW rec = (CStringRWC &) *itsFVSTDRecords.
                                      at(itsCurrentRecordNumber);

      rec.resize((size_t) 62);

      itsCurrentRecord->SetTitle(rec);

      itsPlotID     ->SetText(StashSetString(rec,0,4));
      itsTreeID     ->SetText(StashSetString(rec,4,3));
      itsTreeCount  ->SetText(StashSetString(rec,7,6));

      itsTreeHistory->SetText(StashSetString(rec,13,1));
      int i;
      sscanf (itsTreeHistory->GetText().data(),"%d", &i);
      if (i > -1 && i < itsTreeHistoryCodesList.entries())
           itsTreeHistoryCodesNListButton->SelectItem(i);
      else itsTreeHistoryCodesNListButton->DeselectAll();

      itsSpecies->SetText(StashSetString(rec,14,3));
      itsSpeciesNListButton->DeselectAll();
      RWOrderedIterator aSpIter(itsSpeciesList);
      CStringRWC * aSpecies;
      i=0;
      while (aSpecies = (CStringRWC *) aSpIter())
      {
         int len = aSpecies->first(' ');
         if (strncmp(aSpecies->data(),
                     itsSpecies->GetText().data(),len) == 0)
         {
            itsSpeciesNListButton->SelectItem(i);
            break;
         }
         else i++;
      }

      itsDBH            ->SetText(StashSetString(rec,17,4));
      itsDBHGrowth      ->SetText(StashSetString(rec,21,3));
      itsHeight         ->SetText(StashSetString(rec,24,3));
      itsHeightToTopkill->SetText(StashSetString(rec,27,3));
      itsHeightGrowth   ->SetText(StashSetString(rec,30,3));
      itsCrownRatio     ->SetText(StashSetString(rec,34,1));
      itsDamageCode1    ->SetText(StashSetString(rec,35,2));
      itsDamageSeverity1->SetText(StashSetString(rec,37,2));
      itsDamageCode2    ->SetText(StashSetString(rec,39,2));
      itsDamageSeverity2->SetText(StashSetString(rec,41,2));
      itsDamageCode3    ->SetText(StashSetString(rec,43,2));
      itsDamageSeverity3->SetText(StashSetString(rec,45,2));
      itsTreeValueClass ->SetText(StashSetString(rec,47,1));
      itsCutOrLeave     ->SetText(StashSetString(rec,48,1));

      itsSlope          ->SetText(StashSetString(rec,49,2));
      itsAspect         ->SetText(StashSetString(rec,51,3));

      itsHabCode       ->SetText(StashSetString(rec,54,3));
      itsHabCodeNListButton->DeselectAll();
      RWOrderedIterator aHabIter(itsHabCodeList);
      CStringRWC * aHab;
      i=0;
      while (aHab= (CStringRWC *) aHabIter())
      {
         int len = aHab->first(' ');
         if (strncmp(aHab->data(),
                     itsHabCode->GetText().data(),len) == 0)
         {
            itsHabCodeNListButton->SelectItem(i);
            break;
         }
         else i++;
      }

      itsTopoCode      ->SetText(StashSetString(rec,57,1));
      itsSitePrepCode  ->SetText(StashSetString(rec,58,1));
      itsTreeAge       ->SetText(StashSetString(rec,59,3));
   }
   else
   {
      itsCurrentRecordNumber=0;
      DoCommand (6,NULL);
   }
}



void EditFVSTreeDataWin::StashGetString (MyNLineText * obj, CStringRW & rec,
                                         size_t start, size_t len, int left)
{

   CStringRW tmp = obj->GetText();
   tmp.toUpper();
   int lenOfTmp = tmp.length();

   if (lenOfTmp && lenOfTmp < len && !left)
   {
      CStringRW t;
      t.resize(len);
      rec(start,len) = t;
      rec(start+len-lenOfTmp,lenOfTmp) = tmp;
   }
   else
   {
      tmp.resize(len);
      rec(start,len) = tmp;
   }
}


void EditFVSTreeDataWin::StoreCurrent( void )
{
   if (itsFVSTDRecords.entries())
   {
      CStringRW & rec = (CStringRWC &) *itsFVSTDRecords.
                                        at(itsCurrentRecordNumber);
      rec.resize(62);

      StashGetString(itsPlotID,rec,0,4);
      StashGetString(itsTreeID,rec,4,3);
      StashGetString(itsTreeCount,rec,7,6);
      StashGetString(itsTreeHistory,rec,13,1);
      StashGetString(itsSpecies,rec,14,3,1);
      StashGetString(itsDBH,rec,17,4);
      StashGetString(itsDBHGrowth,rec,21,3);
      StashGetString(itsHeight,rec,24,3);
      StashGetString(itsHeightToTopkill,rec,27,3);
      StashGetString(itsHeightGrowth,rec,30,3);
      StashGetString(itsCrownRatio,rec,34,1);
      StashGetString(itsDamageCode1,rec,35,2);
      StashGetString(itsDamageSeverity1,rec,37,2);
      StashGetString(itsDamageCode2,rec,39,2);
      StashGetString(itsDamageSeverity2,rec,41,2);
      StashGetString(itsDamageCode3,rec,43,2);
      StashGetString(itsDamageSeverity3,rec,45,2);
      StashGetString(itsTreeValueClass,rec,47,1);
      StashGetString(itsCutOrLeave,rec,48,1);
      StashGetString(itsSlope,rec,49,2);
      StashGetString(itsAspect,rec,51,3);
      StashGetString(itsHabCode,rec,54,3);
      StashGetString(itsTopoCode,rec,57,1);
      StashGetString(itsSitePrepCode,rec,58,1);
      StashGetString(itsTreeAge,rec,59,3);

      itsCurrentRecord->SetTitle(rec);
   }
}




int EditFVSTreeDataWin::WriteFile( void )
{
   StoreCurrent();
   if (itsFVSTDRecords.entries() &&
       itsFVSTDFNameNLineText->GetText().length())
   {
      std::ofstream treeData(itsFVSTDFNameNLineText->GetText().data());
      if (treeData)
      {
         int nRec = 0;
         RWOrderedIterator anIter(itsFVSTDRecords);
         CStringRWC * aRec;
         while (aRec = (CStringRWC *) anIter())
         {
            if (aRec->length())
            {
                treeData << *aRec << std::endl;
               nRec++;
            }
         }
         treeData.close();
         xvt_dm_post_message("%d tree records output.", nRec);
         return 1;
      }
      else xvt_dm_post_warning("File %s could not be opened.",
                               itsFVSTDFNameNLineText->
                               GetText().data());
   }
   return 0;
}


void EditFVSTreeDataWin::ReadFile( void )
{
   itsFVSTDRecords.clearAndDestroy();
   itsCurrentRecordNumber = 0;

   if (itsFVSTDFNameNLineText->GetText().length())
   {
      std::ifstream treeData(itsFVSTDFNameNLineText->GetText().data());

      if (treeData)
      {
         CStringRW tmp;
         while (!treeData.eof())
         {
            tmp.readToDelim(treeData);
            if (!treeData.eof())
            {
               CStringRWC * newFVSRecord = new CStringRWC(tmp);
               itsFVSTDRecords.insert(newFVSRecord);
            }
         }
         treeData.close();
      }
      else xvt_dm_post_warning ("Suppose can not open FVS Tree Data File %s",
                                itsFVSTDFNameNLineText->GetText().data());
   }
}



void EditFVSTreeDataWin::LoadCodes(RWOrdered   * theList,
                                   const char  * theName,
                                   PKeyData    * theVariant,
                                   NListButton * theListButton)
{
   theList->clearAndDestroy();

   CStringRWC neededMSText = theName;
   neededMSText += *theVariant;
   MSText * theMSText = (MSText *) theSpGlobals->
      theParmsData->find(&neededMSText);
   if (theMSText)
   {
      RWOrdered * thePKeys = theMSText->GetThePKeys();
      RWOrderedIterator pkeyIter(*thePKeys);
      PKeyData * aPkey;
      while (aPkey = (PKeyData *) pkeyIter())
      {
         CStringRWC * newCode = new CStringRWC((CStringRWC &) *aPkey);
         *newCode += " = ";
         *newCode += aPkey->pString;
         theList->insert(newCode);
      }
   }

   if (theList->entries())
   {
      theListButton->IListButton(*theList);
      theListButton->DeselectAll();
      theListButton->DoShow();
   }
   else theListButton->DoHide();
}



void EditFVSTreeDataWin::FocusOnNextObject(void * curObj)
{
   if (curObj == NULL) return;
   else if (curObj == itsPlotID)          itsTreeID->Activate();
   else if (curObj == itsTreeID)          itsTreeCount->Activate();
   else if (curObj == itsTreeCount)       itsTreeHistory->Activate();
   else if (curObj == itsTreeHistory)     itsSpecies->Activate();
   else if (curObj == itsSpecies)         itsDBH->Activate();
   else if (curObj == itsDBH)             itsDBHGrowth->Activate();
   else if (curObj == itsDBHGrowth)       itsHeight->Activate();
   else if (curObj == itsHeight)          itsHeightToTopkill->Activate();
   else if (curObj == itsHeightToTopkill) itsHeightGrowth->Activate();
   else if (curObj == itsHeightGrowth)    itsCrownRatio->Activate();
   else if (curObj == itsCrownRatio)      itsDamageCode1->Activate();
   else if (curObj == itsDamageCode1)     itsDamageSeverity1->Activate();
   else if (curObj == itsDamageSeverity1) itsDamageCode2->Activate();
   else if (curObj == itsDamageCode2)     itsDamageSeverity2->Activate();
   else if (curObj == itsDamageSeverity2) itsDamageCode3->Activate();
   else if (curObj == itsDamageCode3)     itsDamageSeverity3->Activate();
   else if (curObj == itsDamageSeverity3) itsTreeValueClass->Activate();
   else if (curObj == itsTreeValueClass)  itsCutOrLeave->Activate();
   else if (curObj == itsCutOrLeave)      itsSlope->Activate();
   else if (curObj == itsSlope)           itsAspect->Activate();
   else if (curObj == itsAspect)          itsHabCode->Activate();
   else if (curObj == itsHabCode)         itsTopoCode->Activate();
   else if (curObj == itsTopoCode)        itsSitePrepCode->Activate();
   else if (curObj == itsSitePrepCode)    itsTreeAge->Activate();
   else if (curObj == itsTreeAge)         itsPlotID->Activate();
}
