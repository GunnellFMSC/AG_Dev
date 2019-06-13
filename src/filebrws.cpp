/********************************************************************

   File Name:     filebrws.cpp
   Author:        nlc
   Date:          12/27/00

********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"
#include CText_i
#include NButton_i
#include "mylinetx.hpp"
#include <rw/cstring.h>
#include "dirsept.hpp"
#include <stdio.h>
#include "filebrws.hpp"

FileBrowseObject::FileBrowseObject(CSubview* theEnclosure,
                                   const CRect& theRegion,
                                   const char* theTitle,
                                   const char* theDefaultFileName)

                  : CSubview(theEnclosure, theRegion)
{
   CStringRW aTitle;
   if (theTitle) aTitle = theTitle;

   int height = theRegion.Bottom()-theRegion.Top();
   int width  = theRegion.Right()-theRegion.Left();

   itsTitleCText = new CText(this, CPoint(0,0), aTitle);
   itsFileNameMyNLineText = new MyNLineText(this,
                                            CPoint(0,height-40),
                                            width-80,
                                            0, 0, SZ_FNAME);

   if (theDefaultFileName) SetValue(theDefaultFileName);
   itsBrowseButton = new NButton(this, CRect(width-76,height-40,
                                             width,height-4), "Browse");
   itsBrowseButton->SetCommand(1);
}


void FileBrowseObject::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:
      {
         const char * theType =
               strrchr(itsFileNameMyNLineText->GetText().data(),'.');
         FILE_SPEC fileSpec;
         xvt_fsys_get_dir(&fileSpec.dir);
         if (theType) strncpy(fileSpec.type,++theType,strlen(theType));
         else fileSpec.type[0] = NULL;
         xvt_fsys_save_dir();
         if (xvt_dm_post_file_open(&fileSpec,
                                   (char*)itsTitleCText->GetTitle().data())
             == FL_OK)
         {
            char dirPart[SZ_FNAME];
            char path[SZ_FNAME];
            xvt_fsys_convert_dir_to_str(&fileSpec.dir, dirPart,
                                        sizeof(dirPart));
            sprintf (path, "%s%s%s", dirPart, SEPARATOR, fileSpec.name);
            itsFileNameMyNLineText->SetText(path);
         }
         xvt_fsys_restore_dir();
         break;
      }
      default: CSubview::DoCommand(theCommand,theData); break;
   }
}




const CStringRW FileBrowseObject::GetTitle(void) const
{
   return itsTitleCText->GetTitle();
}

const CStringRW FileBrowseObject::GetValue(void) const
{
   return itsFileNameMyNLineText->GetText();
}

void FileBrowseObject::SetTitle(const char* theTitle)
{
   itsTitleCText->SetTitle(theTitle);
}

void FileBrowseObject::SetValue(const char* theValue)
{
   itsFileNameMyNLineText->SetText(theValue);
   itsFileNameMyNLineText->SetInsertPosition(strlen(theValue));
}
