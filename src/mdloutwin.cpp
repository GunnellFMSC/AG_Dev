/*********************************************************************

   File Name:           mdloutwin.cpp
                                                                     *
 *********************************************************************/

#include "appdef.hpp"

#include NListBox_i
#include NLineText_i
#include NText_i
#include NButton_i

#include CUnits_i
#include CView_i
#include CWindow_i
#include CDocument_i
#include CStringCollection_i
#include CStringRWC_i
#include CBoss_i
#include CApplication_i
#include CDesktop_i

#include "mdloutwin.hpp"
#include "msgwin.hpp"      // for Tell();
#include "spfuncs.hpp"
#include "gpkey.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>


#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines
extern XVT_HELP_INFO helpInfo;    // help file




ModelOutWin::ModelOutWin(CDocument * theDocument)
            :CcatWin(theDocument,"Model Outputs","selectModelOutputs")
{
//   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), ModelOutWindow, 0L);
}


void ModelOutWin::BuildTheWin(ItemInCcat * anItem)
{
   // if anItem does not have a methodID, then we have an error.

   if (!anItem->methodID)
   {
      xvt_dm_post_error("Parameters file error:\n"
                        "The output request is not "
                        "linked to a method.");
      return;
   }

#if XVTWS==WIN32WS
   static char * far methods[] =
#else
   static char *     methods[] =
#endif
   {
      "GPSB",
      NULL
   };

   int method;

   for (int i = 0; methods[i] != NULL; i++)
   {
      if (strcmp(methods[i],anItem->methodID->data()) == 0)
      {
         method = i;
         break;
      }
   }

   xvt_scr_set_busy_cursor();

   switch (method)
   {
      case 0:           // GPSB
      {
         if (!anItem->mstextID)
         {
            xvt_dm_post_error("Parameters file error:\n"
                              "The output request does not "
                              "reference an MSText.");
            break;
         }
         MSText * neededMSText = GetMSTextPtr(anItem->mstextID->data());
         if (!neededMSText)
         {
            xvt_dm_post_error("Parameters file error:\n"
                              "The output request does not "
                              "reference an MSText.");
            break;
         }

         new GPKeyword (*anItem, neededMSText,
                        *anItem->extensID, NULL);

         break;
      }
      default:
      {
         xvt_dm_post_error("Parameters file error:\n"
                           "The output request does not "
                           "reference a valid window ID.");
         break;
      }
   }
}


