#include <ossim/base/Block.h>

ossim::Block::Block(bool releaseFlag)
:m_release(releaseFlag)
{

}

ossim::Block::~Block()
{
   release();
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
      m_conditionVariable.wait(lock, [this]{
         return (m_release.load() == true);
      });
   }
   m_conditionVariable.notify_all();   
}

void ossim::Block::block(ossim_uint64 waitTimeMillis)
{
   std::unique_lock<std::mutex> lock(m_mutex);
   if(!m_release)
   {
      m_conditionVariable.wait_for(lock, 
                                   std::chrono::milliseconds(waitTimeMillis),
                                   [this]{
         return (m_release.load() == true);
      });
   }
   m_conditionVariable.notify_all();   
}

void ossim::Block::release()
{
   {   
      std::unique_lock<std::mutex> lock(m_mutex);
      if(!m_release)
      {
         m_release = true;
      }
   }
   m_conditionVariable.notify_all();
}

void ossim::Block::reset()
{
   std::unique_lock<std::mutex> lock(m_mutex);

   m_release = false;
}
