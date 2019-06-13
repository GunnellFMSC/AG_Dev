/********************************************************************

   File Name:     mylinetx.hpp   Modified NLineText.
   Author:        nlc
   Date:          04/03/96


   Contents:      See below.  Entire class is defined herein.

********************************************************************/

#ifndef MyNLineText_H
#define MyNLineText_H

#include NLineText_i
#include CKey_i
#include CSubview_i

class MyNLineText : public NLineText
{
   public:

      MyNLineText(CSubview * theEnclosure,
                  const CPoint& theTopLeftPoint,
                  UNITS theLength = 100,
                  int theCommand = 0,
                  int theChangeCommand = 0,
                  int theCharacterLimit = 20,
                  int theSpaceOK = 0);

      virtual XVT_WCHAR MyNLineText::Validate(const CKey & theCKey);
      virtual void Activate( void );

   private:

      CSubview * itsEnclosure;
      long enclosureDoCommand;
      long enclosureChangeCommand;
      long itsSpaceOK;

};

#endif
