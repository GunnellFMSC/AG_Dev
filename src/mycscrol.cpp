/********************************************************************

   File Name:     mycscrol.cpp   Modified CScroller.
   Author:        nlc
   Date:          05/09/98


********************************************************************/

#include "PwrDef.h"
#include "mycscrol.hpp"
#include CNavigator_i
#include NScrollBar_i

MyCScroller::MyCScroller(CSubview *   theEnclosure,
                         const CRect& theRegion,
                         UNITS        theVirtualWidth,
                         UNITS        theVirtualHeight)
            :CScroller(theEnclosure,theRegion,
                       theVirtualWidth,theVirtualHeight)
{
   CEnvironment anEnv(*GetEnvironment());
   anEnv.SetBlendColor(anEnv.GetBrushColor());
   SetEnvironment(anEnv);
}


void MyCScroller::AppendToNavigator(CNavigator * theNavigator)
{
   if (theNavigator)
   {
      if (itsVScrollBar) theNavigator->
                               AppendTabStop(new CTabStop(itsVScrollBar));
      if (itsHScrollBar) theNavigator->
                               AppendTabStop(new CTabStop(itsHScrollBar));
   }
}


