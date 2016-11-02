//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License: MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
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
class ossimIStream;

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
