/*********************************************************************
 *

   File Name:    spfuncs.cpp
   Author:       nlc
   Date:         12/20/94

   see spfuncs.hpp and below for notes.

 *********************************************************************/

#include "PwrDef.h"
#include "appdef.hpp"

#include CText_i
#include NButton_i
#include NCheckBox_i
#include NEditControl_i
#include NListButton_i
#include NListEdit_i
#include NScrollText_i
#include NTextEdit_i

#include CStringCollection_i
#include CScroller_i
#include CStringRWC_i
#include CUnits_i
#include CView_i
#include CWindow_i

#include "spparms.hpp"
#include "fvslocs.hpp"
#include "schcond.hpp"
#include "warn_dlg.hpp"
#include "spfuncs.hpp"
#include "spfunc2.hpp"
#include "cparmrtn.hpp"
#include "filebrws.hpp"
#include "runstr.hpp"

#include <rw/bintree.h>
#include <rw/collstr.h>
#include <rw/rstream.h>
#include <rw/collect.h>
#include <rw/ctoken.h>

#include "boxsldr.hpp"
#include "boxtext.hpp"
#include "radiogrp.hpp"
#include "cmdline.hpp"
#include "schedobj.hpp"



void * BuildField(CWindow * theParent, int theCommandNumber,
                  CSubview * theWindow, MSText * theMSText,
                  int * startPoint, int fieldNumber, int * fieldType)
{
/*****
 *
    BuildField is used to build one field for data entry inside the
    GPKeyword window.  The arguments are:

    theParent    (input)  Only used by "scheduleBox"...it is a pointer to
                          an object with a DoCommand.  The DoCommand
                          is called when the scheduleBox "Condition" command
                          is pushed by the user.

                          if theParent is NULL (which is legal), scheduleBox
                          is not allowed, and theCommandNumber is ignored.

    theCommandNumber      The command number that is passed to the DoCommand
                 (input)  of theParent.

    theWindow    (input)  The window into which the Subviews created
                          herein are placed.

    theMSText    (input)  A pointer to the major section that contains
                          the PKeyData used to load theWindow.

    startPoint   (input/output)
                          The pixel on the vertical axes where the next
                          SubView is placed.

    fieldNumber  (input)  The field index number.

    fieldType    (output) The field type (see table below)

    return value (output) A pointer to the object containing the input data.
                          The type of object depends on fieldType.
 *
 *****/

   #if XVTWS==WIN32WS
   static char * far typesOfFields[] =
   #else
   static char * typesOfFields[] =
   #endif
      // fieldTypeString             fieldType  theAnswer
      // ---------------             ---------  ----------
      // End-of-fields                  -1      void * NULL
      // ErrorMessage                    0      void * NULL
      { "noInput",               //      1      void * NULL      WARNING:
        "scheduleBox",           //      2      ScheduleByObject Many routines use
        "intNumberBox",          //      3      BoxTextObject    fieldType values
        "numberBox",             //      4      BoxTextObject
        "intSliderBox",          //      5      BoxSliderObject
        "sliderBox",             //      6      BoxSliderObject
        "speciesSelection",      //      7      NListButton
        "habPaSelection",        //      8      NListButton
        "forestSelection",       //      9      NListButton
        "listButtonString",      //     10      NListButton
        "listButton",            //     11      NListButton
        "longListButton",        //     12      NListButton
        "listEdit",              //     13      NListEdit
        "longListEdit",          //     14      NListEdit
        "textEdit",              //     15      NEditControl
        "longTextEdit",          //     16      NEditControl
        "fileBrowse",            //     17      FileBrowseObject
        "checkBox",              //     18      NCheckBox
        "radioGroup",            //     19      RadioGroupObject
        "longListButtonString",  //     20      NListButton
         NULL
      };

   void * theAnswer = (void *) NULL;
   char pkeyString[15];
   char tmpString[300];
   char * ptrS;
   const char * ptrConst;
   CStringRW ErrorMessage;
   RWOrdered * listButtonList = (RWOrdered *) NULL;
   RWOrdered * radioButtonList = (RWOrdered *) NULL;
   int position, count, nItems;
   int leadingp, ascentp, descent;
   int nSpecies = 0;
   char * defaultSpecies = NULL;
   char * deleteAll      = NULL;
   NTextEdit * nTextEdit;

   *fieldType = 0;

   if (fieldNumber == 0) return theAnswer;

   // create the first pkey to look for.

   sprintf (pkeyString,"f%d", fieldNumber);

   PKeyData * fieldPKeyData = theMSText->
      FindPKeyData(pkeyString,
                   theSpGlobals->theVariants->GetSelectedVariant(),
                   FALSE);

   // if there is no PKeyData for this field, then check on a
   // description.

   if (fieldPKeyData == NULL)
   {
      // Signal that we are at the end of the fields.

      *fieldType = -1;

      // If a description exists, then add it to the scroller
      // inside a scrolled text box.

      PKeyData * descriptionPKeyData = theMSText->
         FindPKeyData("description",
                      theSpGlobals->theVariants->GetSelectedVariant(),
                      FALSE);

      if (descriptionPKeyData)
      {

         // find out how many pixels per line for the text.

         xvt_dwin_get_font_metrics(theWindow->GetCWindow()->GetXVTWindow(),
            &leadingp, &ascentp, &descent);

         new CText((CSubview*) theWindow, CPoint(8,*startPoint),
                  "Description: ");
         (*startPoint) += leadingp+ascentp+descent+8;

         int nlines = numberLines(descriptionPKeyData->pString.data());
         nlines = nlines*(leadingp+ascentp+descent);
         nTextEdit = new NTextEdit((CSubview*) theWindow,
                   CRect(8,*startPoint,555,(*startPoint)+nlines+descent),
                   TX_NOMENU | TX_WRAP | TX_READONLY);
         nTextEdit->AddParagraph(CStringRW (descriptionPKeyData->pString),
                   USHRT_MAX);
         (*startPoint) += nlines+8;

      }
      return theAnswer;
   }

   // if there is a "field", check if there is a "title" string.
   // and... render it in a text subview if it is present.

   sprintf (pkeyString,"f%dtitle", fieldNumber);

   PKeyData * titlePKeyData = theMSText->
      FindPKeyData(pkeyString,
                   theSpGlobals->theVariants->GetSelectedVariant(),
                   FALSE);

   if (titlePKeyData)
   {
      xvt_dwin_get_font_metrics(theWindow->GetCWindow()->GetXVTWindow(),
         &leadingp, &ascentp, &descent);
      int nlines = numberLines(titlePKeyData->pString.data());
      nlines = nlines*(leadingp+ascentp+descent);
      nTextEdit = new NTextEdit((CSubview*) theWindow,
                CRect(8,*startPoint,555,(*startPoint)+nlines),
                TX_NOMENU | TX_WRAP | TX_READONLY);
      nTextEdit->AddParagraph(titlePKeyData->pString.data(),USHRT_MAX);
      (*startPoint) += nlines+2;
   }

   // get the pstring for the original PKeyData.
   // make a copy, get first token.

   strncpy(tmpString, fieldPKeyData->pString.data(),299);
   ptrS = strtok(tmpString," ");

   // find out what kind of field this will be.

   *fieldType = -1;

   if (ptrS)
   {
      for (int i = 0; typesOfFields[i] != NULL; i++)
      {
         if (strcmp(typesOfFields[i],ptrS) == 0)
         {
            *fieldType = i;
            break;
         }
      }

      // set the pointer to the remainder of the text.
      // look for an eot (should provide the rest of the string)

      ptrS = strtok(NULL,"\4");
   }

   (*fieldType)++;

   // if theParent is NULL and fieldType is 2, then we have an error.

   if (!theParent && *fieldType == 2) *fieldType = 0;

   // if the fieldType > 2, then another PKey may be present.

   PKeyData * valuePKeyData = (PKeyData *) NULL;

   if (*fieldType > 1)
   {
      sprintf (pkeyString,"f%dv", fieldNumber);
      valuePKeyData = theMSText->
         FindPKeyData(pkeyString,
                      theSpGlobals->theVariants->GetSelectedVariant(),
                      FALSE);

      // if there is no value PKeyData and speciesSelection (7) is not the
      // field type, then change the field type to 0.
      if (valuePKeyData == (PKeyData *) NULL &&
         (*fieldType != 2 && *fieldType != 7 && *fieldType != 8 &&
     *fieldType != 15 && *fieldType != 16 && *fieldType != 17)) *fieldType = 0;

   }

   switch (*fieldType)
   {
      default:
      case 0:                         // ErrorMessage
      {
         ErrorMessage =  "The type of input element is not defined or not allowed.";
         ErrorMessage += "\nKeyword MSText Name = ";
         ErrorMessage += theMSText->data();
         ErrorMessage += "\nPKey = ";
         ErrorMessage += fieldPKeyData->data();
         ErrorMessage += "\nVariant = ";
         ErrorMessage += theSpGlobals->theVariants->GetSelectedVariant();
         ErrorMessage += "\nPString = ";
         ErrorMessage += fieldPKeyData->pString.data();

         sprintf (tmpString,"Field %d: Error found creating the entry.",
                  fieldNumber);

         new CText((CSubview*) theWindow, CPoint(8,*startPoint),
                    CStringRW(tmpString));

         (*startPoint) += 40;

         Warn (ErrorMessage.data());

         break;
      }
      case 1:                         // noInput
      {
         if (ptrS)
            new CText((CSubview*) theWindow, CPoint(8,*startPoint),
                       CStringRW (ptrS));
         else
         {
            sprintf (tmpString,
               "Field %d: No input data for this field.",
               fieldNumber);
            new CText((CSubview*) theWindow, CPoint(8,*startPoint),
                       CStringRW (tmpString));
         }
         (*startPoint) += 40;
         break;
      }
      case 2:                         // scheduleBox
      {
         theAnswer = new ScheduleByObject((CWindow *) theParent,
                     theCommandNumber, (CSubview*) theWindow,
                     CPoint(8,(*startPoint)+8));
         (*startPoint) += 96;
         if (valuePKeyData != (PKeyData *) NULL)
         {
            ((ScheduleByObject *) theAnswer)->
               itsField2->SetTitle(valuePKeyData->pString);
         }
         break;
      }
      case 3:                         // intNumberBox
      case 4:                         // numberBox
      {
         if (!ptrS) ptrS = " ";
         theAnswer = new BoxTextObject((CSubview*) theWindow,
                                       CRect(8,*startPoint,552,
                                             (*startPoint)+40),
                                       ptrS, valuePKeyData->pString,
                                       SPRIGHT);

         (*startPoint) += 40;
         break;
      }
      case 5:                         // intSliderBox
      case 6:                         // sliderBox
      {
         if (!ptrS) ptrS = " ";
         theAnswer = new BoxSliderObject((CSubview*) theWindow,
                                         CRect(8,*startPoint,
                                               552,(*startPoint)+70),
                                         ptrS,
                                         valuePKeyData->pString.data(),
                                         SPRIGHT,  NULL);

         (*startPoint) += 70;
         break;
      }
      case 7:                         // speciesSelection
      {
         if (!ptrS) ptrS = "Species";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)+4),
                    CStringRW (ptrS));

         // load the species list

         nSpecies = theSpGlobals->theSpecies->getNumberSpecies();

         listButtonList = new RWOrdered(( 2 > nSpecies+1 ) ? 2 : nSpecies+1);

         // if the valuePKeyData data is not NULL, then we will find out if
         // "All Species" should be droped and we will find out out the
         // default species is.

         if (valuePKeyData != (PKeyData *) NULL)
         {
            strncpy(tmpString, valuePKeyData->pString.data(),99);

            // find the deleteAll subCommand, if present.

            deleteAll = strstr(tmpString,"deleteAll");

            // if present, then change the deleteAll to blanks.

            if (deleteAll)
               for (char * ptr = deleteAll; isalpha((int) *ptr);
                    ptr++) *ptr = ' ';

            // now set the default species.

            defaultSpecies = strtok(tmpString," ");
            if (defaultSpecies &&
                *defaultSpecies == ' ') defaultSpecies = NULL;

         }

         // if "All species" will be a selection, then added it to the list.

         if (!deleteAll)
         {
            listButtonList->insert(new CStringRWC ("All species"));
         }

         // if All is the default, then set position = 0, otherwise -1

         position = -1;
         if (defaultSpecies)
            if (strcmp(defaultSpecies,"All") == 0) position = 0;

         count = 1;
         if (deleteAll) count = 0;
         if (nSpecies > -1)
         {
            for (int spIndex=0; spIndex<nSpecies; spIndex++)
            {
               listButtonList->insert(new CStringRWC (
                  theSpGlobals->theSpecies->getCommonName(spIndex)));

               if (defaultSpecies && strcmp(defaultSpecies,
                  theSpGlobals->theSpecies->getAlphaCode(spIndex)) == 0)
                  position = count;
               count++;
            }
         }

         // Load user defined SpGroups
         RWOrdered * SpGroupNames = theSpGlobals->runStream->GetSpGroups();
         CStringRWC * SpGroupName;
         RWOrderedIterator nextSpGroupName(*SpGroupNames);
         while (SpGroupName = (CStringRWC *) nextSpGroupName())
         {
            listButtonList->insert(SpGroupName);
         }
         SpGroupNames->clear(); // These items are shared, do not clearAndDestroy
         delete SpGroupNames;

         theAnswer = new NListButton((CSubview*) theWindow,
                     CRect(342,*startPoint,552,(*startPoint)+240),
                     *listButtonList, 0L);

         if (position > -1)
            ((NListButton *) theAnswer)->SelectItem(position);
         else
         {
            if (deleteAll) ((NListButton *) theAnswer)->DeselectAll();
            else ((NListButton *) theAnswer)->SelectItem(0);
         }
         (*startPoint) += 48;

         break;
      }
      case 8:                         // habPaSelection
      case 9:                         // forestSelection
      {
         if (*fieldType == 8)
         {
            if (!ptrS) ptrS = "HabType/PlantAssoc";
            nItems = theSpGlobals->theHabPa->getNumberItems();
         }
         else
         {
            if (!ptrS) ptrS = "Forests";
            nItems = theSpGlobals->theForests->getNumberItems();
         }

          new CText((CSubview*) theWindow, CPoint(8,(*startPoint)+4),
                    CStringRW (ptrS));

        // load the list

         listButtonList = new RWOrdered(( 2 > nItems+1 ) ? 2 : nItems+1);

         // position points to the default.

         position = 0;
         count = 0;
         if (nItems > -1)
         {
            for (int index=0; index<nItems; index++)
            {
               if (*fieldType == 8)
               {
                  listButtonList->insert(new CStringRWC (
                     theSpGlobals->theHabPa->getName(index)));

                  if (valuePKeyData != (PKeyData *) NULL)
                     if (strcmp(valuePKeyData->pString.data(),
                                theSpGlobals->theHabPa->
                                getNumericCode(index)) == 0)
                        position = count;
               }
               else
               {
                  listButtonList->insert(new CStringRWC (
                     theSpGlobals->theForests->getName(index)));

                  if (valuePKeyData != (PKeyData *) NULL)
                     if (strcmp(valuePKeyData->pString.data(),
                                theSpGlobals->theForests->
                                getNumericCode(index)) == 0)
                        position = count;
               }
               count++;
            }
         }

         theAnswer = new NListButton((CSubview*) theWindow,
                     CRect(342,*startPoint,552,(*startPoint)+240),
                     *listButtonList, 0L);

         ((NListButton *) theAnswer)->SelectItem(position);

         (*startPoint) += 48;

         break;
      }
      case 10:                        // listButtonString
      case 11:                        // listButton
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)+4),
                   CStringRW (ptrS));

         listButtonList = new RWOrdered(5);

         position = LoadRWOrdered(valuePKeyData->pString.data(),
                                  listButtonList);

         theAnswer = new NListButton((CSubview*) theWindow,
                     CRect(272,*startPoint,552,(*startPoint)+120),
                                     *listButtonList, 0L);

         ((NListButton *) theAnswer)->SelectItem(position);

         (*startPoint) += 48;
         break;
      }
      case 20:                        // longListButtonString
      case 12:                        // longListButton
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)),
                    CStringRW (ptrS));

         listButtonList = new RWOrdered(5);

         position = LoadRWOrdered(valuePKeyData->pString.data(),
                                  listButtonList);

         theAnswer = new NListButton((CSubview*) theWindow,
                     CRect(8,(*startPoint)+32,552,(*startPoint)+152),
                     *listButtonList, 0L);

         ((NListButton *) theAnswer)->SelectItem(position);

         (*startPoint) += 80;
         break;
      }
      case 13:                        // listEdit
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)+4),
                    CStringRW (ptrS));

         listButtonList = new RWOrdered((size_t) 5);

         position = LoadRWOrdered(valuePKeyData->pString.data(),
                                  listButtonList);

         theAnswer = new NListEdit ((CSubview*) theWindow,
                     CRect(272,*startPoint,552,(*startPoint)+120),
                     *listButtonList);

         (*startPoint) += 48;
         break;
      }
      case 14:                        // longListEdit
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)),
                    CStringRW (ptrS));

         listButtonList = new RWOrdered(5);

         position = LoadRWOrdered(valuePKeyData->pString.data(),
                                  listButtonList);

         theAnswer = new NListEdit ((CSubview*) theWindow,
                     CRect(8,(*startPoint)+32,552,(*startPoint)+152),
                     *listButtonList);

         (*startPoint) += 80;
         break;
      }
      case 15:                        // textEdit
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)+4),
                    CStringRW (ptrS));

         if (valuePKeyData) ptrConst = valuePKeyData->pString.data();
         else ptrConst = " ";

         theAnswer = new NEditControl ((CSubview*) theWindow,
                     CRect(272,(*startPoint),552,(*startPoint)+32),
                     ptrConst, 0L);

         (*startPoint) += 48;
         break;
      }
      case 16:                        // longTextEdit
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(8,(*startPoint)),
                    CStringRW (ptrS));

         if (valuePKeyData) ptrConst = valuePKeyData->pString.data();
         else ptrConst = " ";

         theAnswer = new NEditControl ((CSubview*) theWindow,
                     CRect(8,(*startPoint)+32,552,(*startPoint)+64),
                     ptrConst, 0L);

         (*startPoint) += 80;
         break;
      }
      case 17:                        // fileBrowse
      {
         ptrConst = NULL;
         if (valuePKeyData) ptrConst = valuePKeyData->pString.data();

         theAnswer = new FileBrowseObject ((CSubview*) theWindow,
                     CRect(8,(*startPoint),552,(*startPoint)+64),
                     ptrS, ptrConst);

         (*startPoint) += 68;
         break;
      }
      case 18:                        // NCheckBox
      {
         if (!ptrS) ptrS = " ";
         new CText((CSubview*) theWindow, CPoint(26,(*startPoint)+2),
                    CStringRW (ptrS));

         if (valuePKeyData) ptrConst = valuePKeyData->pString.data();
         else ptrConst = " ";

         theAnswer = new NCheckBox ((CSubview*) theWindow,
                     CRect(8,(*startPoint),20,(*startPoint)+24),
                     "", 0L);

         if (atoi(ptrConst) == 1)
            ((NCheckBox *) theAnswer)->Select();
         else
            ((NCheckBox *) theAnswer)->Deselect();

         (*startPoint) += 32;
         break;
      }
      case 19:                        // RadioGroupObject
      {
         if (!ptrS) ptrS = " ";

         radioButtonList = new RWOrdered();
         position = LoadRWOrdered(valuePKeyData->pString.data(), radioButtonList);

         theAnswer = new RadioGroupObject((CSubview*) theWindow,
                     CRect(8,*startPoint,552,(*startPoint)+((radioButtonList->entries()+1)*24)),
                     ptrS, radioButtonList);

         ((RadioGroupObject *) theAnswer)->SetSelection(position);

         (*startPoint) += ((radioButtonList->entries()+1)*24) + 12;

         delete radioButtonList;
         break;
      } 
   }

   return theAnswer;
}




