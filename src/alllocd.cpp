/********************************************************************

   File Name:     alllocd.cpp
   Author:        nlc
   Date:          04/16/96 and 6/2007

   See alllocd.hpp for purpose, contents, and usage notes.

   Modified 10/22/2004 by nlc to add StandCN on record E

********************************************************************/
#import "C:\Program Files\Common Files\System\ADO\msado15.dll" \
   no_namespace rename("EOF", "EndOfFile")


#include "alllocd.hpp"
#include <rw/bintree.h>
#include <rw/ordcltn.h>
#include <ctype.h>
#include <string.h>
#include <rw/ctoken.h>
#include "warn_dlg.hpp"
#include "yearobj.hpp"
#include "spglobal.hpp"
#include "spfunc2.hpp"
#include "spprefer.hpp"



AllLocationData::AllLocationData(FILE * locFile)
                :itsExLocations(new RWOrdered((size_t) 20)),
                 itsExGroups(new RWBinaryTree())
{
   char * record= new char[2001];
   char * ptr;
   if (locFile != NULL)
   {
      while (fgets(record, 2000, locFile) &&
             (ptr = strtok (record," \n")))
      {
         switch ((int) *ptr)
         {
            case 'A':                // Location records
            {
               ptr = strtok (NULL,"\n");
               if (ptr)
               {
                  ExLocation * newLoc = new ExLocation(ptr, this, 'A');
                  if (itsExLocations->contains((CStringRWC *) newLoc))
                     delete newLoc;
                  else itsExLocations->insert(newLoc);
               }
               break;
            }

            case 'B':                // Group records.
            {
               ptr = strtok(NULL,"  @\n");
               if (ptr && *ptr != '@')
               {
                  ExGroup * newGroup = FindGroup(ptr);
                  if (!newGroup) newGroup = new ExGroup(ptr,this);
                  itsExGroups->insert(newGroup);
                  for (ptr = strtok(NULL," @\n"); ptr != NULL;
                       ptr = strtok(NULL," @\n"))
                  {
                     if (!isspace((int) *ptr))
                     {
                        if (!newGroup->itsAddFiles)
                        newGroup->itsAddFiles = new RWOrdered((size_t) 20);
                        CStringRWC * newAddFile = new CStringRWC(ptr);
                        if (newGroup->itsAddFiles->contains(newAddFile))
                            delete newAddFile;
                        else
                           newGroup->itsAddFiles->insert(newAddFile);
                     }
                  }
               }
               break;
            }

            case 'C':                // Location records via database
            {
               ptr = strtok (NULL,"\n");
               if (ptr)
               {
                  ExLocation * newLoc = new ExLocation(ptr, this, 'C');
                  if (itsExLocations->contains((CStringRWC *) newLoc))
                     delete newLoc;
                  else itsExLocations->insert(newLoc);
               }
               break;
            }

            default: break;
         }
      }
   }
   delete []record;
}




AllLocationData::~AllLocationData( void )
{
   itsExLocations->clearAndDestroy();
   itsExGroups->clearAndDestroy();
   delete itsExLocations;
   delete itsExGroups;
}



ExGroup * AllLocationData::FindGroup(const char * g)
{
   RWBinaryTreeIterator next(*itsExGroups);
   ExGroup * exGroup;
   while (exGroup = (ExGroup *) next())
   {
      if (strcmp(g,exGroup->data()) == 0)
      return exGroup;
   }
   return NULL;
}



void AllLocationData::LoadAllLocations( void )
{
   RWOrderedIterator next(*itsExLocations);
   ExLocation * exLocation;
   while (exLocation = (ExLocation *) next())
      exLocation->LoadLocationData();
}



RWDEFINE_COLLECTABLE(ExLocation,EXLOCATION)

