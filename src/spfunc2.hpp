/********************************************************************

   File Name:    spfunc2.hpp
   Author:       nlc
   Date:         02/09/00

   Purpose:  These are regular functions used for various purposes

   Contents:

      ConvertValueData
      numberLines
 *                                                                  *
 ********************************************************************/

#ifndef spfunc2_H
#define spfunc2_H

#include <float.h>

int ConvertValueData(const char * theString, float * defaultValue = 0,
                      float * minValue = 0,  float * maxValue = 0,
                      float * minSlider = 0, float * maxSlider = 0);

class CMenuBar;
class CStringRW;
class CScroller;
class CWindow;

long ConvertToLong(const char * theString);
float ConvertToFloat(const char * theString);
CStringRW ConvertToString(const int number);
CStringRW ConvertToString(const long number);
CStringRW ConvertToString(const float number);
int numberLines(const char * theString);
void StripSingleNewLines(CStringRW * theString);
void StripTrailingZeros(char * theString);
long XVT_CALLCONV1 CutCopyPasteDelete_CXO(XVT_CXO cxo, EVENT *ep);
CRect AdjWinPlacement(CRect theRect);
CRect CenterWinPlacement(CRect theRect);
CRect CenterTopWinPlacement(CRect theRect);
CRect RightWinPlacement(CRect theRect);
void AdjustScrollingWinSize(CScroller *theScroller=NULL, CWindow *theWindow=NULL);
void AddDynamicHelpItems(CMenuBar* theMenuBar);

#endif
