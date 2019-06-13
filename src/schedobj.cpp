/********************************************************************

   File Name:     schedobj.cpp
   Author:        kfd & nlc
   Date:          08/25/94 05/06/98


   NOTE:          A ScheduleByOject is 538 X 73

********************************************************************/
#include "appdef.hpp"
#include "PwrDef.h"

#include NGroupBox_i
#include NEditControl_i
#include NRadioButton_i
#include NButton_i
#include CText_i
#include CRectangle_i
#include CRadioGroup_i
#include CNavigator_i

#include "schedobj.hpp"
#include "cyear.hpp"            // for Year member function
#include "yearobj.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "warn_dlg.hpp"



ScheduleByObject::ScheduleByObject(CWindow* theParent,
                                   long theConditionDoCommandNumber,
                                   CSubview* theEnclosure,
                                   const CPoint& theTL,
                                   long theConditionButtonWhenEditingCommand,
                                   long theChangeStatusCommand)
                  :CSubview(theEnclosure,
                            CRect(theTL.H(), theTL.V(),
                                  theTL.H()+530, theTL.V()+64)),
                   conditionParent(theParent),
                   conditionDoCommandNumber(theConditionDoCommandNumber),
                   conditionButtonWhenEditingCommand(theConditionButtonWhenEditingCommand),

                   changeStatusCommand(theChangeStatusCommand)
{
                        // Construct the schedule by object


   itsScheduleGroup = new CRadioGroup(this,CPoint(0,0), FALSE);

   itsYearButton = itsScheduleGroup->
                   AddButton(CPoint(0,0),"Schedule by Year/Cycle",0);
   itsScheduleGroup->SetSelectedButton(itsYearButton);

   itsScheduleButton =
      itsScheduleGroup->AddButton(CPoint(200,0),"Schedule by Condition",1);

                     // schedule in year area

   itsField1 = new NEditControl(this, CRect(0,32,56,64),
                   ConvertToString(theSpGlobals->yearObj->GetSelectedYear()),
                   CTL_FLAG_RIGHT_JUST);
   itsField1->SetFocusCommands(NULLcmd,4);
   itsField1Button = new NButton(this, CRect(64,32,168,64), "Select Year");
   itsField1Button->SetCommand(2);

                     // schedule in condition area

   itsField2 = new NEditControl(this, CRect(200,32,256,64),
                                "0", CTL_FLAG_RIGHT_JUST);
   itsField2->Disable();
   itsField2->SetFocusCommands(NULLcmd,4);
   itsField2Button = new NButton(this, CRect(368,32,472,64), "Condition");
   itsField2Button->SetCommand(3);
   SpDeactivate(itsField2, itsField2Button);

   new CText(this, CPoint(270,40), CStringRW("years after"));
   new CText(this, CPoint(488,40), CStringRW("is met"));

}



ScheduleByObject::~ScheduleByObject()
{
                        // Destruct the schedule by object
}




void ScheduleByObject::DoCommand(long  theCommand, void* theData)

{
   CStringRW itsYearString;

   switch (theCommand)
   {
      // radio button1
      case 0:
      {
         SpActivate(itsField1, itsField1Button);
         SpDeactivate(itsField2, itsField2Button);
         if (changeStatusCommand != NULLcmd)
            conditionParent->DoCommand(changeStatusCommand, NULL);
         break;
      }

      // radio button2
      case 1:
      {
         SpActivate(itsField2, itsField2Button);
         SpDeactivate(itsField1, itsField1Button);
         if (changeStatusCommand != NULLcmd)
           conditionParent->DoCommand(changeStatusCommand, this);
         break;
      }
      // select year button
      case 2:
      {
         itsYearString = Year(itsField1->GetTitle());
         itsField1->SetTitle(itsYearString);
         if (changeStatusCommand != NULLcmd)
           conditionParent->DoCommand(changeStatusCommand, this);
         break;
      }

      // conditionbutton
      case 3:
      {
         if (conditionDoCommandNumber != NULLcmd)
            conditionParent->DoCommand(conditionDoCommandNumber, this);
         break;
      }

      case 4:
      {
         if (changeStatusCommand != NULLcmd)
            conditionParent->DoCommand(changeStatusCommand, NULL);
         break;
      }

      // conditionbutton when in edit mode
      case 50:
      {
         if (conditionButtonWhenEditingCommand != NULLcmd)
            conditionParent->DoCommand(conditionButtonWhenEditingCommand, this);
         break;
      }
      default: break;

   }
}





