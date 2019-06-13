/********************************************************************

   File Name:     mycscrol.cpp   Modified CScroller.
   Author:        nlc
   Date:          05/09/98

   Contents:      See below.

********************************************************************/

#ifndef MyCScroller_H
#define MyCScroller_H

#include CScroller_i
#include CRect_i

class MyCScroller : public CScroller
{
   public:

   MyCScroller(CSubview *   theEnclosure,
               const CRect& theRegion,
               UNITS        theVirtualWidth  = 0,
               UNITS        theVirtualHeight = 0 );

   void AppendToNavigator(CNavigator * theNavigator);
};

#endif
