/********************************************************************

   File Name:    selvar.hpp
   Author:       nlc
   Date:         01/27/95

   Purpose:  This class is used to display and/or select the
             variant and/or the extension used in the simulation.
             Users can use this tool for the selection or let the
             selection of other choices (like stands and components)
             force Suppose to make the selections.

   Contents:

     class SelectVariant
                                                                    *
 ********************************************************************/

#ifndef SelectVariant_H
#define SelectVariant_H

#include "cwwindow.hpp"
#include <rw/ordcltn.h>

class NButton;
class NListBox;
class NCheckBox;
class NListButton;
class CDocument;

class SelectVariant : public CWwindow
{
  public:

    SelectVariant ( CDocument * theDocument );
    virtual ~SelectVariant ( void );
    void SelectVariant::ISelectVariant( void );

    virtual void DoCommand(long theCommand,void* theData = NULL);

  private:

    NButton      * closeButton;

    NListButton  * variantListButton;
    NListButton  * programListButton;
    NListButton  * commandListButton;
    NButton      * extensionsLockButton;
    NListBox     * extensionsListBox;

    RWOrdered      variantsList;
    RWOrdered      programList;
    RWOrdered      commandList;
    RWOrdered      extensionsList;

    NCheckBox    * variantLocked;
    NCheckBox    * programLocked;
    NCheckBox    * commandLocked;

};

#endif
