#ifndef ossimBlockStreamBuffer_HEADER
#define ossimBlockStreamBuffer_HEADER 1
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimConstants.h>
#include <iostream>
#include <vector>

namespace ossim{
   /**
   * @brief This is a utility class used by the BlockStreamBuffer.
   * This class will hold information regarding the block that
   * is currently loaded.  It holds the start byte and the valid 
   * size and the block size of the buffer.  The valid size 
   * is used because if we are at the end of a stream we could 
   * have a partial block.
   *
   */
   class OSSIM_DLL BlockBufInfo{
   public:
      BlockBufInfo():
      m_startByte(0),
      m_blockSize(0),
      m_blockBufferPtr(0),
      m_validSize(0),
      m_blockLoaded(false){

      }

      /**
      * Returns the index of a block
      *
      * @param pos Is the aboslute byte position
      * @return The block index for the abosolute 
      *         position
      */
      ossim_int64 getBlockIndex(ossim_int64 pos){
         ossim_int64 result = -1;
         if(m_blockSize&&m_blockBufferPtr&&(pos>=0))
         {
            result = pos/m_blockSize;
         }

         return result;
      }

      /**
      * This just tests if the given abolute position
      * is within a block window.  We will usually call
      * @see isWithinValidWindow.
      * @param pos Absolute position
      * @return true if we are inside the defined window
      *         and false otherwise.
      */
      bool isWithinWindow(ossim_int64 pos)const{
         return  (m_blockLoaded&&
                  (pos>=m_startByte)&&
                  (pos<(m_startByte+m_blockSize)));
      }

      /**
      * This just tests if the given abolute position
      * is within a valid window.  A valid window represents
      * partial blocks.
      * @param pos Absolute position
      * @return true if we are inside the defined window
      *         and false otherwise.
      */
      bool isWithinValidWindow(ossim_int64 pos)const{
         return  (m_blockLoaded&&
                  (pos>=m_startByte)&&
                  (pos<getEndByte()));
      }

      /**
      * Convenient method to get the absolute byte position 
      * of the end byte.
      *
      * @return the absolute byte position of the end byte
      */
      ossim_int64 getEndByte()const{
         if(m_validSize>=0) return m_startByte+(m_validSize);
         return m_startByte;
      }

      /**
      * Get the starting address in the buffer of the absolute position.
      * @param absolutePosition The absolute byte position
      * @return The starting address at the aboslute position or 0 if
      *         the absolute position is outside the windows
      */
      const char* getBufferStart(ossim_int64 absolutePosition)const{
         if(isWithinValidWindow(absolutePosition))
         {
            return m_blockBufferPtr+(absolutePosition-m_startByte);
         }

         return 0;
      }

      /**
      * @return The starting address of the block buffer
      */
      const char* getBuffer()const{
         return m_blockBufferPtr;
      }

      /**
      * @return The starting address of the block buffer
      */
      char* getBuffer(){
         return m_blockBufferPtr;
      }

      /**
      * @return true if the the block is valid and loaded or false
      *          otherwise.
      */
      bool isLoaded()const{return m_blockLoaded;}

      /**
      *  Will set the buffer and then reset the loaded flag to be false.
      *
      * @param bufPtr is the starting address of the block buffer
      * @param blockSize is the size of the buffer
      *
      */
      void setBuffer(char* bufPtr, ossim_uint32 blockSize)
      {
         m_blockBufferPtr = bufPtr;
         m_blockSize      = blockSize;
         m_blockLoaded    = false;
      }

      /**
      * Is the starting absolute byte offset for the buffer
      */
      ossim_int64 m_startByte;

      /**
      * Is the size of the buffer
      */
      ossim_int64 m_blockSize;

      /**
      * is the valid size of the buffer.  In most cases this is equal to the
      * blockSize but if at the end of a stream you could have a partial 
      * buffer.
      */
      ossim_int64 m_validSize;

      /**
      * Starting address of the block.  This is not managed by this class and
      * will not be deleted.
      */
      char* m_blockBufferPtr;

      /**
      * Variable used to invalidate a block or specify whether the block is loaded
      */
      bool m_blockLoaded;
   };

   /**
   * This is the BlockStreamBuffer class and derives from 
   * stream buf.  This class 
   *
   */
   class OSSIM_DLL BlockStreamBuffer : public std::streambuf{
   public:
      BlockStreamBuffer(ossim::istream* adaptStream=0, ossim_uint64 blockSize=0);
      virtual ~BlockStreamBuffer(){
         m_adaptStream=0;
      }
   protected:
      /**
      * The block buffer that we set the buf pointers to
      */
      std::vector<char> m_blockBuffer;

      /**
      * holds the current absolute byte position
      */
      ossim_int64 m_currentPosValue;

      /**
      * Holds the information about the block.  It tells us
      * if the block is currently loaded and what the valid size 
      * is
      */
      BlockBufInfo m_blockInfo;

      /**
      * The stream we are adapting
      */
      ossim::istream* m_adaptStream;

      /**
      * This is a virtual method that can be overriden.
      * pubsetbuf can be called and calls this protected method 
      * to set a buffer.
      *  
      * @param s The starting address of a byte buffer
      * @param n The size of the buffer
      */
   virtual std::streambuf* setbuf (char* s, std::streamsize n);

      /**
      *
      * setgPtrs calls setg and sets the eback egptr and gptr. to the
      * managed buffer's valid window
      * 
      */
      void setgPtrs();

      /**
      * loadBlock will load data into the current block and call the setgPtrs
      * to adjust the internal pointers tha the base streambuf may use
      *
      */
      void loadBlock();

      /**
      * this is a protected method overriden from streambuf base.
      *
      * we will convert the offset byte to an absolute if we can and 
      * then call the seek pos for the absolute seek
      */
      virtual pos_type seekoff(off_type offset, std::ios_base::seekdir dir,
                               std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

      /**
      *  seekpos is overriden from the base streambuf class.  This is the
      * seek of the aboslute position.  We will check to see if the new
      * position resides in the block and if so we just update our pointers 
      * and return the pos.  If bnot then we use the adapted stream call
      * to seek to the position and update our internal pointers.
      *
      * @param pos The absolute position to seek to
      * @mode the mode used.  Should be in
      * @return The absolute position if successful or EOF if not.
      */
      virtual pos_type seekpos(pos_type pos,
                               std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

      /**
      * xsgetn is a protected virtual method that we override from the base
      * streambuf.  The method will load blocks of data to service the requested
      * buffer to fill.  If the request covers several block it will iterate
      * and load each block of data until the request is satisfied.
      *
      * @param s The destination buffer large enough to hold the characters
      *          being requested
      * @param n The number of characters to request from the input stream
      *
      * @return The number of bytes read or -1 if failed
      */
      virtual std::streamsize xsgetn(char_type* s, std::streamsize n);
      
      /**
      * underflow is overriden from the base streambuf.  It check to see
      * if the current block is loaded and if not load the block.  The method
      * returns what is currently pointed to by the absolute offset or basically
      * return *gptr().
      *
      * @return the current byte we are pointing to.
      */
      virtual int underflow();

      /**
      * syncCurrentPosition is a utility method that we call internally in 
      * the overriden protected methods that will sync the location of the gptr
      * to the absolute byte offset variable we are using internally.  The problem
      * is, when things like ignore(...) peek(...) and other options are used
      * on an input stream the base might adjust the gptr location.  When this 
      * is adjusted outside our control the offsets might get out of sync
      * this is called to ensure this does not happen
      * 
      */
      void syncCurrentPosition();
   };
}
#endif
