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
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <iosfwd>
#include <memory>
#include <string>

class ossimFilename;

namespace ossim
{
   class OSSIM_DLL StreamFactoryBase
   {
   public:
      virtual ~StreamFactoryBase(){}
      
      virtual std::shared_ptr<ossim::istream>
         createIstream(const std::string& connectionString,
                       const ossimKeywordlist& options,
                       std::ios_base::openmode mode) const=0;

      virtual std::shared_ptr<ossim::ostream>
         createOstream(const std::string& connectionString,
                       const ossimKeywordlist& options,
                       std::ios_base::openmode mode) const=0;

      virtual std::shared_ptr<ossim::iostream>
         createIOstream(const std::string& connectionString,
                        const ossimKeywordlist& options,
                        std::ios_base::openmode mode) const=0;

      /**
       * @brief Methods to test if connection exists.
       *
       * @param connectionString
       * 
       * @param continueFlag Initializes by this, if set to false, indicates factory
       * handles file/url and no more factory checks are necessary.  If true,
       * connection is not handled by this factory.
       * 
       * @return true on success, false, if not.  
       */
      virtual bool exists(const std::string& connectionString,
                          bool& continueFlag) const = 0;
   };
}

#endif
