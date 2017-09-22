#include <ossim/base/Barrier.h>

ossim::Barrier::Barrier(ossim_int32 maxCount)
: m_maxCount(maxCount),
  m_blockedCount(0),
  m_waitCount(0)
{

}

ossim::Barrier::~Barrier()
{
   reset();
}

void ossim::Barrier::block()
{
   std::unique_lock<std::mutex> lock(m_mutex);
   ++m_blockedCount;
   if(m_blockedCount < m_maxCount)
   {
      ++m_waitCount;
      m_conditionalBlock.wait(lock, [this]{return m_blockedCount>=m_maxCount;} );
      --m_waitCount;         
   }
   else
   {
      m_conditionalBlock.notify_all();
   }
   // always notify the conditional wait just in case anyone is waiting
   m_conditionalWait.notify_all();
}

void ossim::Barrier::reset()
{
   std::unique_lock<std::mutex> lock(m_mutex);
   // force the condition on any waiting threads
   m_blockedCount = m_maxCount;
   if(m_waitCount.load() >= 1){
      m_conditionalBlock.notify_all(); 
      // wait until the wait count goes back to zero
      m_conditionalWait.wait(lock, [this]{return m_waitCount.load()<1;});
   }
   // should be safe to reset everything at this point 
   m_blockedCount = 0;
   m_waitCount    = 0;
}

void ossim::Barrier::reset(ossim_int32 maxCount)
{
   // all threads should be released after this call
   reset();
   {
      // now safe to update the new max count
      std::unique_lock<std::mutex> lock(m_mutex);
      m_maxCount = maxCount;
   }
}

ossim_int32 ossim::Barrier::getMaxCount()const
{ 
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_maxCount; 
}

ossim_int32 ossim::Barrier::getBlockedCount()const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_blockedCount; 
}


