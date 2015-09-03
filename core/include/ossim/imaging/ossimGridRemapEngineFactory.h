//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains declaration of class ossimGridRemapEngineFactory
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: ossimGridRemapEngineFactory.h 23356 2015-06-03 18:28:03Z dburken $

#ifndef ossimGridRemapEngineFactory_HEADER
#define ossimGridRemapEngineFactory_HEADER 1

#include <ossim/imaging/ossimGridRemapEngine.h>

/*!****************************************************************************
 *
 * CLASS:  ossimGridRemapEngineFactory (SIMPLETON)
 *
 *****************************************************************************/
class OSSIM_DLL ossimGridRemapEngineFactory
{
public:
   /*!
    * Attempts to create an instance of the Product given a Specifier. Returns
    * successfully constructed product or NULL.
    */
   static ossimGridRemapEngine* create(const char* spec);
   
protected:
   ossimGridRemapEngineFactory() {}
};

#endif