int BuildTheKeywords(MSText * theMSText,
                     int * theFieldTypes,
                     void  ** theAnswerPtrs,
                     int numberOfFields,
                     int typeOfAnswer,
                     CStringRW & theAnswerString)
{
/*****
 *
    given the answers supplied in various "objects".

    theMSText     (input)  A pointer to the major section that contains
                           the PKeyData used to making the keyword.

    theFieldTypes (input)  A vector of integers that indicate what is in the
                           fields.

    theAnswerPtrs (input)  A vector of pointers to objects that contain the
                           user's selections.

    numberOfFields (input) The number of defined locations in theFieldTypes and
                           theAnswerPtrs.

    typeOfAnswer  (input)  0=the return string is a condition.
                           1=the return string is a keyword...if there is no
                           PKeyData giving the standard form, then the default
                           form is used.  This method is used in any case
                           where an answerForm PKey is present in theMSText.
                           2=the return string is a keyword but the return is
                           string is set up using PARMS form.  PKeyData must
                           exist for the form.

    theAnswerString (output)
                           a CStringRW containing the result of the getting the
                           answers.

    int return             0 = Did not build.
                           1 = Did build.

 *
 *****/

   CStringRW aCString = " ";
   char smallString[101];
   char helperPKey [21];
   char * theAnswer = NULL;
   const char * theForm = NULL;
   const char * ptr = NULL;
   static char standardAnswer[] = "answerForm";
   static char parmsAnswer[]    = "parmsForm";

   const char theDefaultForm[] =
      "Keyword   !1,10!!2,10!!3,10!!4,10!!5,10!!6,10!!7,10!";

   const char theStandardCrypticErrorMessage[] =
         "Suppose could not use/find necessary parameter data.\n"
         "Attempt to build FVS keywords failed.";


   // if the theFieldTypes are null, then there is no answer.

   if (!theFieldTypes) return 0;

   // set up the "Form" of the answer.

   if (theMSText)
   {
      const char * answerType;
      if (typeOfAnswer<=1)
         answerType = standardAnswer;
      else
         answerType = parmsAnswer;

      PKeyData * answerPKeyData = theMSText->
         FindPKeyData(answerType,
                      theSpGlobals->theVariants->GetSelectedVariant(),
                      FALSE);
      if (answerPKeyData)
         theForm = answerPKeyData->pString.data();
      else
      {
         // if the type is a parmsAnswer, then we must have
         // correct answerPKeyData.

         if (typeOfAnswer==1)
         {
            // we'll use the default form, but we need the keyword, which
            // we will get from the MSText name (the last part).

            theForm = theDefaultForm;

            // ptr will point to the keyword (otherwise it is NULL) ...
            // the keyword is loaded into the answer below.

            ptr =  theMSText->data();
            ptr += theMSText->last('.');
         }
      }
   }

   if (!theForm)
   {
      Warn(theStandardCrypticErrorMessage);
      return 0;
   }

   // allocate some space for the answer.

   unsigned int sizeOfTheAnswer = strlen(theForm)+100;
   unsigned int usedCount = 0;
   theAnswer = new char[sizeOfTheAnswer];

   // if ptr is not NULL, then it points to the keyword.  Load the keyword
   // into theAnswer, and skip the "fake" keyword in the defaultAnswerForm.

   if (ptr)
   {
      int i=0;
      ptr++;
      while (*ptr)
      {
         theAnswer[usedCount++]=*(ptr++);
         i++;
      }
      while (i++<9)  theAnswer[usedCount++]=' ';
      theForm += 9;
   }

   for (ptr = theForm; *ptr; ptr++)
   {
      if (*ptr != '!') theAnswer =
                       AddToString(theAnswer,ptr,&usedCount,&sizeOfTheAnswer);
      else
      {
         int fieldNumber, width, fieldWidth;
         fieldWidth = 0;
         helperPKey[0] = NULL;

         if (!isdigit((int) *(ptr+1)))
         {
            theAnswer = AddToString(theAnswer,ptr++,&usedCount,&sizeOfTheAnswer);
            for (ptr; *ptr; ptr++)
            {
               theAnswer = AddToString(theAnswer,ptr,&usedCount,&sizeOfTheAnswer);
               if (*ptr == '!' || *ptr == '\n') break;
            }
            continue;
         }

         sscanf (++ptr,"%d%n", &fieldNumber, &width);

         // make sure their is a field for "fieldNumber"

         if (fieldNumber == 0 ||
             fieldNumber > numberOfFields)
         {
            // move ahead to the next ! char, but if there isn't one just break.
            const char *tp = strchr (ptr,'!');
            if (tp) ptr=tp;
            else  ptr+=(strlen(ptr)-1);
            break;
         }

         ptr += width;
         if (*ptr != '!')    // get the fieldWidth, if there is one.
         {
            while (isspace((int) *ptr)) ptr++;
            if (*ptr == ',') ptr++;
            sscanf (ptr,"%d%n", &fieldWidth, &width);
            ptr += width;
            while (isspace((int) *ptr) || *ptr == ',') ptr++;
         }

         // get the helper PKey, if there is one.

         if (*ptr != '!')
         {
            const char * bang = strchr(ptr,'!');
            int nch;
            if (bang == 0) nch=strlen(ptr);
            else nch=bang-ptr;
            if (nch > 20) nch = 20;
            strncpy(helperPKey,ptr,nch);
            helperPKey[nch] = NULL;
         }
         const char *tp = strchr (ptr,'!');
         if (tp) ptr=tp;
         else  ptr+=(strlen(ptr)-1);
         
         CStringRW theInput;

         int tmp = 1;
         switch (theFieldTypes[fieldNumber-1])
         {
            case 0:                    //  Answer is a void * NULL
            case 1:                    //  Answer is a void * NULL

               theInput = NULL;
               break;

            case 2:                    //  Answer is a ScheduleByObject

               theInput = ((ScheduleByObject *)
                  theAnswerPtrs[fieldNumber-1])->SpGetTitle();
               break;

            case 3:                    //  Answer is a BoxTextObject
            case 4:                    //  Answer is a BoxTextObject

               if (theAnswerPtrs[fieldNumber-1])
               {
                  aCString = ((BoxTextObject *)
                     theAnswerPtrs[fieldNumber-1])->GetTitle();
                  theInput = (const char *) aCString;
               }
               break;


            case 5:                    //  Answer is a BoxSliderObject
            case 6:                    //  Answer is a BoxSliderObject

               if (theAnswerPtrs[fieldNumber-1])
               {
                  aCString = ((BoxSliderObject *)
                     theAnswerPtrs[fieldNumber-1])->GetTitle();
                  theInput = (const char *) aCString;
               }
               break;

            case 7:                    //  Answer is a NListButton (Species selection).

               if (theAnswerPtrs[fieldNumber-1])
               {
                  if (((NListButton *) theAnswerPtrs[fieldNumber-1])->
                      GetNumSelectedItems())
                  {
                     // Check if user defined SpGroup
                     CStringRW SpGroupName = ((NListButton *)theAnswerPtrs[fieldNumber-1])->GetFirstSelectedItem();
                     if (SpGroupName.index("SpGroup: ") == 0)
                     {
                        theInput = SpGroupName(strlen("SpGroup: "), SpGroupName.length() - strlen("SpGroup: "));
                     }
                     else
                     {
                        tmp = ((NListButton *) theAnswerPtrs[fieldNumber-1])->
                           GetSelectPosition();

                        if (strncmp("All", (const char *)
                           ((NListButton *) theAnswerPtrs[fieldNumber-1])->
                                    GetItem(0), 3)
                           == 0)
                        {
                           if (tmp == 0) theInput = "All";
                           else theInput = theSpGlobals->theSpecies->
                                 getAlphaCode(tmp-1);
                        }
                        else theInput = theSpGlobals->theSpecies->
                              getAlphaCode(tmp);
                     }
                  }
                  else theInput = " ";
               }
               break;

            case 8:                    // Answer is from habPaSelection

               if (theAnswerPtrs[fieldNumber-1])
               {
                  theInput = theSpGlobals->theHabPa->getNumericCode(
                     ((NListButton *) theAnswerPtrs[fieldNumber-1])->
                     GetSelectPosition());
               }
               break;

            case 9:                    // Answer is from forestSelection

               if (theAnswerPtrs[fieldNumber-1])
               {
                  theInput = theSpGlobals->theForests->
                     getNumericCode(
                     ((NListButton *) theAnswerPtrs[fieldNumber-1])->
                     GetSelectPosition());
               }
               break;

			case 20:
			case 10:                   //  Answer is a NListButton
                                       // (get the first string token).

               if (theAnswerPtrs[fieldNumber-1])
               {
                  aCString = ((NListButton *)
                              theAnswerPtrs[fieldNumber-1])->
                              GetFirstSelectedItem();
                  if (strlen((const char *) aCString) > 0)
                  {
                     sscanf ((const char *) aCString,"%100s",smallString);
                     theInput = smallString;
                  }
               }
               break;

            case 18:                   //  Answer is an NCheckBox.
            case 19:                   //  Answer is a RadioGroupObject
            case 11:                   //  Answer is a NListButton
            case 12:                   //  Get the number of the selected item...
                                       //  unless a "helperPKey" is present.
                                       //  Then get the token in the pstring
                                       //  corresponding to the number selected.
            {
               if (theFieldTypes[fieldNumber-1] == 11 || 
                   theFieldTypes[fieldNumber-1] == 12)
                  tmp=((NListButton *)
                       theAnswerPtrs[fieldNumber-1])->GetSelectPosition();
               else if (theFieldTypes[fieldNumber-1] == 18)
                  tmp = ((NCheckBox *) theAnswerPtrs[fieldNumber-1])->IsSelected();
               else
                  tmp = ((RadioGroupObject *)theAnswerPtrs[fieldNumber-1])->GetSelection();

               //  If a helper PKey is present, then theInput will depend on
               //  the values in the helper PKey.

               if (helperPKey[0] && theMSText)
               {
                  const char * pstring = NULL;
                  PKeyData * helperPKeyData = theMSText->
                     FindPKeyData(helperPKey,
                                  theSpGlobals->theVariants->
                                  GetSelectedVariant(),
                                  FALSE);
                  if (helperPKeyData)  // if there is PKeyData...
                  {
                     int count = 0;
                     width = 0;
                     RWCTokenizer next(helperPKeyData->pString);
                     RWCString token;
                     while (!(token=next(" \t\n")).isNull())
                     {
                        if (token(0) == '\"')
                        {
                           token = token.strip(RWCString::leading,'\"');
                           if (token.last('\"') == RW_NPOS)
                           {
                              RWCString rest=next("\n\"");
                              token += rest;
                           }
                           else token = token.strip(RWCString::trailing,'\"');
                        }
                        if (count == tmp)
                        {
                           strncpy(smallString,token.data(),100);
                           theInput = smallString;
                           break;
                        }
                        count++;
                     }
                  }
               }
               else
               {
                  sprintf (smallString,"%4d",tmp);
                  theInput = smallString;
               }
            }
            break;

            case 13:                   //  Answer is an NListEdit
            case 14:                   //  Get the first token of the input string.

               if (theAnswerPtrs[fieldNumber-1])
               {
                  aCString = ((NListEdit *)
                     theAnswerPtrs[fieldNumber-1])->GetTitle();
                  if (strlen(aCString) > 0)
                  {
                     sscanf (aCString,"%100s",smallString);
                     theInput = smallString;
                  }
               }
               break;

            case 15:                   //  Answer is an NEditControl
            case 16:                   //  Get the input string.

               if (theAnswerPtrs[fieldNumber-1])
                  theInput = ((NEditControl *)
                     theAnswerPtrs[fieldNumber-1])->GetTitle();
               break;

            case 17:                  //  Answer is an FileBrowseObject

               if (theAnswerPtrs[fieldNumber-1])
                  theInput = ((FileBrowseObject *)
                     theAnswerPtrs[fieldNumber-1])->GetValue();
               break;

            default: break;
         }

         // this block strips leading blanks.  Don't strip them if the
         // field width is negetive.

         if (fieldWidth < 0) fieldWidth = -fieldWidth;
         else
         {
            while (!theInput.isNull() && isspace((int)theInput[0]))
               theInput.remove(0,1);
         }

         if (theInput.isNull()) theInput = " ";

         // insert theInput string into theAnswer string.  This is done
         // by making sure theInput string is no wider than the field
         // width (truncate if necessary) and that theInput is right
         // justified in the field reserved in theAnswer.

         int theInputWidth = theInput.length();

         // if no fieldWidth specified, use theInputWidth

         if (fieldWidth == 0) fieldWidth = theInputWidth;
         if (theInputWidth > fieldWidth) theInputWidth = fieldWidth;

         const char * endInput = theInput.data()+theInputWidth-1;

         // make sure that theInput will fit in theAnswer.

         if (usedCount+fieldWidth > sizeOfTheAnswer)
         {
            sizeOfTheAnswer += 100;
            char * tmpAnswer = new char[sizeOfTheAnswer];
            strcpy(tmpAnswer,theAnswer);
            delete []theAnswer;
            theAnswer = tmpAnswer;
         }

         // place theInput into theAnswer, starting on the right
         // of both theInput and theAnswer and moving to the left

         char * field  = &theAnswer[usedCount];
         field += (fieldWidth - 1);
         while (field >= (char *) &theAnswer[usedCount])
         {
            if (endInput >= theInput.data()) *field = *endInput--;
            else *field = ' ';
            field--;
         }
         usedCount += fieldWidth;
      }
   }
   theAnswer[usedCount]=NULL;

   if (typeOfAnswer)
      theAnswerString = theAnswer;
   else
   {
      theAnswerString += theAnswer;
      theAnswerString += "\nThen";
   }

   // swap in the !fvsbin! variable, if it is present.

   char * fvsbin = "!fvsbin!";
   int  fvsbinlen = strlen(fvsbin);
   size_t place;
   const char * replaceFVSBIN =
         theSpGlobals->theCommandLine->getFVSBIN();
   if (!replaceFVSBIN) replaceFVSBIN = "";
   int replaceFVSBINLen =  strlen(replaceFVSBIN);
   while ((place = theAnswerString.index(fvsbin, fvsbinlen, 0,
                                       RWCString::ignoreCase))
          != RW_NPOS)
      theAnswerString.replace(place,
                              fvsbinlen,
                              replaceFVSBIN,
                              replaceFVSBINLen);

   delete []theAnswer;
   return 1;
}




