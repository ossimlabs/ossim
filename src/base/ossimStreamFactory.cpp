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
   const std::string& connectionString, 
   const ossimKeywordlist& options,
   std::ios_base::openmode mode ) const
{
   std::shared_ptr<ossim::istream> result(0);
   ossimFilename f =  connectionString;
   if ( f.exists() )
   {
      // there is a bug in gcc < 5.0 and we can't use constructors in the 
      // C++11 build.  Will refactor to do a new ifstream then use open
      //
      std::shared_ptr<ossim::ifstream> testResult = 
               std::make_shared<ossim::ifstream>();
      testResult->open(connectionString.c_str(), mode);
      if(!testResult->is_open())
      {
         testResult.reset();
      }

      result = testResult;
   }
   return result;
}
      
std::shared_ptr<ossim::ostream> ossim::StreamFactory::createOstream(
   const std::string& connectionString, 
   const ossimKeywordlist& options,
   std::ios_base::openmode mode) const
{
   std::shared_ptr<ossim::ostream> result(0);

   std::shared_ptr<ossim::ofstream> testResult = 
      std::make_shared<ossim::ofstream>();
   testResult->open(connectionString.c_str(), mode);
   if ( testResult->is_open() )
   {
      result = testResult;
   }
   else
   {
      testResult.reset();
   }

   return result;
}

std::shared_ptr<ossim::iostream> ossim::StreamFactory::createIOstream(
   const std::string& /*connectionString*/, 
   const ossimKeywordlist& options,
   std::ios_base::openmode /*mode*/) const
{
   return std::shared_ptr<ossim::iostream>(0);
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
      // there is a bug in gcc < 5.0 and we can't use constructors in the 
      // C++11 build.  Will refactor to do a new ifstream then use open
      //
      result = std::make_shared<ossim::ifstream>();
      result->open(file.c_str(), openMode);
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



