/********************************************************************

  File Name:     runstr.cpp
  Author:        nlc
  Date:          05/30/95

    Purpose:       see runstr.hpp

*********************************************************************/

#include "PwrDef.h"
#include CStringRWC_i
#include CDocument_i
#include <rw/ordcltn.h>
#include <rw/vstream.h>  // for debugging.
#include "suppdefs.hpp"
#include "runstr.hpp"
#include "group.hpp"
#include "compon.hpp"
#include "stand.hpp"
#include "spglobal.hpp"
#include "extens.hpp"
#include "variants.hpp"
#include "programs.hpp"
#include "postproc.hpp"
#include "spparms.hpp"
#include "sendupdt.hpp"
#include "yearobj.hpp"
#include "spfuncs.hpp"

RunStream::RunStream ( void )
{
   includedPostProcs  = new RWOrdered(5);
   allGroups          = new RWOrdered(20);
   allStands          = new RWOrdered(200);
   allComponents      = new RWOrdered(100);
   subsetGroups       = new RWOrdered(2);
   subsetStands       = new RWOrdered(2);
   topPPEComponents   = NULL;
   botPPEComponents   = NULL;
   reportPPEComponents= NULL;
   writtenComponents  = (RWOrdered *) 0;
   standNumbers       = 0;
   groupNumbers       = 0;
   componentNumbers   = 0;
   topKeywords        = (CStringRW *) 0;
   topSimNotes        = (CStringRW *) 0;
   componentClipBoard = (Component *) 0;
   allSpGroups        = NULL;
}




RunStream::~RunStream ( void )
{
   // this class is said to own all of the components, groups, and stands.
   // that means that deleting this class implies deleting all of the
   // elements of those lists.

   // if any windows are openned, this may cause problems.  Reinitialize
   // the lists of all existing windows if a RunStream is deleted.

   subsetGroups ->clear();
   delete subsetGroups;
   subsetStands ->clear();
   delete subsetStands;

   includedPostProcs->clearAndDestroy();
   delete includedPostProcs;
   allGroups        ->clearAndDestroy();
   delete allGroups;
   allStands        ->clearAndDestroy();
   delete allStands;
   allComponents    ->clearAndDestroy();
   delete allComponents;

   if (topPPEComponents) topPPEComponents->clearAndDestroy();
   if (botPPEComponents) botPPEComponents->clearAndDestroy();
   if (reportPPEComponents) reportPPEComponents->clearAndDestroy();

   if (topSimNotes)        delete topSimNotes;
   if (componentClipBoard) delete componentClipBoard;
   if (writtenComponents)  delete writtenComponents;

}




int RunStream::WriteRun (FILE * filePtr)
{

   if (!filePtr) return 1;

   // clear the list of written components.  This list is used to suppress
   // the writting of redundant data that describes the component to suppose
   // but is not needed by FVS.

   if (!writtenComponents) writtenComponents =
      new RWOrdered(allComponents->entries());
   writtenComponents->clear();

   // if there are stands, then write the top data, the stands, the bottom,
   // any reports commands, groups that have not components, and the subset.

   if (allStands->entries())
   {
      fprintf (filePtr,"!!Suppose 2.08 %s %d %d\n",  //Updated 04/02/2019
         theSpGlobals->theVariants->GetSelectedVariant(),
         theSpGlobals->theVariants->isPPEBeingUsed,
         theSpGlobals->theVariants->PPEUseIsLocked);

      // lock the variant and "pick" the program.

      theSpGlobals->theVariants->variantSelectionLocked = 1;
      theSpGlobals->thePrograms->PickProgram(1);

      // write the include post processor tags.

      RWOrderedIterator nextIncludedPostProc(*includedPostProcs);
      PostProc * inclPostProc;
      while (inclPostProc = (PostProc *) nextIncludedPostProc())
      {
         fprintf (filePtr,"!!PP %s\n", inclPostProc->theMSTextName->data());
      }

      if (topKeywords)
         fprintf (filePtr,"!!Top\n%s\n!!End\n", topKeywords->data());
      if (topSimNotes)
         fprintf (filePtr,"!!SN\nComment\n%s\nEnd\n", topSimNotes->data());

      // if the PPE is being used, write the timing keywords.

      if (theSpGlobals->theVariants->isPPEBeingUsed)
      {
         int endSim   = theSpGlobals->yearObj->GetEndYear();
         int startSim = theSpGlobals->yearObj->GetStartYear();
         int cycleLen = theSpGlobals->yearObj->GetCycleLength();
         int baseNumberCycles = (endSim-startSim)/cycleLen;
         if ((endSim-startSim)%cycleLen) baseNumberCycles++;

         fprintf (filePtr,"\nStartYr%13d\nMstrInt%23d\nMstrCycs%12d\n\n",
            startSim,cycleLen,baseNumberCycles );
      }

      // write topPPEComponents.

      WritePPEComponents(topPPEComponents, filePtr, "!!CT");

      // loop over all the stands

      RWOrderedIterator nextStand(*allStands);
      Stand * stand;
      while (stand = (Stand *) nextStand())
      {

         // if PPE, write AddStand keyword.

         if (theSpGlobals->theVariants->isPPEBeingUsed)
         {
            fprintf (filePtr,"\nAddStand\n\n");
         }

         stand->WriteStand(filePtr);
      }

      // write botPPEComponents.

      WritePPEComponents(botPPEComponents, filePtr, "!!CB");

      // if PPE, write Project keyword.

      if (theSpGlobals->theVariants->isPPEBeingUsed)
      {
         fprintf (filePtr,"\nProject\n\n");
      }

      // write reportPPEComponents

      WritePPEComponents(reportPPEComponents, filePtr,"!!CR");

      // write STOP.

      fprintf (filePtr,"\nSTOP\n");

      // write groups that have no components.  If they have stand membership
      // then write that also. (all others will be written with the stands)

      RWOrderedIterator nextGroup(*allGroups);
      Group * group;
      while (group = (Group *) nextGroup())
      {
         if (!group->componentList)
         {
            fprintf (filePtr,"!!G %s %d\n", group->data(), group->groupNumber);

            if (group->standMembership)
            {
               RWOrderedIterator standWithinGroups(*group->standMembership);
               while (stand = (Stand *) standWithinGroups())
               {
                  fprintf (filePtr,"!!S %s %d\n", stand->data(),
                           stand->standNumber);
               }
            }
         }
      }

      // write the subset.

      fprintf (filePtr,"!!Subset\n");

      RWOrderedIterator subsetGroup(*subsetGroups);
      while (group = (Group *) subsetGroup())
      {
         fprintf (filePtr,"!!G %s %d\n", group->data(), group->groupNumber);
      }

      RWOrderedIterator subsetStand(*subsetStands);
      while (stand = (Stand *) subsetStand())
      {
         fprintf (filePtr,"!!S %s %d\n", stand->data(), stand->standNumber);
      }

      fprintf (filePtr,"!!End\n");

   }
   else
   {
      // if there are NO stands, then just write the defined components.

      if (allComponents->entries())
      {
         RWOrderedIterator nextComponent(*allComponents);
         Component * component;
         while (component = (Component *) nextComponent())
         {
            component->WriteComponent(NULL, NULL, filePtr);
         }
      }

      // use the generic "tag"...this is not a real runstream

      WritePPEComponents(topPPEComponents,    filePtr, "!!C");
      WritePPEComponents(botPPEComponents,    filePtr, "!!C");
      WritePPEComponents(reportPPEComponents, filePtr, "!!C");

   }
   return 0;
}




