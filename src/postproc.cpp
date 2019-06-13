/********************************************************************

   File Name:     postproc.cpp
   Author:        nlc
   Date:          11/05/95

   Purpose:       see postproc.hpp

*********************************************************************/

#include "PwrDef.h"
#include CStringRWC_i
#include <rw/ordcltn.h>
#include <rw/vstream.h>  // for debugging.
#include "postproc.hpp"
#include "runstr.hpp"
#include "spglobal.hpp"
#include "spparms.hpp"
#include "suppdefs.hpp"


RWDEFINE_COLLECTABLE(PostProc,POSTPROC)

PostProc::PostProc ( const char * name )
         :CStringRWC ( name )

{
   command           = NULL;
   kwBaseStandFirst  = NULL;
   kwBaseStandOthers = NULL;
   kwBaseStandBottom = NULL;
   kwBasePPETop      = NULL;
   kwBasePPEStand    = NULL;
   kwBasePPEBottom   = NULL;
   description       = NULL;

   theMSTextName     = NULL;
   theMSText         = NULL;
}



PostProc::PostProc ( void )
         :CStringRWC ()

{
   command           = NULL;
   kwBaseStandFirst  = NULL;
   kwBaseStandOthers = NULL;
   kwBaseStandBottom = NULL;
   kwBasePPETop      = NULL;
   kwBasePPEStand    = NULL;
   kwBasePPEBottom   = NULL;
   description       = NULL;

   theMSTextName     = NULL;
   theMSText         = NULL;

}



PostProc::~PostProc ( void )
{
   if (command          ) delete command;
   if (kwBaseStandFirst ) delete kwBaseStandFirst;
   if (kwBaseStandOthers) delete kwBaseStandOthers;
   if (kwBaseStandBottom) delete kwBaseStandBottom;
   if (kwBasePPETop     ) delete kwBasePPETop;
   if (kwBasePPEStand   ) delete kwBasePPEStand;
   if (kwBasePPEBottom  ) delete kwBasePPEBottom;
   if (description      ) delete description;
   if (theMSTextName    ) delete theMSTextName;

}




RWBoolean PostProc::isEqual(const RWCollectable *c) const
{

   // two post processors are equal if they are at the same address OR
   // if their name is the same and their Master Section Text (MSText) is the same.

   return                this                ==                 c ||
        ((CStringRWC &) *this                == (CStringRWC &) *c &&
                        *this ->theMSTextName==   *((PostProc *)c)->theMSTextName);

}







