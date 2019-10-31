//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
//
// Contains class definition for ossimNormalizedU9RemapTable.
// 
//*******************************************************************

#include <ossim/imaging/ossimNormalizedU9RemapTable.h>

ossim_float64 ossimNormalizedU9RemapTable::theTable[TABLE_ENTRIES];
bool          ossimNormalizedU9RemapTable::theTableIsInitialized = false;

ossimNormalizedU9RemapTable::ossimNormalizedU9RemapTable()
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

ossimNormalizedU9RemapTable::~ossimNormalizedU9RemapTable()
{}
