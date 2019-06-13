/********************************************************************

   File Name:   suppdefs.hpp
   Author:      nlc.
   Date:        08/21/96
                                                                    *
 ********************************************************************/

#ifndef SupposeDefs_H
#define SupposeDefs_H

// needed window identifications

#define SELECTIONWindowID         150
#define SELECTSIMSTANDWindowID    152
#define GROUPMEMBERSHIPWindowID   153
#define CLEARCUTWindowID          155
#define SEEDTREEWindowID          160
#define SHELTERWOODWindowID       165
#define ThinFromAboveWindowID     167
#define ThinFromBelowWindowID     168
#define ManagementWindowID        200
#define USEFVSKEYWindowID         201
#define SCHBYCONDWindowID         202
#define EDITSimFileWindowID       203
#define SELVARWindowID            204
#define POSTPROCWindowID          205
#define RUNWIZWindowID            206
#define READFVSOUTPUTWindowID     207
#define DELETEStandWindowID       208
#define DELETEGroupWindowID       209
#define SIMNOTESWindowID          210
#define EDITLocsWindowID          211
#define EDITSLFWindowID           212
#define EDITFVSTreeDataWindowID   213
#define PlantNaturalWindowID      214
#define AVERAGEFVSOUTPUTWindowID  215
#define ModelOutWindowID          216
#define ModelModWindowID          217

// Species of RWCollectable

#define GROUP        10
#define STAND        11
#define COMPONENT    12
#define POSTPROC     13

#define EXLOCATION   14
#define EXGROUP      15
#define EXSTAND      16

#define CCATEGORY    17
#define ITEMINCCAT   18

#define FVSCASE         19
#define FVSREPORT       20
#define FVSVARIABLE     21
#define FVSOBSERVATION  22

#define MSTEXT          23
#define PKEYDATA        24

#define EDITLOC         25
#define EDITGROUP       26
#define EDITSTAND       27
#define MYUSERDATA      28
#define PPECOMPONENT    29
#define EDITDSN         30

// ".bmp" image identifications

#define EXPANDEDIMAGE   440
#define COLLAPSEDIMAGE  441 
#define TERMNODEIMAGE   442

// Commands propagated to all opened windows.

#define NULLUpdateMessage         8000
#define SENDUpdateCommand         8001
#define SUPPOSEResetCommand       8002
#define EXTENSIONListChanged      8003
#define VARIANTSelectionChanged   8004
#define PROGRAMSelectionChanged   8005
#define STANDCountChanged         8006
#define STANDNameChanged          8007
#define GROUPCountChanged         8008
#define GROUPNameChanged          8009
#define GROUPMembershipChanged    8010 // not issued when the cause is a 
                                       // change in the stand count. 
#define COMPONENTSListChanged     8011
#define FVSCaseListIncreased      8012 
#define FVSCaseListDecreased      8013
#define CURRStandOrGroupChange    8014
#define EXDataUpdated             8015 // theData can point to the file name
                                       // of the file that changed

// Definition of large neg and positive integers.

#define SpNEGMAX -29999
#define SpPOSMAX  29999

// Definition of menu item numbers, and related info.

#define PROJ1                        100

#define TASK_MENUBAR                 1000

#define M_FILE_OPEN_LOC              1101
#define M_FILE_CHANGE_WORKING_DIR    1102

#define M_DATA_PREP                  1200
#define M_DATA_PREP_LOCATIONS        1201
#define M_DATA_PREP_STANDLIST        1202
#define M_DATA_PREP_FVSTREEDATA      1203

#define M_SIM_PREP                   1300
#define M_SIM_PREP_SIMULATION        1301
#define M_SIM_PREP_TIME              1302
#define M_SIM_PREP_MODEL             1303
#define M_SIM_PREP_MANAGEMENT        1304
#define M_SIM_PREP_MODEL_MOD         1305
#define M_SIM_PREP_USEKEYWORDS       1306
#define M_SIM_PREP_INSERT_FROM_FILE  1307
#define M_SIM_PREP_POST_PROCESSOR    1308
#define M_SIM_PREP_RUN               1309
#define M_SIM_PREP_EDIT_SELECTION    1310
#define M_SIM_PREP_CURRENT           1311
#define M_SIM_PREP_VARIANT           1312
#define M_SIM_PREP_SIMNOTES          1313
#define M_SIM_PREP_VIEWKEYWORDS      1314
#define M_SIM_PREP_DELETESTAND       1315
#define M_SIM_PREP_DELETEGROUP       1316

#define M_AFTER_SIM                  1400
#define M_AFTER_SIM_GRAPHICS         1401
#define M_AFTER_SIM_REPORTS          1402
#define M_AFTER_SIM_READFVSOUTPUT    1403
#define M_AFTER_SIM_AVERAGECASES     1404

#define M_PREFERENCES                1500
#define M_PREFERENCES_EDIT           1502

#define M_HELP_SUPPOSE_TUTORIAL      1601
//#define M_HELP_RUNWIZARD             1602		//Commented out 05/21/09 AJSM
#define M_HELP_DYNAMIC_ITEMS_SEPARATOR 1700
#define M_HELP_DYNAMIC_ITEMS_BASE    1701
#define M_HELP_DYNAMIC_ITEMS_MAX     1750

#define HELP_ONLY_MENU               2000

// new file string.

#define NEWFILE "*new file*"


#endif
