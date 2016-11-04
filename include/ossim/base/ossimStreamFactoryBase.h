//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License: MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id$

#ifndef ossimStreamFactoryBase_HEADER
#define ossimStreamFactoryBase_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimIoStream.h>
#include <iosfwd>
#include <memory>

class ossimFilename;
class ossimString;

namespace ossim
{
   class OSSIM_DLL StreamFactoryBase
   {
   public:
      virtual ~StreamFactoryBase(){}
      
      virtual std::shared_ptr<ossim::istream>
         createIstream(const ossimString& connectionString,
                       std::ios_base::openmode mode) const=0;

      virtual std::shared_ptr<ossim::ostream>
         createOstream(const ossimString& connectionString,
                       std::ios_base::openmode mode) const=0;

      virtual std::shared_ptr<ossim::iostream>
         createIOstream(const ossimString& connectionString,
                        std::ios_base::openmode mode) const=0;
   };
}

class OSSIM_DLL ossimStreamFactoryBase
{
public:
   virtual ~ossimStreamFactoryBase(){}

   virtual std::shared_ptr<ossim::ifstream>
      createIFStream(const ossimFilename& file,
                     std::ios_base::openmode openMode) const=0;
   
   virtual ossimRefPtr<ossimIFStream> createNewIFStream(
      const ossimFilename& file,
      std::ios_base::openmode openMode)const=0;
};

#endif
