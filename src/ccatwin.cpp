/*********************************************************************
 *

   File Name:   ccatwin.cpp
                                                                     *
 *********************************************************************/

#include "appdef.hpp"

#include NListBox_i
#include NButton_i
#include NText_i

#include CWindow_i
#include CDocument_i
#include CStringCollection_i
#include CStringRWC_i
#include CNavigator_i

#include "spparms.hpp"
 #include"spfuncs.hpp"
#include "spfunc2.hpp"
#include "extens.hpp"

#include "ccatwin.hpp"
#include "suppdefs.hpp"

#include "clearwin.hpp"
#include "seedwin.hpp"
#include "sheltwin.hpp"
#include "thinwin.hpp"
#include "cyear.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include "supphelp.hpp"    // for help #defines

extern XVT_HELP_INFO helpInfo;    // help file

CcatWin::CcatWin(CDocument  * theDocument,
                 const char * theWindowTitle,
                 const char * theMSText)
         : CWwindow(theDocument,
                    RightWinPlacement(CRect(20,40,608,356)),
                    theWindowTitle,
                    WSF_ICONIZABLE|WSF_SIZE|WSF_CLOSE|WSF_INVISIBLE,
                    W_DOC,TASK_MENUBAR),
           itsCategoryListBox(NULL),
           itsItemsListBox(NULL)
{
   // create and clear the navitagor

   CNavigator * aNavigator = GetNavigator();
   aNavigator->ClearTabStops();

   itsCategoryList = new RWOrdered((size_t) 15);

   // get the mstext section.

   itsNeededMSText = GetMSTextPtr(theMSText);

   // make sure the extension list is up to date.

   theSpGlobals->theExtensions->LoadListOfLegalExtensions();

   LoadTheCategoryList();

   if (itsCategoryList->entries()>1)
   {
      itsCategoryListBox = new NListBox(this, CRect(8,8,308,265),
                                        *itsCategoryList, NULL);
      itsCategoryListBox->SetGlue(TOPSTICKY | LEFTSTICKY | BOTTOMSTICKY);
      itsCategoryListBox->SetCommand(2);
      aNavigator->AppendTabStop(new CTabStop(itsCategoryListBox));
      RWOrdered tmp;
      itsItemsListBox = new NListBox(this,CRect(316,8,580,265),
                                     tmp, NULL);
   }
   else if (itsCategoryList->entries() == 1)
   {
      Size(RightWinPlacement(CRect(20,40,336,356)));
      Ccategory * aCcat = (Ccategory *) itsCategoryList->first();
      new NText(this, CRect(8,8,308,24), ((CStringRWC *)aCcat)->data(), 0l);
      itsItemsListBox = new NListBox(this,CRect(8,28,308,265),
                                     *aCcat->itemsInCategory, NULL);
   }
   else
   {
      Size(RightWinPlacement(CRect(20,40,336,356)));
      new NText(this, CRect(8,8,308,24), "Nothing to display", 0l);
      itsItemsListBox = new NListBox(this,CRect(8,28,308,265), NULL, NULL);
   }

   itsItemsListBox->SetCommand(3);
   itsItemsListBox->SetGlue(TOPSTICKY | LEFTSTICKY | RIGHTSTICKY
                            | BOTTOMSTICKY);
   aNavigator->AppendTabStop(new CTabStop(itsItemsListBox));

   CRect aRec = GetFrame();
   aRec.Top(276);
   aRec.Bottom(308);
   aRec.Left(((aRec.Right()-aRec.Left())/2)-50);
   aRec.Right(aRec.Left()+100);
   itsCloseButton = new NButton(this, aRec, "Close");
   itsCloseButton->SetGlue(BOTTOMSTICKY | LEFTSTICKY );
   itsCloseButton->SetCommand(1);
   aNavigator->AppendTabStop(new CTabStop(itsCloseButton));
   Show();
   aNavigator->InitFocus();

}



void CcatWin::DoCommand(long theCommand,void* theData)
{

   switch (theCommand)
   {
      case 1:                    // close button pushed
         Close();
         break;

      case 2:                    // element in category list box selected
      {
         int selected = itsCategoryListBox->GetSelectPosition();
         if (selected != -1)
         {
            Ccategory * aCcat = (Ccategory *)(*itsCategoryList)(selected);
            if (aCcat->itemsInCategory)
               itsItemsListBox->IListBox(*aCcat->itemsInCategory);
            else 
            {
               RWOrdered tmp;
               itsItemsListBox->IListBox(tmp);
            }
         }
         break;
      }

      case 3:                   // element in items list box selected
      {
         Ccategory * aCcat;
         if (itsCategoryListBox)
         {
            size_t selected = itsCategoryListBox->GetSelectPosition();
            aCcat = (Ccategory *)(*itsCategoryList)(selected);
         }
         else aCcat = (Ccategory *)itsCategoryList->first();
         if (aCcat->itemsInCategory)
         {
            int selectedItem = itsItemsListBox->GetSelectPosition();
            if (selectedItem != -1)
            {
               ItemInCcat * anItem = (ItemInCcat *) (*(aCcat->itemsInCategory))(selectedItem);
               BuildTheWin(anItem);
            }
         }
         break;
      }

      case EXTENSIONListChanged:
      {
         LoadTheCategoryList();
         ICcatWin();
         break;
      }

      default:
         CWwindow::DoCommand(theCommand,theData);

   }        // end switch(theCommand)
}           // end function



