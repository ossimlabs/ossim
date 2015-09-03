//----------------------------------------------------------------------------
// File: ossimStreamReaderInterface.h
// 
// License:  LGPL
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

#include <iosfwd>

/** @class ossimStreamReaderInterface */
class ossimStreamReaderInterface
{
public:
   
   /** @brief default constructor */
   ossimStreamReaderInterface(){}

   /** @brief virtual destructor. */
   virtual ~ossimStreamReaderInterface(){}

   /**
    *  @brief Pure virtual open method.
    *
    *  This open takes a stream, postition and a flag.
    *
    *  @param str Open stream to image.
    *
    *  @param restartPosition Typically 0, this is the stream offset to the
    *  front of the image.
    *
    *  @param youOwnIt If true this object takes owner ship of the pointer
    *  memory and will destroy on close.
    *  
    *  @return true on success, false on error.
    */
   virtual bool open( std::istream* str,
                      std::streamoff restartPosition,
                      bool youOwnIt ) = 0;
};

#endif /* #ifndef ossimStreamReaderInterface_HEADER */