int RunStream::ReadRun ( FILE * filePtr )
{
   CStringRW * Tbuff = new CStringRW ((RWSize_T) BUFFSIZE);
   Tbuff->maxWaste(BUFFSIZE);
   char * buff  = new char [BUFFSIZE+1];

   char * ptr;
   char * name;
   int number;
   Stand * stand;
   Group * group;

   if (!filePtr)
   {
      number = 1;
      goto OK;
   }

   if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;

   // insure group All exists and is first.

   Group *gr = new Group("All", 0);
   gr->autoAddsDone = 1;
   allGroups->insert(gr);

   ptr = strtok(buff," \n");

   if (ptr && strcmp(ptr,"!!Suppose") == 0)
   {
      ptr = strtok(NULL," \n");
      name = strtok(NULL," \n");
      ptr = strtok(NULL,"\n");

      // WARNING: the calling sequence in SetSelectedVariant uses strtok

      theSpGlobals->theVariants->SetSelectedVariant((const char *) name);
      sscanf (ptr,"%d %d",
         &theSpGlobals->theVariants->isPPEBeingUsed,
         &theSpGlobals->theVariants->PPEUseIsLocked);
      theSpGlobals->theVariants->variantSelectionLocked = 1;
      if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;

      // read the post processor tags

      if (strncmp (buff,"!!PP",4) == 0)
      {
         while (strncmp (buff,"!!PP",4) == 0)
         {
            ptr = strtok(buff," \n");
            name = strtok(NULL," \n");
            CStringRWC searchMSText = name;
            MSText * postProcMSText = (MSText *) theSpGlobals->
               theParmsData->find(&searchMSText);
            if (postProcMSText)
            {

               // To be valid, the post processors must have a name and a
               // command.

               PKeyData * namePKeyData    = postProcMSText->
                  FindPKeyData("name",NULL,0);
               PKeyData * commandPKeyData = postProcMSText->
                  FindPKeyData("command", theSpGlobals->OS,0);

               if (namePKeyData && commandPKeyData)
               {
                  PostProc * aPostProc =
                     new PostProc (namePKeyData->pString.data());
                  aPostProc->theMSText = postProcMSText;
                  aPostProc->theMSTextName = new CStringRW(name);
                  aPostProc->command =
                     new CStringRW(commandPKeyData->pString);
                  includedPostProcs->insert(aPostProc);
               }
            }
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
         }
      }

      if (strncmp (buff,"!!Top",5) == 0)
         if (ReadRunSegment(&topKeywords, filePtr, buff, Tbuff))
            goto ExitEOF;

         // can't call ReadRunSegment because we're looking for Comment, then
         // End not !!End

         if (strncmp (buff,"!!SN",4) == 0)
         {
            // toss the Comment keyword.

            if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            Tbuff->remove(0);
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            while (strncmp (buff,"End\n",4) != 0)
            {
               *Tbuff += buff;
               if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            }
            *Tbuff = Tbuff->strip(RWCString::trailing,'\n');

            if (!topSimNotes) topSimNotes = new CStringRW (Tbuff->data(),
               Tbuff->length());
            else *topSimNotes = *Tbuff;
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
         }

         // scan for and read the topPPEComponents.

         if (ReadPPEComponents(&topPPEComponents,filePtr,
                               Tbuff,buff,"!!CT") == EOF)
            goto ExitEOF;

         if (strncmp (buff,"!!Stand",7) == 0)
         {
            while (strncmp (buff,"!!Stand",7) == 0)
            {
               // parse the stand name and number, create the stand, and
               // call the stand reader on the stand object.

               ptr = strtok(buff," \n");
               name = strtok(NULL," \n");
               ptr = strtok(NULL," \n");
               sscanf (ptr,"%d", &number);
               stand = new Stand(name,number);
               allStands->insert(stand);
               number = stand->ReadStand(filePtr, Tbuff, buff);
               if (number != 0) goto OK;
               if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
               while (strncmp (buff,"!!",2) != 0)
                  if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            }
         }

         // scan for and read the botPPEComponents.

         if (ReadPPEComponents(&botPPEComponents,filePtr,
                               Tbuff,buff,"!!CB") == EOF)
            goto ExitEOF;

         // scan for and read the reprotPPEComponents.

         if (ReadPPEComponents(&reportPPEComponents,filePtr,
                               Tbuff,buff,"!!CR") == EOF)
            goto ExitEOF;

         if (strncmp (buff,"!!G ",4) == 0)
         {
            while (strncmp (buff,"!!G ",4) == 0)
            {
               ptr = strtok(buff," \n");
               name = strtok(NULL," \n");
               ptr = strtok(NULL," \n");
               sscanf (ptr,"%d", &number);
               group = FindGroup (name,number);
               if (!group)
               {
                  // create the group and add it to the run.

                  group = new Group(name,number);
                  group->autoAddsDone = 1;
                  allGroups->insert(group);
               }

               // read a record...while we have
               // !!S records, add stands to group.

               if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
               while (strncmp (buff,"!!S ",4) == 0)
               {
                  ptr = strtok(buff," \n");
                  name = strtok(NULL," \n");
                  ptr = strtok(NULL," \n");
                  sscanf (ptr,"%d", &number);
                  stand = FindStand (name,number);
                  if (stand) group->AddStandToGroup(stand);
                  if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
               }
               while (strncmp (buff,"!!",2) != 0)
                  if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            }
         }

         if (strncmp (buff,"!!Subset",4) == 0)
         {
            if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            while (strncmp (buff,"!!G ",4) == 0)
            {
               ptr = strtok(buff," \n");
               name = strtok(NULL," \n");
               ptr = strtok(NULL," \n");
               sscanf (ptr,"%d", &number);
               group = FindGroup (name,number);
               if (!group) group = new Group (name,number);
               group->autoAddsDone = 1;
               AddGroupToSubset(group);
               if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            }
            while (strncmp (buff,"!!S ",4) == 0)
            {
               ptr = strtok(buff," \n");
               name = strtok(NULL," \n");
               ptr = strtok(NULL," \n");
               sscanf (ptr,"%d", &number);
               stand = FindStand (name,number);
               if (stand) AddStandToSubset(stand);
               if (!fgets (buff, BUFFSIZE, filePtr)) goto ExitEOF;
            }
            if (strncmp (buff,"!!End",5) != 0)
            {
               number = 1;
               goto OK;
            }

            // lock the variant and "pick" the program.

            theSpGlobals->theVariants->variantSelectionLocked = 1;
            theSpGlobals->thePrograms->PickProgram(1);

         }
   }
   else
   {
      // the file is not a Suppose run file.

      number = 1;
      goto OK;

   }
   number = 0;

   goto OK;

   ExitEOF: number = EOF;

   OK:

   delete Tbuff;
   delete []buff;

   return number;
}