ExLocation::ExLocation(char            * locRecord,
                       AllLocationData * theAllData,
                       char              theRecordType)
           :CStringRWC              (),
            itsAllData              (theAllData),
            itsExStands             (NULL),
            itsExGroupMembership    (NULL),
            itsData                 (NULL),
            itsSLFAccessCode        (NULL),
            itsSLFileName           (NULL),
            itsDSN                  (NULL),
            itsUid                  (NULL),
            itsPwd                  (NULL)
{
   // Define all of the elements of a location...using the contents
   // of locRecord.

   char * quoteptr1 = strchr(locRecord, '"');
   char * quoteptr2 = NULL;

   if (quoteptr1 == NULL)
   {
      for (char * i=locRecord; *i != 0; i++)
      {
         if (isspace((int) *i))
         {
            quoteptr2=i;
            break;
         }
      }
      *quoteptr2 = 0;
      if (!strchr(locRecord,'@')) *((CStringRWC *) this) = locRecord;
   }
   else
   {
      quoteptr2 = strchr(++quoteptr1, '"');
      *quoteptr2 = 0;
      if (*quoteptr1 != '@')  *((CStringRWC *) this) = quoteptr1;
   }

   locRecord = ++quoteptr2;

   int len=strlen(locRecord);
   if (len > 0)
   {
      itsData = new char[len+1];
      strcpy(itsData,locRecord);
      char * ptr = strtok (itsData," \n");
      switch (theRecordType)
      {
         case 'A':
         {
            if (ptr && *ptr != '@') itsSLFAccessCode = ptr;
            if (ptr) ptr = strtok (NULL," \n");
            if (ptr && *ptr != '@') itsSLFileName = ptr;
            if (ptr) ptr = strtok (NULL," \n");
            if (ptr && *ptr != '@') itsDSN = ptr;

            itsConnectionType = CT_SLF;
         }
         break;

         case 'C':
         {
            if (ptr && *ptr != '@') itsDSN = ptr;
            if (ptr) ptr = strtok (NULL," \n");
            if (ptr && *ptr != '@') itsUid = ptr;
            if (ptr) ptr = strtok (NULL," \n");
            if (ptr && *ptr != '@') itsPwd = ptr;

            // Determine connection type
            RWCString aDSN = itsDSN;
            size_t aLastDot = aDSN.last('.');
            if (aLastDot == RW_NPOS)
            {
               // No extension present so connection type is unknown at this point,
               // but it could be a DSN to a ODBC connection...
               itsConnectionType = CT_DSN;
            }
            else if (aDSN.index(".mdb", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_MSACCESS03;
            }
            else if (aDSN.index(".accdb", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_MSACCESS;
            }
            else if (aDSN.index(".xlsx", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_MSEXCEL;
            }
            else if (aDSN.index(".xls", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_MSEXCEL03;
            }
            else if (aDSN.index(".db3", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_SQLITE;
            }
            else if (aDSN.index(".db", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_SQLITE;
            }
			else if (aDSN.index(".sqlite", aLastDot, RWCString::ignoreCase) != RW_NPOS)
            {
               itsConnectionType = CT_SQLITE;
            }
            else
            {
               itsConnectionType = CT_UNKNOWN;
            }
         }
         break;

         default:
            break;
      }
   }
}



ExLocation::ExLocation( void )
           :CStringRWC              (),
            itsAllData              (NULL),
            itsExStands             (NULL),
            itsExGroupMembership    (NULL),
            itsData                 (NULL),
            itsSLFAccessCode        (NULL),
            itsSLFileName           (NULL),
            itsDSN                  (NULL),
            itsUid                  (NULL),
            itsPwd                  (NULL)
{}



ExLocation::~ExLocation( void )
{

   if (itsExStands)
   {
      itsExStands->clearAndDestroy();
      delete itsExStands;
   }

   if (itsExGroupMembership) delete itsExGroupMembership;
   if (itsData)              delete []itsData;
}



RWBoolean ExLocation::isEqual  (const RWCollectable* a) const
{
   return this == a;
}



int ExLocation::compareTo(const RWCollectable* a) const
{
   if (this == a) return 0;
   return strcmp(data(),((ExLocation *) a)->data());
}



void ExLocation::LoadLocationData( void )
{

   if (itsExStands) return;

   char * ptr;
   char * record = new char[2001];

   ExStand * currentExStand = NULL;
   itsExStands = new RWOrdered((size_t) 500);

   switch (itsConnectionType)
   {
      case CT_SLF:
      {
         FILE* slfFile = fopen (itsSLFileName,"r");
         if (slfFile)
         {
            while (fgets(record, 2000, slfFile))
            {
               ptr = strtok(record," ");
               if (!ptr) break;
               switch ((int) *ptr)
               {
                  case 'A':                // Required...stand id record.
                  {
                     ptr=strtok(NULL," ");
                     if (!ptr)
                        break;
                     currentExStand = new ExStand(ptr, this);
                     itsExStands->insert(currentExStand);
                     break;
                  }
                  case 'B':                // Required...stand info data.
                  {
                     if (currentExStand->itsRecordB)
                        delete [] currentExStand->itsRecordB;
                     ptr=strtok(NULL," ");
                     if (strcmp(ptr,currentExStand->data()) != 0)
                        break;
                     ptr=strtok(NULL,"\n");

                     // strip trailing blanks and @ symbols.
                     for (char *l = ptr+strlen(ptr)-1; *l == ' ' || *l == '@'; l--) *l = (char) 0;

                     if (strlen(ptr))
                     {
                        char * t = new char[strlen(ptr)+1];
                        strcpy(t,ptr);
                        currentExStand->itsRecordB = t;
                     }
                     break;
                  }
                  case 'C':                // Optional...group membership.
                  {
                     ptr=strtok(NULL," ");
                     if (strcmp(ptr,currentExStand->data()) != 0) break;
                     for (ptr = strtok(NULL," @\n"); ptr != NULL;
                        ptr = strtok(NULL," @\n"))
                     {
                        if (!isspace((int) *ptr))
                        {
                           ExGroup * theGroup = itsAllData->FindGroup(ptr);
                           if (!theGroup)
                           {
                              theGroup = new ExGroup(ptr, itsAllData);
                              itsAllData->itsExGroups->insert(theGroup);
                           }
                           if (!theGroup->itsExLocationMembership)
                              theGroup->itsExLocationMembership = new RWOrdered ((size_t) 20);
                           if (!theGroup->itsExLocationMembership->contains(this))
                              theGroup->itsExLocationMembership->insert(this);
                           if (!itsExGroupMembership)
                              itsExGroupMembership = new RWOrdered ((size_t) 20);
                           if (!itsExGroupMembership->contains(theGroup))
                              itsExGroupMembership->insert(theGroup);
                           if (!currentExStand->itsExGroupMembership)
                              currentExStand->itsExGroupMembership = new RWOrdered ((size_t) 20);
                           if (!currentExStand->itsExGroupMembership->contains(theGroup))
                              currentExStand->itsExGroupMembership->insert(theGroup);
                        }
                     }
                     break;
                  }
                  case 'D':                // Optional...addfile.
                  {
                     ptr=strtok(NULL," ");
                     if (strcmp(ptr,currentExStand->data()) != 0) break;
                     for (ptr = strtok(NULL," @\n"); ptr != NULL;
                        ptr = strtok(NULL," @\n"))
                     {
                        if (!isspace((int) *ptr))
                        {
                           CStringRWC * addfile = new CStringRWC(ptr);
                           if (!currentExStand->itsAddFiles)
                              currentExStand->itsAddFiles = new RWOrdered ((size_t) 10);
                           if (!currentExStand->itsAddFiles->contains(addfile))
                              currentExStand->itsAddFiles->insert(addfile);
                           else delete addfile;
                        }
                     }
                     break;
                  }
                  case 'E':                // Optional...StandCN
                  {
                     ptr=strtok(NULL," ");
                     if (strcmp(ptr,currentExStand->data()) != 0) break;
                     ptr=strtok(NULL,"\n");
                     // strip trailing blanks and @ symbols.
                     for (char *l = ptr+strlen(ptr)-1; *l == ' ' || *l == '@'; l--) *l = (char) 0;
                     if (strlen(ptr))
                     {
                        char * t = new char[strlen(ptr)+1];
                        strcpy(t,ptr);
                        if (currentExStand->itsStandCN) delete [] currentExStand->itsStandCN;
                        currentExStand->itsStandCN = t;
                     }
                     break;
               }
               default: break;
               }
            }
            fclose (slfFile);
         }
         else
         {
            CStringRW warnMsg = "Stand list file could not be opened.\nLocation = ";
            warnMsg += data();
            warnMsg += "\nStand List File = ";
            warnMsg += itsSLFileName;
            Warn (warnMsg);
         }
      }
      break;

      case CT_MSACCESS:
      case CT_MSACCESS03:
      case CT_MSEXCEL:
      case CT_MSEXCEL03:
      case CT_SQLITE:
      {
        databaseTableSelection = theSpGlobals->thePreferences->defaultProcessPlots;
		//Check to see if plot table exists... 0 = No, 1 = Yes, 2 = Plots, 3 = Subplots, 4 = Conditions
          
         RWCString aSQLStmt;
         RWCString aCS;
         HRESULT hr = S_OK;
         int aStandOK=1;
         int nameForm=2;
         _variant_t aV;

         CoInitialize(NULL);

         if (itsConnectionType == CT_MSACCESS || itsConnectionType == CT_MSACCESS03
           || itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03
           || itsConnectionType == CT_SQLITE)
         {
            // Define connection string and set the directory path if one doesn't exist
            RWCString aDirPath;
            char aVolume[256], aDirectory[256], aName[256], anExtension[256], aVersion[256];
            xvt_fsys_parse_pathname(itsDSN, aVolume, aDirectory, aName, anExtension, aVersion);
            if ((strlen(aVolume) == 0) && (strlen(aDirectory) == 0))
            {
               DIRECTORY aDir;
               xvt_fsys_get_dir(&aDir);
               aDirPath = aDir.path + RWCString("\\");
            }

            if (itsConnectionType == CT_MSEXCEL03) 
                aCS = "Driver={Microsoft Excel Driver (*.xls)};";
            else if (itsConnectionType == CT_MSEXCEL) 
                aCS = "Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};";
            else if (itsConnectionType == CT_SQLITE) 
                aCS = "Driver=SQLite3 ODBC Driver;Database=" + aDirPath + RWCString(itsDSN) + ";";
            else if (itsConnectionType == CT_MSACCESS03) 
                aCS = "Driver={Microsoft Access Driver (*.mdb)};";
            else 
                aCS = "Driver={Microsoft Access Driver (*.mdb, *.accdb)};";
            aCS = aCS +
               RWCString("Dbq=") + aDirPath + RWCString(itsDSN) + RWCString(";") +
               RWCString("Uid=") + (itsUid ? RWCString(itsUid) : RWCString("")) + RWCString(";") +
               RWCString("Pwd=") + (itsPwd ? RWCString(itsPwd) : RWCString("")) + RWCString(";");
         }
         else 
         {  // CT_DSN currently branches to default case...see below.
            // (itsConnectionType == CT_DSN)
            aCS = RWCString("DSN=") + RWCString(itsDSN) + RWCString(";") +
               RWCString("Uid=") + (itsUid ? RWCString(itsUid) : RWCString("")) + RWCString(";") +
               RWCString("Pwd=") + (itsPwd ? RWCString(itsPwd) : RWCString("")) + RWCString(";");
         }

         // Create and instantiate a stand record set
         _RecordsetPtr aStandRS("ADODB.Recordset");

         // Open and populate the record set with stands from the following SQL
         if(databaseTableSelection == 0)
            aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03) 
               ? "Select * From [FVS_StandInit$] Where (1 = 0)"
               : "Select * From FVS_StandInit Where (1 = 0)";
		 else if (databaseTableSelection == 1)
			 aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
			 ? "Select * From [FVS_PlotInit$] Where (1 = 0)"
			 : "Select * From FVS_PlotInit Where (1 = 0)";
		 else if (databaseTableSelection == 2)
			 aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
			 ? "Select * From [FVS_StandInit_Plot$] Where (1 = 0)"
			 : "Select * From FVS_StandInit_Plot Where (1 = 0)";
		 else if (databaseTableSelection == 3)
			 aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
			 ? "Select * From [FVS_PlotInit_Plot$] Where (1 = 0)"
			 : "Select * From FVS_PlotInit_Plot Where (1 = 0)";
		 else
			 aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
			 ? "Select * From [FVS_StandInit_Cond$] Where (1 = 0)"
			 : "Select * From FVS_StandInit_Cond Where (1 = 0)";
         if (aStandOK)
         {
            try
            {
               aStandRS->Open(aSQLStmt.data(), aCS.data(), adOpenStatic, adLockReadOnly, adCmdText);
            }
            catch(_com_error & ce)
            {
               nameForm = 1;
            }
         }
         if (nameForm == 1)
         {
            if(databaseTableSelection == 0)
               aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03) 
                  ? "Select * From [NRV_FVS_StandInit_VM$] Where (1 = 0)"
                  : "Select * From NRV_FVS_StandInit_VM Where (1 = 0)";
			else if (databaseTableSelection == 1)
               aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03) 
                  ? "Select * From [NRV_FVS_PlotInit_VM$] Where (1 = 0)"
                  : "Select * From NRV_FVS_PlotInit_VM Where (1 = 0)";
			else if (databaseTableSelection == 2)
				aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
				? "Select * From [NRV_FVS_StandInit_Plot_VM$] Where (1 = 0)"
				: "Select * From NRV_FVS_StandInit_Plot_VM Where (1 = 0)";
			else if (databaseTableSelection == 3)
				aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
				? "Select * From [NRV_FVS_PlotInit_Plot_VM$] Where (1 = 0)"
				: "Select * From NRV_FVS_PlotInit_Plot_VM Where (1 = 0)";
			else if (databaseTableSelection == 4)
				aSQLStmt = (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
				? "Select * From [NRV_FVS_StandInit_Cond_VM$] Where (1 = 0)"
				: "Select * From NRV_FVS_StandInit_Cond_VM Where (1 = 0)";
            try
            {
               aStandRS->Open(aSQLStmt.data(), aCS.data(), adOpenStatic, adLockReadOnly, adCmdText);
            }
            catch(_com_error & ce)
            {
               aStandOK = 0;
               char t[1024];
			   sprintf(t, "**Input Database Table type selected in Modify Preferences not\n     compatible with your input database OR error with .loc file**\n"
				   "\n\n\n\t        **Please check Preferences & .loc file**\n");
               Warn(t);
            }
         }
 
         int haveID=0;
         int haveCN=0;
         int haveVar=0;
         int haveInv=0;
         int haveGrp=0;
         int haveAdd=0;
         int haveKey=0;
         if (aStandOK)
         {
            int nFieldCount = aStandRS->Fields->GetCount();
            _bstr_t bs_CN;
            char * cp_CN;
            CStringRW colName;
            aSQLStmt  = "Select";
            int fcomma = -1;
			if (databaseTableSelection == 0 || databaseTableSelection == 2 || databaseTableSelection == 4)
          {
            for (int i=0; i<nFieldCount; i++)
            {
               bs_CN = aStandRS->Fields->GetItem(_variant_t((long)i))->Name;
               cp_CN = bs_CN;
               colName = cp_CN;
               if      (!colName.compareTo("STAND_CN",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveCN=1;
                aSQLStmt += " STAND_CN";
               }
               else if (!colName.compareTo("STAND_ID",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveID=1;
                aSQLStmt += " STAND_ID";
               }
               else if (!colName.compareTo("VARIANT", RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveVar=1;
                aSQLStmt += " VARIANT";
               }
               else if (!colName.compareTo("INV_YEAR", RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveInv=1;
                aSQLStmt += " INV_YEAR";
               }
               else if (!colName.compareTo("GROUPS",   RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveGrp=1;
                aSQLStmt += " GROUPS";
               }
               /*else if (!colName.compareTo("ADDFILES",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveAdd=1;
                aSQLStmt += " ADDFILES";
               }*/
               else if (!colName.compareTo("FVSKEYWORDS",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveKey=1;
                aSQLStmt += " FVSKEYWORDS";
               }
            } //end i<nFieldCount
          }
          else
          {
            for (int i=0; i<nFieldCount; i++)
            {
               bs_CN = aStandRS->Fields->GetItem(_variant_t((long)i))->Name;
               cp_CN = bs_CN;
               colName = cp_CN;
               if      (!colName.compareTo("STANDPLOT_CN",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveCN=1;
                aSQLStmt += " STANDPLOT_CN";
               }
               else if (!colName.compareTo("STANDPLOT_ID",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveID=1;
                aSQLStmt += " STANDPLOT_ID";
               }
               else if (!colName.compareTo("VARIANT", RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveVar=1;
                aSQLStmt += " VARIANT";
               }
               else if (!colName.compareTo("INV_YEAR", RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveInv=1;
                aSQLStmt += " INV_YEAR";
               }
               else if (!colName.compareTo("GROUPS",   RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveGrp=1;
                aSQLStmt += " GROUPS";
               }
               else if (!colName.compareTo("ADDFILES",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveAdd=1;
                aSQLStmt += " ADDFILES";
               }
               else if (!colName.compareTo("FVSKEYWORDS",RWCString::ignoreCase)) 
               {
                fcomma++; if (fcomma) aSQLStmt += ","; haveKey=1;
                aSQLStmt += " FVSKEYWORDS";
               }
            }
          }
                    
            aStandRS->Close();

            if (haveID+haveInv+haveVar != 3)
            {
               aStandOK = 0;
               char t[1024];
               sprintf(t,"Database error processing location:\n  %s\n"
                        "\nStatement:\n  %s",
                        "Stand_ID, Inv_Year and Variant are required fields in the data base.",
                        this->data(), aSQLStmt.data());
               Warn(t);
            }


           if (aStandOK)
           {
              if(databaseTableSelection ==0)
            {
              if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
               aSQLStmt += (nameForm == 1) ? " FROM [NRV_FVS_StandInit_VM$];" 
                                           : " FROM [FVS_StandInit$];";
              else
               aSQLStmt += (nameForm == 1) ? " FROM NRV_FVS_StandInit_VM;" 
                                           : " FROM FVS_StandInit;";
            }
			  else if (databaseTableSelection == 1)
			  {
				  if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
					  aSQLStmt += (nameForm == 1) ? " FROM [NRV_FVS_PlotInit_VM$];"
					  : " FROM [FVS_PlotInit$];";
				  else
					  aSQLStmt += (nameForm == 1) ? " FROM NRV_FVS_PlotInit_VM;"
					  : " FROM FVS_PlotInit;";
			  }
			  else if (databaseTableSelection == 2)
			  {
				  if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
					  aSQLStmt += (nameForm == 1) ? " FROM [NRV_FVS_StandInit_Plot_VM$];"
					  : " FROM [FVS_StandInit_Plot$];";
				  else
					  aSQLStmt += (nameForm == 1) ? " FROM NRV_FVS_StandInit_Plot_VM;"
					  : " FROM FVS_StandInit_Plot;";
			  }
			  else if (databaseTableSelection == 3)
			  {
				  if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
					  aSQLStmt += (nameForm == 1) ? " FROM [NRV_FVS_PlotInit_Plot_VM$];"
					  : " FROM [FVS_PlotInit_Plot$];";
				  else
					  aSQLStmt += (nameForm == 1) ? " FROM NRV_FVS_PlotInit_Plot_VM;"
					  : " FROM FVS_PlotInit_Plot;";
			  }
			  else if (databaseTableSelection == 4)
			  {
				  if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
					  aSQLStmt += (nameForm == 1) ? " FROM [NRV_FVS_StandInit_Cond_VM$];"
					  : " FROM [FVS_StandInit_Cond$];";
				  else
					  aSQLStmt += (nameForm == 1) ? " FROM NRV_FVS_StandInit_Cond_VM;"
					  : " FROM FVS_StandInit_Cond;";
			  }

              try
              {
                 aStandRS->Open(aSQLStmt.data(), aCS.data(), adOpenStatic, adLockReadOnly, adCmdText);
              }
              catch(_com_error & ce)
              {
                 aStandOK = 0;
                 char t[1024];
                 sprintf(t,"Database error processing location:\n  %s\n"
                          "ADO error number: %08lx\nError message: %s\nConnection:\n  %s\nStatement:\n  %s",
                          this->data(), ce.Error(), ce.ErrorMessage(), aCS.data(), aSQLStmt.data());
                 Warn(t);
              }
           }
         }

         int skip=0;

         RWCString csTmp;
         if (aStandOK && !aStandRS->EndOfFile)
         {
            while(!aStandRS->EndOfFile)
            {
				aV = aStandRS->Fields->GetItem(databaseTableSelection == 1 || databaseTableSelection == 3 ? "StandPlot_ID" : "Stand_ID")->Value;
                if (aV.GetVARIANT().vt == VT_NULL)
                {
                   skip++;
                   aStandRS->MoveNext();
                   continue;
                }
               _bstr_t aStandID = aV;
               aV = aStandRS->Fields->GetItem("Variant")->Value;
               if (aV.GetVARIANT().vt == VT_NULL)
               {
                  skip++;
                  aStandRS->MoveNext();
                  continue;
               }
               _bstr_t aVariant = aV;

               char * aStandCN=NULL;  // can be null
               _bstr_t aCN;
              if (haveCN)
               {
				  aV = aStandRS->Fields->GetItem(databaseTableSelection == 1 || databaseTableSelection == 3 ? "StandPlot_CN" : "Stand_CN")->Value;
                  if (aV.GetVARIANT().vt != VT_NULL)
                  {
                     aCN = aV;
                     aStandCN = aCN;
                  }
               }

               aV = aStandRS->Fields->GetItem("Inv_Year")->Value;
               if (aV.GetVARIANT().vt == VT_NULL)
               {
                  skip++;
                  aStandRS->MoveNext();
                  continue;
               }
               _bstr_t aInvYear = aV;

               currentExStand = new ExStand(aStandID,aVariant,aStandCN,aInvYear,this);
               itsExStands->insert(currentExStand);
 
               if (haveGrp) 
               {
                  aV = aStandRS->Fields->GetItem("Groups")->Value;
                  if (aV.GetVARIANT().vt != VT_NULL)
                  {
                     _bstr_t aGroup = aV;
                     csTmp = aGroup;
                     RWCTokenizer next(csTmp);
                     RWCString token;
                     while (!(token=next(" \r\t\n\0")).isNull())
                     {
                        const char *ptr = token.data();
                        ExGroup * theGroup = itsAllData->FindGroup(ptr);
                        if (!theGroup)
                        {
                           theGroup = new ExGroup(ptr, itsAllData);
                           itsAllData->itsExGroups->insert(theGroup);
                        }
                        if (!theGroup->itsExLocationMembership)
                           theGroup->itsExLocationMembership = new RWOrdered ((size_t) 20);
                        if (!theGroup->itsExLocationMembership->contains(this))
                           theGroup->itsExLocationMembership->insert(this);
                        if (!itsExGroupMembership)
                           itsExGroupMembership = new RWOrdered ((size_t) 20);
                        if (!itsExGroupMembership->contains(theGroup))
                           itsExGroupMembership->insert(theGroup);
                        if (!currentExStand->itsExGroupMembership)
                           currentExStand->itsExGroupMembership = new RWOrdered ((size_t) 20);
                        if (!currentExStand->itsExGroupMembership->contains(theGroup))
                           currentExStand->itsExGroupMembership->insert(theGroup);
                     }
                  }
               }

               if (haveAdd)
               {
                  aV = aStandRS->Fields->GetItem("AddFiles")->Value;
                  if (aV.GetVARIANT().vt != VT_NULL)
                  {
                     _bstr_t anAdd = aV;
                     csTmp = anAdd;
                     RWCTokenizer next(csTmp);
                     RWCString token;
                     while (!(token=next("\r\t\n\0")).isNull())
                     {
                        const char *ptr = token.data();
                        CStringRWC * addfile = new CStringRWC(ptr);
                        if (!currentExStand->itsAddFiles)
                           currentExStand->itsAddFiles = new RWOrdered ((size_t) 3);
                        if (!currentExStand->itsAddFiles->contains(addfile))
                           currentExStand->itsAddFiles->insert(addfile);
                        else delete addfile;      
                     }
                  }
               }

               if (haveKey)
               {
                  aV = aStandRS->Fields->GetItem("FVSKeywords")->Value;
                  if (aV.GetVARIANT().vt != VT_NULL)
                  {
                     _bstr_t aKey_bs = aV;
                     char *aKey=aKey_bs;
                     if (strlen(aKey)) 
                     {
                        CStringRWC * addkey = new CStringRWC(aKey);
                        if (!currentExStand->itsAddKeys)
                           currentExStand->itsAddKeys = new RWOrdered ((size_t) 3);
                        if (!currentExStand->itsAddKeys->contains(addkey))
                           currentExStand->itsAddKeys->insert(addkey);
                        else
                           delete addkey;
                     }
                  }
               }

               aStandRS->MoveNext();
            }
            aStandRS->Close();
         }
                     

         if (skip>0)
         {
            CStringRW t = "Stands were skipped because some required fields were null.";
            t.append("\nLocation: ");
            t.append(this->data());
            t.append("\nTable name: ");
          if(databaseTableSelection == 0)
            t.append((nameForm == 1) ? "NRV_FVS_StandInit_VM" : "FVS_StandInit");
		  else if (databaseTableSelection == 1)
			  t.append((nameForm == 1) ? "NRV_FVS_PlotInit_VM" : "FVS_PlotInit");
		  else if (databaseTableSelection == 2)
			  t.append((nameForm == 1) ? "NRV_FVS_StandInit_Plot_VM" : "FVS_StandInit_Plot");
		  else if (databaseTableSelection == 3)
			  t.append((nameForm == 1) ? "NRV_FVS_PlotInit_Plot_VM" : "FVS_PlotInit_Plot");
		  else if (databaseTableSelection == 4)
			  t.append((nameForm == 1) ? "NRV_FVS_StandInit_Cond_VM" : "FVS_StandInit_Cond");
            t.append("\nNumber skipped: ");
            t.append(ConvertToString(skip));
            Warn (t.data());
         }

         // Create and instantiate a group record set
         _RecordsetPtr aGroupRS("ADODB.Recordset");

         // Open and populate the record set with groups
         aSQLStmt  = "Select Stand_ID, Groups From ";
         if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
            aSQLStmt += (nameForm == 1) ? "[NRV_FVS_StandGroupCodes_VM$]" : "[FVS_StandGroupCodes$]";
         else
            aSQLStmt += (nameForm == 1) ? "NRV_FVS_StandGroupCodes_VM" : "FVS_StandGroupCodes";
         aSQLStmt += ";";
         int aGroupOK=1;
         try
         {
            aGroupRS->Open(aSQLStmt.data(), aCS.data(), adOpenStatic, adLockReadOnly, adCmdText);
         }
         catch(_com_error & ce)
         {
            aGroupOK=0;
         }
         if (aGroupOK && !aGroupRS->EndOfFile)
         {
            while(!aGroupRS->EndOfFile)
            {
                aV = aGroupRS->Fields->GetItem("Stand_ID")->Value;
                if (aV.GetVARIANT().vt == VT_NULL)
                {
                   skip++;
                   aGroupRS->MoveNext();
                   continue;
                }
               _bstr_t aStandID = aV;

               char * aGroupCode=NULL;  // can be null
               _bstr_t aGC;
               aV = aGroupRS->Fields->GetItem("Groups")->Value;;
               if (aV.GetVARIANT().vt != VT_NULL)
               {
                  aGC = aV;
                  aGroupCode = aGC;
                  if (strlen(aGroupCode))
                  {
                     RWCTokenizer next(aGroupCode);
                     RWCString token;
                     while (!(token=next(" \r\t\n\0")).isNull())
                     {
                        const char *ptr = token.data();
                        ExGroup * theGroup = itsAllData->FindGroup(ptr);
                        RWOrderedIterator next(*itsExStands);
                        ExStand * aExStand;
                        while (aExStand = (ExStand *) next())
                        {
                           if (strcmp(aStandID,aExStand->data()) == 0)
                           {
                              if (!theGroup)
                              {
                                 theGroup = new ExGroup(ptr, itsAllData);
                                 itsAllData->itsExGroups->insert(theGroup);
                                 if (!itsExGroupMembership)
                                    itsExGroupMembership = new RWOrdered ((size_t) 20);

                                 if (!itsExGroupMembership->contains(theGroup))
                                    itsExGroupMembership->insert(theGroup);
                                 if (!theGroup->itsExLocationMembership)
                                    theGroup->itsExLocationMembership = new RWOrdered ((size_t) 20);

                                 if (!theGroup->itsExLocationMembership->contains(this))
                                    theGroup->itsExLocationMembership->insert(this);
                              }
                              if (!aExStand->itsExGroupMembership)
                                 aExStand->itsExGroupMembership = new RWOrdered ((size_t) 20);

                              if (!aExStand->itsExGroupMembership->contains(theGroup))
                                 aExStand->itsExGroupMembership->insert(theGroup);
                           }
                        }
                     }
                  }
               }
               aGroupRS->MoveNext();
            }
         }

         // Create and instantiate an add files record set

         _RecordsetPtr aStdAddFileKeyRS("ADODB.Recordset");

         // Open and populate the record set with add files from the following SQL

         aSQLStmt  = "Select Stand_ID, AddFiles, FVSKeywords From ";
         if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
            aSQLStmt += (nameForm == 1) ? "[NRV_FVS_StandAddFilesAndKeywords_VM$]" : "[FVS_StandAddFilesAndKeywords$]";
         else
            aSQLStmt += (nameForm == 1) ? "NRV_FVS_StandAddFilesAndKeywords_VM" : "FVS_StandAddFilesAndKeywords";
         aSQLStmt += ";";
         int anAddOK = 1;
         try
         {
            aStdAddFileKeyRS->Open(aSQLStmt.data(), aCS.data(), adOpenStatic, adLockReadOnly, adCmdText);
         }
         catch(_com_error & ce)
         {
            anAddOK=0;
         }

         // Process any optional add files with record set data for specified stand
         if (anAddOK && !aStdAddFileKeyRS->EndOfFile)
         {
            while(!aStdAddFileKeyRS->EndOfFile)
            {
                aV = aStdAddFileKeyRS->Fields->GetItem("Stand_ID")->Value;
                if (aV.GetVARIANT().vt == VT_NULL)
                {
                   skip++;
                   aStdAddFileKeyRS->MoveNext();
                   continue;
                }
               _bstr_t aStandID = aV;

               char * anAdd=NULL;
               char * aKey=NULL;
               _bstr_t anAdd_bs, aKey_bs;
               aV = aStdAddFileKeyRS->Fields->GetItem("AddFiles")->Value;
               if (aV.GetVARIANT().vt != VT_NULL)
               {
                  anAdd_bs = aV;
                  if (strlen(anAdd_bs)) anAdd = anAdd_bs;
               }
               aV = aStdAddFileKeyRS->Fields->GetItem("FVSKeywords")->Value;
               if (aV.GetVARIANT().vt != VT_NULL)
               {
                  aKey_bs = aV;
                  if (strlen(aKey_bs)) aKey = aKey_bs;
               }
               // if either are not null, then find the stand(s) they belong too.
               if (anAdd || aKey)
               {
                  RWOrderedIterator next(*itsExStands);
                  ExStand * aExStand;
                  while (aExStand = (ExStand *) next())
                  {
                     if (strcmp(aStandID,aExStand->data()) == 0)
                     {
                        if (anAdd)
                        {
                           RWCTokenizer next(anAdd);
                           RWCString token;
                           while (!(token=next("\r\t\n\0")).isNull())
                           {
                              CStringRWC * addfile = new CStringRWC(token);
                              if (!aExStand->itsAddFiles)
                                 aExStand->itsAddFiles = new RWOrdered ((size_t) 3);
                              if (!aExStand->itsAddFiles->contains(addfile))
                                 aExStand->itsAddFiles->insert(addfile);
                              else delete addfile;      
                           }
                        }
                        if (aKey)
                        {
                           CStringRWC * addkey = new CStringRWC(aKey);
                           if (!aExStand->itsAddKeys)
                              aExStand->itsAddKeys = new RWOrdered ((size_t) 5);
                           if (!aExStand->itsAddKeys->contains(addkey))
                              aExStand->itsAddKeys->insert(addkey);
                           else
                              delete addkey;
                        }
                     }
                  }
               }
               aStdAddFileKeyRS->MoveNext();
            }
         }

         _RecordsetPtr anAddFileKeyRS("ADODB.Recordset");
         aSQLStmt  = "Select Groups, AddFiles, FVSKeywords From ";
         if (itsConnectionType == CT_MSEXCEL || itsConnectionType == CT_MSEXCEL03)
            aSQLStmt += (nameForm == 1) ? "[NRV_FVS_GroupAddFilesAndKeywords_VM$]" : "[FVS_GroupAddFilesAndKeywords$]";
         else
            aSQLStmt += (nameForm == 1) ? "NRV_FVS_GroupAddFilesAndKeywords_VM" : "FVS_GroupAddFilesAndKeywords";
         aSQLStmt += ";";
         anAddOK = 1;
         try
         {
            anAddFileKeyRS->Open(aSQLStmt.data(), aCS.data(), adOpenStatic, adLockReadOnly, adCmdText);
         }
         catch(_com_error & ce)
         {
            anAddOK=0;
         }
         if (anAddOK && !anAddFileKeyRS->EndOfFile)
         {
            while(!anAddFileKeyRS->EndOfFile)
            {
               char * aGroup=NULL;  // can be null
               _bstr_t aGRP;
               aV = anAddFileKeyRS->Fields->GetItem("Groups")->Value;
               if (aV.GetVARIANT().vt != VT_NULL)
               {
                  aGRP = aV;
                  aGroup = aGRP;
                  ExGroup * exGroup = itsAllData->FindGroup(aGroup);
                  if (!exGroup)
                  {
                     exGroup = new ExGroup(aGroup, itsAllData);
                    itsAllData->itsExGroups->insert(exGroup);
                  }
                  char * anAdd=NULL;  // can be null
                  _bstr_t aAF;
                  aV = anAddFileKeyRS->Fields->GetItem("AddFiles")->Value;
                  if (aV.GetVARIANT().vt != VT_NULL)
                  {
                     aAF = aV;
                     anAdd = aAF;
                     if (strlen(anAdd))
                     {
                        RWCTokenizer next(anAdd);
                        RWCString token;
                        while (!(token=next("\r\t\n\0")).isNull())
                        {
                           CStringRWC * addfile = new CStringRWC(token);
                           if (!exGroup->itsAddFiles)
                              exGroup->itsAddFiles = new RWOrdered ((size_t) 5);
                           if (!exGroup->itsAddFiles->contains(addfile))
                              exGroup->itsAddFiles->insert(addfile);
                           else
                              delete addfile;
                        }
                        // This bit of code is the same as above for record 'D'
                        CStringRWC * addfile = new CStringRWC(anAdd);
                     }
                  }
                  anAdd=NULL;  // can be null
                  aV = anAddFileKeyRS->Fields->GetItem("FVSKeywords")->Value;
                  if (aV.GetVARIANT().vt != VT_NULL)
                  {
                     aAF = aV;
                     anAdd = aAF;
                     if (strlen(anAdd))
                     {
                        // This bit of code is the same as above for record 'D'
                        CStringRWC * addkey = new CStringRWC(anAdd);
                        if (!exGroup->itsAddKeys)
                           exGroup->itsAddKeys = new RWOrdered ((size_t) 10);
                        if (!exGroup->itsAddKeys->contains(addkey))
                           exGroup->itsAddKeys->insert(addkey);
                        else
                           delete addkey;
                     }
                  }
               }
               anAddFileKeyRS->MoveNext();
            }
         }
      }
      CoUninitialize();

      break;

      default:
      {
         Warn("Data base connection method for this location is not supported.");
         return;
         break;
      }
   }

   delete []record;

   // go over all the stands.  If there is a stand that has NO group, then
   // make sure it is a member of at least ONE group.

   ExGroup * groupForStandsInNoDefinedGroup = NULL;

   RWOrderedIterator next(*itsExStands);
   ExStand * exStand;
   while (exStand = (ExStand *) next())
   {
      if (!exStand->itsExGroupMembership)
      {
         if (!groupForStandsInNoDefinedGroup)
         {
            groupForStandsInNoDefinedGroup = itsAllData-> FindGroup("!StandsInNoDefinedGroup");
            if (!groupForStandsInNoDefinedGroup)
            {
               groupForStandsInNoDefinedGroup = new ExGroup("!StandsInNoDefinedGroup", itsAllData);
               itsAllData->itsExGroups->insert(groupForStandsInNoDefinedGroup);
            }
         }
         if (!groupForStandsInNoDefinedGroup->itsExLocationMembership)
              groupForStandsInNoDefinedGroup->itsExLocationMembership = new RWOrdered ((size_t) 20);
         if (!groupForStandsInNoDefinedGroup->itsExLocationMembership->contains(this))
              groupForStandsInNoDefinedGroup->itsExLocationMembership->insert(this);
         if (!itsExGroupMembership)
            itsExGroupMembership = new RWOrdered ((size_t) 10);
         if (!itsExGroupMembership->contains(groupForStandsInNoDefinedGroup))
              itsExGroupMembership->insert(groupForStandsInNoDefinedGroup);
         exStand->itsExGroupMembership = new RWOrdered ((size_t) 1);
         exStand->itsExGroupMembership->insert(groupForStandsInNoDefinedGroup);
      }
   }
}


void ExLocation::FreeLocationData( void )
{
   // if the location data are loaded, and we want to refresh them,
   // we may need to free them.

   if (itsExStands)
   {
      itsExStands->clearAndDestroy();
      delete itsExStands;
      itsExStands = NULL;
   }
}




RWDEFINE_COLLECTABLE(ExGroup,EXGROUP)

ExGroup::ExGroup (const char      * groupName,
                  AllLocationData * theAllData)
        :CStringRWC (groupName),
         itsAllData (theAllData),
         itsExLocationMembership(NULL),
         itsAddFiles(NULL),
         itsAddKeys(NULL)
{}


ExGroup::ExGroup ( void )
        :CStringRWC (),
         itsAllData (NULL),
         itsExLocationMembership(NULL),
         itsAddFiles(NULL),
         itsAddKeys(NULL)
{}

ExGroup::~ExGroup ( void )
{
   delete itsExLocationMembership;
   if (itsAddFiles)
   {
      itsAddFiles->clearAndDestroy();
      delete itsAddFiles;
   }
   if (itsAddKeys)
   {
      itsAddKeys->clearAndDestroy();
      delete itsAddKeys;
   }
}



RWBoolean ExGroup::isEqual  (const RWCollectable* a) const
{
   return this == a;
}



int ExGroup::compareTo(const RWCollectable* a) const
{
   if (this == a) return 0;
   return strcmp(data(),((ExGroup *) a)->data());
}




RWDEFINE_COLLECTABLE(ExStand,EXSTAND)

ExStand::ExStand ( char * ptr, ExLocation * theExLocation )
        :CStringRWC (),
         itsExLocation(theExLocation),
         itsExGroupMembership( NULL ),
         itsAddFiles( NULL ),
         itsAddKeys( NULL ),
         itsFVSReadyTreeFileName( NULL ),
         itsVariants( NULL ),
         itsRecordB( NULL ),
         itsStandCN( NULL ),
         itsWithPointDataFlag((int) 0)
{
   // Load the data.

   if (ptr && *ptr) *((CStringRWC *) this) = ptr;
   ptr = strtok(NULL," ");
   itsFVSReadyTreeFileName = newChar(ptr);
   ptr = strtok(NULL," ");
   itsWithPointDataFlag = (strncmp("WithPointData",ptr,
                                   (int) strlen(ptr)) == 0) ||
                          (strncmp("1",ptr,
                                   (int) strlen(ptr)) == 0);
   ptr = strtok(NULL,"@\n");
   itsVariants = newChar(ptr);
   for (char * c = itsVariants; *c; c++) *c = (char) tolower((int) *c);
}



ExStand::ExStand (const char * standid, const char * variant,
                  const char * standcn, const char * invyear,
                  ExLocation * theLocation)
        :CStringRWC (standid),
         itsExLocation(theLocation),
         itsExGroupMembership( NULL ),
         itsAddFiles( NULL ),
         itsAddKeys( NULL ),
         itsFVSReadyTreeFileName( NULL ),
         itsVariants( NULL ),
         itsRecordB( NULL ),
         itsStandCN( NULL ),
         itsWithPointDataFlag((int) 0)
{

   itsVariants = newChar(variant);
   for (char * c = itsVariants; *c; c++) *c = (char) tolower((int) *c);

   if (standcn) itsStandCN = newChar(standcn);

   // Only need Inventory Year -- Its easy it being the first item on the B record...
   itsRecordB = newChar(invyear);
}



ExStand::ExStand ( void )
        :CStringRWC (),
         itsExLocation( NULL ),
         itsExGroupMembership( NULL ),
         itsAddFiles( NULL ),
         itsAddKeys( NULL ),
         itsFVSReadyTreeFileName( NULL ),
         itsVariants( NULL ),
         itsRecordB( NULL ),
         itsStandCN( NULL )
{}



ExStand::~ExStand ( void )
{
   if (itsExGroupMembership)    delete itsExGroupMembership;
   if (itsAddFiles)
   {
      itsAddFiles->clearAndDestroy();
      delete itsAddFiles;
   }
   if (itsAddKeys)
   {
      itsAddKeys->clearAndDestroy();
      delete itsAddKeys;
   }
   if (itsFVSReadyTreeFileName) delete [] itsFVSReadyTreeFileName;
   if (itsVariants)             delete [] itsVariants;
   if (itsRecordB)              delete [] itsRecordB;
   if (itsStandCN)              delete [] itsStandCN;
}



RWBoolean ExStand::isEqual  (const RWCollectable* a) const
{
   return this == a;
}



int ExStand::compareTo(const RWCollectable* a) const
{
   if (this == a) return 0;
   return strcmp(data(),((ExStand *) a)->data());
}



inline void SetString (char ** theString, char * blankString)
{
   if (! *theString) *theString = blankString;
   else if (**theString == '@') **theString = ' ';
}



int ExStand::BuildAnExStand (char * standBuffer,
                             int    bufferLength)
{
   int nochars;

   char * where   = standBuffer;
   char * invyear = NULL;
   char * lat     = NULL;
   char * lngt    = NULL;
   char * loc     = NULL;
   char * hab     = NULL;
   char * yrorig  = NULL;
   char * asp     = NULL;
   char * slope   = NULL;
   char * elev    = NULL;
   char * baf     = NULL;
   char * fix     = NULL;
   char * brkdbh  = NULL;
   char * nplots  = NULL;
   char * nonstk  = NULL;
   char * samwt   = NULL;
   char * pstk    = NULL;
   char * dgm     = NULL;
   char * dgp     = NULL;
   char * hgm     = NULL;
   char * hgp     = NULL;
   char * mp      = NULL;
   char * bamax   = NULL;
   char * sdmax   = NULL;
   char * sspe    = NULL;
   char * sindx   = NULL;
   char * modtype = NULL;
   char * physreg = NULL;
   char * fortype = NULL;
   char * state   = NULL;
   char * county  = NULL;
   char * pvRefCode  = NULL;

                                                      // OriginID data.
   nochars = sprintf (where,"Stand %s at %s\n",
             data(), itsExLocation->data());
   where += nochars;

   // Screen keyword is added by default.

   nochars = sprintf (where,"Screen\n");
   where += nochars;

   if (itsStandCN && strlen(itsStandCN) > 0)
   {
      nochars = sprintf (where,"StandCN\n%s\n", itsStandCN);
      where += nochars;
   }

   // The rest of the data comes from RecordB...
   // Make a copy of RecordB...we don't want to destroy it.

   if (itsRecordB)
   {
      char * record = new char[strlen(itsRecordB)+1];
      strcpy(record,itsRecordB);

      // point to the various parts of the record.

      invyear = strtok (record," ");
      if (invyear) lat     = strtok (NULL," ");
      if (lat    ) lngt    = strtok (NULL," ");
      if (lngt   ) loc     = strtok (NULL," ");
      if (loc    ) hab     = strtok (NULL," ");
      if (hab    ) yrorig  = strtok (NULL," ");
      if (yrorig ) asp     = strtok (NULL," ");
      if (asp    ) slope   = strtok (NULL," ");
      if (slope  ) elev    = strtok (NULL," ");
      if (elev   ) baf     = strtok (NULL," ");
      if (baf    ) fix     = strtok (NULL," ");
      if (fix    ) brkdbh  = strtok (NULL," ");
      if (brkdbh ) nplots  = strtok (NULL," ");
      if (nplots ) nonstk  = strtok (NULL," ");
      if (nonstk ) samwt   = strtok (NULL," ");
      if (samwt  ) pstk    = strtok (NULL," ");
      if (pstk   ) dgm     = strtok (NULL," ");
      if (dgm    ) dgp     = strtok (NULL," ");
      if (dgp    ) hgm     = strtok (NULL," ");
      if (hgm    ) hgp     = strtok (NULL," ");
      if (hgp    ) mp      = strtok (NULL," ");
      if (mp     ) bamax   = strtok (NULL," ");
      if (bamax  ) sdmax   = strtok (NULL," ");
      if (sdmax  ) sspe    = strtok (NULL," ");
      if (sspe   ) sindx   = strtok (NULL," ");
      if (sindx  ) modtype = strtok (NULL," ");
      if (modtype) physreg = strtok (NULL," ");
      if (physreg) fortype = strtok (NULL," ");
      if (fortype) state   = strtok (NULL," ");
      if (state  ) county  = strtok (NULL," ");
      if (county  ) pvRefCode  = strtok (NULL," \n");
   }

   char * blankString = " ";

   SetString (&invyear, blankString);
   SetString (&lat    , blankString);
   SetString (&lngt   , blankString);
   SetString (&loc    , blankString);
   SetString (&hab    , blankString);
   SetString (&yrorig , blankString);
   SetString (&asp    , blankString);
   SetString (&slope  , blankString);
   SetString (&elev   , blankString);
   SetString (&baf    , blankString);
   SetString (&fix    , blankString);
   SetString (&brkdbh , blankString);
   SetString (&nplots , blankString);
   SetString (&nonstk , blankString);
   SetString (&samwt  , blankString);
   SetString (&pstk   , blankString);
   SetString (&dgm    , blankString);
   SetString (&dgp    , blankString);
   SetString (&hgm    , blankString);
   SetString (&hgp    , blankString);
   SetString (&mp     , blankString);
   SetString (&bamax  , blankString);
   SetString (&sdmax  , blankString);
   SetString (&sspe   , blankString);
   SetString (&sindx  , blankString);
   SetString (&modtype, blankString);
   SetString (&physreg, blankString);
   SetString (&fortype, blankString);
   SetString (&state,   blankString);
   SetString (&county,  blankString);
   SetString (&pvRefCode,  blankString);

                                                      // InvYear
   if (*invyear != ' ')
   {
      nochars = sprintf (where,"InvYear%13s\n", invyear);
      where += nochars;

      // let the year object know what the inventory year
      // for this stand is.

      theSpGlobals->yearObj->AdjustInvYearRange((int)ConvertToLong(invyear));
   }
   else
   {
      // the inventory year is missing in the data.  Set it to the
      // current year.

      nochars = sprintf (where,"InvYear%13d\n",
                                theSpGlobals->yearObj->currentYear);
      where += nochars;
      theSpGlobals->yearObj->AdjustInvYearRange(
                             theSpGlobals->yearObj->currentYear);
   }

                                                      // ModType

   if (*modtype != ' ' || *physreg != ' ' || *fortype != ' ')
   {
      nochars = sprintf (where,"ModType   ");   where += nochars;
      nochars = sprintf (where,"%10s",modtype); where += nochars;
      nochars = sprintf (where,"%10s",physreg); where += nochars;
      nochars = sprintf (where,"%10s",fortype); where += nochars;
      while (*where == NULL || *where == ' ')   where--;
      *(++where)='\n'; where++;
   }
                                                      // StdInfo

   if ((*invyear != ' ' && *yrorig != ' ') ||
        *loc     != ' ' || *hab    != ' '  || *asp != ' ' ||
        *slope   != ' ' || *elev   != ' '  || *pvRefCode   != ' ')
   {
      nochars = sprintf (where,"StdInfo   "); where += nochars;
      nochars = sprintf (where,"%10s",loc);   where += nochars;
      nochars = sprintf (where,"%10s",hab);   where += nochars;
      if (*invyear != ' ' && *yrorig != ' ')
      {
         int inv, yro;
         sscanf (invyear,"%d", &inv);
         sscanf (yrorig, "%d", &yro);
         nochars = sprintf (where,"%10d", inv-yro); where += nochars;
      }
      else
      {
         nochars = sprintf (where,"%10s"," "); where += nochars;
      }
      nochars = sprintf (where,"%10s",asp);   where += nochars;
      nochars = sprintf (where,"%10s",slope); where += nochars;
      nochars = sprintf (where,"%10s",elev);  where += nochars;
      nochars = sprintf (where,"%10s",pvRefCode);  where += nochars;

      // Scan back to the last non-blank char.  Set the following char
      // equal to a new line and point to the position following
      // the newline.

      while (*where == NULL || *where == ' ') where--;
      *(++where)='\n'; where++;
   }
                                                      // Locate

   if (*lat    != ' ' || *lngt   != ' '  || *state != ' ' ||
       *county != ' ' )
   {
      nochars = sprintf (where,"Locate    ");  where += nochars;
      nochars = sprintf (where,"%10s",lat);    where += nochars;
      nochars = sprintf (where,"%10s",lngt);   where += nochars;
      nochars = sprintf (where,"%10s",state);  where += nochars;
      nochars = sprintf (where,"%10s",county); where += nochars;

      // Scan back to the last non-blank char.  Set the following char
      // equal to a new line and point to the position following
      // the newline.

      while (*where == NULL || *where == ' ') where--;
      *(++where)='\n'; where++;
   }

                                                      // Design

   if (*baf     != ' ' || *fix    != ' ' || *brkdbh != ' ' || *nplots != ' ' ||
       *nonstk  != ' ' || *samwt  != ' ' || *pstk   != ' ')
   {
      nochars = sprintf (where,"Design    ");  where += nochars;
      nochars = sprintf (where,"%10s",baf);    where += nochars;
      nochars = sprintf (where,"%10s",fix);    where += nochars;
      nochars = sprintf (where,"%10s",brkdbh); where += nochars;
      nochars = sprintf (where,"%10s",nplots); where += nochars;
      nochars = sprintf (where,"%10s",nonstk); where += nochars;
      nochars = sprintf (where,"%10s",samwt);  where += nochars;
      nochars = sprintf (where,"%10s",pstk);   where += nochars;
      while (*where == NULL || *where == ' ') where--;
      *(++where)='\n'; where++;
   }

                                                      // Growth
   if (*dgm != ' ' || *dgp != ' ' || *hgm != ' ' ||
       *hgp != ' ' || *mp  != ' ')
   {
      nochars = sprintf (where,"Growth    "); where += nochars;
      nochars = sprintf (where,"%10s",dgm);   where += nochars;
      nochars = sprintf (where,"%10s",dgp);   where += nochars;
      nochars = sprintf (where,"%10s",hgm);   where += nochars;
      nochars = sprintf (where,"%10s",hgp);   where += nochars;
      nochars = sprintf (where,"%10s",mp);    where += nochars;
      while (*where == NULL || *where == ' ') where--;
      *(++where)='\n'; where++;
   }

                                                      // BAMax
   if (*bamax != ' ')
   {
      nochars = sprintf (where,"BAMax%15s\n",bamax); where += nochars;
   }

                                                      // SDIMax
   if (*sdmax != ' ')
   {
      nochars = sprintf (where,"SDIMax%24s\n",sdmax); where += nochars;
   }

                                                      // SiteCode
   if (*sspe  != ' ' || *sindx  != ' ')
   {
      nochars = sprintf (where,"SiteCode  ");  where += nochars;
      nochars = sprintf (where,"%10s",sspe);   where += nochars;
      nochars = sprintf (where,"%10s",sindx);  where += nochars;
      while (*where == NULL || *where == ' ') where--;
      *(++where)='\n'; where++;
   }

   // mark the beginning of the treedata...
   // It will become our treeKeywords for the stand.

   *where++='~';
                                                      // TreeData
   if (itsFVSReadyTreeFileName)
   {
      for (char * ptr = strtok(itsFVSReadyTreeFileName,"+");
           ptr != NULL;
           ptr = strtok(NULL,"+"))
      {
         if (ptr > itsFVSReadyTreeFileName)
         {
            nochars = sprintf (where,"\n");
            where += nochars;
         }
         nochars = sprintf (where,
                            "Open               2\n%s\n",ptr);
         where += nochars;
         nochars = sprintf (where,"TreeData           2");
         where += nochars;
         if (itsWithPointDataFlag)
         {
            nochars = sprintf (where,"         1");
            where += nochars;
         }
         nochars = sprintf (where,"\nClose              2");
         where += nochars;
      }
   }
   //else if (!itsStandCN)
   //{
   //   nochars = sprintf (where,"NoTrees");
   //   where += nochars;
   //}

   *where=0;

   return (int) (where-standBuffer+1);
}


char * ExStand::newChar(const char * ptr)
{
   char * c = NULL;
   if (ptr && *ptr && *ptr != '@')
   {
      int len = strlen(ptr);
      c = new char[len+1];
      strcpy(c,ptr);
   }
   return c;
}

/* Additional sample ADO connections...

ODBC DSN Less Connection
========================

ODBC Driver for dBASE
---------------------
strConnection = _T("Driver={Microsoft dBASE Driver (*.dbf)};DriverID=277;"
                   "Dbq=c:\\DatabasePath;");
Note: You must specify the filename in the SQL statement... For example:
CString strQuery = _T("Select Name, Address From Clients.dbf");

ODBC Driver for Excel
---------------------
strConnection = _T("Driver={Microsoft Excel Driver (*.xls)};DriverId=790;"
    bq=C:\\DatabasePath\\DBSpreadSheet.xls;DefaultDir=c:\\databasepath;");

ODBC Driver for Text
--------------------
strConnection = _T("Driver={Microsoft Text Driver (*.txt; *.csv)};"
        "Dbq=C:\\DatabasePath\\;Extensions=asc,csv,tab,txt;");
If you are using tab delimited files, you must create the schema.ini file, and you must inform the Format=TabDelimited option in your connection string.
Note: You must specify the filename in the SQL statement... For example:
CString strQuery = _T("Select Name, Address From Clients.csv");

Visual FoxPro
-------------
If you are using a database container, the connection string is the following:
strConnection = _T("Driver={Microsoft Visual Foxpro Driver};UID=;"
    ourceType=DBC;SourceDB=C:\\DatabasePath\\MyDatabase.dbc;Exclusive=No");
If you are working without a database container, you must change the SourceType parameter by DBF as in the following connection string:
strConnection = _T("Driver={Microsoft Visual Foxpro Driver};UID=;"
    "SourceType=DBF;SourceDB=C:\\DatabasePath\\MyDatabase.dbc;Exclusive=No");

ODBC Driver for Access
----------------------
strConnection = _T("Driver={Microsoft Access Driver (*.mdb)};"
        "Dbq=c:\\DatabasePath\\dbaccess.mdb;Uid=;Pwd=;");
If you are using a Workgroup (System database): you need to inform the SystemDB Path, the User Name and its password. For that, you have two solutions: inform the user and password in the connection string or in the moment of the open operation. For example:
strConnection = _T("Driver={Microsoft Access Driver (*.mdb)};"
        "Dbq=C:\\VC Projects\\ADO\\Samples\\AdoTest\\dbTestSecurity.mdb;"
        "SystemDB=C:\\Program Files\\Microsoft Office\\Office\\SYSTEM.mdw;"
        "Uid=Carlos Antollini;Pwd=carlos");
or may be:
strConnection = _T("Driver={Microsoft Access Driver (*.mdb)};"
        "Dbq=C:\\VC Projects\\ADO\\Samples\\AdoTest\\dbTestSecurity.mdb;"
        "SystemDB=C:\\Program Files\\Microsoft Office\\Office\\SYSTEM.mdw;");
if(pDB.Open(strConnection, "DatabaseUser", "DatabasePass"))
{
    DoSomething();
    pDB.Close();
}
If you want to open in Exclusive mode:
strConnection = _T("Driver={Microsoft Access Driver (*.mdb)};"
        "Dbq=c:\\DatabasePath\dbaccess.mdb;Exclusive=1;");

ODBC Driver for SQL Server
--------------------------
For Standard security:
strConnection = _T("Driver={SQL Server};Server=MyServerName;"
        "Trusted_Connection=no;"
        "Database=MyDatabaseName;Uid=MyUserName;Pwd=MyPassword;");
For Trusted Connection security (Microsoft Windows NT integrated security):
strConnection = _T("Driver={SQL Server};Server=MyServerName;"
     "Database=myDatabaseName;Uid=;Pwd=;");
Also, you can use the parameter Trusted_Connection that indicates that you are using the Microsoft Windows NT Authentication Mode to authorize user access to the SQL Server database. For example:
strConnection = _T("Driver={SQL Server};Server=MyServerName;"
    "Database=MyDatabaseName;Trusted_Connection=yes;");
If the SQL Server is running in the same computer, you can replace the name of the server by the word (local) like in the following sample:
strConnection = _T("Driver={SQL Server};Server=(local);"
        "Database=MyDatabaseName;Uid=MyUserName;Pwd=MyPassword;");
If you want to connect with a remote SQL Server, you must inform the address, the port, and the Network Library to use:
The Address parameter must be an IP address and must include the port. The Network parameter can be one of the following:
•      dbnmpntw Win32 Named Pipes
•      dbmssocn Win32 Winsock TCP/IP
•      dbmsspxn Win32 SPX/IPX
•      dbmsvinn Win32 Banyan Vines
•      dbmsrpcn Win32 Multi-Protocol (Windows RPC)
For more information, see Q238949.
strConnection = _T("Driver={SQL Server};Server=130.120.110.001;"
     "Address=130.120.110.001,1052;Network=dbmssocn;Database=MyDatabaseName;"
     "Uid=myUsername;Pwd=myPassword;");

ODBC Driver for Oracle
----------------------
For the current Oracle ODBC driver from Microsoft:
strConnect = _T("Driver={Microsoft ODBC for Oracle};Server=OracleServer.world;"
        "Uid=MyUsername;Pwd=MyPassword;");
For the older Oracle ODBC driver from Microsoft:
strConnect = _T("Driver={Microsoft ODBC Driver for Oracle};"
     "ConnectString=OracleServer.world;Uid=myUsername;Pwd=myPassword;");

ODBC Driver for MySQL
---------------------
If you want to connect to a local database, you can use a connection string like the following:
strConnect = _T("Driver={MySQL ODBC 3.51 Driver};Server=localhost;"
     "Database=MyDatabase;User=MyUserName;Password=MyPassword;Option=4;");
If you want to connect with a remote database, you need to specify the name of the server or its IP in the Server parameter. If the Port is distinct to 3306 (default port), you must specify it.
strConnect = _T("Driver={mySQL ODBC 3.51 Driver};Server=MyRemoteHost;"
     "Port=3306;Option=4;Database=MyDatabase;Uid=MyUsername;Pwd=MyPassword;");
The parameter Option can be one or more of the following values:
•      1 - The client can't handle that MyODBC returns the real width of a column.
•      2 - The client can't handle that MySQL returns the true value of affected rows. If this flag is set then MySQL returns 'found rows' instead. One must have MySQL 3.21.14 or newer to get this to work.
•      4 - Make a debug log in c:\myodbc.log. This is the same as putting MYSQL_DEBUG=d:t:O,c::\myodbc.log in AUTOEXEC.BAT.
•      8 - Don't set any packet limit for results and parameters.
•      16 - Don't prompt for questions even if driver would like to prompt.
•      32 - Enable or disable the dynamic cursor support. This is not allowed in MyODBC 2.50.
•      64 - Ignore use of database name in 'database.table.column'.
•      128 - Force use of ODBC manager cursors (experimental).
•      256 - Disable the use of extended fetch (experimental).
•      512 - Pad CHAR fields to full column length.
•      1024 - SQLDescribeCol() will return fully qualified column names.
•      2048 - Use the compressed server/client protocol.
•      4096 - Tell server to ignore space after function name and before '(' (needed by PowerBuilder). This will make all function names keywords!
•      8192 - Connect with named pipes to a MySQLd server running on NT.
•      16384 - Change LONGLONG columns to INT columns (some applications can't handle LONGLONG).
•      32768 - Return 'user' as Table_qualifier and Table_owner from SQLTables (experimental).
•      65536 - Read parameters from the client and ODBC groups from my.cnf.
•      131072 - Add some extra safety checks (should not be needed but...).
If you want to have multiple options, you should add the above flags! For example: 16 + 1024 = 1030 and use Option= 1030;.
For more information, go to MyODBC Reference Manual.

ODBC Driver for AS400
---------------------
strConnect = _T("Driver={Client Access ODBC Driver (32-bit)};System=myAS400;"
      "Uid=myUsername;Pwd=myPassword;");
ODBC Driver for SyBase
strConnect = _T("Driver={Sybase System 10};Srvr=MyServerName;Uid=MyUsername;"
       "Pwd=myPassword;");

ODBC Driver for Sybase SQL AnyWhere
-----------------------------------
strConnect = _T("ODBC;Driver=Sybase SQL Anywhere 5.0;"
    "DefaultDir=C:\\DatabasePath\;Dbf=C:\\SqlAnyWhere50\\MyDatabase.db;"
    "Uid=MyUsername;Pwd=MyPassword;Dsn=\"\";");

DSN Connection
==============

ODBC DSN
--------
strConnect = _T("DSN=MyDSN;Uid=MyUsername;Pwd=MyPassword;");

*/
