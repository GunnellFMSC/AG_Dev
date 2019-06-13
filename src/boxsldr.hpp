/********************************************************************

   File Name:     boxsldr.hpp
   Author:        kfd
   Date:          08/25/94

   Class:         BoxSliderObject
   Inheritance:   CSubview -> BoxSliderObject

   Purpose:       BoxSliderObjects are objects which contain a title,
                  (CText), a text box (NEditControl) and a scrollbar
                  (NScrollbar).
                  Minimum, maximum and default value for the text box and
                  initial minimum and maximum values for the scrollbar are
                  specified.

   Contents:
      BoxSliderObject( 2 constructors)
      IBoxSliderObject( 1 initializer)
      DoCommand
      SetCommand
      HScroll
      GetValue
      GetTitle
      DeactivateBoxSlider
      ActivateBoxSlider
      ValidateCharacters
      ComputeNumberOfDecimalsAndMultiplier
      WriteValueToControl
      SetSliderSettings
      GetBoxSliderSettings
      ResetBoxSliderSettings

      BoxSliderObject  -- constructor form 1 -- see below
      BoxSliderObject  -- constructor form 2 -- see below


      IBoxSliderObject -- initializer
         input (slider title,
                "default [min-value max-value min-slider
                          max-slider allowMinus allowDecimal]",
                optional title)

      DoCommand
      HScroll

      GetValue
         return float value of editcontrol

      GetTitle
         return CStringRW of editcontrol

      DeactivateBoxSlider
         no parameters

      ActivateBoxSlider
         no parameters

      ValidateCharacters
         input  NEditControl* itsString
         return int val=0

      ComputeNumberOfDecimalsAndMultiplier
         no parameters
         reset object member itsMultiplier, itsNumberOfDecimals

      WriteValueToControl
         input float value default or scroll position

      SetSliderSettings
         input float value of default or text in editcontrol

      GetBoxSliderSettings
         return CStringRW of slider settings (editcontrol, min value,
                                    max value, min slider, maxslider default)
      ResetBoxSliderSettings
         input const char* of box slider settings (see above)



   NOTE: A BoxSliderObject consists of a EditControl and a scroll bar
         scroller title and optional boxSlider title. Height of BoxSliders
         with two titles is 70; one title is 40.  Min width of CRect is 300.


   Parameters:    (CONSTRUCTOR FORM 1)
                  theEnclosure,
                  the containing rectangle of boxslider (min 300x44 or 300x70)
                  the slide bar title
                  value string ("default [min max minslider maxslider
                                 allowMinus allowDecimal]")
                  the box position (SPLEFT(default); SPRIGHT)
                  optional title   (default NULL)



********************************************************************/

#ifndef BoxSliderObject_H
#define BoxSliderObject_H

#include "appdef.hpp"
#include "float.h"
#include CSubview_i
#include CWindow_i
#include CText_i
#include NScrollBar_i
#include NLineText_i
#include <rw/cstring.h>


class CText;
class NScollBar;
class NEditControl;


class BoxSliderObject : public CSubview
{
   public:

      BoxSliderObject(CSubview* theEnclosure, const CRect& theRegion,
                     const char* sliderTitle,
                     const char* valueString,
                     BOOLEAN theBoxPosition = SPLEFT,
                     const char* optionalTitle = NULL);

      BoxSliderObject(CSubview* theEnclosure, CWindow * theParent,
                     const CRect& theRegion,
                     const char* sliderTitle,
                     const char* valueString,
                     BOOLEAN theBoxPosition = SPLEFT,
                     const char* optionalTitle = NULL);

      void IBoxSliderObject(const char* sliderTitle,
                            const char* valueString,
                            const char* optionalTitle = NULL);

      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual void SetCommand(long theCommand);
      virtual void HScroll(SCROLL_CONTROL theEventType, UNITS thePosition);
      float        GetValue(void);
      virtual const CStringRW GetTitle(void) const;
      void         DeactivateBoxSlider(void);
      void         ActivateBoxSlider(void);
      void         ComputeNumberOfDecimalsAndMultiplier(void);
      void         WriteValueToControl(float value);
      void         SetSliderSettings(float value);
      CStringRW    GetBoxSliderSettings(void) const;
      void         ResetBoxSliderSettings(const char* valueString);

   protected:
      NScrollBar* itsSlider;
      NEditControl*  itsData;
      float       itsDefault;
      float       itsMinValue;
      float       itsMaxValue;
      float       itsMinSlider;
      float       itsMaxSlider;
      double      itsMultiplier;	//Edited to remove a compiler warning (C4305)
      int         itsNumberOfDecimals;
      float       itsNegOffset;
      CText*      itsOptionalTitle;
      CText*      itsTitle;
      BOOLEAN     allowDecimal;
      BOOLEAN     allowMinus;
      BOOLEAN     blankOK;
      long        itsCommand;
      CWindow  *  itsParent;
};


////////////////////////////////////////////////////////////////////////
inline void BoxSliderObject::SetCommand(long theCommand)
{
  itsCommand = theCommand;
}
////////////////////////////////////////////////////////////////////////

#endif            //BoxSliderObject_H
