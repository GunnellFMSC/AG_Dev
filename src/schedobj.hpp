/********************************************************************

   File Name:     schedobj.hpp
   Author:        kfd
   Date:          08/25/94

   Class:         ScheduleByObject
   Inheritance:

   Purpose:       ScheduleByObjects are objects which are used to contain
                  schedule information.  They consist of:
                     radio buttons : toggle between year and condition
                     text boxes    : entered scheduling year, years after
                     buttons       : year or condition

   Contents:      ScheduleByObject
                  ~ScheduleByObject
                  ScheduleByObject::DoCommand
                  ScheduleByObject::SpGetValue
                  ScheduleByObject::SpGetTitle
                  ScheduleByObject::SpDeactivate
                  ScheduleByObject::SpActivate


********************************************************************/

#ifndef ScheduleByObject_H
#define ScheduleByObject_H

#include CSubview_i
#include NLineText_i

class NEditControl;
class NButton;
class CRadioGroup;
class SchByCond;
class CDocument;

class ScheduleByObject : public CSubview
{
   public:

      ScheduleByObject(CWindow* theParent,
                       long theConditionCommandNumber,
                       CSubview* theEnclosure,
                       const CPoint& theTL,
                       long theConditionButtonWhenEditingCommand = 50,
                       long theChangeStatusCommand = NULLcmd);
      virtual ~ScheduleByObject();
      virtual void DoCommand(long theCommand, void* theData=NULL);
      virtual int  SpGetValue(int &value) const;
      void SetYearButton( BOOLEAN setYearSelected);
      const char * SpGetTitle( void );
      int YearIsSelected( void );
      CStringRW GetSchedObjSettings( void);
      void ResetSchedObjSettings( const char * theString);
      void ResetConditionButtonCommand( int newValue);
      void AppendToNavigator(CNavigator * theNavigator);
      NEditControl * itsField1;
      NEditControl * itsField2;

   protected:

      CRadioGroup  * itsScheduleGroup;
      long           itsYearButton;
      long           itsScheduleButton;
      NButton*       itsField1Button;
      NButton      * itsField2Button;

      long           conditionDoCommandNumber;
      long           conditionButtonWhenEditingCommand;
      long           changeStatusCommand;

      CWindow      * conditionParent;

   private:

      virtual void SpDeactivate(NEditControl* field, NButton* button);
      virtual void SpActivate(NEditControl* field, NButton* button);

};

#endif            //ScheduleByObject_H
