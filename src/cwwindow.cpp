/********************************************************************

   File Name:    cwwindow.cpp
   Author:       kfd & nlc
   Date:         1994-1996

   Purpose:

   CWwindow class is an instance of CWindow.  Its purpose is to use it as
   the base class for instancing other windows.

 *                                                                  *
 ********************************************************************/

#include "appdef.hpp"

#include CWindow_i
#include CDesktop_i
#include CDocument_i
#include CKey_i

#include "cwwindow.hpp"
#include "msgwin.hpp"   // for Tell

#include "suppdefs.hpp"
#include "supphelp.hpp"
#include "spfunc2.hpp"

extern XVT_HELP_INFO helpInfo;    // help file

PWRRegisterClass1(CWwindow, CWwindowWinID, "CWwindow", CWindow)

CWwindow::CWwindow(CDocument *theDocument,
                   const CRect& theRegion,
                   const CStringRW& theTitle,
                   long theAttributes,
                   WIN_TYPE theWindowType,
                   int theMenuBar)

         : CWindow(theDocument, theRegion, theTitle,
                    theAttributes, theWindowType, theMenuBar)
{
   AddDynamicHelpItems(GetMenuBar());

   WINDOW theWindow = GetXVTWindow();

   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_TIME,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_MODEL,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_MANAGEMENT,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_MODEL_MOD,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_USEKEYWORDS,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_POST_PROCESSOR,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_RUN,
                            MENUbar, 0L);
/*   xvt_help_set_menu_assoc (helpInfo, theWindow,		//Commented out 05/21/09 AJSM
                            M_HELP_RUNWIZARD,
                            MENUbar, 0L);
*/   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_AFTER_SIM_REPORTS,
                            MENUbar, 0L);

   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_VARIANT,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_SIMNOTES,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_VIEWKEYWORDS,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_DELETESTAND,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_SIM_PREP_DELETEGROUP,
                            MENUbar, 0L);

   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_PREFERENCES_EDIT,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_DATA_PREP_LOCATIONS,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_DATA_PREP_STANDLIST,
                            MENUbar, 0L);
   xvt_help_set_menu_assoc (helpInfo, theWindow,
                            M_DATA_PREP_FVSTREEDATA,
                            MENUbar, 0L);
}



BOOLEAN CWwindow::Close()
{
   int cnt = CBoss::G->GetDesktop()->GetNumWindows();
   if (cnt <= 2)
   {
      return CWindow::Close();
   }
   else xvt_vobj_destroy(GetXVTWindow());
   return TRUE;
}



void CWwindow::DoCommand(long theCommand,void* theData)
{
   CWindow::DoCommand(theCommand,theData);
}


void CWwindow::DoKey( const CKey& theKey )
{
   CWindow::DoKey(theKey);
}

