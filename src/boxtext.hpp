/********************************************************************

   File Name:     boxtext.hpp
   Author:        kfd
   Date:          12/19/94

   Class:         BoxTextObject
   Inheritance:   CSubview -> BoxTextObject

   Purpose:       BoxTextObjects are objects which contain a title,
                  and a text box (NLineText)
                  Minimum, maximum and default value for the text box

   Contents:      BoxTextObject()
                  BoxTextObject::DoCommand
                  BoxTextObject::HScroll
                  BoxTextObject::DeactivateBoxText
                  BoxTextObject::ActivateBoxText
                  BoxTextObject::GetValue
                  BoxTextObject::GetTitle
                  GetBoxTextSettings
                  ResetBoxTextSettings


********************************************************************/

#ifndef BoxTextObject_H
#define BoxTextObject_H

#include "appdef.hpp"
#include "float.h"
#include CSubview_i
#include NScrollBar_i
#include NLineText_i

class NScollBar;
class NEditControl;


class BoxTextObject : public CSubview
{
   public:
      BoxTextObject(CSubview* theEnclosure, const CRect& theRegion,
                     const char* boxTextTitle,
                     const char* defaultString,
                     BOOLEAN theBoxPosition = SPRIGHT);



      virtual void DoCommand(long theCommand,void* theData);
      virtual void DeactivateBoxText(void);
      virtual void ActivateBoxText(void);
      virtual float GetValue(void);
      virtual const CStringRW GetTitle(void) const;
      CStringRW GetBoxTextSettings(void)  const;
      void ResetBoxTextSettings(const char* valueString);
      int ValidateCharacters(NEditControl* itsEntry);

   protected:
      NEditControl*  itsData;
      float       itsDefault;
      float       itsMinValue;
      float       itsMaxValue;
      const char* itsString;
      BOOLEAN     allowDecimal;
      BOOLEAN     allowMinus;
      BOOLEAN     blankOK;
};

#endif            //BoxTextObject_H
