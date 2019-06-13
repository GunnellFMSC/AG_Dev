/********************************************************************

   File Name:    viewkey.hpp
   Author:       nlc
   Date:         01/27/95

   Purpose:  This class is used to view keywords associated with
             a component.

   Contents:

     class ViewKeywords
                                                                    *
 ********************************************************************/

#ifndef ViewKeywords_H
#define ViewKeywords_H

#include "cwwindow.hpp"
#include NScrollText_i

class ViewKeywords : public CWwindow
{
  public:

    ViewKeywords(const char * theKeywords, const CStringRW & theTitle);

    virtual void DoCommand(long theCommand,void* theData = NULL);
    inline void ScrollColumnRuler(T_CPOS theOrgOffset)
      { itsColumnRuler->SetScrollPosition(0, theOrgOffset); }

  private:
    NScrollText * itsColumnRuler;
};

#endif