BOOLEAN CcatWin::Close( void )
{
   if (itsCategoryList)
   {
      itsCategoryList->clearAndDestroy();
      delete itsCategoryList;
   }
   return CWwindow::Close();
}



void CcatWin::BuildTheWin(ItemInCcat * anItem)
{
   xvt_dm_post_error("CcatWin::BuildTheWin is considerd a pure virtual function.");
}



void CcatWin::LoadTheCategoryList( void )
{

   // make sure the list is clear.

   itsCategoryList->clearAndDestroy();

   // if there is information available, then we can load itsCategoryList.

   if (itsNeededMSText)
   {
      char search[5];
      int catNumber = 1;
      PKeyData * fieldPKeyData;
      do
      {
         sprintf (search,"c%d", catNumber++);
         fieldPKeyData = itsNeededMSText->FindPKeyData(search);
         if (fieldPKeyData)
         {
            Ccategory * aCcat = new Ccategory(fieldPKeyData->pString.data());
            if (aCcat->itemsInCategory &&
                aCcat->itemsInCategory->entries()) itsCategoryList->
                                                   insert(aCcat);
            else delete aCcat;
         }
      } while (fieldPKeyData);
   }
   else
   {
      xvt_dm_post_error("Suppose can not find necessary "
                        "information in the parameters file.");
   }
}




void CcatWin::ICcatWin( void )
{
   // If these visual objects have not yet been created, then
   // don't do anything.

   if (!itsCategoryListBox ||
       !itsItemsListBox) return;

   // Reload the category list, and reset the list boxes.

   RWOrdered tmp;
   itsCategoryListBox->IListBox(tmp);
   itsItemsListBox->IListBox(tmp);

   LoadTheCategoryList();
   itsCategoryListBox->IListBox(*itsCategoryList);
}




RWDEFINE_COLLECTABLE(Ccategory,CCATEGORY)

Ccategory::Ccategory (const char * category)
          :CStringRWC ( category )
{
   strip(both);
   size_t numberOfItemsInCategory = (size_t) numberLines(data())-1;
   if (numberOfItemsInCategory)
   {
      itemsInCategory = new RWOrdered(numberOfItemsInCategory);
      RWCTokenizer nextline(*this);
      RWCString    aLine;
      BOOLEAN firstLine = TRUE;
      while (!(aLine=nextline("\n")).isNull())
      {
         if (firstLine)
         {
            replace(0,999,aLine);
            firstLine = FALSE;
         }
         else
         {
            ItemInCcat * anItem = new ItemInCcat(aLine.data());
            if (anItem->extensID && anItem->methodID)
            {
               if (strcmp(anItem->methodID->data(),"GPSB") == 0)
               {
                  if (anItem->mstextID) itemsInCategory->insert(anItem);
                  else delete anItem;
               }
               else itemsInCategory->insert(anItem);
            }
            else delete anItem;
         }
      }
   }
   else itemsInCategory = NULL;
}


Ccategory::Ccategory (Ccategory * category)
          :CStringRWC ((CStringRWC &) * category )
{
   itemsInCategory = NULL;
}

Ccategory::Ccategory ( void )
          :CStringRWC ()
{
   itemsInCategory = NULL;
}

Ccategory::~Ccategory ( void )
{
   if (itemsInCategory)
   {
      itemsInCategory->clearAndDestroy();
      delete itemsInCategory;
   }
}

RWDEFINE_COLLECTABLE(ItemInCcat,ITEMINCCAT)

ItemInCcat::ItemInCcat (const char * item)
           :CStringRWC ( item )
{
   strip(both);

   extensID = NULL;
   methodID = NULL;
   mstextID = NULL;

   RWCTokenizer nextPart(*this);
   RWCString    aPart;
   if ((aPart=nextPart(";")).isNull()) return;
   replace(0,999,aPart);
   if ((aPart=nextPart(" ;\n")).isNull()) return;
   aPart.strip(both);

   // find out if the extension is allowed.

   if (theSpGlobals->theExtensions->FindLegalExtension(aPart.data()) ==
       -1) return;

   extensID = new CStringRW(aPart);
   if ((aPart=nextPart(" ;\n")).isNull()) return;
   aPart.strip(both);
   methodID = new CStringRW(aPart);
   if ((aPart=nextPart(" ;\n")).isNull()) return;
   aPart.strip(both);
   mstextID = new CStringRW(aPart);
}


ItemInCcat::ItemInCcat (ItemInCcat * item)
          :CStringRWC ((CStringRWC &) * item )
{
   extensID = NULL;
   methodID = NULL;
   mstextID = NULL;
}

ItemInCcat::ItemInCcat ( void )
          :CStringRWC ()
{
   extensID = NULL;
   methodID = NULL;
   mstextID = NULL;
}

ItemInCcat::~ItemInCcat ( void )
{
   if (extensID) delete extensID;
   if (methodID) delete methodID;
   if (mstextID) delete mstextID;
}