void ScheduleByObject::SpDeactivate(NEditControl* field, NButton* button)
{
                        // change the color, disable textbox and button
   CEnvironment tempEnv;

   tempEnv.SetColor(COLOR_WHITE,COLOR_LTGRAY, COLOR_WHITE, COLOR_LTGRAY);
   field->SetEnvironment(tempEnv);
   field->Disable();
   button->Disable();
}





void ScheduleByObject::SpActivate(NEditControl* field, NButton* button)
{
                        // change the color, enable textbox and button
   CEnvironment tempEnv;

   tempEnv.SetColor(COLOR_WHITE,COLOR_BLACK, COLOR_WHITE, COLOR_BLACK);
   field->SetEnvironment(tempEnv);
   field->Enable();
   button->Enable();
}




int ScheduleByObject::SpGetValue(int &value) const
{

   int buttonNumber;
   long selected = itsScheduleGroup->GetSelectedButton();

   if (selected == itsYearButton)
   {
     buttonNumber = 1 ;
     sscanf(itsField1->GetTitle(),"%d", &value);
   }

   else
   {
     buttonNumber = 2 ;
     sscanf(itsField2->GetTitle(),"%d", &value);
   }

   return (buttonNumber);
}



const char * ScheduleByObject::SpGetTitle( void )
{
   static CStringRW aTitle;

   if (itsScheduleGroup->GetSelectedButton() == itsYearButton)
     aTitle = itsField1->GetTitle().data();
   else
     aTitle = itsField2->GetTitle().data();

  return aTitle.data();
}



void ScheduleByObject::SetYearButton( BOOLEAN setYearSelected)
{

   if (setYearSelected)
   {
      SpActivate(itsField1, itsField1Button);
      SpDeactivate(itsField2, itsField2Button);
      itsScheduleGroup->SetSelectedButton(itsYearButton);
   }
   else
   {
      SpActivate(itsField2, itsField2Button);
      SpDeactivate(itsField1, itsField1Button);
      itsScheduleGroup->SetSelectedButton(itsScheduleButton);
   }

}



int ScheduleByObject::YearIsSelected( void )
{
   return (itsScheduleGroup->GetSelectedButton() == itsYearButton);
}



CStringRW ScheduleByObject::GetSchedObjSettings( void)
{
   CStringRW returnString;

   long selected = itsScheduleGroup->GetSelectedButton();

   if (selected == itsYearButton)
      returnString += "0 ";
   else
      returnString += "1 ";

   returnString += (char const *)itsField1->GetTitle();
   returnString += " ";
   returnString += (char const *)itsField2->GetTitle();
   returnString += " ";

   return returnString;
}



void ScheduleByObject::ResetSchedObjSettings( const char * theString)
{
   int button;
   char year[10];
   char conditionYear[10];

   sscanf(theString, "%d %s %s", &button, year, conditionYear);
   if (button == 0)
   {
      itsScheduleGroup->SetSelectedButton(itsYearButton);
      DoCommand(0);
   }
   else
   {
      itsScheduleGroup->SetSelectedButton(itsScheduleButton);
      DoCommand(1);
   }

   itsField1->SetTitle(year);
   itsField2->SetTitle(conditionYear);

}



void ScheduleByObject::ResetConditionButtonCommand( int newValue)
{
   itsField2Button->SetCommand(newValue);
}


void ScheduleByObject::AppendToNavigator(CNavigator * theNavigator)
{
   if (theNavigator)
   {
      theNavigator->AppendSubviews(itsScheduleGroup);
      theNavigator->AppendTabStop(new CTabStop(itsField1Button));
      theNavigator->AppendTabStop(new CTabStop(itsField2Button));
   }
}
