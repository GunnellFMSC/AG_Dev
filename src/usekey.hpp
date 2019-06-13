/********************************************************************

   File Name:    usekey.hpp
   Author:       nlc
   Date:         12/10/94

   Purpose:  This class controls the disply of the Use FVS Keywords
             window accessed by the Extras menu.

   Contents:

     class UseFVSKey
          ~UseFVSKey
           DoCommand
           IUseFVSKey
           LoadCategoryList    (private)
           LoadKeywordList     (private)
           LaunchKeywordWindow (private)

 ********************************************************************/

#ifndef UseFVSKey_H
#define UseFVSKey_H


class NButton;
class NListBox;
class RWOrdered;
class NText;
class NLineText;
class CNavigator;
class MyNLineText;

#include "cwwindow.hpp"

class UseFVSKey : public CWwindow
{
  public:

    UseFVSKey(CDocument *theDocument);

    virtual ~UseFVSKey( void );
    void IUseFVSKey( void );
    void UpdateUseFVSKey( void );

    virtual void DoCommand(long theCommand,void* theData=NULL);

  private:

    CNavigator        * itsNavigator;
    NButton           * itsCloseButton;
    NButton           * itsResetButton;
    NButton           * itsAcceptButton;
    NListBox          * itsExtenListBox;
    NListBox          * itsCategoryListBox;
    NListBox          * itsKeywordListBox;
    RWOrdered           itsExtenList;
    RWOrdered           itsCategoryList;
    RWOrdered           itsKeywordList;
    NText             * itsSelectedKeywordDefinitionText;
    MyNLineText       * itsSelectKeywordEdit;
    int                 itsNumberExtensions;
    int                 itsNumberCategories;
    int                 itsNumberKeywords;
    int                 itsNumberKeywordsInPtrList;
    int                 itsNumberCategoriesInPtrList;
    BOOLEAN             itsInIUseFVSKey;

    typedef PKeyData  * ptrToPKeyData;
    ptrToPKeyData     * itsListOfCategoryPKeyData;
    ptrToPKeyData     * itsListOfKeywordsPKeyData;

    void LoadKeywordList (const char * extension, const char * category);
    void LoadCategoryList ( void );
    void LaunchKeywordWindow(int theSelectedExtension, 
                             int theSelectedKeyword,
                             const char * selectedKeywordString);
};



#endif
