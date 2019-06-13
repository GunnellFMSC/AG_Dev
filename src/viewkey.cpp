/*********************************************************************

   File Name:    viewkey.cpp
   Author:       nlc
   Date:         01/27/95

   see viewkey.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include "compon.hpp"
#include NButton_i
#include NScrollText_i
#include NScrollBar_i
#include CStringRW_i
#include CApplication_i
#include CDesktop_i
#include CNavigator_i

#include "spglobal.hpp"
#include "suppdefs.hpp" 
#include "warn_dlg.hpp"

#include "viewkey.hpp"
#include "spfunc2.hpp"

#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

/////////////////////////////////////////////////////////////////////////
void PWR_CALLCONV1 hookTextEditCallback_ViewKeywords(
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
      ViewKeywords *aViewKeywords = (ViewKeywords *)aNScrollText->GetCWindow();
      aViewKeywords->ScrollColumnRuler(theOrgOffset);
   }
}
/////////////////////////////////////////////////////////////////////////

ViewKeywords::ViewKeywords(const char * theKeywords,
                           const CStringRW & theTitle)

           :CWwindow((CDocument *) theSpGlobals->theAppDocument,
                     CRect(20,50,850,400),
                     theTitle,
                     WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE, 
                     W_DOC, TASK_MENUBAR)
{
   // create and clear the navigator

   CNavigator * aNav = GetNavigator();
   aNav->ClearTabStops();

   CStringRW column = "....+....1....+....2....+....3....+....4"
                      "....+....5....+....6....+....7....+....8"
                      "....+....9....+....0....+....1....+....2"
                      "....+....3....+....4....+....5....+....6";

   itsColumnRuler = new NScrollText(this, CRect(8,8,16,40), FALSE, FALSE,
                                    TX_BORDER | TX_NOMENU | TX_READONLY);
#ifdef AIXV4
   itsColumnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
#else
   itsColumnRuler->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
#endif
   itsColumnRuler->SetText(column);

   // margin for all characters in columnRuler
   int maxmargin = xvt_dwin_get_text_width(itsColumnRuler->GetCWindow()->GetXVTWindow(),
                                           (char *) column.data(), -1);
   // margin for first 80 characters in columnRuler
   int margin = xvt_dwin_get_text_width(itsColumnRuler->GetCWindow()->GetXVTWindow(),
                                        (char *) CStringRW(column(0, 80)).data(), -1);

   int leadingp;
   int ascentp;
   int descent;

   xvt_dwin_get_font_metrics(itsColumnRuler->GetCWindow()->GetXVTWindow(),
               &leadingp, &ascentp, &descent);

   int top = 8;
   int width = margin+16;
   int bottom = 16+leadingp+ascentp+descent;
   itsColumnRuler->Size(CRect(8,top,width,bottom));
   itsColumnRuler->Disable();

   top = bottom+4;                            // allow for 5 - 15 lines.

   int nlines = numberLines(theKeywords);
   nlines = nlines > 15 ? 15 : nlines;
   nlines = nlines <  5 ?  5 : nlines;

   bottom  = 12+top+(nlines*(leadingp+ascentp+descent));

   width = width+NScrollBar::NativeWidth()+2;
   NScrollText * textDisplay = new NScrollText(this, CRect(8,top,width,bottom),
                               TRUE, TRUE, TX_BORDER| TX_AUTOHSCROLL | TX_WRAP | TX_ENABLECLEAR,
                               (UNITS) maxmargin, 10000, TRUE);

   xvt_tx_set_scroll_callback(textDisplay->GetXVTTextEdit(), hookTextEditCallback_ViewKeywords);


#ifdef AIXV4
   textDisplay->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_BOLD,14),FALSE);
#else
   textDisplay->SetFont(CFont(XVT_FFN_COURIER,XVT_FS_NONE,10),FALSE);
#endif
   textDisplay->SetText(column);

   if (theKeywords && strlen(theKeywords))
      textDisplay->SetText(theKeywords);
   else
      textDisplay->SetText("\nNo keywords are defined!");

   width += 8;
   top = bottom+8;
   bottom = top+32;
   int left = (width-96)/2;
   NButton * closeButton =
      new NButton(this, CRect(left,top,left+96,bottom), "Close", 0L);
   closeButton->SetCommand(1);

   RCT newrct = RCT (this->GetClippedFrame());
   newrct.bottom=newrct.top+bottom+8;
   newrct.right =newrct.left+width;

   xvt_vobj_translate_points (GetXVTWindow(), TASK_WIN,
                             (PNT *) &newrct, 2);
   Size(CRect(newrct));

   itsColumnRuler->SetGlue(RIGHTSTICKY  | LEFTSTICKY);
   textDisplay->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                        RIGHTSTICKY | LEFTSTICKY);
   closeButton->SetGlue(BOTTOMSTICKY);

   aNav->AppendTabStop(new CTabStop(closeButton));

   Show();
   aNav->InitFocus();

#if XVTWS == WINWS

   // needed on windows, for sure.

   G->GetDesktop()->SetFrontWindow(this);

#endif

   xvt_help_set_win_assoc(helpInfo, GetXVTWindow(), VIEWKEYWindow, 0L);
}




void ViewKeywords::DoCommand(long theCommand,void* theData)
{


   switch (theCommand)
   {
      case 1:                         // Close button pushed

         Close();
         break;


      default:
         CWwindow::DoCommand(theCommand,theData);

   }        // end switch(theCommand)
}           // end function

