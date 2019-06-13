/********************************************************************

   File Name:     spparms.cpp
   Author:        nlc
   Date:          11/28/94

   See spparms.hpp for purpose, contents, and usage notes.

********************************************************************/

#include "spparms.hpp"
#include "warn_dlg.hpp"
#include <stdio.h>
#include "dirsept.hpp"
#include "cmdline.hpp"
#include "spfuncs.hpp"

#include CStringRWC_i

SpParms::SpParms(const char * parmsFile): RWBinaryTree(), parmsFileName(parmsFile)
{

   char thechar;
   unsigned int bufflen = 0;
   unsigned int sizeofbuff = 200;
   int  lookfor = 1;
   char *buffer = new char[sizeofbuff];
   fpos_t position;
   FILE *filePtr = fopen (parmsFileName.data(),"r");

   if (filePtr)
   {
      // if the file name has been opened, then insure that the
      // full path name to the file is stored...so that is can
      // be found if the current directory is changed.

      if (!strchr(parmsFileName.data(),(int) *SEPARATOR))
      {
         DIRECTORY dir;
         xvt_fsys_get_dir(&dir);
         char path[SZ_FNAME];
         if (xvt_fsys_convert_dir_to_str(&dir,path,SZ_FNAME))
         {
            RWCString tmpFname = path;
            tmpFname += SEPARATOR;
            tmpFname += parmsFileName;
            parmsFileName = tmpFname;
         }
      }
   }
   else
   {
      // if the file can't be opened, then try adding the fvsbin to the
      // file name.

      RWCString tmpFname = theSpGlobals->theCommandLine->getFVSBIN();
      tmpFname += SEPARATOR;
      tmpFname += parmsFileName;
      parmsFileName = tmpFname;
      filePtr = fopen (parmsFileName.data(),"r");
   }

   if (filePtr)
   {
      while ((thechar = (char) fgetc(filePtr)) != (char) EOF)
      {
         if (isspace (thechar))
         {
            switch (lookfor)
            {
               case 1:               //start
                  if (bufflen == 7)
                     if (strcmp(buffer,"//start") == 0) lookfor = 2;
                  bufflen = 0;
				  		 break;

               case 2:               // skip to the beginning of the name.
                  if (thechar == '\n')  // if we found a newline, start over.
                  {
                     lookfor = 1;
                     bufflen = 0;
				  				}
               break;

               case 3:          // found the major section name.
                  {
                     fgetpos(filePtr, &position);
                     insert(new MSText(buffer, position));
                     lookfor = 1;
                     bufflen = 0;
                     break;
                  }

               default:
                  lookfor = 1;
                  bufflen = 0;
                  break;
            }
         }
         else
         {
            if (lookfor == 2) lookfor = 3;
            buffer = AddToString(buffer, &thechar,
                                 &bufflen,&sizeofbuff);
         }
      }
      fclose (filePtr);
   }
   else
   {
      RWCString warnMsg = "Suppose parameter file could not be opened.\n"
                          "Parms file name:\n";
      warnMsg += parmsFileName;
      Warn (warnMsg.data());
   }
   delete []buffer;
}




SpParms::~SpParms ( void )
{
   clearAndDestroy();
}


RWDEFINE_COLLECTABLE(MSText,MSTEXT)


MSText::MSText ( const char * name, fpos_t position ):CStringRWC ( name ),positionInFile(position), isDefined(0){}


MSText::MSText ( void ) : CStringRWC (NULLString),positionInFile(0), isDefined(0){}


void MSText::FreeThePKeys( void )
{
   if (isDefined)
   {
      thePKeys.clearAndDestroy();
      isDefined = 0;
   }
}



