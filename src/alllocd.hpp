/********************************************************************

   File Name:     alllocd.hpp
   Author:        nlc
   Date:          04/16/96 and 11/2006

   Class:         AllLocationData

   Purpose:       The location data is read and stored.  Data are made
                  available to build stand components for a simulation.

   Contents:

   class AllLocationData

   class ExLocation : CStringRWC

   class ExGroup : CStringRWC

   class ExStand : CStringRWC


   Usage:

       AllLocationData is created by passing the a file pointer.
       The file is read and an instance of ExLocation is created for
       each record.  Duplicate locations are not allowed.

       After AllLocationData is created, LoadAllLocation can be called
       to load all the location data.  Or, LoadLocation can be called
       to load one location.

       Once a stand is selected as a simulation stand, BuildAnExStand
       is called to convert the data housed by AnExStand to a keyword
       segment that represents the stand.

       A point about terminology: The "Ex" in some class names refers
       to stands as they exist BEFORE they are "internal" to the run.
       That is, the Ex signifies that the object is "external" to the
       run...it is made internal when it is selected and placed in
       the list of simulation stands.

********************************************************************/

#ifndef AllLocation_h
#define AllLocation_h

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" \
   no_namespace rename("EOF", "EndOfFile")

#include "PwrDef.h"
#include CStringRWC_i
#include "spglobal.hpp"
#include "runstr.hpp"
#include "suppdefs.hpp"

class RWOrdered;
class RWBinaryTree;
class ExLocation;
class ExGroup;
class ExStand;

class AllLocationData
{
   public:

      RWOrdered    * itsExLocations;
      RWBinaryTree * itsExGroups;

      AllLocationData( FILE * locFile );
     ~AllLocationData( void );
      void LoadAllLocations( void );
      ExGroup * FindGroup(const char * g);
      ExStand * FindStand(const char * s);
};



class ExLocation : public CStringRWC
{

   RWDECLARE_COLLECTABLE(ExLocation)

   public:

      ExLocation( char            * locRecord,
                  AllLocationData * theAllData,
                  char              theRecordType);
      ExLocation( void );
      virtual ~ExLocation( void );

      virtual RWBoolean isEqual  (const RWCollectable* a) const;
      virtual int       compareTo(const RWCollectable* a) const;

      void LoadLocationData( void );
      void FreeLocationData( void );

      RWOrdered * itsExStands;
      RWOrdered * itsExGroupMembership;

      AllLocationData * itsAllData;

      enum ConnectionType {
         CT_SLF,
         CT_MSACCESS,
         CT_MSACCESS03,
         CT_MSEXCEL,
         CT_MSEXCEL03,
         CT_SQLITE3,
         CT_SQLITE,
         CT_DSN,
         CT_UNKNOWN
      };

      ConnectionType itsConnectionType;

      char * itsData;
      char * itsSLFAccessCode;
      char * itsSLFileName;
      char * itsDSN;
      char * itsUid;
      char * itsPwd;
      int    isProcessPlotsBeingUsed; // -1 = not set, 0 = no, 1 = yes

};



class ExGroup : public CStringRWC
{

   RWDECLARE_COLLECTABLE(ExGroup)

   public:

      ExGroup( const char      * groupName,
               AllLocationData * theAllData);
      ExGroup( void );
      virtual ~ExGroup( void );

      virtual RWBoolean isEqual  (const RWCollectable* a) const;
      virtual int       compareTo(const RWCollectable* a) const;

      AllLocationData * itsAllData;

      RWOrdered * itsExLocationMembership;
      RWOrdered * itsAddFiles;
      RWOrdered * itsAddKeys;
};




class ExStand : public CStringRWC
{

   RWDECLARE_COLLECTABLE(ExStand)

   public:

      ExStand(char       * recordA,
              ExLocation * theLocation);
      ExStand(const char * standid,
              const char * variant,
              const char * standcn,
              const char * invyear,
              ExLocation * theLocation);
      ExStand( void );
      virtual ~ExStand( void );

      virtual RWBoolean isEqual  (const RWCollectable* a) const;
      virtual int       compareTo(const RWCollectable* a) const;

      int BuildAnExStand (char * standBuffer,
                          int    bufferLength);
      char * newChar(const char * ptr);

      ExLocation * itsExLocation;
      RWOrdered  * itsExGroupMembership;
      RWOrdered  * itsAddFiles;
      RWOrdered  * itsAddKeys;
      char       * itsFVSReadyTreeFileName;
      char       * itsVariants;
      char       * itsRecordB;
      char       * itsStandCN;
      int          itsWithPointDataFlag;
};

#endif