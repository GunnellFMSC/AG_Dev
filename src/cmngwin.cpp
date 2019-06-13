/*********************************************************************

   File Name:           cmngwin.cpp
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

#include "cmngwin.hpp"
#include "msgwin.hpp"      // for Tell();
#include "clearwin.hpp"
#include "seedwin.hpp"
#include "sheltwin.hpp"
#include "thinwin.hpp"
#include "plantwin.hpp"
#include "cyear.hpp"
#include "spfuncs.hpp"
#include "gpkey.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>


#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines
extern XVT_HELP_INFO helpInfo;    // help file


int CheckRange(const CStringRW& theString, int min, int max)
{
   int retVal;
   sscanf(theString, "%d", &retVal);

   if (retVal < min)
   {
      Tell(min, max);
      return min;
   }

   if (retVal > max)
   {
      Tell(min, max);
      return max;
   }
   return retVal;
}



float CheckRange(const CStringRW& theString, float min, float max)
{
   float retVal;
   sscanf(theString, "%f", &retVal);

   if (retVal < min)
   {
      Tell(min, max);
      return min;
   }

   if (retVal > max)
   {
      Tell(min, max);
      return max;
   }
   return retVal;
}



CMngWin::CMngWin(CDocument * theDocument)
        :CcatWin(theDocument,"Management Actions","mgmtCategories")
{
   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), CMNGWINWindow, 0L);
}


void CMngWin::BuildTheWin(ItemInCcat * anItem)
{
   // if anItem does not have a methodID, then we have an error.

   if (!anItem->methodID)
   {
      xvt_dm_post_error("Parameters file error:\n"
                        "The management action is not "
                        "connected to a method.");
      return;
   }

#if XVTWS==WIN32WS
   static char * far methods[] =
#else
   static char *     methods[] =
#endif
   {
      "GPSB",
      "ClearcutWin",
      "SeedTreeWin",
      "ShelterwoodWin",
      "ThinFromAboveWin",
      "ThinFromBelowWin",
      "PlantNaturalFullWin",
      "PlantNaturalPartialWin",
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
                              "The management action does not "
                              "reference a MSText.");
            break;
         }
         MSText * neededMSText = GetMSTextPtr(anItem->mstextID->data());
         if (!neededMSText)
         {
            xvt_dm_post_error("Parameters file error:\n"
                              "The management action does not "
                              "reference a valid MSText.");
            break;
         }

         new GPKeyword (*anItem, neededMSText,
                        *anItem->extensID, NULL);

         break;
      }
      case 1:           // ClearcutWin
      {
         ClearcutWin * itsClearcutWin;
         itsClearcutWin = (ClearcutWin*)GetDocument()->
                          FindWindow(CLEARCUTWindowID);
         if (itsClearcutWin)
            G->GetDesktop()->SetFrontWindow(itsClearcutWin);
         else
         {
            itsClearcutWin = new ClearcutWin("Clearcut");
            itsClearcutWin->SetId(CLEARCUTWindowID);
         }
         break;
      }
      case 2:           // SeedTreeWin
      {
         SeedTreeWin * itsSeedTreeWin;
         itsSeedTreeWin = (SeedTreeWin *) GetDocument()->
                          FindWindow(SEEDTREEWindowID);
         if (itsSeedTreeWin)
            G->GetDesktop()->SetFrontWindow(itsSeedTreeWin);
         else
         {
            itsSeedTreeWin = new SeedTreeWin("Seedtree");
            itsSeedTreeWin->SetId(SEEDTREEWindowID);
         }
         break;
      }
      case 3:           // ShelterwoodWin
      {
         ShelterwoodWin * itsShelterwoodWin;
         itsShelterwoodWin = (ShelterwoodWin *) GetDocument()->
                             FindWindow(SHELTERWOODWindowID);
         if (itsShelterwoodWin)
            G->GetDesktop()->SetFrontWindow(itsShelterwoodWin);
         else
         {
            itsShelterwoodWin = new ShelterwoodWin("Shelterwood");
            itsShelterwoodWin->SetId(SHELTERWOODWindowID);
         }
         break;
      }
      case 4:           // ThinFromAboveWin
      {
         ThinFromWin * itsThinFromWin;
         itsThinFromWin = (ThinFromWin *) GetDocument()->
                          FindWindow(ThinFromAboveWindowID);
         if (itsThinFromWin) G->GetDesktop()->SetFrontWindow(itsThinFromWin);
         else
         {
            itsThinFromWin = new ThinFromWin("Thin from Above", 0);
            itsThinFromWin->SetId(ThinFromAboveWindowID);
         }
         break;
      }
      case 5:           // ThinFromBelowWin
      {
         ThinFromWin * itsThinFromWin;
         itsThinFromWin = (ThinFromWin *) GetDocument()->
                          FindWindow(ThinFromBelowWindowID);
         if (itsThinFromWin) G->GetDesktop()->SetFrontWindow(itsThinFromWin);
         else
         {
            itsThinFromWin = new ThinFromWin("Thin from Below", 1);
            itsThinFromWin->SetId(ThinFromBelowWindowID);
         }
         break;
      }
      case 6:           //  PlantNaturalFullWin
      case 7:           //  PlantNaturalPartialWin
      {
         PlantNaturalWin * itsPlantNaturalWin;
         itsPlantNaturalWin = (PlantNaturalWin *) GetDocument()->
                              FindWindow(PlantNaturalWindowID);
         if (itsPlantNaturalWin)
            G->GetDesktop()->SetFrontWindow(itsPlantNaturalWin);
         else
         {
            itsPlantNaturalWin =
               new PlantNaturalWin("Planting and Natural Regeneration",
                                   method==6);
            itsPlantNaturalWin->SetId(PlantNaturalWindowID);
         }
         break;
      }
      default:
      {
         xvt_dm_post_error("Parameters file error:\n"
                           "The management action does not "
                           "reference a valid window ID.");
         break;
      }
   }
}


