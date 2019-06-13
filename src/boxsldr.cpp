/********************************************************************


   File Name:     boxsldr.cpp
   Author:        kfd & nlc
   Date:          08/25/94

   Contents:      see header

   NOTE: A BoxSliderObject consists of a EditControl and a scroll bar
         scroller title and optional boxSlider title. Height of BoxSliders
         with two titles is 76; one title is 50.  Min width of CRect is 300.

********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include CText_i
#include NScrollBar_i
#include NEditControl_i
#include <rw/cstring.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "boxsldr.hpp"
#include "cmngwin.hpp"   // for checkrange
#include "msgwin.hpp"    // for testing
#include "spfuncs.hpp"
#include "spfunc2.hpp"


//             constructor form 1


BoxSliderObject::BoxSliderObject(CSubview * theEnclosure,
                                 const CRect& theRegion,
                                 const char* sliderTitle,
                                 const char* valueString,
                                 BOOLEAN theBoxPosition,
                                 const char* optionalTitle)

                : CSubview(theEnclosure,  theRegion)
{

   itsCommand   = NULL;
   allowDecimal = TRUE;
   allowMinus   = FALSE;
   blankOK      = FALSE;

   // find the right edge of the window; use this
   // to position scroll bar, edit control

   UNITS rightEdge = (int)theRegion.Right()-(int)theRegion.Left();

   // vertical offset; changes if optional title included

   UNITS vOffset = 2;

   if (optionalTitle)
   {
      itsOptionalTitle = new CText(this, CPoint(0,vOffset),
                                   CStringRW(optionalTitle));
      vOffset = 26;
   }

   // place controls when left box position specified

   if (theBoxPosition == SPLEFT)
   {
      // place the title

      itsTitle = new CText(this, CPoint(100,vOffset),
                           CStringRW(sliderTitle));

      // place the scrollbar

      itsSlider = new NScrollBar(this,
                                 CRect(100,28+vOffset,
                                       rightEdge,46+vOffset),
                                 HORIZONTAL);

      // place the text box

      itsData = new NEditControl(this, CRect(0,12+vOffset,88,46+vOffset),
                                 "", CTL_FLAG_RIGHT_JUST);
   }


   // place controls when right box position specified

   else
   {

      // place the title

      itsTitle = new CText(this, CPoint(0,vOffset),
                           CStringRW(sliderTitle));

      // place the scrollbar

      itsSlider = new NScrollBar(this,
                                 CRect(0,28+vOffset,
                                       rightEdge - 100,46+vOffset),
                                 HORIZONTAL);

      // place the text box

      itsData = new NEditControl(this,
                                 CRect(rightEdge - 88,12+vOffset,
                                       rightEdge,46+vOffset),
                                 "", CTL_FLAG_RIGHT_JUST);
   }

   itsData->SetFocusCommands(0,1);
   itsData->SetTextCommand(2);

   // initialize the object with titles and values

   IBoxSliderObject(sliderTitle,
                    valueString,
                    optionalTitle);

}


//             constructor form 2

BoxSliderObject::BoxSliderObject(CSubview * theEnclosure,
                                 CWindow  * theParent,
                                 const CRect& theRegion,
                                 const char* sliderTitle,
                                 const char* valueString,
                                 BOOLEAN theBoxPosition,
                                 const char* optionalTitle)

               : CSubview(theEnclosure,  theRegion),
                 itsParent(theParent)
{

   itsCommand = NULL;
   allowDecimal = TRUE;
   allowMinus   = FALSE;
   blankOK      = FALSE;

   // find the right edge of the window; use this
   // to position scroll bar, edit control

   int rightEdge = (int)theRegion.Right()-(int)theRegion.Left();

   // vertical offset; changes if optional title included

   int vOffset = 2;

   if (optionalTitle)
   {
      itsOptionalTitle = new CText(this, CPoint(0,vOffset),
                                   CStringRW(optionalTitle));
      vOffset = 32;
   }

   // place controls when left box position specified

   if (theBoxPosition == SPLEFT)
   {

      // place the title

      itsTitle = new CText(this, CPoint(100,vOffset),
                           CStringRW(sliderTitle));

      // place the scrollbar
      itsSlider = new NScrollBar(this,
                                 CRect(100,28+vOffset,
                                       rightEdge,46+vOffset),
                                 HORIZONTAL);

      // place the text box

      itsData = new NEditControl(this,
                                 CRect(0,12+vOffset,88,46+vOffset),
                                 "", CTL_FLAG_RIGHT_JUST);
   }


   // place controls when right box position specified

   else
   {
      // place the title

      itsTitle = new CText(this, CPoint(0,vOffset),
                           CStringRW(sliderTitle));

      // place the scrollbar

      itsSlider = new NScrollBar(this,
                                 CRect(0,28+vOffset,
                                       rightEdge - 100,46+vOffset),
                                 HORIZONTAL);

      // place the text box

      itsData = new NEditControl(this,
                                 CRect(rightEdge - 88,12+vOffset,
                                       rightEdge,46+vOffset),
                                 "", CTL_FLAG_RIGHT_JUST);
   }


   itsData->SetFocusCommands(0,1);
   itsData->SetTextCommand(2);

   // initialize the object with titles and values

   IBoxSliderObject(sliderTitle,
                    valueString,
                    optionalTitle);

}



void BoxSliderObject::IBoxSliderObject(const char * sliderTitle,
                                       const char * valueString,
                                       const char * optionalTitle)
{
   if (sliderTitle) itsTitle->SetTitle(CStringRW(sliderTitle));

   if (optionalTitle)
      itsOptionalTitle->SetTitle(CStringRW(optionalTitle));

   if (valueString)
   {
      // set up default values
      itsDefault   = 0;
      itsMinValue  = 0;
      itsMaxValue  = 100;
      itsMinSlider  = 0;
      itsMaxSlider  = 100;
      allowMinus   = FALSE;
      allowDecimal = TRUE;

      // convert string to default and values, nominus, nodecimal

      CStringRW aTmpData;
      int theStringLen = strlen(valueString);
      char * theCopy   = new char[theStringLen+1];
      char * newString = new char[theStringLen+1];

      aTmpData="NoB";

      strcpy(theCopy,valueString);

      // use strtok to get the tokens.

      char * ptr = strtok(theCopy," ");
      int cnt = 0;
      while(ptr)
      {

         // parse default data or word blank

         if (cnt == 0)
         {
            if (strcmp(ptr, "blank") == 0)
            {
               strcpy(newString,"0");
               aTmpData=NULLString;
               blankOK = TRUE;
            }
            else
            {
               strcpy(newString,ptr);
               float val;
               sscanf(newString,"%f", &val);
               if (val > 0)
               {
                  itsMinValue = itsMinSlider =  .5 * val;
                  itsMaxValue = itsMaxSlider = 1.5 * val;
               }
               if (val < 0)
               {
                  itsMinValue = itsMinSlider = 1.5 * val;
                  itsMaxValue = itsMaxSlider =  .5 * val;
               }
            }
         }

         // parse min and max values and slider ranges

         if (cnt > 0 && cnt < 5)
         {
            strcat(newString," ");
            strcat(newString,ptr);
         }

         // parse allowMinus

         if (cnt == 5)
         if (strcmp(ptr,"TRUE") == 0 || strcmp(ptr,"true") == 0 ||
             strcmp(ptr,"1")    == 0)  allowMinus = 1;

         // parse allowDecimal

         if (cnt == 6)
            if (strcmp(ptr,"FALSE") == 0 || strcmp(ptr,"false") == 0 ||
                strcmp(ptr,"0")     == 0)  allowDecimal = 0;

         // next word in string

         ptr = strtok((char*) 0," ");
         cnt++;
      }

      // convert string to boxslider default settings

      ConvertValueData(newString, &itsDefault,
                                  &itsMinValue, &itsMaxValue,
                                  &itsMinSlider,&itsMaxSlider);

      if (itsMinValue < 0 || itsMinSlider < 0) allowMinus = 1;


      delete [] theCopy;
      delete [] newString;


      // set the number of decimals based on the range between min
      // and max slider range values; use fabs in case negative numbers

      if (!allowMinus && itsMinSlider < 0) itsMinSlider = 0;

      ComputeNumberOfDecimalsAndMultiplier();

      if (aTmpData == NULLString)
      {
         itsData->SetTitle(aTmpData);
         itsDefault = itsMinSlider;
      }

      else WriteValueToControl(itsDefault);

      // itsNegOffset used to shift the scroll values when the
      // minimum is less than 0

      itsNegOffset = itsMinSlider < 0 ?  fabs(itsMinSlider) : 0;

      SetSliderSettings(itsDefault);
   }
}




void BoxSliderObject::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 0:           // itsData got focus
         break;

      case 1:           // itsData lost focus
      {
        // see if number is within acceptable min, max

        CStringRW curTitle = itsData->GetTitle();
        if (!blankOK && curTitle.length() < 1)
        {
            xvt_scr_beep();
            itsData->SetTitle(ConvertToString(itsDefault));
        }
        else
        {
            float value;
            sscanf(curTitle, "%f", &value);

            if (value < itsMinValue)
            {
               xvt_scr_beep();
               itsData->SetTitle(ConvertToString(itsMinValue));
               value=itsMinValue;
            }
            if (value > itsMaxValue)
            {
               xvt_scr_beep();
               itsData->SetTitle(ConvertToString(itsMaxValue));
               value=itsMaxValue;
            }

            // if new value is outside old slider range, shift
            // the slider range
            // position the slider using entered number; change
            // slider slider range if number exceeds itsMaxSlider

            if ((value * itsMultiplier)+itsNegOffset >
               (itsMaxSlider * itsMultiplier) + itsNegOffset  ||
               (value * itsMultiplier)+itsNegOffset <=
               (itsMinSlider * itsMultiplier) + itsNegOffset)
            {
               // use these factors to calculate the new slider range

               float aMinFactor = value >= 0 ?  .5 : 1.5;
               float aMaxFactor = value >= 0 ? 1.5 :  .5;

               itsMaxSlider = value * aMaxFactor;
               itsMinSlider = value * aMinFactor;

               // reset itsNegOffset based on new slider range
               // itsNegOffset = min(0, minSlider) OR minSlider

               itsNegOffset = value >= 0 ?
                        fabs(min(0,itsMinSlider)):
                        fabs(itsMinSlider);

               itsMaxSlider = value * aMaxFactor;
               itsMinSlider = value * aMinFactor;

               // set slider ranges for integers

               if (!allowDecimal)
               {
                  itsMinSlider = floor (value * aMinFactor);
                  if (value == 1)
                  {
                     itsMinSlider = 0;
                     itsMaxSlider = 2;
                  }
                           // set ranges for negative integers
                  if (value < 0)
                  {
                     itsMinSlider = (value * aMinFactor) -1;
                     itsMaxSlider = (value * aMaxFactor) -1;
                     if (value == -1)
                     {
                        itsMinSlider = -3;
                        itsMaxSlider = 0;
                     }
                  }
               }
            }

            // reset number of decimals and multiplier

            ComputeNumberOfDecimalsAndMultiplier();

            // ZERO is a special case

            if (value == 0)
            {
               if (allowMinus)
               {
                  itsMinSlider = -.5;
                  itsMaxSlider = .5;
                  itsNegOffset = .5;
               }
               else
               {
                  itsMinSlider = 0.;
                  itsMaxSlider = 1.;
                  itsNegOffset = 0.;
               }
               itsNumberOfDecimals = 4;
               itsMultiplier = 1000;
            }

            // ZERO, no decimal is a special case

            if (value == 0 && !allowDecimal)
            {
               itsMinSlider = 0; itsMaxSlider = 1;
               itsNegOffset = 0;
               itsNumberOfDecimals = 0;
               itsMultiplier = 1;
            }
            // reset the slider bar
            SetSliderSettings(value);

            // do parents command if necessary if
            // parent did a SetCommand() on this

           if (itsCommand) itsParent->DoCommand(itsCommand);
         }
         break;
      }

      case 2:           // user entered something in itsData
      {
         CStringRW curTitle = itsData->GetTitle();
         for (int i = 0; i<curTitle.length(); i++)
         {
            int theChar = curTitle[i];

            // remove all except numbers (48-57)
            // decimal (46) & minus (45)

            if( (theChar < 47 || theChar > 57) || theChar == 47)
            {
               if (allowDecimal && theChar == 46)  ;     // noop  (;)
               else
                  if (allowMinus && theChar == 45) ;     // noop  (;)
                  else
                  {
                     xvt_scr_beep();
                     curTitle=curTitle.remove(i,1);
                     itsData->SetTitle(curTitle);
                  }
            }
         }
         break;
      }
      default: CSubview::DoCommand(theCommand,theData); break;
   }
}




void BoxSliderObject::HScroll(SCROLL_CONTROL theEventType, UNITS thePosition)
{
  // get the thumb position on the scroll bar

  float pos = (itsSlider->GetPosition()-itsNegOffset*itsMultiplier) /
    (float)itsMultiplier;

  // write position to aBuf

  WriteValueToControl(pos);
  if (itsCommand) itsParent->DoCommand(itsCommand);

}



float BoxSliderObject::GetValue(void)
{

  // return the value which is entered in the textbox

  float value;

  sscanf(itsData->GetTitle(),"%f", &value);
  return  value;
}



const CStringRW BoxSliderObject::GetTitle(void) const
{
   // return the sting which is entered in the textbox

   return itsData->GetTitle();
}




void BoxSliderObject::DeactivateBoxSlider(void)
{
  // change the color, disable textbox and slider

   CEnvironment tempEnv;

   tempEnv.SetColor(COLOR_WHITE,COLOR_LTGRAY, COLOR_WHITE, COLOR_LTGRAY);
   itsData->SetEnvironment(tempEnv);
   itsData->Disable();
   itsSlider->Disable();
}




void BoxSliderObject::ActivateBoxSlider(void)
{
   // change the color, enable textbox and slider

   CEnvironment tempEnv;

   tempEnv.SetColor(COLOR_WHITE,COLOR_BLACK, COLOR_WHITE, COLOR_BLACK);
   itsData->SetEnvironment(tempEnv);
   itsData->Enable();
   itsSlider->Enable();
}



void BoxSliderObject::ComputeNumberOfDecimalsAndMultiplier(void)
{
   itsNumberOfDecimals = 0;  itsMultiplier = .001;
   float sliderRange = fabs(fabs(itsMaxSlider) - fabs(itsMinSlider));

   if (sliderRange <= 10000)
   {
      itsNumberOfDecimals = 1;
      itsMultiplier = .01;
   }

   if (sliderRange <= 1000)
   {
      itsNumberOfDecimals = 1;
      itsMultiplier = 1;      // was .1
   }

   if (sliderRange <= 100)
   {
      itsNumberOfDecimals = 2;
      itsMultiplier = 10;     // was 1
   }

   if (sliderRange <= 10)
   {
      itsNumberOfDecimals = 3;
      itsMultiplier = 1000;   // was 1000
   }

   if (sliderRange <= 1)
   {
      itsNumberOfDecimals = 4;
      itsMultiplier = 1000;
   }

   // non decimal number < 100 handled differently

   if (sliderRange <= 100 && !allowDecimal) itsMultiplier = 1;
}





void BoxSliderObject::WriteValueToControl(float value)
{

   // write non decimals as int

   if (!allowDecimal) itsData->SetTitle(ConvertToString((int) (value + .5)));
   else               itsData->SetTitle(ConvertToString(value));

}




void BoxSliderObject::SetSliderSettings(float value)
{
   UNITS minS,maxS,valS,lineinc,pageinc;

   minS = (itsMinSlider+itsNegOffset)*itsMultiplier;
   maxS = (itsMaxSlider+itsNegOffset)*itsMultiplier;
   valS = (value+itsNegOffset)*itsMultiplier;

   if (allowDecimal || itsMultiplier < 1)
   {
      lineinc = ((itsMaxSlider+itsNegOffset)-(itsMinSlider+itsNegOffset))
                                            / 100*itsMultiplier;
      pageinc = ((itsMaxSlider+itsNegOffset)-(itsMinSlider+itsNegOffset))
                                            / 10*itsMultiplier;
   }
   else
   {
      lineinc = 1.;
      pageinc = 10.;
   }

   if (valS < minS || valS > maxS || lineinc > pageinc)
      xvt_dm_post_error("BoxSlider error: minS= %f, maxS= %f, valS= %f, "
                        "lineinc= %f, pageinc= %f",
                        minS,maxS,valS,lineinc,pageinc);

   itsSlider->IScrollBar(minS, maxS, valS, lineinc, pageinc);
}




CStringRW BoxSliderObject::GetBoxSliderSettings(void)  const
{
   CStringRW tmp;

   // find out if the box is empty
   // if so, write "blank"

   if (itsData->GetTitle().strip().length() == 0) tmp="blank";
   else tmp=itsData->GetTitle();
   tmp += " ";

   // concatenate the slider range values

   if (itsMinValue <= FLT_MIN && itsMinValue > 0.0) tmp += "Fmin";
   else tmp += ConvertToString(itsMinValue);
   tmp += " ";

   if (itsMaxValue >= 1e20) tmp += "Fmax";
   else tmp += ConvertToString(itsMaxValue);
   tmp += " ";

   tmp += ConvertToString(itsMinSlider);
   tmp += " ";
   tmp += ConvertToString(itsMaxSlider);
   tmp += " ";
   tmp += ConvertToString(itsDefault);

   return tmp;
}



void BoxSliderObject::ResetBoxSliderSettings(const char* valueString)
{

   // Set the box blank, if blank is the default.

   if (ConvertValueData(valueString,  &itsDefault,
                        &itsMinValue, &itsMaxValue,
                        &itsMinSlider,&itsMaxSlider) == 1)
   {
     itsData->SetTitle(NULLString);
   }
   else
   {
      char t[30];
      sscanf(valueString,"%s",t);
      CStringRW t2=t;
      t2 = t2.strip(RWCString::both);
      itsData->SetTitle(t2);
   }

   // set the number of decimals based on the range between min
   // and max slider values; use fabs in case negative numbers

   ComputeNumberOfDecimalsAndMultiplier();

   if(itsData->GetTitle().length() > 0)
      WriteValueToControl(ConvertToFloat(itsData->GetTitle()));

   // itsNegOffset used to shift the scroll values when the
   // minimum is less than 0

   itsNegOffset = itsMinSlider < 0 ?  fabs(itsMinSlider) : 0;

   SetSliderSettings(itsDefault);
}