void ResetObjects (int * theFieldTypes, void ** theAnswerPtrs,
                   int numberOfFields, int offSet, const char * theCParms)
{
/*****
 *
    ResetObjects is used to change the object settings to values stored
    in theCParms string.

    theFieldTypes (input)  A vector of integers that indicate what is in the
                           fields.

    theAnswerPtrs (input)  A vector of pointers to objects that contain the
                           user's selections.

    numberOfFields (input) The number of defined locations in theFieldTypes and
                           theAnswerPtrs.

    offSet        (input)  The number parameters to skip in theCParms

    theCParms     (input) a CStringRW containing the screen settings.

 *
 *****/


   if (!(theFieldTypes || theAnswerPtrs || numberOfFields < 0)) return;

   for (int fieldNumber = 0; fieldNumber < numberOfFields;
            fieldNumber++)
   {

      if (!(theFieldTypes[fieldNumber] ||
            theAnswerPtrs[fieldNumber])) return;

      switch (theFieldTypes[fieldNumber])
      {
         case 0:                    //  Answer is a void * NULL
         case 1:                    //  Answer is a void * NULL

            break;

         case 2:                    //  Answer is a ScheduleByObject

            ((ScheduleByObject *) theAnswerPtrs[fieldNumber])->
               ResetSchedObjSettings(GetCParm(theCParms,fieldNumber+offSet));
            break;

         case 3:                    //  Answer is a BoxTextObject
         case 4:                    //  Answer is a BoxTextObject

            ((BoxTextObject *) theAnswerPtrs[fieldNumber])->
               ResetBoxTextSettings(GetCParm(theCParms,fieldNumber+offSet));
            break;

         case 5:                    //  Answer is a BoxSliderObject
         case 6:                    //  Answer is a BoxSliderObject

            ((BoxSliderObject *) theAnswerPtrs[fieldNumber])->
               ResetBoxSliderSettings(GetCParm(theCParms,fieldNumber+offSet));
            break;

         case 7:                    // Answer is a NListButton (Species selection).
         {
            CStringRW fieldParam;
            int listIndex = -1;
            CStringRW listIndexName;

            fieldParam = GetCParm(theCParms,fieldNumber+offSet);

            RWCString token;
            RWCTokenizer next(fieldParam);
            if (!(token = next(".", 1)).isNull())
               listIndex = atoi(token.data());
            if (!(token = next(".", 1)).isNull())
               listIndexName = token;

            if (listIndex >= 0)
               ((NListButton *) theAnswerPtrs[fieldNumber])->SelectItem(listIndex);
            else if ((listIndex == -1) && (listIndexName.length() > 0))
            {
               // Set the selection based on listIndexName
               if (FALSE == ((NListButton *) theAnswerPtrs[fieldNumber])->SelectItem(CStringRW(listIndexName)))
               {
                  xvt_dm_post_note("%s is no longer available for selection.\n\nPlease make new selection.", listIndexName.data());
                  ((NListButton *) theAnswerPtrs[fieldNumber])->DeselectAll();
               }
            }
            else
               ((NListButton *) theAnswerPtrs[fieldNumber])->DeselectAll();
            break;
         }

         case 8:                    // Answer is from habPaSelection
         case 9:                    // Answer is from forestSelection
         case 10:                   // Answer is a NListButton (get the first string token).
         case 11:                   // Answer is a NListButton
         case 12:                   // Get the number of the selected item...
		 		 case 20:
         {
            int number;
            sscanf ((const char *) GetCParm(theCParms,fieldNumber+offSet),"%d", &number);
            if (number >= 0)
               ((NListButton *) theAnswerPtrs[fieldNumber])->SelectItem(number);
            else
               ((NListButton *) theAnswerPtrs[fieldNumber])->DeselectAll();
            break;
         }

         case 13:                   //  Answer is an NListEdit
         case 14:                   //  Get the input string.

            ((NListEdit *) theAnswerPtrs[fieldNumber])->
               SetTitle(GetCParm(theCParms,fieldNumber+offSet));
            break;

         case 15:                   //  Answer is an NEditControl
         case 16:                   //  Get the input string.

            ((NEditControl *) theAnswerPtrs[fieldNumber])->
               SetTitle(GetCParm(theCParms,fieldNumber+offSet));
            break;

         case 17:                   //  Answer is an FileBrowseObject

            ((FileBrowseObject *) theAnswerPtrs[fieldNumber])->
               SetValue(GetCParm(theCParms,fieldNumber+offSet));
            break;

         case 18:                   //  Answer is an NCheckBox
            if (atoi(GetCParm(theCParms,fieldNumber+offSet)) == 1)
               ((NCheckBox *) theAnswerPtrs[fieldNumber])->Select();
            else
               ((NCheckBox *) theAnswerPtrs[fieldNumber])->Deselect();
            break;

         case 19:                   //  Answer is a RadioGroupObject

            ((RadioGroupObject *) theAnswerPtrs[fieldNumber])->
               SetSelection(atoi(GetCParm(theCParms,fieldNumber+offSet)));
            break;

         default: break;
      }
   }
}






