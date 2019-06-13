/********************************************************************

   File Name:     compon.hpp
   Author:        nlc
   Date:          05/30/95

   Purpose:       A component is a set of keywords and an associated
                  window used to create or edit the component (in general).
                  Components are used to hold the data that represents
                  some management activity (in general).

   Class membership notes:

   The Component class has the following membership.  The PPEComponent
   class is exactly the same except that it isA(PPECOMPONENT) and the
   componentType variable as a different meaning.

   componentType
      specifies what kind of component this is using the following
      integer codes...when the component isA(COMPONENT):
      -1  or 0 undefined component, component is processed as if a type 2.
       1 "IF" keyword set
       2 keyword set, outside an IF.
       3 keyword set, inside an IF.
      when the component isA(PPECOMPONENT):
       4  the keyword is a member of topPPEComponents
       5  the keyword is a member of botPPEComponents
       6  the keyword is a member of reportPPEComponents

   componentNumber
       a unique serial number.  the string name and the number of each
       component must be unique.

   conditionSerialNum
       the unique serial number of the component that acts as this components.
       condition, when componentType is 3.

   comment
       a comment string for the component (if NULL, then there is none).

   userEditedComment
       an interger, defined if comment is non-null, if defined 1, then the
       user edited the comment, of 0, the user did not edit the comment.

   sourceWindow
       the name of the component source window (NULL for untitled components).

   windowParms
       the parameters string for the source window.

   keywords
       the keywords themselves

   variant
       the variant onwhich the component is based

   extension
       the extension onwhich the component is based (if NULL, then base).

   openedWindow
       a pointer to the CWindow being used to edit and/or create the
       component.  NULL if the component is not being edited.

*
*********************************************************************/

#ifndef Component_h
#define Component_h

#include "PwrDef.h"
#include CStringRWC_i
#include "spglobal.hpp"
#include "runstr.hpp"

class RWOrdered;
class CStringRW;
class Stand;
class Group;
class CWindow;

class Component : public CStringRWC
{

   RWDECLARE_COLLECTABLE(Component)

   public:

      Component ( const char * name,
                  int CNumber = theSpGlobals->runStream->componentNumbers);
      Component ( void );
      Component ( Component * c); // copy constructor
      virtual ~Component ( void );

      int WriteComponent( Component * lastComponent,
                          Group * group, FILE * filePtr,
                          int specialWriteAppend = 0,
                          const char * tag = NULL);
      int ReadComponent ( Stand * stand, FILE * filePtr,
                          CStringRW * Tbuffptr, char * buff );

      virtual void SetComponent ( Component * c, int all=FALSE);

      virtual RWBoolean Component::isEqual(const RWCollectable *c) const;

      int           componentType;
      int           componentNumber;
      int           conditionSerialNum;
      int           userEditedComment;
      CStringRW   * comment;
      CStringRW   * sourceWindow;
      CStringRW   * windowParms;
      CStringRW   * keywords;
      CStringRW   * variant;
      CStringRW   * extension;
      CWindow     * openedWindow;
};


class PPEComponent : public Component
{

   RWDECLARE_COLLECTABLE(PPEComponent)

      PPEComponent ( const char * name,
                     int CNumber = theSpGlobals->runStream->componentNumbers);
      PPEComponent ( void );
      PPEComponent ( PPEComponent * c); // copy constructor
      PPEComponent ( Component * c); // copy constructor
      virtual ~PPEComponent ( void );
      int WriteComponent( Component * lastComponent,
                          Group * group, FILE * filePtr,
                          int specialWriteAppend = 0,
                          const char * tag = NULL);
      int ReadComponent ( FILE * filePtr,
                          CStringRW * Tbuffptr, char * buff);

};

#endif //Component_h







