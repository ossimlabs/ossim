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

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimStreamFactory.h>

#if OSSIM_HAS_LIBZ
#include <ossim/base/ossimGzStream.h>
#endif

#include <fstream>

static bool trace = true; // tmp drb...

ossim::StreamFactory* ossim::StreamFactory::m_instance = 0;

ossim::StreamFactory::~StreamFactory()
{
}

ossim::StreamFactory* ossim::StreamFactory::instance()
{
   if(!m_instance)
   {
      m_instance = new ossim::StreamFactory();
   }

   return m_instance;
}

std::shared_ptr<ossim::istream> ossim::StreamFactory::createIstream(
   const ossimString& connectionString, std::ios_base::openmode mode ) const
{
   // tmp drb
   if ( trace ) std::cout << "ossim::StreamFactory::createIstream entered..." << std::endl;

   std::shared_ptr<ossim::istream> result(0);

   ossimFilename f =  connectionString;
   if ( f.exists() )
   {
      result = std::make_shared<ossim::ifstream>(
         ossim::ifstream( connectionString.c_str(), mode ) );
   }

   // tmp drb
   if ( trace )std::cout << "ossim::StreamFactory::createIstream exited..." << std::endl;  

   return result;
}
      
std::shared_ptr<ossim::ostream> ossim::StreamFactory::createOstream(
   const ossimString& /*connectionString*/, std::ios_base::openmode /*mode*/) const
{
   std::shared_ptr<ossim::ostream> result(0);
   return result;
}

std::shared_ptr<ossim::iostream> ossim::StreamFactory::createIOstream(
   const ossimString& /*connectionString*/, std::ios_base::openmode /*mode*/) const
{
   std::shared_ptr<ossim::iostream> result(0);
   return result;
}

// Hidden from use:
ossim::StreamFactory::StreamFactory()
{
}

// Hidden from use:
ossim::StreamFactory::StreamFactory(const ossim::StreamFactory& )
{
}





// Deprecated code...
ossimStreamFactory* ossimStreamFactory::theInstance = 0;

ossimStreamFactory::ossimStreamFactory()
   : ossimStreamFactoryBase()
{
}

ossimStreamFactory::~ossimStreamFactory()
{
}

ossimStreamFactory* ossimStreamFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new ossimStreamFactory();
   }

   return theInstance;
}

std::shared_ptr<ossim::ifstream> ossimStreamFactory::createIFStream(
   const ossimFilename& file, std::ios_base::openmode openMode) const
{
   std::shared_ptr<ossim::ifstream> result(0);

   if ( file.exists() )
   {
      result = std::make_shared<ossim::ifstream>(ossim::ifstream(file.c_str(), openMode));
      if ( result->is_open() == false )
      {
         result.reset();
      }
   }
   return result;
}

ossimRefPtr<ossimIFStream> ossimStreamFactory::createNewIFStream(
   const ossimFilename& file,
   std::ios_base::openmode mode) const
{
   ossimRefPtr<ossimIFStream> result = 0;
   
#if OSSIM_HAS_LIBZ
   ossimFilename copyFile = file;

   if(!copyFile.exists())
   {
      ossimString ext = copyFile.ext();
      copyFile.setExtension("gz");
      if(!copyFile.exists())
      {
         copyFile.setExtension(ext);
         copyFile += ".gz";

         if(!copyFile.exists())
         {
            return result;
         }
      }
   }
   
   std::ifstream in(copyFile.c_str(), std::ios::in|std::ios::binary);

   if(!in) return result;

   unsigned char buf[2];

   in.read((char*)buf, 2);
   in.close();
   // check for gzip magic number
   //
   if((buf[0] == 0x1F) &&
      (buf[1] == 0x8B))
   {
      result = new ossimIgzStream(copyFile.c_str(), mode);
   }
#endif
   return result;
}

ossimStreamFactory::ossimStreamFactory(const ossimStreamFactory&)
   : ossimStreamFactoryBase()
{}



