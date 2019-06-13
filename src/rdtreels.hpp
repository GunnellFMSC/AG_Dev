/********************************************************************

   File Name:     rdtreels.hpp
   Author:        nlc
   Date:          11/18/97

   Classes:       ParseTreeListFile

   Purpose:  Parse the tree list data file.

 *                                                                  *
 ********************************************************************/

#ifndef ParseTreeListFile_H
#define ParseTreeListFile_H

class FVSCase;
class FVSReport;
class FVSVariable;

class ParseTreeListFile
{
   public:

      ParseTreeListFile( void );
      int ParseTable(char       * theRecord,
                     FILE       * theOpenedFilePtr,
                     CStringRWC & theOpenedFileName,
                     FVSCase    * theCurrentCase);
      void ParseClose( void );

   private:

      int parseCounter;

      FVSReport   * itsCurrentFVSReport;
      FVSVariable * DBH;
//.......Not done.

};


#endif

