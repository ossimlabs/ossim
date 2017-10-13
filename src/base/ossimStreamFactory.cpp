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

#if defined(_WIN32)
#  include <io.h>     /* _access(...) */
#else
#  include <unistd.h> /* access(...) */
#endif


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
   const ossimKeywordlist& /* options */,
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
   const ossimKeywordlist& /* options */,
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
   const ossimKeywordlist& /* options */,
   std::ios_base::openmode /*mode*/) const
{
   return std::shared_ptr<ossim::iostream>(0);
}

bool ossim::StreamFactory::exists(const std::string& connectionString, bool& continueFlag) const
{
   bool result = false;
   if ( connectionString.size() )
   {
      std::string file;
      std::size_t pos = connectionString.find( "://" );
      if ( pos != std::string::npos )
      {
         // is url:
         ossimString protocol = connectionString.substr( 0, pos );
         if ( (protocol.downcase() == "file") && ( connectionString.size() > pos+3) )
         {
            // Strip off "file://" for access(...) function:
            file = connectionString.substr( pos+3 );
         }
      }
      else // not a url
      {
         file = connectionString;
      }

      if ( file.size() )
      {
         // Set continueFlag to false to stop downstream factory exists checks.
         continueFlag = false;
#if defined(_WIN32)
         result = (_access(file.c_str(), ossimFilename::OSSIM_EXIST) == 0);
#else
         result = ((access(file.c_str(), ossimFilename::OSSIM_EXIST)) == 0);
#endif
      }
   }
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