void BuildCParms (int * theFieldTypes, void ** theAnswerPtrs,
                  int numberOfFields, CStringRW & theCParms)
{
/*****
 *
    BuildCParms is used to the CParms string necessry to reset the
    screen settings to the values used when the screen was created.

    theFieldTypes (input)  A vector of integers that indicate what is in the
                           fields.

    theAnswerPtrs (input)  A vector of pointers to objects that contain the
                           user's selections.

    numberOfFields (input) The number of defined locations in theFieldTypes and
                           theAnswerPtrs.

    theCParms     (output) a CStringRW containing the screen settings.

 *
 *****/

   if (!(theFieldTypes || theAnswerPtrs || numberOfFields < 0)) return;

   for (int fieldNumber = 0; fieldNumber < numberOfFields;
            fieldNumber++)
   {

      if (!(theFieldTypes[fieldNumber] ||
            theAnswerPtrs[fieldNumber])) return;

      switch (theFieldTypes[fieldNumber])
      {
         case 0:                    //  Answer is a void * NULL
         case 1:                    //  Answer is a void * NULL

            StuffCParm(theCParms," ");
            break;

         case 2:                    //  Answer is a ScheduleByObject

            StuffCParm(theCParms,((ScheduleByObject *)
               theAnswerPtrs[fieldNumber])->GetSchedObjSettings());
            break;

         case 3:                    //  Answer is a BoxTextObject
         case 4:                    //  Answer is a BoxTextObject

            StuffCParm(theCParms,((BoxTextObject *)
               theAnswerPtrs[fieldNumber])->GetBoxTextSettings());
            break;

         case 5:                    //  Answer is a BoxSliderObject
         case 6:                    //  Answer is a BoxSliderObject

            StuffCParm(theCParms,((BoxSliderObject *)
               theAnswerPtrs[fieldNumber])->GetBoxSliderSettings());
            break;

         case 7:                    // Answer is a NListButton (Species selection).
         {
            char tmpS[256];
            if (((NListButton *) theAnswerPtrs[fieldNumber])->GetNumSelectedItems())
            {
               if (((NListButton *) theAnswerPtrs[fieldNumber])->GetFirstSelectedItem().index("SpGroup: ") == 0)
               {
                  CStringRW item = ((NListButton *) theAnswerPtrs[fieldNumber])->GetFirstSelectedItem();
                  sprintf (tmpS,"-1.%s",item.data());
               }
               else
               {
                  int pos = ((NListButton *) theAnswerPtrs[fieldNumber])->GetSelectPosition();
                  sprintf (tmpS,"%d",pos);
               }
            }
            else
               sprintf (tmpS,"-1");
            StuffCParm(theCParms,tmpS);
            break;
         }

         case 8:                    // Answer is from habPaSelection
         case 9:                    // Answer is from forestSelection
         case 10:                   // Answer is a NListButton (get the first string token).
         case 11:                   // Answer is a NListButton
         case 12:                   // Get the number of the selected item...
		 		 case 20:
         {
            char tmpS[5];
            if (((NListButton *) theAnswerPtrs[fieldNumber])->GetNumSelectedItems())
               sprintf (tmpS,"%d",((NListButton *) theAnswerPtrs[fieldNumber])->GetSelectPosition());
            else
               sprintf (tmpS,"-1");
            StuffCParm(theCParms,tmpS);
            break;
         }

         case 13:                   //  Answer is an NListEdit
         case 14:                   //  Get the input string.

            StuffCParm(theCParms,((NListEdit *)
               theAnswerPtrs[fieldNumber])->GetTitle());
            break;

         case 15:                   //  Answer is an NListEdit
         case 16:                   //  Get the input string.

            StuffCParm(theCParms,((NEditControl *)
               theAnswerPtrs[fieldNumber])->GetTitle());
            break;

         case 17:                   //  Answer is an FileBrowseObject

            StuffCParm(theCParms,((FileBrowseObject *)
               theAnswerPtrs[fieldNumber])->GetValue());
            break;

         case 18:                   //  Answer is an NCheckBox

            StuffCParm(theCParms,((NCheckBox *) 
               theAnswerPtrs[fieldNumber])->IsSelected() ? "1" : "0");
            break;

         case 19:                   //  Answer is a RadioGroupObject
         {
            char tmpS[5];
            sprintf(tmpS, "%d", ((RadioGroupObject *)theAnswerPtrs[fieldNumber])->GetSelection());
            StuffCParm(theCParms,tmpS);
            break;
         }

         default: break;
      }
   }
}