int RunStream::AddComponent(Component * c,
                            Component * place,
                            int         after)
{
   // Return codes:
   // 0 = all went well
   // 1 = items are not components, or variant tag is for a
   //     variant that we can not use.

   // Actions:
   // 1) add the component to the list of all components
   // 2) add the component to the componentsOrGroups list in the
   //    current subset stands,
   // 3) add to the component list in the current subset groups

   // in each of these 3 cases, the component is added in the above
   // lists just above the location of the "place" component.

   // if place is null, c the placement is found via the routine
   // FindInsertPlace.

   // if after is 1, c is added after place.  if place is null, after
   // is ignored.

   // process PPEComponents first.

   if (c->isA() == PPECOMPONENT)
   {
      if (place && place->isA() != PPECOMPONENT) return 1;
      if (c->variant)
      {
         if (!theSpGlobals->theVariants->
            NewStandVariantsInteraction(c->variant->data())) return 1;
      }
      if (theSpGlobals->theExtensions->AddExtensionInUse("ppe"))
      {
         SendUpdateMessage (EXTENSIONListChanged);
      }
      if (c->extension && theSpGlobals->theExtensions->
         AddExtensionInUse(c->extension->data()))
      {
         SendUpdateMessage (EXTENSIONListChanged);
      }
      switch (c->componentType)
      {
      case 4:  // topPPEComponents
         {
            if (!topPPEComponents) topPPEComponents = new RWOrdered(30);
            AddToCollection(topPPEComponents, c, place, after);
            break;
         }
      case 5:  // botPPEComponents
         {
            if (!botPPEComponents) botPPEComponents = new RWOrdered(30);
            AddToCollection(botPPEComponents, c, place, after);
            break;
         }
      case 6:  // reportPPEComponents
         {
            if (!reportPPEComponents) reportPPEComponents = new RWOrdered(30);
            AddToCollection(reportPPEComponents, c, place, after);
            break;
         }
      default: return 1;
      }
      return 0;
   }

   if (c->isA() != COMPONENT) return 1;
   if (place && place->isA() != COMPONENT) return 1;

   // if the variant is defined, make sure it is OK...

   if (c->variant)
   {
      if (!theSpGlobals->theVariants->
         NewStandVariantsInteraction(c->variant->data())) return 1;
   }

   // find out if the component is already in the list of all
   // components.

   RWOrdered * tmpSubsetStands = NULL;
   RWOrdered * tmpSubsetGroups = NULL;
   RWOrdered * tmp             = NULL;

   if (allComponents->contains(c))
   {

      // temporarily change the current subset to one
      // that reflects the component's current placement.

      tmpSubsetStands = new RWOrdered(5);

      RWOrderedIterator aStand(*allStands);
      Stand * stand;
      while (stand = (Stand *) aStand())
      {
         if (stand->IsComponentInStand(c)) tmpSubsetStands->insert(stand);
      }
      tmp = subsetStands;
      subsetStands = tmpSubsetStands;
      tmpSubsetStands = tmp;

      tmpSubsetGroups = new RWOrdered(5);

      RWOrderedIterator aGroup(*allGroups);
      Group * group;
      while (group = (Group *) aGroup())
      {
         if (group->IsComponentInGroup(c)) tmpSubsetGroups->insert(group);
      }
      tmp = subsetGroups;
      subsetGroups = tmpSubsetGroups;
      tmpSubsetGroups = tmp;

      // check the placement of the component in the list of all
      // components...insure that it is correct.

      if (InvalidPlacement(allComponents, c))
      {
         allComponents->remove(c);
         RemoveComponentFromSpGroups(c);  // Occurs only if component type is SpGroup

         Component * placeInList = place;
         if (!placeInList) placeInList = FindInsertPlace (allComponents, c, &after);
         AddToCollection(allComponents, c, placeInList, after);
         AddComponentToSpGroups(c);  // Occurs only if component type is SpGroup
      }
   }
   else
   {
      // component is not in the list... add it!

      Component * placeInList = place;
      if (!placeInList) placeInList = FindInsertPlace (allComponents, c, &after);
      AddToCollection(allComponents, c, placeInList, after);

      AddComponentToSpGroups(c);  // Occurs only if component type is SpGroup

      // the component was not in the list.  Update the list of
      // extensions in use.  If this list changed, notify the
      // CDocument class for the need to update windows, as necessary.

      if (c->extension && theSpGlobals->theExtensions->
         AddExtensionInUse(c->extension->data()))
      {
         SendUpdateMessage (EXTENSIONListChanged);
      }
   }

   if (c->componentNumber > componentNumbers)
      componentNumbers = c->componentNumber+1;

   RWOrderedIterator aStand(*subsetStands);
   Stand * stand;
   while (stand = (Stand *) aStand())
   {
      stand->AddComponentOrGroupToStand(c,place,after);
   }

   RWOrderedIterator aGroup(*subsetGroups);
   Group * group;
   while (group = (Group *) aGroup())
   {
      group->AddComponentToGroup(c,place,after);
   }

   // if the subset groups and stands have been switched, then
   // switch them back and delete the temporary ones.

   if (tmpSubsetStands)
   {
      delete subsetStands;
      subsetStands = tmpSubsetStands;

      delete subsetGroups;
      subsetGroups = tmpSubsetGroups;
   }
   return 0;
}


