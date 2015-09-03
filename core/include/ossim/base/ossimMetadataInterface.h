//----------------------------------------------------------------------------
// File: ossimFileProcessorInterface.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Class interface ossimMetadataInterface.
//
// Has pure virtual "addMetadata" method that derived classes must
// implement to be concrete.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimMetadataInterface_HEADER
#define ossimMetadataInterface_HEADER 1

#include <string>

class ossimKeywordlist;

/** @class ossimMetadataInterface */
class ossimMetadataInterface
{
public:
   
   /** @brief default constructor */
   ossimMetadataInterface(){}

   /** @brief virtual destructor. */
   virtual ~ossimMetadataInterface(){}

   /**
    * @brief Pure virtual addMetadata method. Derived classed must implement.
    * @param kwl Initialized by this.
    * @param prefix e.g. "image0." (can be empty)
    */
   virtual void addMetadata( ossimKeywordlist* kwl,
                             const std::string& prefix ) const = 0;
};

#endif /* #ifndef ossimMetadataInterface_HEADER */