int LoadRWOrdered(const char * theString, RWOrdered * theList)
{
   if (!theString) return 0;

   // if theString is null, then return.

   int theStringLen = strlen((const char *) theString);

   if (theStringLen == 0) return 0;

   // allocate memory for a copy and make it.

   char * theCopy = new char[theStringLen+1];

   strcpy(theCopy,theString);

   // use strtok to get the tokens.

   int count=0;
   int position=0;
   for (char * ptr = strtok(theCopy,"\n"); ptr != NULL;
               ptr = strtok(NULL,"\n"))
   {
      while (isspace((int) *ptr)) ptr++;  //delete leading white space.
      if (*ptr == '>')
      {
         position=count;
         ptr++;
         while (isspace((int) *ptr)) ptr++;
      }

      theList->insert(new CStringRWC (ptr));
      count++;
   }

   delete []theCopy;

   return position;
}





PKeyData * GetPKeyData(const char * stringOfMSText, const char * stringOfPKey, const char * stringOfATList, int matchMethod, int postWarn)
{
/*****
 *
    GetPKeyData is used to get a pointer to specific PKeyData.  The
    arguments indicate which major section, PKey, search ATList, and
    matching method the caller desires.  The return is a NULL pointer
    if the PKeyData corresponding to the PKey is not found.  A warning
    is optionally posted if no PKeyData are found.


    stringOfMSText  The desired MSText
    stringOfPKey    The desired PKey
    stringOfATList  The desired ATList...often the variant.
    matchMethod     0=get best match, 1=exact match only
    postWarn        0=no warning posted, 1=warning posted.

    return value    PKeyData *.  Pointer to desired PKeyData
                    NULL if desired PKeyData not found.
 *
 *****/

   CStringRW warnMsg;
   CStringRWC searchMSText = stringOfMSText;
   PKeyData *desiredKeyData = (PKeyData *) NULL;

   MSText *neededMSTextPointer =  
	   (MSText *) 
	   theSpGlobals->
	   theParmsData->
	   find(
	   &searchMSText);
   if (neededMSTextPointer) desiredKeyData = neededMSTextPointer->
                               FindPKeyData(stringOfPKey,
                                            stringOfATList,
                                            matchMethod);
   if (!desiredKeyData)
      warnMsg = "Needed data from the Suppose Parameters file is missing.";


   if (warnMsg.length() && postWarn)
   {
      warnMsg += "\nNeeded major section is\n\t";
      warnMsg +=  stringOfMSText;
      warnMsg += "\nNeeded PKey section is\n\t";
      warnMsg +=  stringOfPKey;
      warnMsg += "\nNeeded ATList is\n\t";
      warnMsg +=  stringOfATList;
   }

   return desiredKeyData;
}