void RunStream::AddToCollection(RWOrdered * theList, Component * c,
                                Component * placeInList, int after)
{
   if (theList)
   {
      if (!theList->contains(c))
      {
         size_t location = RW_NPOS;
         if (placeInList) location = theList->index(placeInList);
         if (location != RW_NPOS)
         {
            if (after)
            {
               if (location < theList->entries())
                  theList->insertAt(location+1, c);
               else theList->insert(c);
            }
            else theList->insertAt(location, c);
         }
         else theList->insert(c);
      }
   }
}


int RunStream::AddGroup (Group * g)
{
   // add a group to the list of all groups.

   if (!allGroups->contains(g)) allGroups->insert(g);

   if (g->groupNumber > groupNumbers) groupNumbers = g->groupNumber+1;

   return 0;
}



int RunStream::AddStand (Stand * s)
{
   // add a stand to the list of all stands.

   if (!allStands->contains(s)) allStands->insert(s);

   if (s->standNumber > standNumbers) standNumbers = s->standNumber+1;

   return 0;
}



int RunStream::DelComponent (Component * c)
{
   // Return codes:
   // 0 = all ok.
   // 1 = what you want to delete is not a component, or if
   // a ppe component, not a valid componentType.

   if (c->isA() == COMPONENT)
   {
      // remove components form all stands.

      RWOrderedIterator nextStand(*allStands);
      Stand * stand;
      while (stand = (Stand *) nextStand())
      {
         stand->DelComponentOrGroupFromStand(c);
      }

      // remove components form all groups.

      RWOrderedIterator nextGroup(*allGroups);
      Group * group;
      while (group = (Group *) nextGroup())
      {
         group->DelComponentFromGroup(c);
      }

      RemoveComponentFromSpGroups(c); // Occurs only if component type is SpGroup

      // delete the component.

      if (componentClipBoard == c) allComponents->remove(c);
      else                         allComponents->removeAndDestroy(c);

      return 0;
   }

   else if (c->isA() == PPECOMPONENT)
   {
      switch (c->componentType)
      {
      case 4:  // topPPEComponents
         {
            if (topPPEComponents)
            {
               if (componentClipBoard == c) topPPEComponents->remove(c);
               else                         topPPEComponents->removeAndDestroy(c);
               if (!topPPEComponents->entries())
               {
                  delete topPPEComponents;
                  topPPEComponents = NULL;
               }
            }
            break;
         }
      case 5:  // botPPEComponents
         {
            if (botPPEComponents)
            {
               if (componentClipBoard == c) botPPEComponents->remove(c);
               else                         botPPEComponents->removeAndDestroy(c);
               if (!botPPEComponents->entries())
               {
                  delete botPPEComponents;
                  botPPEComponents = NULL;
               }
            }
            break;
         }
      case 6:  // reportPPEComponents
         {
            if (reportPPEComponents)
            {
               if (componentClipBoard == c) reportPPEComponents->remove(c);
               else                         reportPPEComponents->removeAndDestroy(c);
               if (!reportPPEComponents->entries())
               {
                  delete reportPPEComponents;
                  reportPPEComponents = NULL;
               }
            }
            break;
         }
      default: return 1;
      }
      return 0;
   }
   return 1;
}



