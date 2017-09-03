#include <ossim/base/ossimBlockStreamBuffer.h>
#include <ossim/base/ossimTrace.h>
#include <cstring> /* for memcpy */

static ossimTrace traceDebug("BlockStreamBuffer:debug");

ossim::BlockStreamBuffer::BlockStreamBuffer(ossim::istream* adaptStream, ossim_uint64 blockSize)
:  m_adaptStream(adaptStream),
   m_currentPosValue(0)
{
   m_blockInfo.m_blockSize = blockSize;
   if(m_blockInfo.m_blockSize)
   {
      m_blockBuffer.resize(m_blockInfo.m_blockSize);
      m_blockInfo.m_blockBufferPtr = &m_blockBuffer.front();
   }
   setg(0, 0, 0);
}

std::streambuf* ossim::BlockStreamBuffer::setbuf (char* s, 
                                             std::streamsize n)
{
   if(n>0)
   {
      m_blockBuffer.clear();
      m_blockInfo.setBuffer(s,n);
      setg(0, 0, 0);
   }
   return this;
}

void ossim::BlockStreamBuffer::setgPtrs()
{
   if(m_blockInfo.isLoaded())
   {
      setg(m_blockInfo.getBuffer(),
           m_blockInfo.getBuffer() + (m_currentPosValue - m_blockInfo.m_startByte),
           m_blockInfo.getBuffer() + m_blockInfo.m_validSize);
   }
   else
   {
      setg(0,0,0);
   }

}

void ossim::BlockStreamBuffer::loadBlock()
{
   m_blockInfo.m_blockLoaded = false;
   m_blockInfo.m_validSize = 0;
   if(m_adaptStream)
   {
      if(m_currentPosValue < 0) m_currentPosValue = 0;
      ossim_int64 blockIndex = m_blockInfo.getBlockIndex(m_currentPosValue);
      m_blockInfo.m_startByte = blockIndex*m_blockInfo.m_blockSize;
      if(!m_adaptStream->good()) m_adaptStream->clear();
      if(m_blockInfo.m_startByte != m_adaptStream->tellg())
      {
         m_adaptStream->seekg(m_blockInfo.m_startByte);
      }
      m_adaptStream->read(m_blockInfo.m_blockBufferPtr, 
                          m_blockInfo.m_blockSize);
      ossim_int64 bytesRead = m_adaptStream->gcount();
      if(!m_adaptStream->bad()&&(bytesRead>0))
      {
         m_blockInfo.m_blockLoaded = true;
         m_blockInfo.m_validSize = bytesRead;
         if((m_blockInfo.m_validSize<=0)||(!m_blockInfo.isWithinValidWindow(m_currentPosValue)))
         {
            m_blockInfo.m_blockLoaded = false;
         }
      }
   }
   setgPtrs();
}

std::streambuf::pos_type ossim::BlockStreamBuffer::seekoff(off_type offset, 
                         std::ios_base::seekdir dir,
                         std::ios_base::openmode mode)
{
   // make sure we are in synch with current pos
   // gptr can be updated by other means
   syncCurrentPosition();
   pos_type result = pos_type(off_type(-1));
   if(m_adaptStream)
   {
      ossim_int64 pos = m_currentPosValue;
      switch(dir)
      {
         case std::ios_base::beg:
         {
            pos = offset;
            result = seekpos(pos, std::ios_base::in);
            break;
         }
         case std::ios_base::cur:
         {
            pos += offset;
            result = seekpos(pos, std::ios_base::in);
           break;
         }
         case std::ios_base::end:
         {
            // at this point in time of implementation
            // we do not know the offset to the end of the stream
            // we are adapting
            //
            // we have no choice but to call the adapted stream
            // implementation
            result = m_adaptStream->rdbuf()->pubseekoff(pos, dir);
            m_currentPosValue = result;
            setgPtrs();

            break;
         }
         default:
         {
            break;
         }
      }
   }
   return result;
}

std::streambuf::pos_type ossim::BlockStreamBuffer::seekpos(pos_type pos,
                         std::ios_base::openmode mode)
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "BlockStreamBuffer::seekpos DEBUG: entered with absolute position: " << pos << "\n";

   }
   pos_type result = pos_type(off_type(-1));

   if(m_adaptStream)
   {
      if(pos == m_currentPosValue)
      {
         result = pos;
      }
      else if(m_blockInfo.isLoaded()&&m_blockInfo.isWithinValidWindow(pos))
      {
         result = pos;
         m_currentPosValue = result;
      }
      else
      {
         result = m_adaptStream->rdbuf()->pubseekpos(pos, mode);
         m_currentPosValue = result;
      }
      setgPtrs();
   }

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "BlockStreamBuffer::seekpos DEBUG: leaving\n";

   }
   return result;

}

std::streamsize ossim::BlockStreamBuffer::xsgetn(char_type* s, 
                                            std::streamsize n)
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "BlockStreamBuffer::xsgetn DEBUG: entered ......"<<n<<"\n";

   }
   std::streamsize result = EOF;

   if(m_adaptStream)
   {
      ossim_int64 bytesNeedToRead = n;
      ossim_int64 bytesRead = 0;
      syncCurrentPosition();

      while(bytesNeedToRead>0)
      {
         if(!m_blockInfo.isWithinValidWindow(m_currentPosValue))
         {
            loadBlock();
         }

         if(m_blockInfo.isLoaded())
         {
            ossim_int64 delta = (m_blockInfo.getEndByte()-
                                 m_currentPosValue);
            if(delta <= bytesNeedToRead)
            {
               std::memcpy(s+bytesRead,
                           m_blockInfo.getBufferStart(m_currentPosValue),
                           delta);
               bytesRead+=delta;
               bytesNeedToRead-=delta;
               m_currentPosValue+=delta;
            }
            else
            {
               std::memcpy(s+bytesRead,
                           m_blockInfo.getBufferStart(m_currentPosValue),
                           bytesNeedToRead);
               m_currentPosValue+=bytesNeedToRead;
               bytesRead+=bytesNeedToRead;
               bytesNeedToRead=0;
            }
         }
         else
         {
            bytesNeedToRead=0;
         }
      }
      // specify the current absolute position after read
      // so we are in sync.
      setgPtrs();
      result = bytesRead;
   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "BlockStreamBuffer::xsgetn DEBUG: leaving ......\n";

   }
   return result;
}

int ossim::BlockStreamBuffer::underflow()
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "BlockStreamBuffer::underflow DEBUG: entered ......\n";

   }
   if(!m_adaptStream) return EOF;
   else{
      syncCurrentPosition();
      if(!m_blockInfo.isWithinValidWindow(m_currentPosValue))
      {
         loadBlock();
      }

      if(!m_blockInfo.isLoaded())
      {
         return EOF;
      }
   } 
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "BlockStreamBuffer::underflow DEBUG: leaving ......\n";

   }
   return (int)static_cast<ossim_uint8>(*gptr());
}

void ossim::BlockStreamBuffer::syncCurrentPosition()
{
   if(m_blockInfo.isLoaded()&&gptr())
   {
      m_currentPosValue = (m_blockInfo.m_startByte+(gptr()-eback()));
   } 
}
