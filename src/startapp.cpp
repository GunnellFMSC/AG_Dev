/*********************************************************************

   File Name:     startapp.cpp
   Author:        kfd
   Date:          11/29/94

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include CEnvironment_i
#include CTaskWin_i
#include "suppdefs.hpp"
#include "startapp.hpp"
#include "appdoc.hpp"
#include "spfunc2.hpp"


// Define the Run-Time Type Identification for this class
PWRRegisterClass1(CsupposeApp, CsupposeAppID, "CsupposeApp", CApplication)


CsupposeApp::CsupposeApp(void) : CApplication()
{
   // Set any System Attribute Table values that
   // need to be set before GUI initialization

   // Suppose has NONE of these...
}


void CsupposeApp::StartUp(void)
{
   // Define the startup behavior for the application - whether an
   // initial about box is created, or an initial document object
   // and its associated windows are created.
   //
   // NOTE: The base class method must be called before adding any
   // non-XVT code.

   CApplication::StartUp();

   // Install CXO to aid in enabling/disabling standard edit menu items
   xvt_cxo_create(SCREEN_WIN, 0L, XVT_CXO_POS_FIRST, &CutCopyPasteDelete_CXO, EM_ALL, "CCPD Class", 1L);

   // Set the base environment ... use unique fonts for AIX.

#ifdef AIXV4

   DoSetEnvironment(CEnvironment(
     COLOR_WHITE,   // COLOR        theBackground
     COLOR_BLACK,   // COLOR        theForeground
     COLOR_WHITE,   // COLOR        theBrushColor
     PAT_SOLID,     // PAT_STYLE    theBrushPattern
     COLOR_BLACK,   // COLOR        thePenColor
     PAT_SOLID,     // PAT_STYLE    thePenPattern
     1,             // short        thePenWidth
     CFont(         // const CFont& theFont
           XVT_FFN_HELVETICA,   // const CStringRW &   theFamily
           XVT_FS_BOLD,         // XVT_FONT_STYLE_MASK theStyle
           14),                 // long                theSize
     M_COPY,        // DRAW_MODE    theDrawningMode
     P_SOLID,       // PEN_STYLE    thePenStyle
     FALSE,         // BOOLEAN      theTextIsOpaque
     COLOR_WHITE,   // COLOR        theMonoBackground
     COLOR_BLACK,   // COLOR        theMonoForeground
     COLOR_WHITE,   // COLOR        theMonoBrushColor
     COLOR_BLACK    // COLOR        theMonoPenColor
     ));            // ***** End of Environment *****

#else

   // if the font the user has set up is too big for Suppose,
   // set the font to one that works.

   int leadingp, ascentp, descent;
   xvt_dwin_get_font_metrics(G->GetTaskWin()->GetXVTWindow(),
                             &leadingp, &ascentp, &descent);
   if (leadingp+ascentp+descent > 19)
   {
      CEnvironment theEnv = *GetEnvironment();
      CFont myFont = theEnv.GetFont();
      CStringRW fam  = myFont.GetFamily();
      myFont.SetSize(9);
      myFont.SetFamily(XVT_FFN_HELVETICA);
      myFont.SetStyle(XVT_FS_NONE);
      theEnv.SetFont(myFont);
      DoSetEnvironment(theEnv);
   }

#endif


  // Create an instance of CsupposeDoc and display its initial
  // window(s) as defined by CsupposeDoc::BuildWindow

  DoNew();
}





void CsupposeApp::DoCommand(long theCommand,void* theData)
{
  // Handle commands here that have to do with behavior specific
  // to the application, or that create, destroy or otherwise
  // manage the data (document) objects of the application.

  switch (theCommand)
  {
    default: CApplication::DoCommand(theCommand,theData);
             break;
  }
}






BOOLEAN CsupposeApp::DoNew(void)
{
          // This method is automatically called when the New option
          // on the default File menu is selected.  Create a new
          // document object and let it do its initialization.

      CsupposeDoc* aDoc = new CsupposeDoc(this, GetNumDocuments()+1);
      PwrAssert(aDoc, 311, "Creation of CsupposeDoc failed");

      BOOLEAN fromDoNew = aDoc->DoNew();

      // Load from the simulation file command line, if it is present.

      if (fromDoNew) aDoc->LoadSimFileUsingCommandLine();

      return fromDoNew;
}






BOOLEAN CsupposeApp::DoOpen(void)
{
   // With Suppose, this function is not used.

   return FALSE;
}