RWOrdered * MSText::GetThePKeys( void )
{

   // First, find out if the PKeys for this MSText have been defined.
   // If they have been defined, simply return the pointer to
   // the PKeys container.  Otherwise, define the PKeyData by
   // reading the major section text and parsing the contents.

   if (isDefined) return &thePKeys;

   isDefined = 1;

   RWCString pkey;
   RWCString atlist;
   RWCString pkey_r;
   RWCString atlist_r;
   RWCString pstring;

   PKeyData * existingPKeyData = NULL;

   char charS[20];

   RWCString warnMsg;

   char thechar;

   //start is line 1, the beginning of the "file" is line 2.

   int  lineNumber = 2;

   unsigned int  bufflen = 0;
   unsigned int  sizeofbuff = 1000;
   int  state   = 1;
   char * buffer = new char [sizeofbuff];
   buffer[0] = 0;

   // open the file...and set up the PKeys objects.

   FILE* filePtr = fopen (theSpGlobals->theParmsData->parmsFileName.data(),
                          "r");
   if (filePtr)
   {
      fsetpos(filePtr, (const fpos_t *) &positionInFile);

      while ((thechar = fgetc(filePtr)) != EOF)
      {
         if (thechar == (int) '\n') lineNumber++;
         switch (state)
         {
            case 1:  // looking for the beginning of a pkey or a
                     // comment (starts with !, ends with newline)

               if (isspace(thechar)) break;

               pkey     = NULLString;
               atlist   = NULLString;
               pkey_r   = NULLString;
               atlist_r = NULLString;
               pstring  = NULLString;

               if (thechar == '!')     // we have a comment.
               {
                  state = 15;
                  break;
               }

               // if a printing char, save....
               // (note that / will detect the //end token.

               if (isprint(thechar))
               {
                  state=2;
                  bufflen=0;
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
               }
               break;

            case 2:  // looking for the end of a pkey.

               // if a printing char but not { : = the char is part of pkey.

               if (isprint(thechar) && thechar != '{' && thechar != '=' && thechar != ':' && thechar != ' ')
               {
                  buffer = AddToString(buffer, &thechar, &bufflen,&sizeofbuff);
                  break;
               }

               // if we found the end of the set, then we are completely done.

               if (strncmp(buffer,"//end",5) == 0)
               {
                  fclose (filePtr);
                  if (warnMsg.length()) Warn (warnMsg.data());
                  return &thePKeys;
               }

               // otherwise, we found the end...save the buffer as pkey
               // unless it has zero length.

               if (bufflen == 0)
               {
                  state = 1;
                  break;
               }

               pkey = buffer;
               bufflen=0;
               buffer[0]=(char) 0;

               // if the char is white space, then set state for looking
               // for the beginning of the ATList, =, or :

               if (isspace(thechar))
               {
                  state=3;
                  break;
               }             // the char is not white space.
                             // NO break...we could already be at state 3.

            case 3:  // looking for { : =

               if (isspace(thechar)) break;

               switch (thechar)
               {
                   case '{':
                      state=4;
                      break;
                   case ':':
                      state=7;
                      break;
                   case '=':
                      state=11;
                      break;
                   default:
                      warnMsg += "Expecting { : = and found ";
                      sprintf (charS,"\"%c\"", (unsigned char) thechar);
                      warnMsg += charS;
                      warnMsg += "\nSome data are not stored.";
                      warnMsg += "\nMajor section name = ";
                      warnMsg += data();
                      warnMsg += "\nLine number within major section = ";
                      sprintf (charS,"%d", lineNumber);
                      warnMsg += charS;
                      warnMsg += "\n------------\n\n";
               }
               break;

            case 4:  // looking for the beginning of the ATList.
               if (isspace(thechar)) break;
               if (isprint(thechar) && thechar != '}')
               {
                  state = 5;
                  bufflen=0;
                  buffer = AddToString(buffer, &thechar, &bufflen,&sizeofbuff);
               }
               if (thechar == '}') // we found the end of the atlist.
               {
                  bufflen=0;
                  buffer[0]=(char) 0;
                  atlist = buffer;
                  state = 6;
               }
               break;

            case 5:  // looking for the end of the ATList.

               if (thechar == '}') // we found the end of the atlist.
               {
                  atlist = buffer;
                  bufflen=0;
                  buffer[0]=(char) 0;
                  state = 6;
               }
               else
               {
                  buffer = AddToString(buffer, &thechar, &bufflen, &sizeofbuff);
               }
               break;

            case 6:  // looking for : =

               if (isspace(thechar)) break;

               switch (thechar)
               {
                   case ':':
                      state=7;
                      break;
                   case '=':
                      state=11;
                      break;
                   default:
                      warnMsg = "Expecting { : = and found ";
                      sprintf (charS,"\"%c\"", (unsigned char) thechar);
                      warnMsg += charS;
                      warnMsg += "\nSome data are not stored.";
                      warnMsg += "\nMajor section name = ";
                      warnMsg += data();
                      warnMsg += "\nLine number within major section = ";
                      sprintf (charS,"%d", lineNumber);
                      warnMsg += charS;
                      warnMsg += "\n------------\n\n";
               }
               break;

            case 7:  // looking for the beginning of the pstring.

               if (isspace(thechar)) break;

               if (thechar == '{')
               {
                  state = 9;
                  break;
               }

               if (isprint(thechar))
               {
                  state = 8;
                  bufflen=0;
                  buffer = AddToString(buffer, &thechar, &bufflen,&sizeofbuff);
               }
               break;

            case 8:  // looking for the end of the pstring.

               // if a print char, then char is part of PKey.

               if (isprint(thechar))
               {
                  buffer = AddToString(buffer, &thechar, &bufflen,&sizeofbuff);
                  break;
               }

               // if we found the end of the set, then we completely done.
               // construct the pkey with null pstring... and return.

               if (strncmp(buffer,"//end",5) == 0)
               {
                  thePKeys.insert(new PKeyData (pkey.data(),
                                                atlist.data(),
                                                pstring.data()));
                  fclose (filePtr);
                  if (warnMsg.length()) Warn (warnMsg.data());
                  return &thePKeys;
               }

               // otherwise, we found the end...save the buffer as pkey
               // construct the pkey with null pstring... and set the state to 1.

               pstring = buffer;
               bufflen=0;
               buffer[0]=(char) 0;

               thePKeys.insert(new PKeyData (pkey.data(),
                                             atlist.data(),
                                             pstring.data()));
               state = 1;
               break;

            case 9:  // looking for the end of the pstring...keep all blanks...
                     // use the backslash char to escape the following char.
                     // (state=10)

               if (thechar == 92)  // 92 is the backslash
               {
                  state=10;
                  break;
               }

               if (thechar == '}') // we found the end of the pstring.
               {
                  pstring = buffer;
                  thePKeys.insert(new PKeyData (pkey.data(),
                                                atlist.data(),
                                                pstring.data()));
                  bufflen=0;
                  buffer[0]=(char) 0;
                  state = 1;
               }
               else
               {
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
               }
               break;

            case 10: // last char was a backslash

               if (thechar == 'n') // we have a new line requested.
               {
                  thechar = '\n';
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
               }
               else if (thechar == 't') // we have a horizontal tab requesed.
               {
                  thechar = '\t';
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
               }
               else if (thechar != '\n') // if the char is not newline, save it.
               {
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
               }                    // otherwise it will be skipped.

               state=9;
               break;

            case 11: // we found an equal sign.  We are looking for a pkey_r

               if (isspace(thechar)) break;

               // if printing char, save

               if (isprint(thechar))
               {
                  state=12;
                  bufflen=0;
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
               }
               break;

            case 12:  // looking for the end of a pkey_r

               // if print and not { then char is part of pkey_r

               if (isprint(thechar) && thechar != '{')
               {
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
                  break;
               }

               // if we found the end of the major section, (the pkey_r is
               // NULL) simply add pkey, atlist, and pstring to the list
               // and return.

               if (strncmp(buffer,"//end",5) == 0)
               {
                  thePKeys.insert(new PKeyData (pkey.data(),
                                                atlist.data(),
                                                pstring.data()));
                  fclose (filePtr);
                  if (warnMsg.length()) Warn (warnMsg.data());
                  return &thePKeys;
               }

               // otherwise, we found the end of the pkey_r.

               pkey_r = buffer;
               bufflen=0;
               buffer[0]=(char) 0;

               // if the char is { we have found an atlist_r

               if (thechar == '{')
               {
                  state = 14;
                  break;
               }

               // we need to find an atlist_r, or detect that there isn't one.
               // set state equal to 13 and break.

               state = 13;
               break;

            case 13:  // looking for the beginning of an atlist_r or the fact
                      // that there isn't one.

               if (isspace(thechar)) break;

               if (thechar != '{')
               {
                  //there is no at_list ... put back the char.

                  ungetc(thechar,filePtr);

                  // find the correct pstring for this pkey... and store.

                  existingPKeyData = FindPKeyData(pkey_r.data(),
                                                  atlist_r.data(),
                                                  1);

                  if (existingPKeyData)
                     thePKeys.insert(new PKeyData (pkey.data(),
                                                   atlist.data(),
                                                   existingPKeyData->
                                                   pString.data()));
                  else
                     thePKeys.insert(new PKeyData (pkey.data(),
                                                   atlist.data(),
                                                   pstring.data()));
                  state = 1;
                  break;
               }

            case 14:  // looking for the end of the atlist_r

               if (!isprint(thechar)) break;
               if (thechar == '}')
               {
                  atlist_r = buffer;
                  bufflen  = 0;
                  buffer[0]= (char) 0;

                  // find the correct pstring for this pkey... and store.

                  existingPKeyData = FindPKeyData(pkey_r.data(),
                                                  atlist_r.data(), 1);

                  if (existingPKeyData)
                     thePKeys.insert(new PKeyData (pkey.data(),
                                                   atlist.data(),
                                                   existingPKeyData->
                                                   pString.data()));
                  else
                     thePKeys.insert(new PKeyData (pkey.data(),
                                                   atlist.data(),
                                                   pstring.data()));
                  state = 1;
               }
               else
               {
                  buffer = AddToString(buffer, &thechar,
                                       &bufflen,&sizeofbuff);
              }
               break;

            case 15:  // looking for the end of a comment.

               if (thechar == '\n') state = 1;
               break;

            default:
               break;
         }
      }
      warnMsg += "Expecting //end and found end-of-file.\n"
                 "Some data are not stored.";
      warnMsg += "\nMajor section name = ";
      warnMsg += data();

      fclose (filePtr);
   }
   else
   {
      warnMsg += "Suppose parameter file could not be opened.\n"
                 "Could not define parameter data for a major section.\n";
      warnMsg += "Major section name = ";
      warnMsg += data();
   }
   delete []buffer;

   if (warnMsg.length()) Warn (warnMsg.data());
   return &thePKeys;
}




