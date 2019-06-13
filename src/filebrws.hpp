/********************************************************************

   File Name:     filebrws.hpp
   Author:        nlc
   Date:          12/27/00

   Class:         BoxTextObject
   Inheritance:   CSubview -> FileBrowseObject

   Purpose:       FileBrowseObjects are objects which contain a title,
                  and a text box (NLineText)
                  Minimum, maximum and default value for the text box

   Contents:      FileBrowseObject()
                  FileBrowseObject::DoCommand
                  FileBrowseObject::GetValue
                  FileBrowseObject::GetTitle
                  FileBrowseObject::GetBoxTextSettings
                  FileBrowseObject::ResetBoxTextSettings


********************************************************************/

#ifndef FileBrowseObject_H
#define FileBrowseObject_H

#include CSubview_i

class NButton;
class MyNLineText;
class CText;

class FileBrowseObject : public CSubview
{
   public:

      FileBrowseObject(CSubview* theEnclosure,
                       const CRect& theRegion,
                       const char* theTitle = NULL,
                       const char* theDefaultFileName = NULL);

      virtual void DoCommand(long theCommand,void* theData);
      virtual const CStringRW GetTitle(void) const;
      virtual const CStringRW GetValue(void) const;
      void SetTitle(const char* theTitle);
      void SetValue(const char* theValue);

   private:

      NButton     * itsBrowseButton;
      MyNLineText * itsFileNameMyNLineText;
      CText       * itsTitleCText;

};

#endif



