/********************************************************************
 *

   File Name:           ccatwin.hpp
   Author:              nlc
   Date:                08/20/96

   Class:               CcatWin
   Inheritance:         CWwindow->CcatWin

   This class is used to build a list of "categories" followed by a
   list of items with the categories.  It is an abstract class.  For
   an example of its use, see the management actions window.

                                                                   *
 ********************************************************************/

#ifndef CcatWin_H
#define CcatWin_H

#include "PwrDef.h"
#include "appdef.hpp"
#include CStringRWC_i
#include "cwwindow.hpp"

class RWOrdered;
class NButton;
class NListBox;
class ItemInCcat;
class MSText;

class CcatWin : public CWwindow
{
   public:

      CcatWin(CDocument  * theDocument,
              const char * theWindowTitle,
              const char * theMSText);

      virtual void DoCommand(long theCommand,void* theData=NULL);
      virtual BOOLEAN Close( void );
      virtual void BuildTheWin(ItemInCcat * anItem);
      virtual void LoadTheCategoryList();
      virtual void ICcatWin( void );

   private:

      MSText    * itsNeededMSText;
      RWOrdered * itsCategoryList;
      RWOrdered * itsItemsList;
      NButton   * itsCloseButton;
      NListBox  * itsCategoryListBox;
      NListBox  * itsItemsListBox;
};


class Ccategory : public CStringRWC
{

   RWDECLARE_COLLECTABLE(Ccategory)

   public:

      Ccategory (const char * category);
      Ccategory (Ccategory  * category);
      Ccategory (void);
      virtual ~Ccategory (void);

      RWOrdered * itemsInCategory;
};

class ItemInCcat : public CStringRWC
{

   RWDECLARE_COLLECTABLE(ItemInCcat)

   public:

      ItemInCcat (const char * item);
      ItemInCcat (ItemInCcat * item);
      ItemInCcat (void);
      virtual ~ItemInCcat (void);

      CStringRW * extensID;
      CStringRW * methodID;
      CStringRW * mstextID;
};


#endif //CcatWin_H


