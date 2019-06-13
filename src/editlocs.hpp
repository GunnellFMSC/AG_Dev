/********************************************************************

   File Name:    editlocs.hpp
   Author:       nlc
   Date:         04/11/97 & 10/2006

   Purpose:  This class is used to display, edit, or both, the
             records in a locations file.

   Contents:

     class EditLocationWin
     class EditLocation
                                                                    *
 ********************************************************************/

#ifndef EditLocationWin_H
#define EditLocationWin_H

#include "cwwindow.hpp"
#include <rw/ordcltn.h>

class NButton;
class NText;
class NLineText;
class CDocument;
class MyNLineText;
class CRadioGroup;
class NRadioButton;
class RWOrdered;

class EditLocationWin : public CWwindow
{
   public:

      EditLocationWin ( CDocument * theDocument );
      virtual ~EditLocationWin ( void );
      virtual void DoCommand(long theCommand, void* theData = NULL);

   private:

      NButton      * itsOkButton;
      NButton      * itsCancelButton;
      NButton      * itsSaveButton;
      NButton      * itsSaveAsButton;

      MyNLineText  * itsLocsFileNameNLineText;
      NButton      * itsBrowseButton;

      CRadioGroup  * itsABRadioGroup;
      long           itsTypeA;
      long           itsTypeB;
      long           itsTypeC;
      NRadioButton * itsTypeAButton;
      NRadioButton * itsTypeBButton;
      NRadioButton * itsTypeCButton;

      NText        * itsRecordCountNText;
      NButton      * itsPreviousButton;
      NButton      * itsNextButton;
      NButton      * itsNewButton;
      NButton      * itsDeleteButton;
      NButton      * itsDuplicateButton;
      NText        * itsLocNameNText;
      NLineText    * itsLocNameNLineText;
      NText        * itsSLFNameNText;
      MyNLineText  * itsSLFNLineText;
      NButton      * itsSLFBrowseButton;
      NText        * itsGroupCodeNText;
      MyNLineText  * itsGroupCodeNLineText;
      NText        * itsAddFileNText;
      NLineText    * itsAddFileNLineText;
      NButton      * itsAddFileBrowseButton;

      NText        * itsDataSourceNText;
      MyNLineText  * itsDataSourceNLineText;
      NButton      * itsDataSourceBrowseButton;
      NText        * itsUserNameNText;
      MyNLineText  * itsUserNameNLineText;
      NText        * itsPasswordNText;
      MyNLineText  * itsPasswordNLineText;

      int            itsEditingType;
      size_t         itsCurrentTypeA;
      size_t         itsCurrentTypeB;
      size_t         itsCurrentTypeC;
      RWOrdered      itsTypeARecords;
      RWOrdered      itsTypeBRecords;
      RWOrdered      itsTypeCRecords;

      void StoreCurrent( void );
      int  WriteFile( void );
      void ReadFile( void );
      void DisplayRecord( void );

};



class LocTypeARec : public CStringRWC
{

   RWDECLARE_COLLECTABLE(LocTypeARec)

   public:

      LocTypeARec ( char  * record );
      LocTypeARec ( LocTypeARec * c);   // copy constructor
      LocTypeARec ( void );
      virtual RWBoolean isEqual(const RWCollectable * t) const;

   private:

      CStringRW SLFName;
      CStringRW DSN;

   friend EditLocationWin;

};



class LocTypeBRec : public CStringRWC
{

   RWDECLARE_COLLECTABLE(LocTypeBRec)

   public:

      LocTypeBRec ( char  * record);
      LocTypeBRec ( LocTypeBRec * c);   // copy constructor
      LocTypeBRec ( void );
      virtual RWBoolean isEqual(const RWCollectable * t) const;

   private:

      CStringRW addFiles;

   friend EditLocationWin;

};

class LocTypeCRec : public CStringRWC
{

   RWDECLARE_COLLECTABLE(LocTypeCRec)

   public:

      LocTypeCRec ( char  * record);
      LocTypeCRec ( LocTypeCRec * c);   // copy constructor
      LocTypeCRec ( void );
      virtual RWBoolean isEqual(const RWCollectable * t) const;

   private:

      CStringRW dataSourceName;
      CStringRW userName;
      CStringRW passWord;


   friend EditLocationWin;

};


#endif