int RunStream::DelGroup (Group * g)
{
   // Return codes:
   // 0 = all ok.
   // 1 = what you want to delete is not a group.

   if (g->isA() != GROUP) return 1;

   // 1. remove components that are in the group's list of components.
   // this is done with a simple delete to avoid side effects of calling
   // DelComponent().  That routine is designed to remove a component
   // and it removes it from the groups or stands to which it is attached.
   // I don't use it here because of the logical problem of removing
   // a component from a group that is being deleted.

   if (g->componentList)
   {
      RWOrderedIterator nextComponent(*g->componentList);
      Component * c;
      while (c = (Component *) nextComponent())
      {
         if (componentClipBoard == c) allComponents->remove(c);
         else                         allComponents->removeAndDestroy(c);
      }
      g->componentList->clear();
   }

   // 2. remove stands form the group's list of standMembership.
   // this results in the group being removed from every stand's
   // list of componentsOrGroups...making it safe to delete the
   // group.

   g->ClearStandMembership();

   // remove the group from the subsetGroups list

   DelGroupFromSubset(g);

   // delete the group.

   allGroups->removeAndDestroy(g);

   return 0;
}



int RunStream::DelStand (Stand * s)
{
   // Return codes:
   // 0 = all ok.
   // 1 = what you want to delete is not a stand.

   if (s->isA() != STAND) return 1;

   // remove stands form all groups.

   RWOrderedIterator nextGroup(*allGroups);
   Group * group;
   while (group = (Group *) nextGroup())
   {
      group->DelStandFromGroup(s);
   }

   // delete stand from subset (if it is present)

   DelStandFromSubset(s);

   // delete the stand.

   allStands->removeAndDestroy(s);

   return 0;
}




Component * RunStream::FindComponent (const char * name,
                                      const int number) const
{
   RWOrderedIterator nextComponent(*allComponents);
   Component * component;
   while (component = (Component *) nextComponent())
   {
      if (component->componentNumber == number &&
         strcmp(component->data(),name) == 0)
         return component;
   }
   return (Component *) 0;
}



Group * RunStream::FindGroup (const char * name, const int number) const
{
   RWOrderedIterator nextGroup(*allGroups);
   Group * group;
   while (group = (Group *) nextGroup())
   {
      if (group->groupNumber == number &&
         strcmp(group->data(),name) == 0)
         return group;
   }
   return (Group *) 0;
}



Stand * RunStream::FindStand (const char * name, const int number) const
{
   RWOrderedIterator nextStand(*allStands);
   Stand * stand;
   while (stand = (Stand *) nextStand())
   {
      if (stand->standNumber == number &&
         strcmp(stand->data(),name) == 0)
         return stand;
   }
   return (Stand *) 0;
}



int RunStream::AddGroupToSubset (Group * g)
{
   // Return codes
   // 0 = all ok.
   // 1 = group was already in the subset
   // 2 = adding the group would imply that a stand in the
   //     group is already in the subset, either as a stand
   //     or as a member of a group that is in the subset.

   if (subsetGroups->contains(g)) return 1;

   if (g->standMembership)
   {
      RWOrderedIterator next(*g->standMembership);
      Stand * stand;
      while (stand = (Stand *) next())
      {
         if (subsetStands->contains(stand)) return 2;

         RWOrderedIterator subsetGroup(*subsetGroups);
         Group * group;
         while (group = (Group *) subsetGroup())
         {
            if (group->standMembership &&
               group->standMembership->contains(stand)) return 2;
         }
      }
   }

   // insure group is in the list of all groups

   if (!allGroups->contains(g)) allGroups->insert(g);

   subsetGroups->insert(g);

   return 0;
}



int RunStream::AddStandToSubset (Stand * s)
{
   // Return codes
   // 0 = all ok.
   // 1 = stand was already in the subset of stands
   // 2 = stand was already in a group that is already
   //     in the subset

   if (subsetStands->contains(s)) return 1;

   RWOrderedIterator subsetGroup(*subsetGroups);
   Group * group;
   while (group = (Group *) subsetGroup())
   {
      if (group->standMembership &&
         group->standMembership->contains(s)) return 2;
   }

   // insure stand is in the list of all stands.

   if (!allStands->contains(s)) allStands->insert(s);

   subsetStands->insert(s);

   return 0;
}




int RunStream::DelGroupFromSubset (Group * g)
{
   // delete a group from the list of subset stands.

   subsetGroups->remove(g);

   return 0;
}



int RunStream::DelStandFromSubset (Stand * s)
{
   // delete a stand from the list of subset stands.

   subsetStands->remove(s);

   return 0;
}



RWCollectable * RunStream::GetCurrent(void)
{
   RWCollectable * curr = NULL;
   if (subsetStands->entries()) curr=subsetStands->first();
   if (subsetGroups->entries()) curr=subsetGroups->first();
   return curr;
}



