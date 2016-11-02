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

#ifndef ossimStreamFactory_HEADER
#define ossimStreamFactory_HEADER 1

#include <ossim/base/ossimStreamFactoryBase.h>
#include <ossim/base/ossimIoStream.h>

class OSSIM_DLL ossimStreamFactory : public ossimStreamFactoryBase
{
public:
   static ossimStreamFactory* instance();
   virtual ~ossimStreamFactory();

   virtual std::shared_ptr<ossim::ifstream>
      createIFStream(const ossimFilename& file,
                     std::ios_base::openmode openMode) const;
   
   virtual ossimRefPtr<ossimIFStream>
      createNewIFStream(const ossimFilename& file,
                        std::ios_base::openmode openMode) const;
   
protected:
   ossimStreamFactory();
   ossimStreamFactory(const ossimStreamFactory&);
   static ossimStreamFactory* theInstance;
   
};

#endif