PKeyData * MSText::FindPKeyData( const char * searchPKey,
                                 const char * searchATList,
                                 const int    allMustMatch)
{

   // if the PKeyData has not been read, then isDefined will be false.
   // in this case, call GetThePKeys().

   if (!isDefined) GetThePKeys();

   PKeyData * foundPKeyData = NULL;

   // if the pkey is null, then there is no match.

   if (*searchPKey == NULL) return foundPKeyData;

   // search thepkeys

   RWOrderedIterator nextPKeyData(thePKeys);
   PKeyData * onePKeyData;
   while (onePKeyData = (PKeyData *) nextPKeyData())
   {
      if (strcmp(onePKeyData->data(), searchPKey) == 0)
      {
         // now check the atlist match.

         int len;
         if (searchATList && (len = strlen(searchATList)))
         {
            char * buff = new char[++len];
            strcpy(buff,searchATList);
            for (char * ptr  = strtok(buff," ");
                        ptr != NULL;
                        ptr  = strtok(NULL," "))
            {
               const CStringRWC searchElement = ptr;
               if (onePKeyData->aTList.contains(&searchElement))
               {
                  foundPKeyData = onePKeyData;
                  if (!allMustMatch) break;
               }
               else
               {
                  if (allMustMatch)
                  {
                     foundPKeyData = NULL;
                     break;
                  }
               }
            }
            delete []buff;
         }
         else
         {

            // there is no searchATList... so find a pkey
            // for which there is no ATList.
            // If allMustMatch is true, then WE MUST have
            // a pkey that has no ATList, otherwise just
            // return the current first matching pkey found.

            foundPKeyData = onePKeyData;

            if (onePKeyData->aTList.entries() == 0) break;
            if (allMustMatch) foundPKeyData = NULL;
         }
      }
   }

   if (!allMustMatch           &&
       (foundPKeyData == NULL) &&
       searchATList            &&
       strlen(searchATList))
      foundPKeyData = FindPKeyData(searchPKey, NULL, 1);

   return foundPKeyData;
}




