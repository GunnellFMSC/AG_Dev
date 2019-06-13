/********************************************************************

   File Name:     variants.hpp
   Author:        nlc
   Date:          04/20/94

   Purpose:  To house and manipulate all data related to the selection
             of the variant.  This class does not provide a graphic
             interface, rather another class that does provide for
             graphic interface is used to access and/or set data stored
             herein.

   PPE:      The data related to selection, nonselection of the PPE
             is also in this class.

 Notes on overall approach.........

 Simulation Commands, FVS Programs, and FVS variants:
 
 This is a description of how Suppose keeps track of commands, FVS programs, 
 variants, and extensions.  
 
 The idea is that a user can select a variant and weather or not they prefer
 to use the PPE.  These preferences are set, in sequence, first from
 Suppose code (in SpPreferences::SpPreferences), then from the preferences
 portion of the parameters file, then from the preferences file, then from
 the command line.  However, these selections are only "suggestions" to
 suppose.  The selections are "locked" as users make selections from various
 menus that imply a specific variant, extension, program, and command be
 used to run the simulation.  The exception to these "suggestions" is that
 in the case where the user codes a variant= parameter on the command line,
 that that variant is locked...it is not considered a default, but a 
 requirement.
 
 Here is a list of "processing" steps taken by Suppose to implement logic:
 
 1. Make a list of valid commands by calling SpCommands::SpCommands.  
 
 2. Make a list of valid programs by calling SpPrograms::SpPrograms.  
 
 3. Make a list of valid variants by calling SpVariants::SpVariants.
 
 (Note: all of these classes can be accessed via SpGlobals.)
 
 When an action is taken that limits the list of valid commands, then reset
 the list of valid programs, then the list of valid variants.  When the list
 of valid programs and/or variants happens, then the "selected" program or
 variant is also reviewed.  Changing the use or nonuse of the PPE is an
 example of an action that could change the list of valid commands.
 
 When an action that changes the list of valid programs happens, then review
 the list of variants and reset as necessary.  An example of such an action
 is to read an existing simulation file.  If that happens, the program that
 was used in the simulation file is the one needed so Suppose locks out
 other programs.  Another case is the selection or deselection of the PPE or
 other extension.
 
 When an action that is taken to change the variant, the list of valid
 variants is modified accordingly.
 
 Often, an action "to change" the variant or and FVS program, is not being 
 taken, but rather an action that "locks" the current choice.  When an item
 is locked, making changes to depending choices is limited to selections
 that are compatible to the locked selection.  For example, say a stand is
 added to the list of simulation stands that requires a specific variant.
 The selection of the stand "locks" the variant selection.  Subsequent
 attempts to make a selection not compatible with the variant is then
 blocked.  Say a user selects variant ci, selects stands that require ci,
 and then trys to select the PPE.  If there is no program that contains both
 the variant ci and the PPE, then the selection of the PPE is blocked as is
 the change of the selected program to one that contains the PPE.  
 
 The code to implement this approach is found in classes SpCommands,
 SpPrograms, and SpVariants.  Calls to these classes are made from various
 locations in the program.  
 
 ********************************************************************/

#ifndef __SpVariants_HPP
#define __SpVariants_HPP

#include "PwrDef.h"

class PKeyData;
class MSText;
class RWOrdered;

class SpVariants
{
   public:

      SpVariants ( void );
      ~SpVariants ( void );

      const char    *GetSelectedVariant( void );
      int            SetSelectedVariant( const char * theVariant);
      int            LoadVariantCollection (RWOrdered * theVariantList);
      int            LoadListOfLegalVariants( void );
      int            FindLegalVariant( const char * theVariant );
      int            NewStandVariantsInteraction ( const char * theRequiredVariants );

      int            isPPEBeingUsed;            // -1 = not set, 0 = no, 1 = yes
      int            PPEUseIsLocked;

      int            theSelectedVariant;
      int            numberVariants;
      MSText        *variantsMSTextPointer;
      int            variantSelectionLocked;

      typedef PKeyData *ptrToPKeyData;
      ptrToPKeyData    *listOfVariantsPKeyData;

   private:

      int OKtoSendMsg;
};

#endif