void RunStream::BuildSubSetComponentList (RWOrdered  * theList,
                                          int          theType,
                                          const char * theExtension,
                                          const char * theVariant,
                                          const Component * curComponent)
{
/******************************************************************

  Loads an RWOrdered list of components that make up those
  components attached to the current subset.  For a component
  to be included it must be included in ALL stands of the
  subset, not just a subset of the stands in the subset.

    Only components of a specific type will be in the list if
    the specific type is identified in calling arguments.  Type
    matching can be on any one or all of the following:
    componentType, variant (a member of the string), or extension.

      Calling arguments:

        theList
        the list of components.  it is owned by the calling
        routine.  If passed in as a NULL, this routine returns.

        theType
        the component type must match this number.  If this number
        is passed in as a negetive number, then any component is
        matched.

        theExtension
        the extension of each component must contain the extension
        contained in this string.  If the string is NULL, then
        NO matching is necessary.

        theVariant
        the variant of each component must contain the variant(s)
        contained in this string.  If the string is NULL, then
        NO matching is necessary.

        curComponent
        if not NULL, the list should contain components that are
        in the same stand or group as curComponent (not the current subset).

   ******************************************************************/

   if (!theList) return;

   theList->clear();

   if (allComponents->entries())
   {
      if (curComponent)
      {
         RWOrderedIterator nextGroup(*allGroups);
         Group * group;
         while (group = (Group *) nextGroup())
         {
            if (group->componentList &&
               group->componentList->contains(curComponent))
            {
               RWOrderedIterator nextComponent(*group->componentList);
               Component * component;
               while (component = (Component *) nextComponent())
               {
                  if (theType > -1)
                  {
                     if (theType == component->componentType)
                        theList->insert(component);
                  }
                  else  theList->insert(component);
               }
               break;
            }
         }

         if (!theList->entries())
         {
            RWOrderedIterator nextStand(*allStands);
            Stand * stand;
            while (stand = (Stand *) nextStand())
            {
               if (stand->IsComponentInStand(curComponent))
               {
                  RWOrderedIterator nextItem(*stand->componentsOrGroups);
                  RWCollectable * item;
                  while (item = (RWCollectable *) nextItem())
                  {
                     if (item->isA() == COMPONENT)
                     {
                        Component * component = (Component *) item;
                        if (theType > -1)
                        {
                           if (theType == component->componentType)
                              theList->insert(component);
                        }
                        else  theList->insert(component);
                     }
                  }
                  break;
               }
            }
         }
      }
      else
      {
         RWOrderedIterator nextComponent(*allComponents);
         Component * component;
         while (component = (Component *) nextComponent())
         {
            RWOrderedIterator subsetGroup(*subsetGroups);
            Group * group;
            while ((group = (Group *) subsetGroup()) != NULL)
            {
               if (group->componentList &&
                  group->componentList->contains(component))
               {
                  if (theType > -1)
                  {
                     if (theType == component->componentType)
                     {
                        theList->insert(component);
                        break;
                     }
                  }
                  else theList->insert(component);
               }
            }
            RWOrderedIterator subsetStand(*subsetStands);
            Stand * stand;
            while ((stand = (Stand *) subsetStand()) != NULL)
            {
               if (stand->componentsOrGroups &&
                  stand->componentsOrGroups->contains(component))
               {
                  if (theType > -1)
                  {
                     if (theType == component->componentType)
                     {
                        theList->insert(component);
                        break;
                     }
                  }
                  else theList->insert(component);
               }
            }
         }
      }
   }
}





void RunStream::PutComponentOnClipBoard (Component * c)
{
   // set a pointer to the componentClipBoard.

   ClearComponentClipBoard();
   componentClipBoard = c;

}



Component * RunStream::GetComponentFromClipBoard (void)
{
   // return the componentClipBoard.

   return componentClipBoard;

}


void RunStream::WritePPEComponents(RWOrdered * theList,
                                   FILE * filePtr, const char * tag)
{
   if (theList && filePtr)
   {
      RWOrderedIterator next(*theList);
      PPEComponent * c;
      PPEComponent * lastC = NULL;
      while (c = (PPEComponent *) next())
      {
         c->WriteComponent(lastC, NULL, filePtr, 0, tag);
         lastC = c;
      }
      if (lastC && lastC->extension &&
         strcmp(lastC->extension->data(),"ppe") != 0)
         fprintf (filePtr,"End\n");
   }
}


int RunStream::ReadPPEComponents(RWOrdered ** theList, FILE * filePtr,
                                 CStringRW * Tbuff, char * buff,
                                 const char * tag)
{
   int taglen=strlen(tag);
   if (strncmp (buff,tag,taglen) == 0)
   {
      int number;
      while (strncmp (buff,tag,taglen) == 0)
      {
         // parse the name and number, create a component, and call
         // the reader on the component passing the entire string.

         char * ptr = strtok(buff," \n");
         char * name = strtok(NULL,"~\n");
         ptr = strtok(NULL," \n");
         sscanf (ptr,"%d", &number);
         PPEComponent * component = new PPEComponent(name,number);
         if (!(*theList)) *theList = new RWOrdered(30);
         (*theList)->insert(component);
         number = component->ReadComponent (filePtr, Tbuff, buff);
         if (number) return number;
      }

      // skip to the next Suppose control line, marked by !!.

      while (strncmp (buff,"!!",2) != 0)
         if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }
   return 0;
}


Component * RunStream::PPEChecking(MSText * keywordMSText,
                                   Component * theComponent)
{
   // insure that theComponent is returned as with the correct
   // componentType set and that it is the correct species

   if (keywordMSText)
   {

      // find out if this keyword is actually a PPE keyword
      // and of what "type" (top, bot, report).

      PKeyData * ppePlacement =
         keywordMSText->FindPKeyData("ppe",NULL,0);

      if (ppePlacement)
      {
         if (ppePlacement->pString.
            compareTo("top",RWCString::ignoreCase) == 0)
            theComponent->componentType= 4;

         else if (ppePlacement->pString.
            compareTo("bottom",RWCString::ignoreCase) == 0)
            theComponent->componentType= 5;

         else if (ppePlacement->pString.
            compareTo("report",RWCString::ignoreCase) == 0)
            theComponent->componentType = 6;

      }
   }

   if (theComponent->componentType > 3 &&
      theComponent->isA() == COMPONENT)
   {
      // convert to PPEComponet and set component type

      Component * t = new PPEComponent(theComponent);
      delete theComponent;
      theComponent = t;

      // make sure that ppe is in the extensions list.

      if (theSpGlobals->theExtensions->AddExtensionInUse("ppe"))
      {
         SendUpdateMessage (EXTENSIONListChanged);
      }
   }
   return theComponent;
}




void RunStream::ClearComponentClipBoard (void)
{
   // clear the componentClipBoard.  If the component
   // is not listed in allComponents, then delete it.

   if (componentClipBoard)
   {
      if (!allComponents->contains(componentClipBoard))
         delete componentClipBoard;
      componentClipBoard = NULL;
   }
}


