//---
// File: ossimFileInfoInterface.h
// 
// License: MIT
//
// Description: Class ossimFileInfoInterface.
//
// Interface class for file info things. Written for stream code from url,
// e.g. AWS ossim::S3IStream.
// 
//---
// $Id$

#ifndef ossimFileInfoInterface_HEADER
#define ossimFileInfoInterface_HEADER 1

#include <ossim/base/ossimConstants.h>

/** @class ossimFileInfoInterface */
class ossimFileInfoInterface
{
public:

   /** @brief virtual destructor. */
   virtual ~ossimFileInfoInterface(){}

   /**
    * @brief Pure virtual file size method. Derived classed must implement.
    * @return File size in bytes.
    */
   virtual ossim_int64 getFileSize() const = 0;
};

#endif /* #ifndef ossimFileInfoInterface_HEADER */
