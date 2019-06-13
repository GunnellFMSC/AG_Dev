/********************************************************************

   File Name:     spparms.hpp
   Author:        nlc
   Date:          11/28/94 and 04/02/96
                  01/16/97 complete rewrite, replace the SW classes.

   Purpose:       The Suppose parameter file (suppose.prm) is read and
                  made available for use by other objects.

   Contents:
            class SpParms: public RWBinaryTree
                        SpParms
                       ~SpParms

            class MSText: public CStringRWC
                        MSText
                       ~MSText
                        FreeThePKeys (added 04/02/96).
                        FindPKeyData
                        compareTo (over ride from RWCollectable)
                        isEqual   (over ride from RWCollectable)

            class PKeyData: public CStringRWC
                        PKeyData
                       ~PKeyData
                        compareTo (over ride from RWCollectable)
                        isEqual   (over ride from RWCollectable)

   Usage:
       SpParms is created by passing the name of a suppose
       parms file.  The file is read and an instance of SpParms
       is created.  For each major secton in the parms file, an
       instance of MSText is created.  MSText contains all the data
       available for a given major section.

       When a given MSText is created, the parms data is not
       necessarily loaded.  But, the location data necessary to find
       and load the data are stored.  When any of the data are needed
       they are loaded and made ready for access.

********************************************************************/


#ifndef __SPPARMS_H
#define __SPPARMS_H

#include "PwrDef.h"

#include CStringRWC_i

#include <rw/collect.h>
#include <rw/bintree.h>
#include <rw/ordcltn.h>
#include "spglobal.hpp"

#include "suppdefs.hpp"


class SpParms: public RWBinaryTree
{

   public:

      SpParms(const char * parmsFile);
      virtual ~SpParms ( void );

      RWCString parmsFileName;
};



class PKeyData;

class MSText: public CStringRWC
{

   RWDECLARE_COLLECTABLE(MSText)

   public:

      MSText ( const char * name,
               fpos_t position);
      MSText ( void );
      virtual ~MSText ( void );
      void        FreeThePKeys( void );
      PKeyData  * FindPKeyData( const char * searchPKey   = NULL,
                                const char * searchATList = NULL,
                                const int    allMustMatch = 0);
      RWOrdered * GetThePKeys( void );
      RWOrdered   thePKeys;
      virtual int       compareTo(const RWCollectable * t) const;
      virtual RWBoolean isEqual  (const RWCollectable * t) const;

   private:

      fpos_t    positionInFile;
      int       isDefined;
};


class ATList: public RWOrdered
{
   public:

      ATList ( void ) : RWOrdered(10) {}
      virtual RWCollectable* find(const CStringRW *t) const;
      virtual RWBoolean contains(const CStringRW *t) const;
};


class PKeyData: public CStringRWC
{

   RWDECLARE_COLLECTABLE(PKeyData)

   public:

      PKeyData ( const char * pkey,
                 const char * atlist,
                 const char * pstring);
      PKeyData ( void );
      virtual ~PKeyData ( void );
      virtual int       compareTo(const RWCollectable * t) const;
      virtual RWBoolean isEqual  (const RWCollectable * t) const;

      CStringRW pString;
      ATList    aTList;
};

#endif