void RunStream::ProcessPPEComponents(RWOrdered * theList)
{

   // goto through theList and remove components that are
   // either already PPE components or should be
   // and are not.  Those that should be and are not are converted
   // to ppe components.  Any such components are removed from theList and
   // passed to AddComponent.

   Component * insertPlace = NULL;
   int after = 1;

   if (theList && theList->entries())
   {
      Component * component;
      size_t item = 0;
      while (item < theList->entries())
      {

         component = (Component *) theList->at(item++);
         if (component->isA() == PPECOMPONENT)
         {
            AddComponent(component, insertPlace, after);
            insertPlace = component;
            theList->remove(component);
            item--;
         }
         else
         {

            // this sequence is crafted to insure that the
            // removeAndDestroy does not delete a component
            // that has already been deleted inside of PPEChecking

            Component * newComponent = new Component(component);
            newComponent = PPEChecking(NULL,newComponent);

            if (newComponent->isA() == PPECOMPONENT)
            {
               AddComponent(newComponent, insertPlace, after);
               insertPlace = newComponent;
               theList->removeAndDestroy(component);
               item--;
            }
         }
      }
   }
}


void RunStream::CreateSpGroups( void )
{
   Component * component;

   // Create a collection of SpGroup -- caching increases lookup speed
   if (allSpGroups == NULL)
   {
      allSpGroups = new RWOrdered();

      RWOrderedIterator nextComponent(*theSpGlobals->runStream->allComponents);
      while (component = (Component *) nextComponent())
      {
         if (component->keywords->index("SpGroup") == 0)
         {
            allSpGroups->append(component);
         }
      }
   }
}


void RunStream::AddComponentToSpGroups( Component * component )
{
   if (allSpGroups == NULL)
      CreateSpGroups();

   if (component->keywords->index("SpGroup") == 0)
      allSpGroups->append(component);
}


void RunStream::RemoveComponentFromSpGroups( Component * component )
{
   if (allSpGroups == NULL)
      CreateSpGroups();

   if (component->keywords->index("SpGroup") == 0)
      allSpGroups->remove(component);
}