MSText * GetMSTextPtr(const char * MSTextName)
{
/*****
 *
    GetMSTextPtr is used to get a pointer to MSText.
    The return is a NULL pointer
    if the MSText is not found.

    MSTextName  The desired MSText
 *
 *****/

   if (MSTextName)
   {
      CStringRWC searchMSText = MSTextName;
      return (MSText *) theSpGlobals->theParmsData->find(&searchMSText);
   }
   return NULL;
}


void FreePKeyData (const char * stringOfMSText)
{
/*****
 *
    FreePKeyData is used to free the memory associated with a
    bunch of PKeyData (all the PKeyData associated with the MSText).
    Be carefull where you use this, as you can delete data towhich
    you are maintaining a pointer.

    If the routine can not find the MSText, it simply returns.

    stringOfMSText  The MSText name.
 *
 *****/

   MSText * neededMSTextPointer = GetMSTextPtr(stringOfMSText);
   if (neededMSTextPointer) neededMSTextPointer->FreeThePKeys();
}


char * AddToString(char * theAnswer,
                   const char * ptrChar,
                   unsigned int * usedCount,
                   unsigned int * sizeOfTheAnswer,
                   unsigned int stdToAdd)
{
   if ((*usedCount)+2 > *sizeOfTheAnswer)
   {
      unsigned int toAdd = stdToAdd;
      if (toAdd < 10) toAdd = 10;
      *sizeOfTheAnswer += toAdd;
      char * tmpAnswer = new char[*sizeOfTheAnswer];
      strcpy(tmpAnswer,theAnswer);
      delete []theAnswer;
      theAnswer = tmpAnswer;
   }
   theAnswer[(*usedCount)++]=*ptrChar;
   theAnswer[(*usedCount)]=NULL;
   return theAnswer;
}
