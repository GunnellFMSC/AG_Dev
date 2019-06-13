/********************************************************************

   File Name:           mdloutwin.hpp
   Author:              nlc
   Date:

   Class:               mdloutwin
   Inheritance:         mdloutwin->CcatWin->CWwindow->CWindow
                                                                    *
 ********************************************************************/

#ifndef mdloutwin_H
#define mdloutwin_H

#include "ccatwin.hpp"

class ModelOutWin : public CcatWin
{
   public:

      ModelOutWin(CDocument *theDocument);
      virtual void BuildTheWin(ItemInCcat * anItem);

};

#endif //mdloutwin_H


