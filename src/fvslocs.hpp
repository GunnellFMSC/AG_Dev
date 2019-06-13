/********************************************************************

   File Name:     fvslocs.cpp
   Author:        nlc
   Date:          12/06/94 & 01/31/95

   Contents:

   SpSpecies::    SpSpecies
                 ~SpSpecies
                  ISpSpecies            (private)
                  isVariantCurrent      (private)
                  getNumericCode
                  getAlphaCode
                  getCommonName
                  getNumberSpecies

   HabPa::        HabPa
                 ~HabPa
                  IHabPa                (private)
                  isVariantCurrent      (private)
                  getNumericCode
                  getName
                  getNumberItems

   Forests::      Forests
                 ~Forests
                  IForests              (private)
                  isVariantCurrent      (private)
                  getNumericCode
                  getName
                  getNumberItems

   Purpose:       To house, process, information form the parameters
                  file regarding species, habitat type/plant
                  association and forest codes.

 ********************************************************************/

#ifndef __FVSLocs_HPP
#define __FVSLocs_HPP

#include "variants.hpp"
#include "spglobal.hpp"


class SpSpecies
{
   public:

                   SpSpecies( void );
                  ~SpSpecies( void );

      const char * getNumericCode(const int spIndex);
      const char * getAlphaCode(const int spIndex);
      const char * getCommonName(const int spIndex);
      const int    getNumberSpecies( void );

   private:

      void          ISpSpecies( void );
      int           isVariantCurrent( void ) const;
      char *        forVariant;
      int           numberSpecies;
      char **       alphaCodes;
      char **       numericCodes;
      char **       commonNames;

};


inline int SpSpecies::isVariantCurrent() const
{
   return (forVariant &&
           strcmp(theSpGlobals->theVariants->GetSelectedVariant(),
                  forVariant) == 0);
}




inline const char * SpSpecies::getNumericCode(const int spIndex)
{
   if (!isVariantCurrent()) ISpSpecies();
   if (spIndex >= 0 && spIndex <= numberSpecies)
      return (const char *) numericCodes[spIndex];
   else return NULL;
}



inline const char * SpSpecies::getAlphaCode(const int spIndex)
{
   if (!isVariantCurrent()) ISpSpecies();
   if (spIndex >= 0 && spIndex <= numberSpecies)
      return (const char *) alphaCodes[spIndex];
   else return NULL;
}




inline const char * SpSpecies::getCommonName(const int spIndex)
{
   if (!isVariantCurrent()) ISpSpecies();
   if (spIndex >= 0 && spIndex <= numberSpecies)
      return (const char *) commonNames[spIndex];
   else return NULL;
}




inline const int SpSpecies::getNumberSpecies( void )
{
   if (!isVariantCurrent()) ISpSpecies();
   return (const int) numberSpecies;
}


//**************************************************************************



class HabPa
{
   public:

                   HabPa( void );
                  ~HabPa( void );

      const char * getNumericCode(const int index);
      const char * getName(const int index);
      const int    getNumberItems( void );

   private:

      void          IHabPa( void );
      int           isVariantCurrent( void ) const;
      char *        forVariant;
      int           numberItems;
      char **       numericCodes;
      char **       names;

};


inline int HabPa::isVariantCurrent() const
{
   return (forVariant &&
           strcmp(theSpGlobals->theVariants->GetSelectedVariant(),
                  forVariant) == 0);
}




inline const char * HabPa::getNumericCode(const int index)
{
   if (!isVariantCurrent()) IHabPa();
   if (index >= 0 && index <= numberItems)
      return (const char *) numericCodes[index];
   else return NULL;
}



inline const char * HabPa::getName(const int index)
{
   if (!isVariantCurrent()) IHabPa();
   if (index >= 0 && index <= numberItems)
      return (const char *) names[index];
   else return NULL;
}




inline const int HabPa::getNumberItems( void )
{
   if (!isVariantCurrent()) IHabPa();
   return (const int) numberItems;
}


//**************************************************************************


class Forests
{
   public:

                   Forests( void );
                  ~Forests( void );

      const char * getNumericCode(const int index);
      const char * getName(const int index);
      const int    getNumberItems( void );

   private:

      void          IForests( void );
      int           isVariantCurrent( void ) const;
      char *        forVariant;
      int           numberItems;
      char **       numericCodes;
      char **       names;

};


inline int Forests::isVariantCurrent() const
{
   return (forVariant &&
           strcmp(theSpGlobals->theVariants->GetSelectedVariant(),
                  forVariant) == 0);
}




inline const char * Forests::getNumericCode(const int index)
{
   if (!isVariantCurrent()) IForests();
   if (index >= 0 && index <= numberItems)
      return (const char *) numericCodes[index];
   else return NULL;
}



inline const char * Forests::getName(const int index)
{
   if (!isVariantCurrent()) IForests();
   if (index >= 0 && index <= numberItems)
      return (const char *) names[index];
   else return NULL;
}




inline const int Forests::getNumberItems( void )
{
   if (!isVariantCurrent()) IForests();
   return (const int) numberItems;
}

#endif



