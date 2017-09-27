//*******************************************************************
//
// License: MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id$

#ifndef ossimStreamFactoryRegistry_HEADER
#define ossimStreamFactoryRegistry_HEADER 1

#include <ossim/base/ossimFactoryListInterface.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimStreamFactoryBase.h>
#include <ossim/base/ossimRegExp.h>

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
namespace ossim
{
  /**
  * This is a generic stream registry.  We could try to create streams for
  * http, https, s3 protocols as well as local file.  We have also exposed an
  * exists for supporting exists() calls for different stream types.
  *
  * We have added support for Buffered reads.  You can enable buffered 
  * reads to be block boundary.  If it's not block then it will use
  * a pubsetbuf for a buffered I/O
  * It will read the @see ossimPreferences for the prefix key
  * ossim.stream.factory.registry.istream.buffer[0-9]+
  * Example key list:
  * @code
  * ossim.stream.factory.registry.istream.buffer1.enabled: false
  * ossim.stream.factory.registry.istream.buffer1.includePattern: ^/
  * ossim.stream.factory.registry.istream.buffer1.enableBlocked: true
  * ossim.stream.factory.registry.istream.buffer1.size: 65536
  * @endcode
  *
  * The includePattern keyword is a regular expression.
  * Examples:
  *   - ^/  Anything that starts with /
  *   - ^s3:// Anything starting with s3://
  *
  * Local file example:
  * @code
  * ossimString connectionString = "/data/foo.ntf"
  * std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->createIstream(connectionString);
  * if(in)
  * {
  *   std::vector<char> buf(1024)
  *   in->seekg(0);
  *   in->read(&buf.front(), buf.size());
  * }
  * @endcode
  * S3 file example:
  * @code
  * ossimString connectionString = "s3://foo-bucket/path/foo.ntf"
  * std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->createIstream(connectionString);
  * if(in)
  * {
  *   std::vector<char> buf(1024)
  *   in->seekg(0);
  *   in->read(&buf.front(), buf.size());
  * }
  * @endcode
  * https or https file example:
  * @code
  * ossimString connectionString = "https://foo-host/path/foo.ntf"
  * std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->createIstream(connectionString);
  * if(in)
  * {
  *   std::vector<char> buf(1024)
  *   in->seekg(0);
  *   in->read(&buf.front(), buf.size());
  * }
  * @endcode
  */
   class OSSIM_DLL StreamFactoryRegistry : public StreamFactoryBase
   {
   public:
      static StreamFactoryRegistry* instance();
      virtual ~StreamFactoryRegistry();
   
      /**
      * Registers a stream factory.  This allows for a pluggable stream
      * Please see ossim-plugins and look at curl, aws for example
      * stream definitions.
      *
      * @param factory To be added to the registry
      */
      void registerFactory(StreamFactoryBase* factory);

      /**
      * Removes a factory from the registry.
      * @param factory To be removed from the registry
      */
      void unregisterFactory(StreamFactoryBase* factory);

      /**
      * Will try to creates an istream interface to the connectionString 
      * passed in
      * 
      * @param connectionString Is the connection string used to create
      *        a stream.  Possible examples: s3://<bucket>/path or 
      *        /data/foo.tif or https://<host>/<path>
      * @param options If a stream is found then this holds specific options 
      *                for the stream. Defaults to empty options
      * @param mode Is the mode to be used.  Defaults to a binary input stream.
      * @return A shared pointer to an istream if successful.   
      */
      virtual std::shared_ptr<ossim::istream>
         createIstream(const std::string& connectionString,
                       const ossimKeywordlist& options=ossimKeywordlist(),
                       std::ios_base::openmode mode=
                       std::ios_base::in|std::ios_base::binary) const;
      
      /**
      * Will try to creates an ostream interface to the connectionString 
      * passed in
      * 
      * @param connectionString Is the connection string used to create the
      *                         stream
      * @param options If a stream is found then this holds specific options 
      *                for the stream. Defaults to empty options
      * @param mode Is the mode to be used.  Defaults to a binary output
      *             stream.
      * @return A shared pointer to an ostream if successful.   
      */
      virtual std::shared_ptr<ossim::ostream>
         createOstream(const std::string& connectionString,
                       const ossimKeywordlist& options=ossimKeywordlist(),
                       std::ios_base::openmode mode=
                       std::ios_base::out|std::ios_base::binary) const;
      
      /**
      * Will try to creates an iostream interface to the connectionString 
      * passed in
      * 
      * @param connectionString Is the connection string used to create the
      *                         stream
      * @param options If a stream is found then this holds specific options 
      *                for the stream. Defaults to empty options
      * @param mode Is the mode to be used.  Defaults to a binary output
      *             stream.
      * @return A shared pointer to an ostream if successful.   
      */
      virtual std::shared_ptr<ossim::iostream>
         createIOstream(const std::string& connectionString,
                        const ossimKeywordlist& options=ossimKeywordlist(),
                        std::ios_base::openmode mode=
                        std::ios_base::in|std::ios_base::out|std::ios_base::binary) const;

      /**
       * @brief Methods to test if connection exists.
       * @return true on success, false, if not.  
       */
      bool exists(const std::string& connectionString) const;

      /**
       * @brief Methods to test if connection exists.
       * @param connectionString
       * @param continueFlag Initializes by this, if set to true, indicates factory
       * handles file/url and no more factory checks are necessary.
       * @return true on success, false, if not.  
       */
      virtual bool exists(const std::string& connectionString,
                          bool& continueFlag) const;
   
      /**
      * This will load the buffer information from the 
      * preferences.
      */
      void loadPreferences();

   protected:
      StreamFactoryRegistry();
      
   private:

      /**
      * This is an internal class used to store the buffer information
      * loaded from the preferences.
      * @see StreamFactoryRegistry
      */
      class BufferInfo{
      public:
        BufferInfo():m_enabled(false),
                     m_enableBlocked(false),
                     m_pattern(""),
                     m_size(4096){}
        bool                 m_enabled;
        bool                 m_enableBlocked;
        ossimString          m_pattern;
        ossim_uint64         m_size;

      };     
      /** @brief copy constructor hidden from use */
      StreamFactoryRegistry(const StreamFactoryRegistry&);

      /**
      * @param bufferInfo Holds the result of the first buffer info 
      *        matching the connection string
      * @param connecitonString The connection string
      */
      bool getBufferInfo(BufferInfo& bufferInfo, 
                      const ossimString& connectionString)const;
      
      std::vector<StreamFactoryBase*> m_factoryList;
      std::vector<BufferInfo>  m_bufferInfoList;
      static StreamFactoryRegistry*   m_instance;
      mutable ossimRegExp m_patternMatcher;
      mutable std::mutex m_mutex;
   };
   
} // End: namespace ossim

#endif