RWOrdered * RunStream::GetSpGroups( void )
{
   RWOrdered * SpGroupNames = new RWOrdered(); // List of group names -- freed by calling method
   Component * component;
   CStringRW groupName;

   if (allSpGroups == NULL)
      CreateSpGroups();

   RWOrderedIterator nextComponent(*theSpGlobals->runStream->allSpGroups);
   while (component = (Component *) nextComponent())
   {
      groupName = (*component->keywords)(10, 10);
      SpGroupNames->append(new CStringRWC(CStringRWC("SpGroup: ") + groupName.strip(RWCString::both)));
   }

   return SpGroupNames;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Global funcitons associated with RunStream, Group, Stand, & Component
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


int ReadRunSegment (CStringRW ** element, FILE * filePtr,
                    char * buff, CStringRW * Tbuff)
{

   Tbuff->remove(0);  // clear Tbuff
   if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   while (strncmp (buff,"!!End\n",6) != 0)
   {
      if (!*element) *Tbuff += buff; // if already defined, don't save.
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   }
   if (!*element) // if already defined, don't save.
   {
      Tbuff->remove(Tbuff->length()-1);
      *element = new CStringRW (Tbuff->data(), Tbuff->length());
   }
   if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   while (strncmp (buff,"!!",2) != 0)
      if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
      return 0;
}


void WriteWindowParms (FILE * filePtr, CStringRW * windowParms)
{
   // find out how long the string is.  If less than or equal to
   // 75...then just write it.

   if (windowParms->length() <= 75)
      fprintf (filePtr,"!!P %s.\n", windowParms->data());

   else
   {
      int n = (windowParms->length()/70);
      if ( (size_t) n*70 == windowParms->length()) n--;
      size_t j = 70;
      RWCString part = (*windowParms)(0,j);
      fprintf (filePtr,"!!P # %i %s.\n", n,  part.data());
      for (int i = 0; i < n; i++)
      {
         int len = min(70,windowParms->length()-j);
         part = (*windowParms)(j,len);
         fprintf (filePtr,"!! %s.\n", part.data());
         j=j+70;
      }
   }
}




int ReadWindowParms (FILE * filePtr, char * buff,
                     CStringRW ** windowParmsPtr)
{
   char * ptr = buff+4;
   int n = 0;
   int i = 0;
   int L = *windowParmsPtr == NULL;

   if ((int) *ptr == (int) '#')
   {
      sscanf (++ptr,"%i%n",&n,&i);
      ptr+=(i+1);
   }
   if (L)
   {
      *(ptr+strlen(ptr)-2) = 0;  // remove ending period and the line feed.
      i=strlen(ptr)+(n*70);
      *windowParmsPtr = new CStringRW ((RWSize_T) i);
      (**windowParmsPtr).maxWaste((size_t) i);
      **windowParmsPtr = ptr;
   }
   if (n > 0)
   {
      for (i = 0; i < n; i++)
      {
         if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
         if (L)
         {
            ptr=buff+3;
            *(ptr+strlen(ptr)-2) = 0;
            **windowParmsPtr+=ptr;
         }
      }
   }
   if (!fgets (buff, BUFFSIZE, filePtr)) return EOF;
   return 0;
}





Component * FindInsertPlace (RWOrdered * theList,
                             Component * c,
                             int       * after)
{

/******************************************************************

   theList
      the list of components.  it is owned by the calling
      routine.  If passed in as a NULL, the routine returns NULL.

   c
      the component we want to insert.  If NULL or not a component,
      the routine returns NULL.

   after
      set by this routine to 0 if the insert point is before
      the component and to 1 if the insert point is after.
      If NULL, the routine returns NULL.

   Rules for finding the insert place:

   Case 1:

      when a component is added that depends a conditionComponent.

         search for the conditionComponent

       if the conditionComponent exists:

            find the last component that depends on the same
       conditionComponent.  if the component is related to an
       extension, then find the last component that is related
       to the same extension that also depends on the same
       conditionComponent.

       if no other component exists, then select the
       conditionComponent.

       insert the component after the selected component.

         else

            pretend the component does not depend on a condition.

   Case 2:

      when the new component does not depend on a condition.  (it is
      either a conditionComponent or some other kind).

         if the component is related to an extension:

            find the last component that is related to the same
       extension and is also *not* dependent on a condition.

            insert after the found component.

         else

            append the component.

******************************************************************/

   if (!theList || !c || !after) return NULL;

   if (c->isA() != COMPONENT) return NULL;

   Component * c1;
   Component * c2;
   Component * c3;
   Component * c4;

   if (c->componentType == 3) // c depends on a conditionComponent.
   {
      c2 = NULL;
      c3 = NULL;
      c4 = NULL;
      RWOrderedIterator componentOrGroupIter(*theList);
      RWCollectable * aComponentOrGroup;
      while (aComponentOrGroup = componentOrGroupIter())
      {
         if (aComponentOrGroup->isA() == COMPONENT)
         {
            c1 = (Component *) aComponentOrGroup;
            // c2 points to the condition
            if (c1->componentType   == 1 &&
               c1->componentNumber == c->conditionSerialNum) c2 = c1;
            // c3 points to the last
            // type 3 that uses c2.
            if (c2 &&
               c1->componentType      == 3 &&
               c1->conditionSerialNum == c->conditionSerialNum)
            {
               c3 = c1;
               if ( c->extension &&
                  c3->extension &&
                  strcmp ( c->extension->data(),
                  c3->extension->data()) == 0) c4 = c3;
            }
         }
      }
      if (c4) c3 = c4;
      if (c3) c2 = c3;
      if (c2) *after = 1;
      return c2;
   }
   else                       // c does not depend on a conditionComponent.
   {
      if (c->extension)
      {
         c2 = NULL;
         RWOrderedIterator componentOrGroupIter(*theList);
         RWCollectable * aComponentOrGroup;
         while (aComponentOrGroup = componentOrGroupIter())
         {
            if (aComponentOrGroup->isA() == COMPONENT)
            {
               c1 = (Component *) aComponentOrGroup;

               if ( c1->extension &&
                  (c1->componentType <= 0 || c1->componentType == 2) &&
                  strcmp ( c->extension->data(),
                  c1->extension->data()) == 0) c2 = c1;
            }
         }
         if (c2) *after = 1;
         return c2;
      }
   }
   return NULL;
}




int InvalidPlacement (RWOrdered * theList,
                      Component * c)
{
/******************************************************************

  uses a set of rules to determine if a component is placed in
  a list at a know, "invalid" place.  Calling this routine does
  not change the placement.  And, if InvalidPlacement returns a 0,
  the placement *may* still be invalid.

    theList
    the list of components.  it is owned by the calling
    routine.  If passed in as a NULL, the routine returns 0.

    c
    the component we want to check.  If NULL or not a component,
    the routine returns 0.

    Rules for finding out if a component has an invalid placement are
    listed in the code below.

******************************************************************/

   if ( !theList || !c ) return 0;

   if (c->isA() != COMPONENT) return 0;

   // Case 1: c is component type 1: an IF condition.

   // rule:  not invalid.

   if (c->componentType == 1) return 0;

   // Case 2: c is component type 2 (or -1 or 0): keyword set
   // outside an IF.

   // rule:  if a component follows that is of type 3 (keyword set inside
   //        an if), then the placement of c is invalid.

   RWCollectable * aComponentOrGroup;
   size_t place;

   if (c->componentType == 2 || c->componentType <= 0)
   {
      place = theList->index(c);

      // if place is -1, c does not exist in theList.

      if (place == -1) return 0;

      // find the next component.

      if (place+1 >= theList->entries()) return 0;

      aComponentOrGroup = (RWCollectable *) ((*theList)[place+1]);

      if (aComponentOrGroup->isA() != COMPONENT) return 0;

      if (((Component *) aComponentOrGroup)->componentType == 3) return 1;
      return 0;
   }

   // Case 3: c is component type 3: keyword set inside an IF.

   // rule:  placement of c is invalid if...
   //     ...c follows a component that is not its condition component, OR
   //     ...c follows a component that is not a type 3 that is
   //        dependent on the SAME condition as c, then the placement of
   //        c is invalid, OR
   //     ...c preceeds its condition component.

   if (c->componentType == 3)
   {
      place = theList->index(c);

      // if c is not in the list, it is not invlaid.

      if (place < 0) return 0;

      // if there is only one component in the list, the placement is
      // not invalid.

      if (theList->entries() == 1) return 0;

      if (place > 0)
      {
         aComponentOrGroup = (RWCollectable *) ((*theList)[place-1]);

         if (aComponentOrGroup->isA() != COMPONENT) return 0;

         Component * c1 = (Component *) aComponentOrGroup;

         if (c1->componentType == 1 &&
            (c->conditionSerialNum == c1->componentNumber)) return 0;

         if (c1->componentType == 3 &&
            (c->conditionSerialNum == c1->conditionSerialNum)) return 0;
         return 1;
      }

      // find out if the condition component follows is in the list.  If it
      // does, then the position of c is invalid.

      RWOrderedIterator nextComponentOrGroup(*theList);
      RWCollectable * componentOrGroup;
      while (componentOrGroup = (RWCollectable *) nextComponentOrGroup())
      {
         if (componentOrGroup->isA() == COMPONENT)
         {
            Component * c1 = (Component *) componentOrGroup;
            if (c1->componentType == 1 &&
               (c->conditionSerialNum == c1->componentNumber)) return 1;
         }
      }
   }
   return 0;
}


