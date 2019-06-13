/*********************************************************************

   File Name:    usekey.cpp
   Author:       nlc
   Date:         12/12/94 redraft 01/29/97

   see usekey.hpp for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include NListBox_i
#include NText_i
#include NButton_i
#include NEditControl_i

#include CStringCollection_i
#include CNavigator_i
#include CUnits_i
#include CView_i
#include CWindow_i
#include CDocument_i
#include CStringRWC_i
#include CBoss_i
#include CApplication_i
#include CDesktop_i

#include "mylinetx.hpp"
#include "spparms.hpp"
#include "suppdefs.hpp"
#include "fvslocs.hpp"
#include "extens.hpp"
#include "gpkey.hpp"
#include "warn_dlg.hpp"
#include "spfunc2.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>

#include "usekey.hpp"

#include "suppdefs.hpp"
#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

UseFVSKey::UseFVSKey(CDocument *theDocument)
          :CWwindow(theDocument, RightWinPlacement(CRect(8,25,615,413)), "Use FVS Keywords",
                    WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                    W_DOC,TASK_MENUBAR)
{

   // initialize.

   itsInIUseFVSKey = FALSE;

   itsListOfCategoryPKeyData   = NULL;
   itsListOfKeywordsPKeyData   = NULL;

                                                     // Close
   itsCloseButton = new NButton(this, CRect(464,348,600,380), "Close");
   itsCloseButton->SetCommand(1);
   itsCloseButton->SetGlue(BOTTOMSTICKY);

                                                     // Reset
   itsResetButton = new NButton(this, CRect(228,348,364,380), "Reset");
   itsResetButton->SetCommand(2);
   itsResetButton->SetGlue(BOTTOMSTICKY);

                                                     // Accept
   itsAcceptButton = new NButton(this, CRect(8,348,144,380), "Accept");
   itsAcceptButton->SetCommand(3);
   itsAcceptButton->SetGlue(BOTTOMSTICKY);


                                                     // Extension
   new NText(this, CRect(8,8,88,24), "Extension", 0L);
   itsExtenListBox = new NListBox(this, CRect(8,32,248,256),
                               itsExtenList, NULL);
   itsExtenListBox->SetCommand(4);
   itsExtenListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY | LEFTSTICKY);

                                                     // Category
   new NText(this, CRect(256,8,336,24), "Category", 0L);
   itsCategoryListBox = new NListBox(this, CRect(256,32,472,256),
                         itsCategoryList, NULL);

   itsCategoryListBox->SetCommand(5);
   itsCategoryListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY |
                               RIGHTSTICKY  | LEFTSTICKY);

                                                     // Keyword

   NText * text = new NText(this, CRect(488,8,568,24), "Keyword", 0L);
   text->SetGlue(TOPSTICKY | RIGHTSTICKY);
   itsKeywordListBox = new NListBox(this, CRect(488,32,600,256),
                                 itsKeywordList, NULL);

   itsKeywordListBox->SetCommand(6);
   itsKeywordListBox->SetDoubleCommand(3);
   itsKeywordListBox->SetGlue(BOTTOMSTICKY | TOPSTICKY | RIGHTSTICKY);

                                                     // Select Keyword
   text = new NText(this, CRect(360,272,472,292),
                            "Select Keyword", 0L);
   text->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);

   itsSelectKeywordEdit = new MyNLineText(this, CPoint(488,264),
                                          (UNITS) 96, 7, 0);
   itsSelectKeywordEdit->SetGlue(BOTTOMSTICKY | RIGHTSTICKY);

                                                 // Selected Keyword Definition
   itsSelectedKeywordDefinitionText =
      new NText(this, CRect(8,304,600,340), "", 0L);
   itsSelectedKeywordDefinitionText->SetGlue(BOTTOMSTICKY | LEFTSTICKY);

   // Set up the linkage to help.

   xvt_help_set_win_assoc (helpInfo, GetXVTWindow(), USEKEYWindow, 0L);

   Show();

   // set up keyborad navigation

   itsNavigator = GetNavigator();
   itsNavigator->ClearTabStops();
   itsNavigator->AppendTabStop(new CTabStop(itsSelectKeywordEdit));
   itsNavigator->AppendTabStop(new CTabStop(itsAcceptButton));
   itsNavigator->AppendTabStop(new CTabStop(itsCloseButton));
   itsNavigator->AppendTabStop(new CTabStop(itsResetButton));
   itsNavigator->AppendTabStop(new CTabStop(itsExtenListBox));
   itsNavigator->AppendTabStop(new CTabStop(itsCategoryListBox));
   itsNavigator->AppendTabStop(new CTabStop(itsKeywordListBox));

   IUseFVSKey();  // call the list initializer.
}




void UseFVSKey::DoCommand(long theCommand,void* theData)
{

   int theSelectedKeyword   = -1;
   int theSelectedExtension =  0;
   int theSelectedCategory  =  0;

   switch (theCommand)
   {
      case 1:                         // Close button pushed

         Close();
         break;


      case 2:                         // Reset button pushed
         IUseFVSKey();
         break;


      case 3:                         // Accept button pushed
      {
         theSelectedExtension = itsExtenListBox->GetSelectPosition();
         theSelectedKeyword = itsKeywordListBox->GetSelectPosition();
         CStringRW desiredKeyword = itsSelectKeywordEdit->GetText();

         CStringRW candidateKeyword;

         if (theSelectedKeyword != -1)
         {
            candidateKeyword = *itsListOfKeywordsPKeyData[theSelectedKeyword];
            if (candidateKeyword.compareTo(desiredKeyword,
                                           RWCString::ignoreCase)
                != 0) theSelectedKeyword = -1;
         }
         if (theSelectedKeyword == -1)
         {
            LoadKeywordList (theSpGlobals->theExtensions->
                             listOfExtensionsPKeyData[theSelectedExtension]->
                             data(),  NULL);
            LoadCategoryList();

            for (int ikeys=0; ikeys<itsNumberKeywordsInPtrList; ikeys++)
            {
               candidateKeyword = *itsListOfKeywordsPKeyData[ikeys];
               if (candidateKeyword.compareTo(desiredKeyword,
                                              RWCString::ignoreCase) == 0)
               {
                  theSelectedKeyword=ikeys;
                  itsKeywordListBox->SelectItem(theSelectedKeyword);
                  DoCommand ( 6, this);
                  desiredKeyword = itsSelectKeywordEdit->GetText();
                  continue;
               }
            }
            if (theSelectedKeyword == -1)
            {
               xvt_dm_post_warning("Keyword \"%s\" not found.",
                                   desiredKeyword.data());
               break;
            }
         }

         LaunchKeywordWindow(theSelectedExtension, theSelectedKeyword,
                             desiredKeyword.data());
         break;
      }

      case 4:                         // Extension List Selection

         theSelectedExtension = itsExtenListBox->GetSelectPosition();

         // set the label to initial state.

         itsSelectedKeywordDefinitionText->SetTitle("No keyword picked");

         LoadKeywordList (theSpGlobals->theExtensions->
            listOfExtensionsPKeyData[theSelectedExtension]->data(),  NULL);
         LoadCategoryList();

         break;


      case 5:                         // Category List Selection
      {
         theSelectedCategory  = itsCategoryListBox->GetSelectPosition();

         if (theSelectedCategory < 0 ||
             theSelectedCategory >= itsCategoryList.entries()) break;

         theSelectedExtension = itsExtenListBox->GetSelectPosition();

         if (theSelectedExtension < 0 ||
             theSelectedExtension >= itsExtenList.entries()) break;

         // set the label to initial state.

         itsSelectedKeywordDefinitionText->SetTitle("No keyword picked");

         if (theSelectedCategory == 0)
         {
            LoadKeywordList (theSpGlobals->theExtensions->
               listOfExtensionsPKeyData[theSelectedExtension]->data(),
                             NULL);
         }
         else
         {
            LoadKeywordList (theSpGlobals->theExtensions->
                  listOfExtensionsPKeyData[theSelectedExtension] ->data(),
               itsListOfCategoryPKeyData  [theSelectedCategory-1]->data());
         }

         break;
      }

      case 6:                         // Keyword List Selection
      {
         theSelectedKeyword = itsKeywordListBox->GetSelectPosition();

         if (theSelectedKeyword < 0 ||
             theSelectedKeyword >= itsKeywordList.entries()) break;

         // load the keyword edit control with the selected keyword

         CStringRW tmp = (CStringRWC &) *itsKeywordList[theSelectedKeyword];
         itsSelectKeywordEdit->SetText(tmp);

         // display the selected keyword definition

         itsSelectedKeywordDefinitionText->
           SetTitle(itsListOfKeywordsPKeyData[theSelectedKeyword]->pString);
         break;
      }

      case 7:          // NewLine entered in keyword edit field.
      {
         itsKeywordListBox->DeselectAll();
         DoCommand (3, this);
         break;
      }

      case EXTENSIONListChanged:

         if (!itsInIUseFVSKey)
            UpdateUseFVSKey();
         break;


      default:
         CWwindow::DoCommand(theCommand,theData);

   }
}





UseFVSKey::~UseFVSKey( void )
{
   if (itsListOfCategoryPKeyData)   delete []itsListOfCategoryPKeyData;
   if (itsListOfKeywordsPKeyData)   delete []itsListOfKeywordsPKeyData;
   itsExtenList.clearAndDestroy();
   itsCategoryList.clearAndDestroy();
}




void UseFVSKey::IUseFVSKey( void )
{
   itsInIUseFVSKey = TRUE;

   // set the label to initial state.

   xvt_vobj_set_title(itsSelectedKeywordDefinitionText->GetXVTWindow(),
                      "No keyword picked");

   // make sure the extension list is empty.

   if (itsExtenList.entries())
   {
      itsExtenList.clearAndDestroy();
      itsExtenListBox->IListBox(itsExtenList);
   }

   // load the extension list.

   theSpGlobals->theExtensions->LoadExtensionCollection(&itsExtenList);

   // load itsExtenList into the list box.

   itsExtenListBox->IListBox(itsExtenList);

   itsExtenListBox->SelectItem(theSpGlobals->theExtensions->
      FindLegalExtension("base"));

   LoadKeywordList ("base", NULL);

   LoadCategoryList();

   itsNavigator->ActivateTabStop(itsNavigator->
                                 FindTabStopHavingView(itsSelectKeywordEdit));

   itsInIUseFVSKey = FALSE;
}


void UseFVSKey::UpdateUseFVSKey( void )
{

   // remember the currently selected extension, category, and keyword.

   CStringRW extenName;
   int theSelectedExtension = itsExtenListBox->GetSelectPosition();
   if (theSelectedExtension >= 0 &&
       theSelectedExtension < itsExtenList.entries())
      extenName = (CStringRWC&) *itsExtenList.at(theSelectedExtension);

   CStringRW catName;
   int theSelectedCategory = itsCategoryListBox->GetSelectPosition();
   if (theSelectedCategory >= 0 &&
       theSelectedCategory < itsCategoryList.entries())
      catName = (CStringRWC&) *itsCategoryList.at(theSelectedCategory);;

   CStringRW keyName;
   int theSelectedKeyword = itsKeywordListBox->GetSelectPosition();
   if (theSelectedKeyword >= 0 &&
       theSelectedKeyword < itsKeywordList.entries())
      keyName = (CStringRWC&) *itsKeywordList.at(theSelectedKeyword);

   // reinitialize the window

   IUseFVSKey();

   // reselect the extension, category, and then keyword.

   if (extenName.length())
   {
      for (theSelectedExtension = 0;
           theSelectedExtension < itsExtenList.entries();
           theSelectedExtension++)
      {
         if ((CStringRWC&) *itsExtenList.at(theSelectedExtension) ==
             extenName)
         {
            itsExtenListBox->SelectItem(theSelectedExtension);
            DoCommand(4);
            extenName = NULLString;
            break;
         }
      }
   }
   if (extenName.length())
   {
      itsSelectKeywordEdit->Clear();
      itsKeywordListBox->DeselectAll();
      return;
   }

   if (catName.length())
   {
      for (theSelectedCategory = 0;
           theSelectedCategory < itsCategoryList.entries();
           theSelectedCategory++)
      {

         if ((CStringRWC&) *itsCategoryList.at(theSelectedCategory) ==
             catName)
         {
            itsCategoryListBox->SelectItem(theSelectedCategory);
            DoCommand(5);
            catName = NULLString;
            break;
         }
      }
   }
   if (catName.length())
   {
      itsSelectKeywordEdit->Clear();
      itsKeywordListBox->DeselectAll();
      return;
   }

   if (keyName.length())
   {
      for (theSelectedKeyword = 0;
           theSelectedKeyword < itsKeywordList.entries();
           theSelectedKeyword++)
      {
         if ((CStringRWC&) *itsKeywordList.at(theSelectedKeyword) ==
             keyName)
         {
            itsKeywordListBox->SelectItem(theSelectedKeyword);
            DoCommand(6);
            keyName = NULLString;
            break;
         }
      }
   }
   if (keyName.length())
   {
      itsSelectKeywordEdit->Clear();
      itsKeywordListBox->DeselectAll();
      return;
   }
}





void UseFVSKey::LoadCategoryList ( void )
{
/****
 *
 *    Load the category list.
 *
 ****/

   // clear out the list.

   if (itsListOfCategoryPKeyData)
   {
      delete []itsListOfCategoryPKeyData;
      itsListOfCategoryPKeyData = NULL;
   }

   if (itsCategoryList.entries())
   {
      itsCategoryList.clearAndDestroy();
      itsCategoryListBox->IListBox(itsCategoryList);
   }

   CStringRWC categoryString = "keyword_categories";

   itsNumberCategories = 0;
   itsNumberCategoriesInPtrList=0;

   MSText * neededMSTextPointer =  (MSText *) theSpGlobals->
         theParmsData->find(&categoryString);
   RWOrdered * categoryPKeyData;
   if (neededMSTextPointer)
   {
     categoryPKeyData = neededMSTextPointer->GetThePKeys();
     itsNumberCategories = categoryPKeyData->entries();
   }

   // insure that the category "All keywords" is in the list
   // as the first item (done by default)

   itsCategoryList.insert(new CStringRWC("All keywords"));

   // if the itsNumber of categories is not null, then
   if (itsNumberCategories > 0)
   {

      // allocate memory for pointers to categories
      // and load the pointers to the category PKeydata.

      itsListOfCategoryPKeyData = new ptrToPKeyData [itsNumberCategories];

      RWOrderedIterator nextCategory(*categoryPKeyData);
      PKeyData * oneCategory;
      while (oneCategory = (PKeyData *) nextCategory())
      {
         // if the category is associated with the any of the keywords
         // being used in the list of keywords, then add it to the
         // list of categories.

         for (int key=0; key<itsNumberKeywordsInPtrList; key++)
         {
            if(itsListOfKeywordsPKeyData[key]->aTList.contains(oneCategory))
            {
               itsListOfCategoryPKeyData[itsNumberCategoriesInPtrList++] =
                 oneCategory;

               // add the name to itsCategoryList.

               itsCategoryList.insert(new CStringRWC(oneCategory->pString));
               break;
            }
         }
      }
      // load itsCategoryList into the list box.

      itsCategoryListBox->IListBox(itsCategoryList);
      itsCategoryListBox->SelectItem(0);
   }
}





