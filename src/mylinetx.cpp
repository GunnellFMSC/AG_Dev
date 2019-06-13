/********************************************************************

   File Name:     mylinetx.cpp   Modified NLineText.
   Author:        nlc
   Date:          04/03/96


********************************************************************/

#include "PwrDef.h"
#include "mylinetx.hpp"

MyNLineText::MyNLineText(CSubview * theEnclosure,
                         const CPoint& theTopLeftPoint,
                         UNITS theLength,
                         int theCommand,
                         int theChangeCommand,
                         int theCharacterLimit,
                         int theSpaceOK)
              :NLineText(theEnclosure,
                         theTopLeftPoint,
                         theLength,
                         TX_BORDER | TX_AUTOHSCROLL,
                         theLength,
                         theCharacterLimit),
               itsEnclosure(theEnclosure),
               enclosureDoCommand(theCommand),
               enclosureChangeCommand(theChangeCommand),
               itsSpaceOK(theSpaceOK)
{}

XVT_WCHAR MyNLineText::Validate(const CKey & theCKey)
{
   // look for the NewLine (CR).

   if (enclosureDoCommand && theCKey.GetChar() == 13)
   {
      itsEnclosure->DoCommand(enclosureDoCommand, (void *) this);
   }
   else if (theCKey.IsVirtualKey() || // can't use isspace on virtual keys (arrows, PgUp, ...)
            !isspace(theCKey.GetChar()) ||
            (itsSpaceOK && theCKey.GetChar() == 32))
   {
      if (enclosureChangeCommand)
         itsEnclosure->DoCommand(enclosureChangeCommand,
                                 (void *) this);
      return NLineText::Validate(theCKey.GetChar());
   }
   return NULL;
}



void MyNLineText::Activate( void )
{
   // The version of XVT's CNativeTextEdit Activate code that I got does
   // not include the xvt_scr_set_focus_vobj( GetXVTTextEdit( )); line.
   // I'm doing it this way for now:

   CNativeTextEdit::Activate();
   xvt_scr_set_focus_vobj( GetXVTTextEdit( ));
}






