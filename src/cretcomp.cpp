/********************************************************************

   File Name:     cretcomp.cpp
   Author:        kfd & nlc
   Date:          01/30/95


   Contents:      CreateComponent

   Creates a component

********************************************************************/

#include "cretcomp.hpp"
#include "compon.hpp"

Component * CreateComponent(const char* itsName,
                            const char* itsScreen,
                            const char* itsKeywords,
                            const char* itsCParms,
                            const char* itsComment,
                            const char* itsExtension,
                            int         itsCType,
                            const char* itsVariant)
{
/*****
 *
    CreateComponent creates a component and returns a pointer to the
    object.  

    NOTE: All this should be done with a constructor...I'm not sure 
    why I did it this way, but it is done for now.

    itsName      (input)  The component name

    itsScreen    (input)  The screen name used to create the component

    itsKeywords  (input)  The keyword string

    itsCParms    (input)  The string of parameter data used to recreate
                          the screen and set its values.

    itsComment   (input)  A comment string.

    itsCType     (input)  A component type, see compon.hpp

    itsExtension (input)  The abbreviation for the extension name

    itsVariant   (input)  The abbreviation for the variant name

*
*****/

   Component * itsComponent = new Component(itsName);

   if (itsScreen)  itsComponent->sourceWindow = new CStringRW (itsScreen);
   else            itsComponent->sourceWindow = new CStringRW ("");
   if (itsKeywords)itsComponent->keywords     = new CStringRW (itsKeywords);
   else            itsComponent->keywords     = new CStringRW ("");
   if (itsCParms)  itsComponent->windowParms  = new CStringRW (itsCParms);
   else            itsComponent->windowParms  = new CStringRW ("");
   if (itsComment && strlen(itsComment))   
                     itsComponent->comment   = new CStringRW (itsComment);
   if (itsExtension) itsComponent->extension = new CStringRW (itsExtension);
   if (itsVariant)   itsComponent->variant   = new CStringRW (itsVariant);
   itsComponent->componentType = itsCType;

   return itsComponent;

}