void UseFVSKey::LoadKeywordList (const char * extension,
                                 const char * category)
{
/****
 *
 *    Load the keyword list for the "extension" and "category".
 *
 ****/

   // make sure the keyword list is empty.

   if (itsListOfKeywordsPKeyData)
   {
      delete []itsListOfKeywordsPKeyData;
      itsListOfKeywordsPKeyData = NULL;
   }

   if (itsKeywordList.entries())
   {
      itsKeywordList.clearAndDestroy();
      itsKeywordListBox ->IListBox(itsKeywordList);
   }

   itsNumberKeywords = 0;
   itsNumberKeywordsInPtrList=0;

   CStringRWC keywordString = "keyword_list";

   MSText * neededMSTextPointer =  (MSText *) theSpGlobals->
     theParmsData->find(&keywordString);
   RWOrdered * keyPKeySet;
   if (neededMSTextPointer)
   {
     keyPKeySet = neededMSTextPointer->GetThePKeys();
     itsNumberKeywords = keyPKeySet->entries();
   }

   // if the number of keywords is zero, then we are done.

   if (itsNumberKeywords <= 0) return;

   // otherwise, allocate memory for pointers to keywords
   // and load the pointers to the keyword PKeydata.

   itsListOfKeywordsPKeyData = new ptrToPKeyData [itsNumberKeywords];

   CStringRWC extensionString;
   if (extension) extensionString = extension;
   CStringRWC categoryString;
   if (category)  categoryString  = category;

   RWOrderedIterator nextKeyword(*keyPKeySet);
   PKeyData * keywordData;
   while (keywordData = (PKeyData *) nextKeyword())
   {
      // if the keyword is associated with the current extension, then
      // keep a pointer to the PKeydata.

      if (keywordData->aTList.contains(&extensionString))
      {
         if (categoryString.length() > 0)
         {
            if (keywordData->aTList.contains(&categoryString))
            {
               itsListOfKeywordsPKeyData[itsNumberKeywordsInPtrList++] =
                 keywordData;

               // add the name into itsKeywordList.

               itsKeywordList.insert(new CStringRWC(*keywordData));
            }
         }
         else
         {
            itsListOfKeywordsPKeyData[itsNumberKeywordsInPtrList++] =
               keywordData;
            itsKeywordList.insert(new CStringRWC(*keywordData));
         }
      }
   }

   // load the keywords into the keyword box.
   itsKeywordListBox->IListBox(itsKeywordList);
}





