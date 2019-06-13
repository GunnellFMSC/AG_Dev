/********************************************************************

   File Name:     radiogrp.hpp
   Author:        
   Date:          02/27/06

   Class:         RadioGroupObject
   Inheritance:   CSubview -> RadioGroupObject

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


********************************************************************/

#ifndef RadioGroupObject_H
#define RadioGroupObject_H

#include "appdef.hpp"
#include CSubview_i
#include CRadioGroup_i
#include NGroupBox_i
#include NRadioButton_i


class RadioGroupObject : public CSubview
{
   public:
      RadioGroupObject(CSubview* theEnclosure, const CRect& theRegion,
                       const char* groupBoxTitle, RWOrdered * radioButtonList);
      ~RadioGroupObject(void);

      virtual void SetSelection(int theSelection);
      virtual int GetSelection(void);

   protected:
      CRadioGroup* itsCRadioGroup;
      NGroupBox* itsNGroupBox;
};

#endif            //RadioGroupObject_H
