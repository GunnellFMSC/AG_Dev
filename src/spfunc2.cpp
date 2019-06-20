/*********************************************************************

   File Name:    spfunc2.cpp
   Author:       nlc, pg
   Date:         01/30/95 and 9/04/96 and 10/10/2006

   see spfunc2.hpp and below for notes.

 *********************************************************************/

#include "PwrDef.h"

#include CBoss_i
#include CMenuBar_i
#include CMenuItem_i
#include CStringRWC_i
#include CSubmenu_i
#include CTaskWin_i
#include CView_i
#include CWindow_i
#include CScroller_i

#include "suppdefs.hpp"
#include "spfunc2.hpp"
#include "appdoc.hpp"
#include "spglobal.hpp"

#include <string.h>
#include <stdio.h>

#include <rw/collstr.h>
#include <rw/ordcltn.h>
#include <rw/rstream.h>

int ConvertValueData(const char * theString, float * defaultValue,
                          float * minValue,  float * maxValue,
                          float * minSlider, float * maxSlider)

{
   // set the default return values.

   int defaultBlank = 0;

   if (defaultValue) *defaultValue = 0;
   if (minValue)     *minValue     = 0;
   if (maxValue)     *maxValue     = FLT_MAX;
   if (minSlider)    *minSlider    = 0;
   if (maxSlider)    *maxSlider    = 100.;

   if (!theString) return defaultBlank;

   int theStringLen = strlen(theString);

   // if theString is (char*) 0, then return.

   if (theStringLen == 0) return defaultBlank;

   // allocate memory for a copy and make it.

   char * theCopy = new char[theStringLen+1];

   strcpy(theCopy,theString);

   // use strtok to get the tokens.  When the tokens
   // are all used, or we have all the data we need.

   char * ptr = strtok(theCopy," ");
   if (ptr && defaultValue)
   {
      if (strcmp(ptr,"blank") == 0) defaultBlank = 1;
      else sscanf (ptr,"%f", defaultValue);
      ptr = strtok((char*) 0," ");
   }

   if (ptr && minValue)
   {
      if (strcmp(ptr,"FMin") == 0) *minValue = FLT_MIN;
      else sscanf (ptr,"%f", minValue);
      ptr = strtok((char*) 0," ");
   }

   if (ptr && maxValue)
   {
      if (strcmp(ptr,"FMax") == 0) *maxValue = FLT_MAX;
      else sscanf (ptr,"%f", maxValue);
      ptr = strtok((char*) 0," ");
   }

   if (ptr && minSlider)
   {
      sscanf (ptr,"%f", minSlider);
      ptr = strtok((char*) 0," ");
   }

   if (ptr && maxSlider)
   {
      sscanf (ptr,"%f", maxSlider);
      ptr = strtok((char*) 0," ");
   }

   if (defaultBlank && ptr && defaultValue)
   {
      sscanf (ptr,"%f", defaultValue);
      ptr = strtok((char*) 0," ");
   }

   delete []theCopy;

   return defaultBlank;
}


long ConvertToLong(const char * theString)
{
   // convert a string to its long integer representation

   long returnValue;
   sscanf(theString, "%ld", &returnValue);
   return returnValue;
}


float ConvertToFloat(const char * theString)
{
   // convert a string to its floating point representation

   if (strcmp(theString,"FMin") == 0) return FLT_MIN;
   if (strcmp(theString,"FMax") == 0) return FLT_MAX;

   float returnValue;
   sscanf(theString, "%f", &returnValue);
   return returnValue;
}


CStringRW ConvertToString(const int number)
{
   // convert an integer to its string representation.
   char buff[20];
   sprintf(buff, "%d", number);    // convert to string
   CStringRW out;
   out = buff;
   out = out.strip(RWCString::both);
   return out;
}



CStringRW ConvertToString(const long number)
{
   // convert an integer to its string representation.
   char buff[20];
   sprintf(buff, "%ld", number);    // convert to string
   CStringRW out;
   out = buff;
   out = out.strip(RWCString::both);
   return out;
}



CStringRW ConvertToString(const float number)
{
   // convert an integer to its string representation.

   if (number == FLT_MIN) return CStringRW("FMin");
   if (number == FLT_MAX) return CStringRW("FMax");

   char buff[20];
   sprintf(buff, "%9.4f", number);    // convert to string
   char *dec = strchr(buff,46);   // 46 = period
   if (dec)
   {
      for (char *end=buff+strlen(buff)-1; end>dec; end--) 
         if (*end==48) *end=0; // 48 = zero
         else break;
   }

   CStringRW out;
   out = buff;
   out = out.strip(RWCString::both);
   return out;
}



