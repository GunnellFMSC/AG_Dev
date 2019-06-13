/********************************************************************

   File Name:           cmngwin.hpp
   Author:              nlc
   Date:

   Class:               CMngWin
   Inheritance:         CMngWin->CcatWin->CWwindow->CWindow
                                                                    *
 ********************************************************************/

#ifndef CMngWin_H
#define CMngWin_H

#include "ccatwin.hpp"

int CheckRange(const CStringRW& theString, int min, int max);
float CheckRange(const CStringRW& theString, float minf, float maxf);

class CMngWin : public CcatWin
{
   public:

      CMngWin(CDocument *theDocument);
      virtual void BuildTheWin(ItemInCcat * anItem);

};

#endif //CMngWin_H


