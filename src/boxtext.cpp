/********************************************************************

   File Name:     boxtext.cpp
   Author:        kfd
   Date:          12/19/94

   Contents:      BoxTextObject()
                  BoxTextObject::DoCommand
                  BoxTextObject::DeactivateBoxText
                  BoxTextObject::ActivateBoxText
                  BoxTextObject::GetValue
                  BoxTextObject::GetTitle

   NOTE:          A BoxTextObject consists of a EditControl and title(s)

   Parameters:    theEnclosure,
                  the containing rectangle of boxtext (min 300x44 or 300x70)
                  the text title
                  the default string (NULL is the default)
                  minimum box setting
                  maximum box setting
                  the box position (SPLEFT(default); SPRIGHT)


********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include CText_i
#include NScrollBar_i
#include NEditControl_i
#include <rw/cstring.h>
#include <string.h>
#include <stdio.h>

#include "boxtext.hpp"
#include "cmngwin.hpp"   // for checkrange
#include "spfunc2.hpp"   // for convert data
#include "warn_dlg.hpp"



BoxTextObject::BoxTextObject(CSubview* theEnclosure, const CRect& theRegion,
                             const char* boxTextTitle,
                             const char* defaultString,
                             BOOLEAN theBoxPosition)

              : CSubview(theEnclosure,  theRegion),
                itsString(defaultString)
{




                           // set up default values
   itsDefault   = 0;
   itsMinValue  = 0;       // using FLT_MIN causes problems  ****
   itsMaxValue  = FLT_MAX;
   allowDecimal = TRUE;
   allowMinus   = TRUE;

                           // parse the defaultString;


   char * aTmpData = new char[50];

   int theStringLen = strlen(itsString);
   char * theCopy = new char[theStringLen+1];
   strcpy(theCopy,itsString);

               // use strtok to get the tokens.  When the tokens
               // are all used, or we have all the data we need.

                        // parse default data or word blank
   char * ptr = strtok(theCopy," ");
   if (ptr)
   {
      sscanf (ptr,"%s", aTmpData);
      if (strcmp(aTmpData,"blank") == 0)
      {
         strcpy(aTmpData,"");
         blankOK = TRUE;
      }
      else
      {
         blankOK = FALSE;
         sscanf(ptr,"%f", &itsDefault);
         sprintf(aTmpData,"%f",  itsDefault);
         StripTrailingZeros(aTmpData);
      }
      ptr = strtok((char*) 0," ");
   }

                        // parse default itsMinValue
   if (ptr)
   {
      if (strcmp(ptr,"FMin")) itsMinValue = FLT_MIN;
      sscanf (ptr,"%f", &itsMinValue);
      ptr = strtok((char*) 0," ");
   }

                        // parse default itsMaxValue
   if (ptr)
   {
      if (strcmp(ptr,"FMax")) itsMaxValue = FLT_MAX;
      sscanf (ptr,"%f", &itsMaxValue);
      ptr = strtok((char*) 0," ");
   }


                        // parse TRUE or FALSE for allowMinus
   if (ptr)
   {
      if (strcmp(ptr,"FALSE") == 0 || strcmp(ptr,"false") == 0 ||
                                  strcmp(ptr,"0") == 0)
         allowMinus = 0;
      ptr = strtok((char*) 0," ");
   }

                        // parse TRUE or FALSE for allowDecimal
   if (ptr)
   {
      if (strcmp(ptr,"FALSE") == 0 || strcmp(ptr,"false") == 0 ||
                                  strcmp(ptr,"0") == 0)
         allowDecimal = 0;
      ptr = strtok((char*) 0," ");
   }


                  // find the right edge of the window; use this
                  // to position scroll bar, edit control

   UNITS rightEdge = theRegion.Right()-theRegion.Left();

                  // vertical offset; changes if optional title included
   UNITS vOffset = 0;

               // place controls when left box position specified

   if (theBoxPosition == SPLEFT)
   {
                           // Construct a boxText object
                           // place the text box
      itsData = new NEditControl(this, CRect(0,vOffset,88,32+vOffset),
                "", CTL_FLAG_RIGHT_JUST);

      new CText(this, CPoint(100,4+vOffset), CStringRW(boxTextTitle));
   }


               // place controls when right box position specified
   else
   {
                           // Construct a boxslider object
                           // place the text box
      new CText(this, CPoint(0,4+vOffset), CStringRW(boxTextTitle));

      itsData = new NEditControl(this,
                CRect(rightEdge - 88,vOffset,rightEdge,32+vOffset),
                "", CTL_FLAG_RIGHT_JUST);
   }

                        // remove decimal, if necessary and round the default
   if (!allowDecimal && !blankOK)
      sprintf(aTmpData,"%d",  (int) (itsDefault+.5));

                        // copy the default to the edit control
   itsData->SetTitle(aTmpData);
   delete [] aTmpData;
   delete [] theCopy;

   itsData->SetFocusCommands(0,1);
   itsData->SetTextCommand(2);
}


void BoxTextObject::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 0:
         break;

      case 1:
                        // see if number is within acceptable range
         if (!blankOK && itsData->GetTitle().length() < 1)
            Warn("You must enter a value for the entry.");

         else
         {
            if (itsData->GetTitle().length() > 0)
            {
                                  // checkrange for floats
               if (allowDecimal)
                  CheckRange(itsData->GetTitle(),
                                       itsMinValue,itsMaxValue);

                                  // checkrange for ints
               else
                  CheckRange(itsData->GetTitle(),(int)itsMinValue,
                                       (int) itsMaxValue);
            }
         }
         break;


      case 2:           // user entered something in itsData
      {
         ValidateCharacters(itsData);     // must be valid character
         break;
      }

      default: CSubview::DoCommand(theCommand,theData); break;
   }
}


float BoxTextObject::GetValue(void)
{
                        // return the value which is entered in the textbox
   float value;

   sscanf(itsData->GetTitle(),"%f", &value);
   return  value;
}


void BoxTextObject::DeactivateBoxText(void)
{
                        // change the color, disable textbox and slider
   CEnvironment tempEnv;

   tempEnv.SetColor(COLOR_WHITE,COLOR_LTGRAY, COLOR_WHITE, COLOR_LTGRAY);
   itsData->SetEnvironment(tempEnv);
   itsData->Disable();
}


void BoxTextObject::ActivateBoxText(void)
{
                        // change the color, enable textbox and slider
   CEnvironment tempEnv;

   tempEnv.SetColor(COLOR_WHITE, COLOR_BLACK, COLOR_WHITE, COLOR_BLACK);
   itsData->SetEnvironment(tempEnv);
   itsData->Enable();
}


const CStringRW BoxTextObject::GetTitle(void) const
{
                        // return the sting which is entered in the textbox

   return itsData->GetTitle();
}




CStringRW BoxTextObject::GetBoxTextSettings(void)  const
{

   char * aBuf = new char[200];

                        // find out if the box is empty
                        // if so, write "BLANK"
   if (itsData->GetTitle().strip().length() == 0)
      sprintf(aBuf,"blank ");
   else
                        // else get what is there
      sprintf(aBuf,"%s ",(char const *) itsData->GetTitle());

   char * ptr = aBuf+strlen(aBuf);

   if (itsMinValue <= FLT_MIN && itsMinValue > 0.0)
        ptr += sprintf(ptr," Fmin");
   else ptr += sprintf(ptr," %.4f",itsMinValue);

   if (itsMaxValue >= 1e20)
        ptr += sprintf(ptr," Fmax");
   else ptr += sprintf(ptr," %.4f",itsMaxValue);

   ptr += sprintf(ptr," %.2f", itsDefault);

                        // if there is blanks are allowed, indicate that
   if(blankOK)
      strcat(aBuf," Bok");

                        // otherwise write "NoB"
   else
      strcat(aBuf," NoB");

                        // convert to CStringRW
   CStringRW text(aBuf);
   delete [] aBuf;

  return text;
}



void BoxTextObject::ResetBoxTextSettings(const char* valueString)
{

   char * aBuf = new char[strlen(valueString)+1];
   strcpy(aBuf,valueString);
   char * ptr = strtok(aBuf," ");

   if (strcmp(ptr,"blank") == 0)
   {
      itsData->SetTitle("");
   }
   else
   {
      itsData->SetTitle(ptr);
   }

   ptr = strtok(NULL," ");
   if (strcmp(ptr,"FMin") == 0) itsMinValue = FLT_MIN;
   else sscanf (ptr,"%f", &itsMinValue);

   ptr = strtok(NULL," ");
   if (strcmp(ptr,"FMax") == 0) itsMaxValue = FLT_MAX;
   else sscanf (ptr,"%f", &itsMaxValue);

   ptr = strtok(NULL," ");
   sscanf(ptr,"%f", &itsDefault);

   ptr = strtok(NULL," ");

   if (strcmp(ptr, "NoB") == 0)
      blankOK = 0;
   else
      blankOK = 1;

   delete [] aBuf;

   return;
}




int BoxTextObject::ValidateCharacters(NEditControl* itsEntry)
{
                // Validate the text entered.  Accept 0-9, arrow keys + others

   int theChar;
   const char  *aString = itsEntry->GetTitle().data();
   int i = 0;
   while(aString[i])
   {
      theChar = aString[i];

                              // remove all except numbers (48-57)
                              // decimal (46) & minus (45)
      if( (theChar < 47 || theChar > 57) || theChar == 47)
      {
         if (allowDecimal && theChar == 46);       // noop
         else
            if (allowMinus && theChar == 45);      // noop
            else
            {
               xvt_scr_beep();
               CStringRW tmp(itsEntry->GetTitle());
               itsEntry->SetTitle(tmp.remove(i,1));
               return 0;
            }
      }
      i++;
   }
   return 0;
}