int numberLines(const char * theString)
{
   // return the number of lines in a string.
   if (!theString) return 0;
   int count = 1;
   while(*theString++) if (*theString == '\n') count++;
   return count;
}



void StripSingleNewLines(CStringRW * theString)
{
   size_t theLen = theString->length();
   char * tmpString = new char[(int) theLen+1];
   char * pts = tmpString;
   for (size_t i=0; i<theLen; i++)
   {
      if ((int) (*theString)(i) == '\n')
      {
         if ((i<theLen && (int) (*theString)(i+1) == '\n') ||
             (i>0      && (int) (*theString)(i-1) == '\n')) *(pts++) = '\n';
         else *(pts++) = ' ';
      }
      else *(pts++) = (*theString)(i);
   }
   *pts = NULL;
   *theString = tmpString;
   delete []tmpString;
}



void StripTrailingZeros(char * theString)
{
   char *dec = strchr(theString,'.');
   if (dec > theString)
   {
      for (char *last=dec+strlen(dec)-1; last>dec; last--)
      {
         if (*last == '0') *last=0;
         else break;
      }
   }
}


static BOOLEAN g_CutCopyPasteDelete_CXO_textedit_active = FALSE;
long XVT_CALLCONV1 CutCopyPasteDelete_CXO(XVT_CXO cxo, EVENT *ep)
{
   switch(ep->type)
   {
   case E_CONTROL:
      switch(ep->v.ctl.ci.type)
      {
      case WC_EDIT:
         {
            CMenuBar *aMenuBar = ((CWindow *)((CView *)xvt_vobj_get_data(ep->v.ctl.ci.win))->GetCWindow())->GetMenuBar();
            if (ep->v.ctl.ci.v.edit.focus_change && ep->v.ctl.ci.v.edit.active && aMenuBar)
            {
               aMenuBar->SetEnabled(M_EDIT_CUT, TRUE);
               aMenuBar->SetEnabled(M_EDIT_COPY, TRUE);
               aMenuBar->SetEnabled(M_EDIT_PASTE, TRUE);
               aMenuBar->SetEnabled(M_EDIT_CLEAR, TRUE);
            }
            else if (ep->v.ctl.ci.v.edit.focus_change && !ep->v.ctl.ci.v.edit.active &&
               !g_CutCopyPasteDelete_CXO_textedit_active && aMenuBar)
            {
               aMenuBar->SetEnabled(M_EDIT_CUT, FALSE);
               aMenuBar->SetEnabled(M_EDIT_COPY, FALSE);
               aMenuBar->SetEnabled(M_EDIT_PASTE, FALSE);
               aMenuBar->SetEnabled(M_EDIT_CLEAR, FALSE);
            }
            g_CutCopyPasteDelete_CXO_textedit_active = FALSE;
         }
         break;

      case WC_TEXTEDIT:
         {
            if (ep->v.ctl.ci.v.textedit.focus_change && ep->v.ctl.ci.v.edit.active)
            {
               g_CutCopyPasteDelete_CXO_textedit_active = TRUE;
            }
            else if (ep->v.ctl.ci.v.textedit.focus_change && !ep->v.ctl.ci.v.edit.active)
            {
               g_CutCopyPasteDelete_CXO_textedit_active = FALSE;
            }
         }
         break;

      default:
         break;
      }
      break;

   default:
      break;
   }

   return xvt_cxo_call_next(cxo, ep);
}

CRect AdjWinPlacement(CRect theRect)
{
   // Used to adjust the placement of existing windows while retaining original size

   UNITS aWidth = (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_DOCFRAME_WIDTH);
   UNITS aHeight = (UNITS)xvt_vobj_get_attr(NULL_WIN, ATTR_DOC_STAGGER_VERT);
   CRect aNewRect(aWidth, aHeight, theRect.Right()-theRect.Left()+(aWidth * 3), theRect.Bottom()-theRect.Top()+(aHeight + aWidth * 2));

   return aNewRect;
}

CRect CenterWinPlacement(CRect theRect)
{
   // Used to center the placement of existing windows while retaining original size

   CRect aRec=AdjWinPlacement(theRect);

   UNITS aWidth = CBoss::G->GetTaskWin()->GetFrame().Width();
   UNITS aHeight = CBoss::G->GetTaskWin()->GetFrame().Height();

   return CRect(0.0, 0.0, aRec.Width(), aRec.Height()) +
      CPoint((aWidth/2)-(aRec.Width()/2), (aHeight/2)-(aRec.Height()/2));
}

