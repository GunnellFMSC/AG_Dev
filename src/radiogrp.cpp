/********************************************************************

   File Name:     radiogrp.cpp
   Author:        
   Date:          02/27/06

   Purpose:       RadioGroupObjects are objects which contain a bordered
                  container (NGroupBox) with radio buttons (NRadioButton)
                  managed so that only one radio button in the group can 
                  selected at any time (CRadioGroup). The container and
                  radio buttons can have titles. One radio button can be
                  specified as default.

   Contents:      RadioGroupObject()
                  ~RadioGroupObject()
                  RadioGroupObject::SetSelection
                  RadioGroupObject::GetSelection

   Parameters:    theEnclosure,
                  the containing rectangle of radio groupbox (min 300x44 or 300x70)
                  the groupbox title
                  list of radio button titles


********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include CRadioGroup_i
#include CStringRWC_i
#include CSubview_i
#include NGroupBox_i
#include NRadioButton_i

#include <rw/cstring.h>
#include <string.h>
#include <stdio.h>

#include "radiogrp.hpp"


RadioGroupObject::RadioGroupObject(CSubview* theEnclosure, const CRect& theRegion,
                                   const char* groupBoxTitle, RWOrdered * radioButtonList)
   : CSubview(theEnclosure, theRegion),
     itsCRadioGroup(NULL),
     itsNGroupBox(NULL)
{
   // The height of theRegion should be the number of radioButtonList 
   // entries, plus one for offset vertical spacing, times 24

   itsNGroupBox = new NGroupBox(this, GetLocalFrame(), CStringRW(groupBoxTitle), 0L);
   itsCRadioGroup = new CRadioGroup(this, CPoint(0, 0), FALSE);

   // Create radiobuttons from list
	int anIndex = 0;
	RWOrderedIterator doTo(*radioButtonList);
	CStringRWC *aStr = (CStringRWC *)doTo();
	while (aStr)
	{
      itsCRadioGroup->AddButton(CPoint((UNITS)24, (UNITS)(anIndex++ * 24) + 18), *aStr);
		aStr = (CStringRWC *)doTo();
	}
}


RadioGroupObject::~RadioGroupObject(void)
{
   if (itsCRadioGroup)
      delete itsCRadioGroup;
   if (itsNGroupBox)
      delete itsNGroupBox;
}


void RadioGroupObject::SetSelection(int theSelection)
{
   if ((theSelection >= 0) && (theSelection < itsCRadioGroup->GetSubviews()->entries()))
   {
      NRadioButton *aButton = (NRadioButton *)itsCRadioGroup->GetSubviews()->at(theSelection);
      itsCRadioGroup->SetSelectedButton(aButton->GetId());
   }
}


int RadioGroupObject::GetSelection(void)
{
   long theSelection = itsCRadioGroup->GetSelectedButton();

   if (theSelection != -1)
   {
      int anIndex = 0;
      RWOrderedIterator doTo(*itsCRadioGroup->GetSubviews());
      NRadioButton *aButton;
      while( (aButton = (NRadioButton *)doTo()) != NULL )
      {
	      if( aButton->GetId() == theSelection )
	      {
		      theSelection = anIndex;
            break;
	      }

         anIndex++;
      }
   }

   return theSelection;
}