void UseFVSKey::LaunchKeywordWindow(int theSelectedExtension,
                                    int theSelectedKeyword,
                                    const char * theSelectedKeywordString)
{

/****
 *
 *    Figure out which window to launch for the keyword...and "do it"
 *
 ****/

   // save pointers to the extension and keyword PKeyData.

   PKeyData * curExtension = theSpGlobals->theExtensions->
              listOfExtensionsPKeyData[theSelectedExtension];
   PKeyData * curKeyword = NULL;
   if (theSelectedKeyword > -1)
      curKeyword = itsListOfKeywordsPKeyData[theSelectedKeyword];

   MSText * keywordMSTextPointer =  (MSText *) NULL;

   // here is a list of keywords that have "custom" windows.
   // This list (may/will) change as Suppose progresses.
   // The GPKeyword screen is used by any keyword that does not have
   // an entry for a custom window but does have an entry in the
   // parameters file.

   // If a keyword does not have a custom window, nor an entry in
   // the parameters file, then a free-form keyword edit is started.

   // the form of the list is exten.keyword

   char * haveCustomWindows[] =
   {
      "base.Comment",
      NULL
   };

   // see if the selected keyword has a custom window.

   int i, fireCase;
   fireCase = -1;
   CStringRW tmpString;

   tmpString = *curExtension;
   tmpString +=".";
   tmpString +=theSelectedKeywordString;

   for (i = 0; haveCustomWindows[i] != NULL; i++)
   {
      if (strcmp(haveCustomWindows[i],tmpString.data()) == 0)
      {
         fireCase = i;
         break;
      }
   }

   switch (++fireCase)
   {

      case 1:                         // launch the Debug window.

         Warn ("The Comment keyword is not yet supported");
         break;

      default:                        // *NO* custom window.
      {
         // Find out if there is info in the paramters file.

         CStringRWC searchMSText = "keyword.";
         searchMSText += *curExtension;
         searchMSText += ".";

         if (theSelectedKeyword > -1)
            searchMSText += *curKeyword;
         else
            searchMSText += theSelectedKeywordString;

         keywordMSTextPointer =  (MSText *) theSpGlobals->
               theParmsData->find(&searchMSText);

         // Special case: if the extension is "strp" or "estb" and if
         // the keywordMSTextPointer is NULL, then try looking for
         // estbstrp as the extension.

         if (!keywordMSTextPointer &&
             strstr("estbstrp", curExtension->data()))
         {
            searchMSText = "keyword.estbstrp.";
            if (theSelectedKeyword > -1)
               searchMSText += *curKeyword;
            else
               searchMSText += theSelectedKeywordString;

            keywordMSTextPointer =  (MSText *) theSpGlobals->
                  theParmsData->find(&searchMSText);
         }

         // create a title for the window.

         tmpString  = curExtension->pString;
         tmpString += ": ";
         tmpString += theSelectedKeywordString;

         // if there is no PKeyData for the keyword, call the free-form
         // keyword entry window.

         if (keywordMSTextPointer == NULL)
         {

           Warn (tmpString);

           // ++++++++++  do later ++++++++++++
         }
         else
         {
                                      // launch the GPKeyword window.

            new GPKeyword (tmpString, keywordMSTextPointer,
                           *curExtension, NULL);
         }

         break;
      }
   }
}
