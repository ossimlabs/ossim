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

namespace ossim
{
   class OSSIM_DLL StreamFactory : public StreamFactoryBase
   {
   public:
      static StreamFactory* instance();
      
      virtual ~StreamFactory();

      virtual std::shared_ptr<ossim::istream>
         createIstream(const std::string& connectionString,
                       const ossimKeywordlist& options=ossimKeywordlist(),
                       std::ios_base::openmode mode=
                       std::ios_base::in|std::ios_base::binary) const;
      
      virtual std::shared_ptr<ossim::ostream>
         createOstream(const std::string& connectionString,
                       const ossimKeywordlist& options=ossimKeywordlist(),
                       std::ios_base::openmode mode=
                       std::ios_base::out|std::ios_base::binary) const;
      
      virtual std::shared_ptr<ossim::iostream>
         createIOstream(const std::string& connectionString,
                        const ossimKeywordlist& options=ossimKeywordlist(),
                        std::ios_base::openmode mode=
                        std::ios_base::in|std::ios_base::out|std::ios_base::binary) const;

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
                          bool& continueFlag) const;

   protected:
      StreamFactory();
      StreamFactory(const StreamFactory&);

      static StreamFactory* m_instance;
   };
}
#endif
