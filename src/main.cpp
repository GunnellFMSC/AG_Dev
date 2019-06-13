/*********************************************************************

   File Name: main.cpp

   Author: kfd & nlc

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include "spglobal.hpp"
#include "cmdline.hpp"

#include "suppdefs.hpp"
#include "startapp.hpp"


void SBInit(void);

SpGlobal *theSpGlobals = (SpGlobal *) 0;

// the following include is only done if the error stream is desired.

// #include "error.cpp"

#if XVTWS==WIN32WS

#include <stdlib.h>

int main(int argc, char* argv[])
{

   char ** envp = _environ;

#else

int main(int argc, char* argv[], char* envp[])
{

#endif

   // initialize Suppose Global pointers

   theSpGlobals = new SpGlobal();

   // capture data from the command line.

   theSpGlobals->theCommandLine = new CommandLine (argc, argv, envp);

#if XVTWS==WIN32WS

   // set the task window size (if the command line contains the
   // necessary switch)...only on WIN32.

   const char * startUpRect = theSpGlobals->theCommandLine->getStartUpRect();
   if (startUpRect)
   {
      long screenHeight = xvt_vobj_get_attr(NULL, ATTR_SCREEN_HEIGHT);
      long screenWidth  = xvt_vobj_get_attr(NULL, ATTR_SCREEN_WIDTH);

      float top  = 0.;
      float bot  = 0.;
      float left = 0.;
      float right= 0.;
      int itop,ibot,ileft,iright;

      sscanf (startUpRect,"%f,%f,%f,%f", &top, &bot, &left, &right);

      if (top > 0.)   itop   = screenHeight*(top*.01);
      else            itop   = (int) -top;
      if (bot > 0.)   ibot   = screenHeight*(bot*.01);
      else            ibot   = (int) -bot;
      if (left > 0.)  ileft  = screenWidth*(left*.01);
      else            ileft  = (int) -left;
      if (right > 0.) iright = screenWidth*(right*.01);
      else            iright = (int) -right;

      if (ibot > screenHeight ||
          ibot == 0) ibot = screenHeight;
      if (iright > screenWidth ||
          iright == 0) iright = screenWidth;

      if (itop >= ibot) itop = 0;
      if (ileft >= iright) ileft = 0;

      RCT taskWinRCT;
      taskWinRCT.top   = itop +
                         (int) xvt_vobj_get_attr(NULL, ATTR_TITLE_HEIGHT);
      taskWinRCT.bottom= ibot;
      taskWinRCT.left  = ileft;
      taskWinRCT.right = iright;

      xvt_vobj_set_attr(NULL, ATTR_WIN_PM_TWIN_STARTUP_RCT,
                        (long) &taskWinRCT);
   }
   // Simple fix so that the  new control panel (from v2.01) would not be cut off 
    // so that the window is almost maximized within the vertical and horizontal 
   // display space (but not actually maximized).
   if(xvt_vobj_get_attr(NULL, ATTR_SCREEN_HEIGHT) < 1024)
   {
		long screenHeight = xvt_vobj_get_attr(NULL, ATTR_SCREEN_HEIGHT);
		long screenWidth  = xvt_vobj_get_attr(NULL, ATTR_SCREEN_WIDTH);

		RCT taskWinRCT;
		taskWinRCT.top   = screenHeight * 0.03;
		taskWinRCT.bottom= screenHeight * .95;
		taskWinRCT.left  = screenWidth * 0.01;
		taskWinRCT.right = screenWidth * .90;
		xvt_vobj_set_attr(NULL, ATTR_WIN_PM_TWIN_STARTUP_RCT,
						 (long) &taskWinRCT);
   }

   // Comment out the following line for standard XP look-n-feel (no 3D effect)
   xvt_vobj_set_attr(NULL, ATTR_WIN_USE_CTL3D, TRUE);

#endif

   // start the application.
   // Version incremented 04/02/2019
   char * sTitle = "Suppose v2.08";

   CsupposeApp theApplication;
   theApplication.Go(argc, argv, TASK_MENUBAR, 0,
                     "suppose", sTitle, sTitle);
   return 0;
}

