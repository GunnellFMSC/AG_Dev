/********************************************************************

   File Name:           startapp.hpp
   Author:              kfd
   Date:                11/29/94

   Class:               CsupposeApp
   Inheritance:         CApplication
   Helper(s):           CsupposeDoc, CsupposeWin

   Purpose:     This is an example of how to override the minimal set of
                methods of CApplication. It initializes the inherited
                application and creates a shell document.

   Usage:       You may rename this class and add or override methods as
                nessesary.

   Override:

   Modifications:

 ********************************************************************/

#ifndef StartApp_H
#define StartApp_H

#include "PwrDef.h"

#include CTypeInfo_i
#include CApplication_i

class CsupposeApp : public CApplication
{
  public:

        CsupposeApp(void);

        virtual void StartUp(void);
  //    virtual void SetUpMenus(CMenuBar* theMenuBar);
        virtual void DoCommand(long theCommand,void* theData=NULL);

        virtual BOOLEAN DoNew(void);
        virtual BOOLEAN DoOpen(void);

  private:

        PWRClassInfo
};

#endif // StartApp_H
