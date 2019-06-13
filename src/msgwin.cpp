/*********************************************************************

   File Name:           msgwin.cpp

   Modifications:
                                                                     *
 *********************************************************************/
#include "PwrDef.h"
#include CWindow_i
#include "appdef.hpp"

#include "msgwin.hpp"



void Tell(const int min, const int max)
{
  // This function puts up a "dialog box" and returns when the user
  // presses ok

  xvt_dm_post_message("Enter an integer between %d and %d", min, max);
}



void Tell(const float minf, const float maxf)
{
  // This function puts up a "dialog box" and returns when the user
  // presses ok

  xvt_dm_post_message("Enter an number between %.2f and %.2f", minf, maxf);

}





