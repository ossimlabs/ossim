//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
//
// Contains class definition for ossimNormalizedU15RemapTable.
// 
//*******************************************************************
//  $Id: ossimNormalizedU15RemapTable.cpp 15743 2009-10-17 13:00:45Z dburken $

#include <ossim/imaging/ossimNormalizedU15RemapTable.h>

ossim_float64 ossimNormalizedU15RemapTable::theTable[TABLE_ENTRIES];
bool          ossimNormalizedU15RemapTable::theTableIsInitialized = false;

ossimNormalizedU15RemapTable::ossimNormalizedU15RemapTable()
   : ossimNormalizedRemapTable()
{
   if (!theTableIsInitialized)
   {
      const ossim_int32   ENTRIES     = getEntries();
      const ossim_float64 DENOMINATOR = getNormalizer();
      
      //---
      // Initialize the remap table.
      //---
      theTable[0] = 0.0; // Index zero always for null.
      for (ossim_int32 i = 1; i < ENTRIES; ++i)
      {
         theTable[i] = static_cast<ossim_float64>(i)/DENOMINATOR;
      }

      theTableIsInitialized = true;
   }
}

ossimNormalizedU15RemapTable::~ossimNormalizedU15RemapTable()
{}
