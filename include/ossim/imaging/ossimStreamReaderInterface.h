//----------------------------------------------------------------------------
// File: ossimStreamReaderInterface.h
// 
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class interface ossimMetadataInterface.
//
// Has pure virtual "open" method that derived classes must
// implement to be concrete.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimStreamReaderInterface_HEADER
#define ossimStreamReaderInterface_HEADER 1

#include <ossim/base/ossimIosFwd.h>
#include <memory>
#include <string>

/** @class ossimStreamReaderInterface */
class ossimStreamReaderInterface
{
public:
   
   /** @brief default constructor */
   ossimStreamReaderInterface(){}

   /** @brief virtual destructor. */
   virtual ~ossimStreamReaderInterface(){}

   /**
    *  @brief Pure virtual open method that takes a stream.
    *  @param str Open stream to image.
    *  @param connectionString
    *  @return true on success, false on error.
    */
   virtual bool open( std::shared_ptr<ossim::istream>& str,
                      const std::string& connectionString ) = 0;
   
#if 0
   virtual bool open( std::istream* str,
                      std::streamoff restartPosition,
                      bool youOwnIt ) = 0;
#endif
};

#endif /* #ifndef ossimStreamReaderInterface_HEADER */