MSText::~MSText ( void )
{
   thePKeys.clearAndDestroy();
}



int MSText::compareTo(const RWCollectable* t) const
{
   return RWCString::compareTo((CStringRWC&) *t);
}



RWBoolean MSText::isEqual(const RWCollectable* t) const
{
   return (this == t ||
           (CStringRWC&) *this == (CStringRWC&) *t);
}


RWDEFINE_COLLECTABLE(PKeyData,PKEYDATA)

PKeyData::PKeyData (const char * pkey,
                    const char * atlist,
                    const char * pstring)
         :CStringRWC(pkey),
          pString(pstring)
{

   int len;
   if (len = strlen(atlist))
   {
      char * buff = new char[++len];
      strcpy(buff,atlist);
      for (char * ptr  = strtok(buff," ");
                  ptr != NULL;
                  ptr  = strtok(NULL," "))
      {
         aTList.insert(new CStringRWC(ptr));
      }
      delete [] buff;
   }
}


PKeyData::PKeyData ( void )
         :CStringRWC(NULLString),
          pString(NULLString)
{}


PKeyData::~PKeyData ( void )
{
   aTList.clearAndDestroy();
}


int PKeyData::compareTo(const RWCollectable* t) const
{
   return RWCString::compareTo((CStringRWC&) *t);
}


RWBoolean PKeyData::isEqual(const RWCollectable* t) const
{
   return (this == t ||
           (CStringRWC&) *this == (CStringRWC&) *t);
}



RWCollectable * ATList::find(const CStringRW *t) const
{
   RWOrderedIterator next(*this);
   CStringRWC * item;
   while(item = (CStringRWC *) next())
   {
      if (!item->RWCString::compareTo(*t))
         return (RWCollectable*) item;
   }
   return NULL;
}


RWBoolean ATList::contains(const CStringRW *t) const
{
   if (find(t)) return TRUE;
   return FALSE;
}