CRect CenterTopWinPlacement(CRect theRect)
{
   // Used to place a window ot the right side of the task window, retaining the original size.

   CRect aRec=AdjWinPlacement(theRect);

   UNITS move = (CBoss::G->GetTaskWin()->GetFrame().Width() -
                aRec.Width())/2;
   if (move > 0) return CRect(aRec.Left()+move,  aRec.Top(),
                              aRec.Right()+move, aRec.Bottom());
   else          return CRect(aRec);
}

CRect RightWinPlacement(CRect theRect)
{
   // Used to place a window ot the right side of the task window, retaining the original size.

   CRect aRec=AdjWinPlacement(theRect);

   UNITS move = CBoss::G->GetTaskWin()->GetFrame().Width() -
                aRec.Width();
   if (move > 0) return CRect(aRec.Left()+move,  aRec.Top(),
                              aRec.Right()+move, aRec.Bottom());
   else          return CRect(aRec);
}

void AdjustScrollingWinSize(CScroller *theScroller, CWindow *theWindow)
{
   if (!theScroller || !theWindow) return;

   CRect taskFrame  = CBoss::G->GetTaskWin()->GetFrame();
   UNITS taskHeight = taskFrame.Bottom()-taskFrame.Top();

   CRect theWinFrame= theWindow->GetFrame();
   RCT newrct = RCT(theWinFrame);
   xvt_vobj_translate_points (theWindow->GetXVTWindow(), TASK_WIN,
                              (PNT *) &newrct, 2);
   theWinFrame = newrct;

   CRect theScrollerVFrame= theScroller->GetVirtualFrame();
   CRect theScrollerFrame= theScroller->GetFrame();
   UNITS scrollVHt  = theScrollerVFrame.Bottom()-theScrollerVFrame.Top();
   UNITS scrollHt   = theScrollerFrame.Bottom()-theScrollerFrame.Top();

   UNITS toAdd = scrollVHt > scrollHt ? scrollVHt - scrollHt : 0;
   UNITS maxToAdd = taskFrame.Bottom()-theWinFrame.Bottom()-2;
   toAdd = toAdd > maxToAdd ? maxToAdd : toAdd;
   if   (toAdd > 0)
   {
      theWinFrame.Bottom(theWinFrame.Bottom()+toAdd);
      theWindow->Size(theWinFrame);
   }
}

static char *SampleWebNamesStrings[] =
{
  "Download the FVS Self-Tutorials",
  "Forest Vegetation Simulator Web Site",
  "USDA Forest Service Web Site",
  "First Gov: US Government Web Site",
  "",
};

static char *SampleWebLinkStrings[] =
{
  "http://www.fs.fed.us/fmsc/fvs/training/tutorials.shtml",
  "http://www.fs.fed.us/fmsc/fvs/index.shtml",
  "http://www.fs.fed.us",
  "http://www.firstgov.gov/",
  "",
};

void AddDynamicHelpItems(CMenuBar* theMenuBar)
{
  // Locate help submenu
  CSubmenu* aHelpSubmenu = theMenuBar->FindSubmenu(M_HELP);

  // Create separator...
  CMenuItem aMenuItemSeparator("", M_HELP_DYNAMIC_ITEMS_SEPARATOR, 0, CMenu::SEPARATOR);
  aHelpSubmenu->Append(aMenuItemSeparator);

  // Have we previously saved the web links...
  bool saveWebLinks = ((CsupposeDoc*)theSpGlobals->theAppDocument)->itsDynamicHelpWebLinks.entries() == 0;

  // For this sample, the menu title is the same as the web link but it can be any string
  // M_HELP_DYNAMIC_ITEMS_BASE can be incremented up to and including M_HELP_DYNAMIC_ITEMS_MAX
  for (int i = 0; (strlen(SampleWebNamesStrings[i]) != 0) && (M_HELP_DYNAMIC_ITEMS_BASE + i <= M_HELP_DYNAMIC_ITEMS_MAX); i++)
  {
    CMenuItem aMenuItem(SampleWebNamesStrings[i], M_HELP_DYNAMIC_ITEMS_BASE + i, 0, CMenu::ENABLED);
    aHelpSubmenu->Append(aMenuItem);

    if (saveWebLinks)
      ((CsupposeDoc*)theSpGlobals->theAppDocument)->itsDynamicHelpWebLinks.insert(new RWCollectableString(SampleWebLinkStrings[i]));
  }

  theMenuBar->DoUpdate();
}


