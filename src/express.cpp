/*********************************************************************

   File Name:    express.cpp
   Author:       nlc
   Date:         02/27/95

   see express.hpp for notes.
 *
 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include "express.hpp"
#include "compon.hpp"
#include "cparmrtn.hpp"
#include "appdoc.hpp"

#include NScrollBar_i
#include NButton_i
#include NText_i
#include NScrollBar_i
#include NScrollText_i
#include NEditControl_i
#include CStringCollection_i
#include NListButton_i
#include CStringRWC_i
#include CDesktop_i
#include CNavigator_i

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include "mycscrol.hpp"
#include "spglobal.hpp"
#include "variants.hpp"
#include "spparms.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "fvslocs.hpp"
#include "extens.hpp"
#include "suppdefs.hpp"
#include "warn_dlg.hpp"
#include "sendupdt.hpp"
#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

/////////////////////////////////////////////////////////////////////////
void PWR_CALLCONV1 hookTextEditCallback_BuildExpression(
	TXEDIT theTxtHandle,
	T_LNUM theOrigin,
	T_LNUM theLines,
	T_CPOS theOrgOffset )
{
   // Call base method first
	NScrollText::ScrollTextCallback(theTxtHandle, theOrigin, theLines, theOrgOffset);

   if (theOrgOffset != USHRT_MAX)
   {
      NScrollText *aNScrollText = (NScrollText*)xvt_tx_get_data(theTxtHandle);
      BuildExpression *aBuildExpression = (BuildExpression *)aNScrollText->GetCWindow();
      aBuildExpression->ScrollColumnRuler(theOrgOffset);
   }
}
/////////////////////////////////////////////////////////////////////////

BuildExpression::BuildExpression(const CStringRW & theTitle,
                                 Component * aComponent,
                                 Component * relatedCondition,
                                 int aCondition)

                :CWwindow((CDocument *) theSpGlobals->theAppDocument,
                          RightWinPlacement(CRect(5,22,685,614)),
                          theTitle,
                          WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                          W_DOC, TASK_MENUBAR),
                 theComponent(aComponent),
                 theRelatedConditionComponent(relatedCondition),
                 isACondition(aCondition)
{
   xvt_scr_set_busy_cursor();
                                                 // Initialize
   speciesList = NULL;
   forestsList = NULL;
   habPaList   = NULL;
   FVSFunctionsList = NULL;
   mathFunctionsList = NULL;
   variablesList = NULL;
   operatorsList = NULL;
   theFieldTypes = NULL;
   theAnswerPtrs = NULL;
   FVSFunctionMSText = NULL;
   expressionContentsPKeyPtr = NULL;
   insertFromDescriptiveText = 1;

   // create and clear the navigator

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   // insure that the components knows its openned window.
   theComponent->openedWindow = this;

   // Reset the source window.
   // isACondition is 2 if FreeForm is the window tag. (don't change it).

   if (isACondition <= 1)
   {
      if (theComponent->sourceWindow)
         *theComponent->sourceWindow="BuildExpression";
      else theComponent->sourceWindow=new CStringRW("BuildExpression");
   }

   // find expressionContents.  Absence of expressionContents
   // implies that {always} is the only legal class of
   // variables or functions in the lists.

   if (theComponent->windowParms) expressionContentsPKeyPtr =
      GetPKeyData(GetCParm(theComponent->windowParms->data(), 1),
                  "expressionContents", "always after");

   // if there are expressionContents, make sure that the ATList is
   // not empty.  If it is, set expressionContentsPKeyPtr = NULL.

   if (expressionContentsPKeyPtr &&
       expressionContentsPKeyPtr->aTList.entries() == 0)
      expressionContentsPKeyPtr = NULL;

   int top = 8;
   new NText(this, CRect(8,12,64,32), "Name: ", 0L);

   componentName = new NEditControl(this, CRect(72,top-4,600,top+28),
      (CStringRW &) *theComponent, 0L);

   top += 36;

                                                 // Variables Button
   variablesText = new NText(this, CRect(8,top,104,top+24), "Variables", 0L);
   BuildVariablesList();
   theVariablesButton = new NListButton(this, CRect(8,top+32,128,top+258),
                        *variablesList, 0, 0L);
   theVariablesButton->DeselectAll();
   theVariablesButton->SetCommand( 1);


                                                 // Operators Button
   operatorsText = new NText(this, CRect(136,top,240,top+24), "Operators", 0L);
   BuildOperatorsList();
   theOperatorsButton = new NListButton(this, CRect(136,top+32,216,top+258),
                        *operatorsList, 0, 0L);
   theOperatorsButton->DeselectAll();
   theOperatorsButton->SetCommand( 2);


                                                 // Function Choices
   functionsChoices = new RWOrdered((size_t) 2);
   functionsChoices->insert(new CStringRWC ("FVS Functions"));
   functionsChoices->insert(new CStringRWC ("Math Functions"));

   theFunctionsChoicesButton = new NListButton(this, CRect(224,top-4,376,top+76),
                                               *functionsChoices, 0, 0L);
   theFunctionsChoicesButton ->SelectItem(0);
   theFunctionsChoicesButton ->SetCommand(3);

                                                 // Functions (initially loaded with Math).

   BuildFVSFunctionsList();
   theFunctionsButton = new NListButton(this, CRect(224,top+32,376,top+258),
                        *FVSFunctionsList, 0, 0L);
   theFunctionsButton->DeselectAll();
   theFunctionsButton->SetCommand( 4);

                                                 // Constants Choices Button

   constantsChoices = new RWOrdered((size_t) 3);
   constantsChoices->insert(new CStringRWC ("Species"));
   constantsChoices->insert(new CStringRWC ("Forests"));
   constantsChoices->insert(new CStringRWC ("Habitat types/Plant Ass."));

   theConstantsChoicesButton = new NListButton(this, CRect(384,top-4,600,top+88),
                                  *constantsChoices, 0, 0L);

   theConstantsChoicesButton->SelectItem(0);
   theConstantsChoicesButton->SetCommand(5);

                            // Constants Button (initially loaded with species).
   BuildSpeciesList();
   theConstantsButton = new NListButton(this, CRect(384,top+32,600,top+258),
                        *speciesList, 0, 0L);
   theConstantsButton->DeselectAll();
   theConstantsButton->SetCommand( 6);

                                                 // column Ruler
   CStringRW column = "....+....1....+....2....+....3....+....4"
                      "....+....5....+....6....+....7....+....8"
                      "....+....9....+....0....+....1....+....2"
                      "....+....3....+....4....+....5....+....6";

   top += 70;
   columnRuler = new NScrollText(this, CRect(8,top,16,top+32), FALSE, FALSE,
                 TX_BORDER | TX_READONLY);
   #ifdef AIXV4
      columnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
   #else
      columnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
   #endif
   columnRuler->SetText(column);

   // margin for all characters in columnRuler
   int maxmargin = xvt_dwin_get_text_width(columnRuler->GetCWindow()->GetXVTWindow(),
                                           (char *) column.data(), -1);
   // margin for first 80 characters in columnRuler
   int margin = xvt_dwin_get_text_width(columnRuler->GetCWindow()->GetXVTWindow(),
                                        (char *) CStringRW(column(0, 80)).data(), -1);

   int leadingp;
   int ascentp;
   int descent;
   xvt_dwin_get_font_metrics(columnRuler->GetCWindow()->GetXVTWindow(),
               &leadingp, &ascentp, &descent);

   int bottom = top+32;
   int width = margin+16;
   bottom = 8+top+leadingp+ascentp+descent;
   columnRuler->Size(CRect(8,top,width,bottom));
   columnRuler->Disable();

                                                 // Text Edit (set CRect only)
   top = bottom+4;

   bottom  = 12+top+(8*(leadingp+ascentp+descent));  // allow for 8 lines.
   if (bottom < 300) bottom = 300;

   width = width+NScrollBar::NativeWidth()+2;

   // define the location of the textEdit object...build it later.

   CRect textEditRect = CRect(8,top,width,bottom);

                                                 // Scrolling Window
   int tempWidth = 592;
   if (width > tempWidth) tempWidth = width;
   cScrollerFrameWidth = tempWidth-2-NScrollBar::NativeWidth();
   theScrollingWindow = new MyCScroller(this, CRect(8,40,tempWidth,bottom),
                                        cScrollerFrameWidth, 3000);
   theScrollingWindow ->IScroller(FALSE, TRUE, TRUE, 24, 240);
   theScrollingWindow ->DoHide();

   top = bottom+4;
   bottom  = 12+top+(4*(leadingp+ascentp+descent));    // allow for 4 lines.

   xvt_dwin_get_font_metrics(this->GetCWindow()->GetXVTWindow(),
               &leadingp, &ascentp, &descent);

                                                 // Descriptive Text
   int tempbottom = 12+top+(2*(leadingp+ascentp+descent));  // allow for 2 lines
   if (bottom < tempbottom) bottom = tempbottom;            // of descriptive text

                                                 // Function Render
   functionRender = new NScrollText(this, CRect(8,top,width,bottom), FALSE,
                                    TRUE, TX_BORDER,
                                    (UNITS) margin, 1000, TRUE);

   functionRender->Disable();

   #ifdef AIXV4
      functionRender->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
   #else
      functionRender->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
   #endif

   functionRender->DoHide();

                                                 // text edit (create)
   textEdit = new NScrollText(this, textEditRect, TRUE, TRUE,
                              TX_BORDER | TX_ENABLECLEAR | TX_AUTOHSCROLL | TX_WRAP,
                              (UNITS) maxmargin, 2000, TRUE);

	xvt_tx_set_scroll_callback(textEdit->GetXVTTextEdit(), hookTextEditCallback_BuildExpression);

   #ifdef AIXV4
      textEdit->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
   #else
      textEdit->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
   #endif

   if (theComponent->keywords)
      textEdit->SetText(*theComponent->keywords);

   descriptiveText = new NText(this, CRect(8,top,tempWidth,bottom),
                               NULLString, 0L);

                                                // Button spacing (create)
   tempWidth += 8;
   top = bottom+8;
   bottom = top+32;

   int left = 8; int right = 104;
   width = (3*96)+224;  // total width of 4 buttons.
   if (tempWidth < width+40) tempWidth = width+40;   // 40 = 5*8
   int space = (tempWidth-16-width)/3;

                                                 // Ok
   okButton = new NButton(this, CRect(left,top,right,bottom), "Ok", 0L);
   okButton->SetCommand(7);

                                                 // Set Function Arguments
   left = right+space; right = left+224;
   setFunctionButton = new NButton(this, CRect(left,top,right,bottom),
                           "Set Function Arguments", 0L);
   setFunctionButton->SetCommand(8);
   setFunctionButton->Disable();

                                                 // Finished Setting Function
   finishedSettingButton = new NButton(this,
      CRect(left,top,right,bottom), "Finished Setting Function", 0L);
   finishedSettingButton->SetCommand(9);
   finishedSettingButton->DoHide();

                                                 // Insert
   left = right+space; right = left+96;
   insertButton = new NButton(this, CRect(left,top,right,bottom), "Insert", 0L);
   insertButton->SetCommand(10);

                                                 // Cancel
   left = right+space; right = left+96;
   cancelButton = new NButton(this, CRect(left,top,right,bottom), "Cancel", 0L);
   cancelButton->SetCommand(11);

   left = right+8;
   if (tempWidth < left) tempWidth = left;

   RCT newrct = RCT (this->GetClippedFrame());
   newrct.bottom=newrct.top+bottom+8;
   newrct.right =newrct.left+tempWidth;

#ifdef AIXV4
   xvt_app_process_pending_events();
#endif

   // Note: the cast (PNT *) on a RCT is OK here because XVT said so!

   xvt_vobj_translate_points (GetXVTWindow(), TASK_WIN,
                              (PNT *) &newrct, 2);
   Size(CRect(newrct));

   Show();

   SetDescriptiveText( 0, 0);

   // NOTICE:  For some strange reason, the sequence of Sizing and
   // then setting the glue must be maintained in this routine....
   // DON'T change the sequence!!!!!

#ifdef AIXV4
   xvt_app_process_pending_events();
#endif

   columnRuler->SetGlue(RIGHTSTICKY  | LEFTSTICKY);
   textEdit->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                     RIGHTSTICKY  | LEFTSTICKY);
   theScrollingWindow->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                               RIGHTSTICKY  | LEFTSTICKY);
   okButton->SetGlue(BOTTOMSTICKY);
   functionRender->SetGlue(BOTTOMSTICKY);
   descriptiveText->SetGlue(BOTTOMSTICKY);
   setFunctionButton->SetGlue(BOTTOMSTICKY);
   finishedSettingButton->SetGlue(BOTTOMSTICKY);
   insertButton->SetGlue(BOTTOMSTICKY);
   cancelButton->SetGlue(BOTTOMSTICKY);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), EXPRESSWindow, 0L);

   aNav->AppendTabStop(new CTabStop(textEdit));
   aNav->AppendTabStop(new CTabStop(okButton));
   aNav->AppendTabStop(new CTabStop(setFunctionButton));
   aNav->AppendTabStop(new CTabStop(theScrollingWindow));
   //   theScrollingWindow->AppendToNavigator(aNav);
   aNav->AppendTabStop(new CTabStop(finishedSettingButton));
   aNav->AppendTabStop(new CTabStop(insertButton));
   aNav->AppendTabStop(new CTabStop(cancelButton));
   aNav->AppendTabStop(new CTabStop(componentName));
   aNav->AppendTabStop(new CTabStop(theVariablesButton));
   aNav->AppendTabStop(new CTabStop(theFunctionsChoicesButton));
   aNav->AppendTabStop(new CTabStop(theFunctionsButton));
   aNav->AppendTabStop(new CTabStop(theConstantsChoicesButton));
   aNav->InitFocus();
}





BuildExpression::~BuildExpression( void )
{
   if (theFieldTypes) delete []theFieldTypes;
   if (theAnswerPtrs) delete []theAnswerPtrs;
   if (variablesList)
   {
      variablesList->clearAndDestroy();
      delete variablesList;
   }
   if (operatorsList)
   {
      operatorsList->clearAndDestroy();
      delete operatorsList;
   }
   functionsChoices->clearAndDestroy();
   if (FVSFunctionsList)
   {
      FVSFunctionsList->clearAndDestroy();
      delete FVSFunctionsList;
   }
   if (mathFunctionsList)
   {
      mathFunctionsList->clearAndDestroy();
      delete mathFunctionsList;
   }
   constantsChoices->clearAndDestroy();
   if (speciesList)
   {
      speciesList->clearAndDestroy();
      delete speciesList;
   }
   if (forestsList)
   {
      forestsList->clearAndDestroy();
      delete forestsList;
   }
   if (habPaList)
   {
      habPaList->clearAndDestroy();
      delete habPaList;
   }
}






void BuildExpression::DoCommand(long theCommand, void* theData)
{
   switch (theCommand)
   {
      case 1:                         // Variables List Button

         theOperatorsButton->DeselectAll();
         theFunctionsButton->DeselectAll();
         theConstantsButton->DeselectAll();

         SetDescriptiveText( 1, theVariablesButton->GetSelectPosition());

         break;

      case 2:                         // Operators List Button

         theVariablesButton->DeselectAll();
         theFunctionsButton->DeselectAll();
         theConstantsButton->DeselectAll();

         SetDescriptiveText( 2, theOperatorsButton->GetSelectPosition());

         break;

      case 3:                         // Functions Choices Button

         theVariablesButton->DeselectAll();
         theOperatorsButton->DeselectAll();
         theConstantsButton->DeselectAll();
         if (theFunctionsChoicesButton->GetSelectPosition() == 1)
         {
            if (!mathFunctionsList) BuildMathFunctionsList();
            theFunctionsButton->IListButton(*mathFunctionsList);
         }
         else
         {
            if (!FVSFunctionsList) BuildFVSFunctionsList();
            theFunctionsButton->IListButton(*FVSFunctionsList);
         }
         theFunctionsButton->DeselectAll();
         SetDescriptiveText( 0, 0);
         break;

      case 4:                         // Functions Button

         theVariablesButton->DeselectAll();
         theOperatorsButton->DeselectAll();
         theConstantsButton->DeselectAll();

         if (theFunctionsChoicesButton->GetSelectPosition() == 0)
            SetDescriptiveText( 3, theFunctionsButton->GetSelectPosition());
         else
            SetDescriptiveText( 4, theFunctionsButton->GetSelectPosition());

         break;

      case 5:                         // ConstantsChoices Button
      {
         theVariablesButton->DeselectAll();
         theOperatorsButton->DeselectAll();
         theFunctionsButton->DeselectAll();
         SetDescriptiveText( 0, 0);
         switch (theConstantsChoicesButton->GetSelectPosition())
         {
            case 0:
               if (!speciesList) BuildSpeciesList();
               theConstantsButton->IListButton(*speciesList);
               break;
            case 1:
               if (!forestsList) BuildForestsList();
               theConstantsButton->IListButton(*forestsList);
               break;
            case 2:
               if (!habPaList) BuildHabPaList();
               theConstantsButton->IListButton(*habPaList);
               break;
            default: break;
         }
         theConstantsButton->DeselectAll();
         break;
      }

      case 6:                         // ConstantsList Button
      {
         theVariablesButton->DeselectAll();
         theOperatorsButton->DeselectAll();
         theFunctionsButton->DeselectAll();
         int pick = theConstantsChoicesButton->GetSelectPosition();
         if (pick == -1) SetDescriptiveText (0, 0);
         else            SetDescriptiveText (5+pick,theConstantsButton->
                                                    GetSelectPosition());

         break;
      }

      case 7:                         // Ok button pushed
      {
         if (theScrollingWindow->IsVisible())
         {
            if (xvt_dm_post_ask("Yes","No",NULL,
                                "Finished setting the function?")
                == RESP_DEFAULT)
            {
               DoCommand(9,this);
               break;
            }
            else break;
         }

         if (insertFromDescriptiveText == 0)
         {
            if (xvt_dm_post_ask("Yes","No",NULL,"Insert the function?")
                == RESP_DEFAULT)
            {
               DoCommand(10,this);
               break;
            }
         }

         *theComponent->keywords=textEdit->GetText();
         (CStringRW &) *theComponent=componentName->GetTitle();

         if (theRelatedConditionComponent)
         {
            theComponent->conditionSerialNum=
               theRelatedConditionComponent->componentNumber;
            theSpGlobals->runStream->
               AddComponent(theRelatedConditionComponent);
         }
         theSpGlobals->runStream->AddComponent(theComponent);

         SendUpdateMessage(COMPONENTSListChanged,NULL,this);

         theSpGlobals->theAppDocument->SetSave(TRUE);
         Close();
         break;
      }

      case 8:                         // Set Function Arugments
      {
         if (LoadScrollingWindow())
         {
            variablesText        ->DoHide();
            operatorsText        ->DoHide();
            theFunctionsChoicesButton->DoHide();
            textEdit             ->DoHide();
            theVariablesButton   ->DoHide();
            theOperatorsButton   ->DoHide();
            theFunctionsButton   ->DoHide();
            theConstantsChoicesButton->DoHide();
            theConstantsButton   ->DoHide();
            columnRuler          ->DoHide();
            setFunctionButton    ->DoHide();

            theScrollingWindow   ->DoShow();
            finishedSettingButton->DoShow();

            insertButton         ->Disable();

            DoDraw();
         }
         else setFunctionButton  ->Disable();

         break;
      }

      case 9:                         // Finished Setting Function
      {
         CStringRW theAnswer;
         if (BuildTheKeywords(FVSFunctionMSText,
                              theFieldTypes,
                              theAnswerPtrs,
                              numberOfFields,
                              1, theAnswer))
         {
            descriptiveText->DoHide();
            functionRender->DoShow();
            functionRender->SetText(theAnswer.data());
            insertFromDescriptiveText = 0;
         }
         else descriptiveText->DoShow();

         ClearScrollingWindow();
         theScrollingWindow   ->DoHide();
         finishedSettingButton->DoHide();

         variablesText        ->DoShow();
         operatorsText        ->DoShow();
         theFunctionsChoicesButton->DoShow();
         textEdit             ->DoShow();
         theVariablesButton   ->DoShow();
         theOperatorsButton   ->DoShow();
         theFunctionsButton   ->DoShow();
         theConstantsChoicesButton->DoShow();
         theConstantsButton   ->DoShow();
         columnRuler          ->DoShow();
         setFunctionButton    ->DoShow();
         setFunctionButton    ->Disable();

         insertButton         ->Enable();

         DoDraw();
         break;
      }

      case 10:                        // Insert button pushed
      {
         T_PNUM thePar;
         T_LNUM theLine;
         T_CNUM theChar;
         textEdit->GetInsertPosition(&thePar, &theLine, &theChar);
         CStringRW line = textEdit->GetParagraph(thePar);
         CStringRW newLineContents;
         int insertLength;

         if (insertFromDescriptiveText)
         {
            CStringRW tmpLine = descriptiveText->GetTitle();
            const char * endLine = strchr(tmpLine.data(),(int) ' ');
            if (endLine) insertLength = int (endLine - tmpLine.data());
            else         insertLength = tmpLine.length();

            if (theChar)
            {
               newLineContents = line(0, theChar);
               newLineContents += tmpLine(0, insertLength);
               newLineContents += line(theChar, line.length() - theChar);
            }
            else
            {
               newLineContents = tmpLine(0, insertLength);
               newLineContents += line;
            }
         }
         else
         {
            insertLength = functionRender->GetText().length();
            if (theChar)
            {
               newLineContents = line(0, theChar);
               newLineContents += functionRender->GetText();
               newLineContents += line(theChar, line.length() - theChar);
            }
            else
            {
               newLineContents = functionRender->GetText();
               newLineContents += line(theChar, line.length() - theChar);
            }
            functionRender ->SetText(" ");
            functionRender ->DoHide();
            descriptiveText->DoShow();
            DoDraw();
         }

         textEdit->SetParagraph(newLineContents, thePar);

         T_PNUM newPar;
         T_LNUM newLine;
         T_CNUM newChar;
         textEdit->GetInsertPosition(&newPar, &newLine, &newChar);
         if (newPar == thePar && newLine == theLine)
            textEdit->SetInsertPosition(thePar, theLine,
                                        theChar+insertLength);
         else textEdit->SetInsertPosition(thePar, theLine, theChar);

         // make sure that the keyboard focus is on the textEdit box.

         textEdit->Activate();

         insertFromDescriptiveText = 1;
         SetDescriptiveText(0, 0);

         break;
      }

      case 11:                        // Cancel button pushed

         Close();
         break;

      case EXTENSIONListChanged:
      {
         if (theComponent->extension &&
             theSpGlobals->theExtensions->
             FindLegalExtension(theComponent->extension->data()) == -1)
         {
            CWindow * curFront = G->GetDesktop()->GetFrontWindow();
            if (this != curFront) G->GetDesktop()->SetFrontWindow(this);
            xvt_dm_post_warning("%s uses an extension that is "
                                "no longer being used in this simulation.",
                                GetTitle().data());
            if (this != curFront) G->GetDesktop()->SetFrontWindow(curFront);
         }
         break;
      }

      case VARIANTSelectionChanged:
      {
         CWindow * curFront = G->GetDesktop()->GetFrontWindow();
         if (this != curFront) G->GetDesktop()->SetFrontWindow(this);
         xvt_dm_post_warning("%s is set up for variant that is "
                             "no longer being used in this simulation.",
                             GetTitle().data());
         if (this != curFront) G->GetDesktop()->SetFrontWindow(curFront);
      }


      default:
         CWwindow::DoCommand(theCommand,theData);
         break;


   }        // end switch(theCommand)
}           // end function



BOOLEAN BuildExpression::Close( void )
{
   theComponent->openedWindow = NULL;
   return CWwindow::Close();
}





void BuildExpression::BuildOperatorsList( void )
{
   if (isACondition) operatorsList = new RWOrdered(14);
   else              operatorsList = new RWOrdered(5);
   operatorsList->insert(new CStringRWC ("+"));
   operatorsList->insert(new CStringRWC ("-"));
   operatorsList->insert(new CStringRWC ("*"));
   operatorsList->insert(new CStringRWC ("/"));
   operatorsList->insert(new CStringRWC ("**"));
   if (isACondition)
   {
      operatorsList->insert(new CStringRWC ("EQ"));
      operatorsList->insert(new CStringRWC ("NE"));
      operatorsList->insert(new CStringRWC ("LT"));
      operatorsList->insert(new CStringRWC ("LE"));
      operatorsList->insert(new CStringRWC ("GT"));
      operatorsList->insert(new CStringRWC ("GE"));
      operatorsList->insert(new CStringRWC ("AND"));
      operatorsList->insert(new CStringRWC ("OR"));
      operatorsList->insert(new CStringRWC ("NOT"));
   }
}






void BuildExpression::BuildMathFunctionsList( void )
{
   mathFunctionsList = new RWOrdered(16);
   mathFunctionsList->insert(new CStringRWC ("ABS()"));
   mathFunctionsList->insert(new CStringRWC ("ALog()"));
   mathFunctionsList->insert(new CStringRWC ("ALog10()"));
   mathFunctionsList->insert(new CStringRWC ("ArcCos()"));
   mathFunctionsList->insert(new CStringRWC ("ArcSin()"));
   mathFunctionsList->insert(new CStringRWC ("ArcTan()"));
   mathFunctionsList->insert(new CStringRWC ("Cos()"));
   mathFunctionsList->insert(new CStringRWC ("Exp()"));
   mathFunctionsList->insert(new CStringRWC ("Frac()"));
   mathFunctionsList->insert(new CStringRWC ("Int()"));
   mathFunctionsList->insert(new CStringRWC ("Max()"));
   mathFunctionsList->insert(new CStringRWC ("Min()"));
   mathFunctionsList->insert(new CStringRWC ("Mod()"));
   mathFunctionsList->insert(new CStringRWC ("Sin()"));
   mathFunctionsList->insert(new CStringRWC ("Sqrt()"));
   mathFunctionsList->insert(new CStringRWC ("Tan()"));
}






void BuildExpression::SetDescriptiveText( int whichList, int whichMember)
{

   if (whichList == -1 || whichMember == -1)
   {
      SetDescriptiveText( 0, 0);
      return;
   }

   switch (whichList)
   {
      case 0:                         // Nothing is selected...default text.
      {
         theVariablesButton->DeselectAll();
         theOperatorsButton->DeselectAll();
         theFunctionsButton->DeselectAll();
         theConstantsButton->DeselectAll();

         insertButton->Disable();
         setFunctionButton->Disable();
         descriptiveText->SetTitle("Nothing is selected.");
         break;
      }
      case 1:                         // Variables List
      {
         if (numberVariablesInPtrList <= 0)
            SetDescriptiveText( 0, 0);
         else
         {
            CStringRW tmpString = *listOfVariablesPKeyData[whichMember];
            tmpString += " - ";
            tmpString += listOfVariablesPKeyData[whichMember]->pString;
            descriptiveText->SetTitle(tmpString);
            insertButton   ->Enable();
         }
         break;
      }
      case 2:                         // Operators List
      {
         #if XVTWS==WIN32WS
         static char * far text[] =
         #else
         static char * text[] =
         #endif
         {
            "+ Simple addition",
            "- Subtraction or change sign",
            "* Multiplication",
            "/ Division",
            "** Exponentiate, X**Y is X raised to the power Y",
            "EQ - Logical Equal",
            "NE - Logical Not Equal",
            "LT - Logical Less than",
            "LE - Logical Less than or equal",
            "GT - Logical Greater than",
            "GE - Logical Greater than or equal",
            "AND - Logical AND",
            "OR - Logical OR",
            "NOT - Logical NOT",
            NULL
         };
         descriptiveText->SetTitle(text[whichMember]);
         insertButton   ->Enable();
         break;
      }
      case 3:                         // FVS Functions
      {
         if (numberFunctionsInPtrList <= 0) SetDescriptiveText( 0, 0);
         else
         {
            CStringRW tmpString = *listOfFunctionsPKeyData[whichMember];
            tmpString += "() - ";
            tmpString += listOfFunctionsPKeyData[whichMember]->pString;
            descriptiveText->SetTitle(tmpString);
            insertButton->Enable();

            // see if there is a special section of MSText for this
            // function.  If so, then set a pointer to it and
            // enable the Set Functions Button.

            CStringRWC searchString = "evmon.function.";
            searchString += *listOfFunctionsPKeyData[whichMember];
            FVSFunctionMSText = (MSText *) theSpGlobals->
                  theParmsData->find(&searchString);
            if (FVSFunctionMSText) setFunctionButton->Enable();
         }
         break;
      }
      case 4:                         // Math Functions
      {
         #if XVTWS==WIN32WS
         static char * far text[] =
         #else
         static char * text[] =
         #endif
         {
            "ABS() - Absolute value, ABS(-3) is 3.",
            "ALog() - Natural logarithm (base e)",
            "ALog10() - Common logarithm (base 10)",
            "ArcCos() - Arc cosine (argument in radians)",
            "ArcSin() - Arc sine (argument in radians)",
            "ArcTan() - Arc tangent (argument in radians)",
            "Cos() - Cosine (argument in radians)",
            "Exp() - e raised to power",
            "Frac() - Fractional part of a number, Frac(3.4) is .4",
            "Int() - Integer part of a number, Int(3.4) is 3",
            "Max() - Maximum value of the arguments, Max(5,3,-1,10,2) is 10",
            "Min() - Minimum value of the arguments, Min(5,3,-1,10,2) is -1",
            "Mod() - Returns the remainder after the first argument is divided by the second",
            "Sin() - Sine (argument in radians)",
            "Sqrt() - Square root",
            "Tan() - Tangent (argument in radians)",
            NULL
         };
         insertButton   ->Enable();
         descriptiveText->SetTitle(text[whichMember]);
         break;
      }
      case 5:                         // Species Constants
      {
         int position = theConstantsButton->GetSelectPosition();
         if (position == 0)
         {
            descriptiveText->SetTitle("All - All Species");
         }
         else
         {
            CStringRW text = theSpGlobals->theSpecies->getAlphaCode((const int) --position);
            text += " - ";
            text += theSpGlobals->theSpecies->getCommonName((const int) position);
            descriptiveText->SetTitle(text);
         }
         insertButton   ->Enable();
         break;
      }
      case 6:                         // Forests
      {
         if (theSpGlobals->theForests->getNumberItems())
         {
            int position = theConstantsButton->GetSelectPosition();
            CStringRW text = theSpGlobals->theForests->getNumericCode((const int) position);
            text += " - ";
            text += theSpGlobals->theForests->getName((const int) position);
            descriptiveText->SetTitle(text);
            insertButton   ->Enable();
         }
         else
         {
            SetDescriptiveText( 0, 0);
         }
         break;
      }
      case 7:                         // Habitat type/Plant associations
      {
         if (theSpGlobals->theHabPa->getNumberItems())
         {
            int position = theConstantsButton->GetSelectPosition();
            CStringRW text = theSpGlobals->theHabPa->getNumericCode((const int) position);
            text += " - ";
            text += theSpGlobals->theHabPa->getName((const int) position);
            descriptiveText->SetTitle(text);
            insertButton   ->Enable();
         }
         else
         {
            SetDescriptiveText( 0, 0);
         }
         break;
      }
   }
}






void BuildExpression::BuildVariablesList( void )
{
   // the variables list will only contain functions that
   // are available for the variant for the event monitor
   // phase.  these data are indicated in the useparms PKey
   // associated with the keyword.

   numberVariables = 0;
   numberVariablesInPtrList=0;
   CStringRWC tmpSearch = "evmon.variables";
   MSText * variablesMSTextPtr = (MSText *) theSpGlobals->
         theParmsData->find(&tmpSearch);
   if (variablesMSTextPtr)
      numberVariables = variablesMSTextPtr->GetThePKeys()->entries();

   if (numberVariables <= 0)
   {
      variablesList = new RWOrdered( 1 );
      variablesList ->insert(new CStringRWC (" "));
      return;
   }

   variablesList = new RWOrdered(( 2 > numberVariables+1 ) ?
                                   2 : numberVariables+1);

   listOfVariablesPKeyData = new ptrToPKeyData [numberVariables];

   RWOrderedIterator nextPKeyData(variablesMSTextPtr->thePKeys);
   PKeyData * onePKeyData;
   while (onePKeyData = (PKeyData *) nextPKeyData())
   {
      if (OkToStore(onePKeyData))
      {
         listOfVariablesPKeyData[numberVariablesInPtrList++] = onePKeyData;
         CStringRWC * tmpString = new CStringRWC(*onePKeyData);
         variablesList->insert(tmpString);
      }
   }
   if (numberVariablesInPtrList == 0)
      variablesList->insert(new CStringRWC (NULLString));
}






void BuildExpression::BuildFVSFunctionsList( void )
{
   // the functions list will only contain functions that
   // are available for the variant and for the event monitor
   // phase.  these data are indicated in the useparms PKey
   // associated with the keyword.

   numberFunctions = 0;
   numberFunctionsInPtrList=0;

   CStringRWC tmpSearch = "evmon.functions";
   MSText * functionsMSTextPtr = (MSText *) theSpGlobals->
         theParmsData->find(&tmpSearch);
   if (functionsMSTextPtr)
      numberFunctions = functionsMSTextPtr->GetThePKeys()->entries();

   if (numberFunctions <= 0)
   {
      FVSFunctionsList = new RWOrdered( 1 );
      FVSFunctionsList ->insert(new CStringRWC (NULLString));
      return;
   }

   FVSFunctionsList = new RWOrdered(( 2 > numberFunctions+1 ) ?
                                      2 : numberFunctions+1);

   listOfFunctionsPKeyData = new ptrToPKeyData [numberFunctions];
   RWOrderedIterator nextPKeyData(functionsMSTextPtr->thePKeys);
   PKeyData * onePKeyData;
   while (onePKeyData = (PKeyData *) nextPKeyData())
   {
      if (OkToStore(onePKeyData))
      {
         listOfFunctionsPKeyData[numberFunctionsInPtrList++] = onePKeyData;
         CStringRWC * tmpString = new CStringRWC(*onePKeyData);
         *tmpString += "()";
         FVSFunctionsList->insert(tmpString);
      }
   }
   if (numberFunctionsInPtrList == 0)
      FVSFunctionsList->insert(new CStringRWC (NULLString));
}






void BuildExpression::BuildSpeciesList( void )
{
   int nSpecies = theSpGlobals->theSpecies->getNumberSpecies();
   speciesList = new RWOrdered(( 2 > nSpecies+1 ) ? 2 : nSpecies+1);

   speciesList->insert(new CStringRWC ("All species"));

   if (nSpecies > -1)
   {
      for (int spIndex=0; spIndex<nSpecies; spIndex++)
      {
         speciesList->insert(new CStringRWC (
            theSpGlobals->theSpecies->getCommonName((const int) spIndex)));
      }
   }
}






void BuildExpression::BuildForestsList( void )
{

   int nItems = theSpGlobals->theForests->getNumberItems();
   forestsList = new RWOrdered(( 2 > nItems+1 ) ? 2 : nItems+1);
   if (nItems > -1)
   {
      for (int index=0; index<nItems; index++)
      {
         forestsList->insert(new CStringRWC (
            theSpGlobals->theForests->getName((const int) index)));
      }
   }
   else forestsList->insert(new CStringRWC (NULLString));
}






void BuildExpression::BuildHabPaList( void )
{

   int nItems = theSpGlobals->theHabPa->getNumberItems();
   habPaList = new RWOrdered(( 2 > nItems+1 ) ? 2 : nItems+1);
   if (nItems > -1)
   {
      for (int index=0; index<nItems; index++)
      {
         habPaList->insert(new CStringRWC (
            theSpGlobals->theHabPa->getName((const int) index)));
      }
   }
   else habPaList->insert(new CStringRWC (NULLString));
}






int BuildExpression::LoadScrollingWindow( void )
{
   if (!FVSFunctionMSText) return 0;

   int maxFields = FVSFunctionMSText->GetThePKeys()->entries();
   if (maxFields == 0)
   {
      FVSFunctionMSText = NULL;
      return 0;
   }
   theFieldTypes = new int[maxFields];
   theAnswerPtrs = new void* [maxFields];
   // set up the starting vertical point.

   int startPoint = 8;

   // Load the scroller with the fields of data.

   int fieldNumber = 0;
   int fieldType = 0;
   numberOfFields = 0;
   while (fieldType > -1)
   {
      void * theAnswer = BuildField(NULL, 0, theScrollingWindow,
                                    FVSFunctionMSText, &startPoint,
                                    ++fieldNumber, &fieldType);
      if (fieldType == -1) break;
      theFieldTypes[numberOfFields]  =fieldType;
      theAnswerPtrs[numberOfFields++]=theAnswer;  // make sure there is room for
                                                  // the next element
      theScrollingWindow->SetVirtualFrame( cScrollerFrameWidth, startPoint+500);
   }

   theScrollingWindow->SetVirtualFrame( cScrollerFrameWidth, startPoint+100);

   return 1;
}






void BuildExpression::ClearScrollingWindow( void )
{

   if (theFieldTypes)
   {
      delete []theFieldTypes;
      theFieldTypes = NULL;
   }
   if (theAnswerPtrs)
   {
      delete []theAnswerPtrs;
      theAnswerPtrs = NULL;
   }

   // go through the scroller and delete the the subviews.
   // it seems we need to iterate until the scrolling window is empty.

   CNavigator * aNav = GetNavigator();
   CTabStop   * aStop;
   while (!theScrollingWindow->GetSubviews()->isEmpty())
   {
      CSubview * aSubview;
      RWOrderedIterator subviewIterator (*theScrollingWindow->GetSubviews());

      while ((aSubview = (CSubview *) subviewIterator()) != NULL)
      {
         if ((aStop = aNav->FindTabStopHavingView(aSubview)))
         {
            aNav->RemoveTabStop(aStop);
            delete aStop;
         }
         RemoveSubview(aSubview);
         delete aSubview;
      }
   }
   theScrollingWindow->IScroller(FALSE, TRUE, TRUE, 24, 240);

}






int BuildExpression::OkToStore (PKeyData * variableOrFunction)
{

   CStringRWC testString = "always";

   if (expressionContentsPKeyPtr)
   {
      // when there is an expressionContents PKey, and the
      // variableOrFunction ATList is empty, then it is OK
      // to store if the expressionContents contains {always}.

      if (variableOrFunction->aTList.entries() == 0)
      {
         if (expressionContentsPKeyPtr->aTList.contains(&testString))
            return 1;
      }
      else
      {
         // there is an expressionContents PKey and the variableOrFunction
         // ATList is NOT empty.

         // if ANY one member of the expressionContents ATList
         // is also a member of the variableOrFunction ATList, then
         // it is OK to store.  Otherwise it is not.

         int store=0;
         RWOrderedIterator nextAT(expressionContentsPKeyPtr->aTList);
         CStringRWC * oneAT;
         while (oneAT = (CStringRWC *) nextAT())
         {
            if (variableOrFunction->aTList.contains(oneAT))
            {
               store = 1;
               break;
            }
         }
         return store;
      }
   }
   else
   {
      // there is no expressionContents PKey.
      // if the variableOrFunction ATList is empty, then it is
      // ok to store the variableOrFunction

      if (variableOrFunction->aTList.entries() == 0) return 1;
      else
      {
         // otherwise, the variableOrFunction ATList must contain {always}.

         if (variableOrFunction->aTList.contains(&testString))
            return 1;
      }
   }
   return 0;
}
