#ifndef ossimBlockStream_HEADER
#define ossimBlockStream_HEADER 1
#include <ossim/base/BlockStreamBuffer.h>

namespace ossim {
   /**
   *
   * @brief Allows one to adapt any input stream to be block aligned
   * for any read it will internally read overlapping blocks filling
   * the request.
   *
   * Example:
   * @code
   *
   * ossim_uint64 blockSize = 4096
   * std::shared_ptr<ossim::BlockReader> reader = std::make_shared<ossim::BlockReader>(streamToAdapt, blockSize)
   *
   * reader->seekg(10);
   * reader->read(buf, 100);
   * @endcode
   *
   * This will read a block of data from 0-4095 and then fill the buffer
   * with 100 bytes of data.
   * 
   * if another call to: reader->read(buf, 10)
   * it will not reload the block but instead read from memory
   *
   */
   class BlockIStream : public ossim::istream
   {
   public:
      /**
      * Constructor must be initialized with an inputstream.
      *
      * @param adaptStream Currenlty a required parameter and is initialized on construction.
                           Takes an input std::istream to force block aligned
      *                    requests
      * @param blockSize Specify the block size to use 
      */
      BlockIStream(std::shared_ptr<ossim::istream> adaptStream, 
                  ossim_uint64 blockSize=4096):
      ossim::istream(&m_blockStreamBuffer),
      m_adaptStream(adaptStream),
      m_blockStreamBuffer(adaptStream.get(), blockSize)
      {
      }
      
      /**
      * @brief Destructor will set any shared pointer to 0
      */
      virtual ~BlockIStream(){
         m_adaptStream = 0;
      }

      /**
      * Maintain a shared pointer to the stream we are adapting
      * to be block aligned.  
      */
      std::shared_ptr<ossim::istream> m_adaptStream;

      /**
      *
      * The buffer where all the block align implementation resides
      *
      */
      BlockStreamBuffer m_blockStreamBuffer;
   };
}

#endif
