/********************************************************************

   File Name:     postproc.hpp
   Author:        nlc
   Date:          11/05/95

   Purpose:       A postProc is a class that defines all necessary
                  elements of a Post Processor.

   Class membership notes:

*
*********************************************************************/

#ifndef PostProc_h
#define PostProc_h

#include "PwrDef.h"
#include CStringRWC_i
#include "spglobal.hpp"
#include "runstr.hpp"

class RWOrdered;
class CStringRW;
class MSText;

class PostProc : public CStringRWC
{

   RWDECLARE_COLLECTABLE(PostProc)

   public:

      PostProc ( const char * name );
      PostProc ( void );
      virtual ~PostProc ( void );

      virtual RWBoolean isEqual(const RWCollectable *c) const;

      CStringRW      * command;
      CStringRW      * kwBaseStandFirst;
      CStringRW      * kwBaseStandOthers;
      CStringRW      * kwBaseStandBottom;
      CStringRW      * kwBasePPETop;
      CStringRW      * kwBasePPEStand;
      CStringRW      * kwBasePPEBottom;
      CStringRW      * description;

      CStringRW      * theMSTextName;
      MSText         * theMSText;
};

#endif //PostProc_h
