#include <ossim/base/Block.h>
#include <iostream>

ossim::Block::Block(bool releaseFlag)
:m_release(releaseFlag),
m_waitCount(0)
{

}

ossim::Block::~Block()
{
   release();
   {
      std::unique_lock<std::mutex> lock(m_mutex);
      if(m_waitCount>0)
      {
         m_conditionalWait.wait(lock, [this]{return m_waitCount.load()<1;});
      }
   }
}

void ossim::Block::set(bool releaseFlag)
{
   {
      std::unique_lock<std::mutex> lock(m_mutex);

      m_release = releaseFlag;      
   }
   m_conditionVariable.notify_all();
}

void ossim::Block::block()
{
   std::unique_lock<std::mutex> lock(m_mutex);
   if(!m_release)
   {
      ++m_waitCount;
      m_conditionVariable.wait(lock, [this]{
         return (m_release.load() == true);
      });
      --m_waitCount;
      if(m_waitCount < 0) m_waitCount = 0;
   }
   m_conditionVariable.notify_all();   
   m_conditionalWait.notify_all();
}

void ossim::Block::block(ossim_uint64 waitTimeMillis)
{
   std::unique_lock<std::mutex> lock(m_mutex);
   if(!m_release)
   {
      ++m_waitCount;
      m_conditionVariable.wait_for(lock, 
                                   std::chrono::milliseconds(waitTimeMillis),
                                   [this]{
         return (m_release.load() == true);
      });
      --m_waitCount;
      if(m_waitCount < 0) m_waitCount = 0;
   }
   m_conditionVariable.notify_all();   
   m_conditionalWait.notify_all();
}

void ossim::Block::release()
{
   {   
      std::unique_lock<std::mutex> lock(m_mutex);
      if(!m_release)
      {
         m_release = true;
      }
      m_conditionVariable.notify_all();
   }
}

void ossim::Block::releaseOne()
{
   {   
      std::unique_lock<std::mutex> lock(m_mutex);
      if(!m_release)
      {
         m_release = true;
      }
      m_conditionVariable.notify_one();
   }
}

void ossim::Block::reset()
{
   std::unique_lock<std::mutex> lock(m_mutex);

   m_release   = false;
   m_waitCount = 0;
}
