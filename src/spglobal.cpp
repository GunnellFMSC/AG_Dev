/********************************************************************

   File Name:     spglobal.cpp
   Author:        nlc
   Date:          11/30/94

   see spglobal.hpp for comments.

 ********************************************************************/

#include "spglobal.hpp"
#include "cmdline.hpp"
#include "spparms.hpp"
#include "spprefer.hpp"
#include "variants.hpp"
#include "extens.hpp"
#include "programs.hpp"
#include "commands.hpp"
#include "fvslocs.hpp"
#include "yearobj.hpp"
#include "runstr.hpp"
#include "reportd.hpp"
#include "shlobj.h"

SpGlobal::SpGlobal( void )
         :theMainWindow (NULL),
          theAppDocument(NULL),
          theCommandLine(NULL),
          theParmsData  (NULL),
          thePreferences(NULL),
          theVariants   (NULL),
          theExtensions (NULL),
          theCommands   (NULL),
          thePrograms   (NULL),
          theSpecies    (NULL),
          theHabPa      (NULL),
          theForests    (NULL),
          runStream     (NULL),
          yearObj       (NULL),
          theReportData (NULL)
{
#ifdef AIXV4
   OS = "unix";
#else
   OS = "dos";
#endif
		//Moved the pref file out of MyDoc due to uninstall issues
   //std::string aPrefFilePath = GetMyDocumentsPath() + "\\suppose.prf";
   std::string aPrefFilePath = GetPrefFilePath() + "\\suppose.prf";
   SetPrefFile(aPrefFilePath.c_str());
}

SpGlobal::~SpGlobal( void )
{
   if (theCommandLine)   delete theCommandLine;
   if (theParmsData)     delete theParmsData;
   if (thePreferences)
   {
      thePreferences->saveDefaultsToPrefFile();
      delete thePreferences;
   }
   if (theVariants)      delete theVariants;
   if (theExtensions)    delete theExtensions;
   if (theCommands)      delete theCommands;
   if (thePrograms)      delete thePrograms;
   if (theSpecies)       delete theSpecies;
   if (theHabPa)         delete theHabPa;
   if (theForests)       delete theForests;
   if (runStream)        delete runStream;
   if (yearObj)          delete yearObj;
   if (theReportData)    delete theReportData;
}

std::string SpGlobal::GetMyDocumentsPath(void)
{
   TCHAR szPath[MAX_PATH];
   std::string aDirPath;

   if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath)))  // My Documents
   {
      aDirPath = szPath;
   }

   return aDirPath;
}

std::string SpGlobal::GetPrefFilePath(void)
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

std::string SpGlobal::ReadPrefString(const char* theSection, const char* theKey)
{
   char aBuffer[64*1024];
   std::string aString;

   GetPrivateProfileString(theSection, theKey, NULL, aBuffer, sizeof(aBuffer), itsPrefFilePath.c_str());
   aString = aBuffer;

   return aString;
}

void SpGlobal::WritePrefString(const char* theSection, const char* theKey, const char* theString) 
{
   WritePrivateProfileString(theSection, theKey, theString, itsPrefFilePath.c_str());
}

void SpGlobal::ClearPrefString(const char* theSection, const char* theKey)
{
   WritePrivateProfileString(theSection, theKey, NULL, itsPrefFilePath.c_str());
}

int SpGlobal::ReadPrefInt(const char* theSection, const char* theKey)
{
   return GetPrivateProfileInt(theSection, theKey, 0, itsPrefFilePath.c_str());
}

void SpGlobal::WritePrefInt(const char* theSection, const char* theKey, int theInt) 
{
   char anIntStr[256];

   sprintf(anIntStr, "%d", theInt);
   WritePrivateProfileString(theSection, theKey, anIntStr, itsPrefFilePath.c_str());
}

void SpGlobal::ClearPrefInt(const char* theSection, const char* theKey)
{
   WritePrivateProfileString(theSection, theKey, NULL, itsPrefFilePath.c_str());
}

float SpGlobal::ReadPrefFloat(const char* theSection, const char* theKey)
{
   char aBuffer[64*1024];

   GetPrivateProfileString(theSection, theKey, NULL, aBuffer, sizeof(aBuffer), itsPrefFilePath.c_str());

   return (float)atof(aBuffer);
}

void SpGlobal::WritePrefFloat(const char* theSection, const char* theKey, float theFloat) 
{
   char aFloatStr[256];

   sprintf(aFloatStr, "%f", theFloat);
   WritePrivateProfileString(theSection, theKey, aFloatStr, itsPrefFilePath.c_str());
}

void SpGlobal::ClearPrefFloat(const char* theSection, const char* theKey)
{
   WritePrivateProfileString(theSection, theKey, NULL, itsPrefFilePath.c_str());
}

std::string SpGlobal::ReadPrefPersistenceString(const char* theSection, const char* theKey)
{
   char* aBuffer;
   int aBufferLength;
   std::string aKey, aString;

   aKey = std::string(theKey) + "_Persisted_Data_Length";
   aBufferLength = GetPrivateProfileInt(theSection, aKey.c_str(), 0, itsPrefFilePath.c_str());
   aBuffer = (char*)calloc(aBufferLength + 1, 1);

   aKey = std::string(theKey) + "_Persisted_Data";
   GetPrivateProfileStruct(theSection, aKey.c_str(), (LPVOID)aBuffer, aBufferLength, itsPrefFilePath.c_str());

   aString = aBuffer;
   free((void*)aBuffer);

   return aString;
}

void SpGlobal::WritePrefPersistenceString(const char* theSection, const char* theKey, const char* theString) 
{
   char aStringLength[256];
   std::string aKey;

   sprintf(aStringLength, "%d", strlen(theString));

   aKey = std::string(theKey) + "_Persisted_Data_Length";
   WritePrivateProfileString(theSection, aKey.c_str(), aStringLength, itsPrefFilePath.c_str());

   aKey = std::string(theKey) + "_Persisted_Data";
   WritePrivateProfileStruct(theSection, aKey.c_str(), (LPVOID)theString, strlen(theString), itsPrefFilePath.c_str());
}

void SpGlobal::ClearPrefPersistenceString(const char* theSection, const char* theKey)
{
   std::string aKey;

   aKey = std::string(theKey) + "_Persisted_Data_Length";
   WritePrivateProfileString(theSection, aKey.c_str(), NULL, itsPrefFilePath.c_str());

   aKey = std::string(theKey) + "_Persisted_Data";
   WritePrivateProfileString(theSection, aKey.c_str(), NULL, itsPrefFilePath.c_str());
}
