/********************************************************************

   Copyright (c) 1994     Suppose...

   File Name:     spglobal.hpp
   Author:        nlc
   Date:          11/30/94

   Purpose:       To process and house handy Suppose Global pointers.

 ********************************************************************/

#ifndef __SpGlobal_HPP
#define __SpGlobal_HPP


class CsupposeWin;
class PWR_CDocument;
class CommandLine;
class SpParms;
class SpPreferences;
class SpVariants;
class SpExtensions;
class SpSpecies;
class HabPa;
class Forests;
class SpParms;
class YearObj;
class SpCommands;
class SpPrograms;
class RunStream;
class AllFVSCases;


class SpGlobal
{
   public:

      SpGlobal (void);
     ~SpGlobal (void);
                                        // -------------------------
                                        // First defined in:
                                        // -------------------------
      CsupposeWin   *theMainWindow;    // CsupposeDoc::BuildWindow
      PWR_CDocument *theAppDocument;   // CsupposeDoc::BuildWindow
      CommandLine   *theCommandLine;   // main
      SpParms       *theParmsData;     // main
      SpPreferences *thePreferences;   // main
      SpVariants    *theVariants;      // CsupposeDoc::BuildWindow
      SpExtensions  *theExtensions;    // CsupposeDoc::BuildWindow
      SpCommands    *theCommands;      // CsupposeDoc::BuildWindow
      SpPrograms    *thePrograms;      // CsupposeDoc::BuildWindow
      SpSpecies     *theSpecies;       // CsupposeDoc::BuildWindow
      HabPa         *theHabPa;         // CsupposeDoc::BuildWindow
      Forests       *theForests;       // CsupposeDoc::BuildWindow
      RunStream     *runStream;        // CsupposeDoc::BuildWindow
      YearObj       *yearObj;          // main
      AllFVSCases   *theReportData;    // CsupposeDoc::DoMenuCommand
      const char    *OS;               // main, via the constructor.
                                        // OS is dos or unix.

      std::string GetMyDocumentsPath(void);
	  std::string SpGlobal::GetPrefFilePath(void);

      inline std::string GetPrefFile(void) { return itsPrefFilePath; }
      inline void SetPrefFile(const char* thePrefFilePath) { itsPrefFilePath = thePrefFilePath; }

      std::string ReadPrefString(const char* theSection, const char* theKey);
      void WritePrefString(const char* theSection, const char* theKey, const char* theString);
      void ClearPrefString(const char* theSection, const char* theKey);

      int ReadPrefInt(const char* theSection, const char* theKey);
      void WritePrefInt(const char* theSection, const char* theKey, int theInt);
      void ClearPrefInt(const char* theSection, const char* theKey);

      float ReadPrefFloat(const char* theSection, const char* theKey);
      void WritePrefFloat(const char* theSection, const char* theKey, float theFloat);
      void ClearPrefFloat(const char* theSection, const char* theKey);

      std::string ReadPrefPersistenceString(const char* theSection, const char* theKey);
      void WritePrefPersistenceString(const char* theSection, const char* theKey, const char* theString);
      void ClearPrefPersistenceString(const char* theSection, const char* theKey);

      std::string itsPrefFilePath;       // The path to Pref file
};

extern SpGlobal * theSpGlobals;

#endif
