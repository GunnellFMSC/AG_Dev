/********************************************************************

   File Name:           cmmodwin.hpp
   Author:              nlc
   Date:

   Class:               CModelModWin
   Inheritance:         CModelModWin->CcatWin->CWwindow->CWindow
                                                                    *
 ********************************************************************/

#ifndef CModelModWin_H
#define CModelModWin_H

#include "ccatwin.hpp"

class CModelModWin : public CcatWin
{
   public:

      CModelModWin(CDocument *theDocument);
      virtual void BuildTheWin(ItemInCcat * anItem);

};

#endif //CModelModWin_H


