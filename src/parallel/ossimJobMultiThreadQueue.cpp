#include <ossim/parallel/ossimJobMultiThreadQueue.h>

ossimJobMultiThreadQueue::ossimJobMultiThreadQueue(std::shared_ptr<ossimJobQueue> q, 
                                                   ossim_uint32 nThreads)
:m_jobQueue(q?q:std::make_shared<ossimJobQueue>())
{
   setNumberOfThreads(nThreads);
}

ossimJobMultiThreadQueue::~ossimJobMultiThreadQueue()
{
   cancel();
   waitForCompletion();
}

std::shared_ptr<ossimJobQueue> ossimJobMultiThreadQueue::getJobQueue()
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_jobQueue;
}
const std::shared_ptr<ossimJobQueue> ossimJobMultiThreadQueue::getJobQueue()const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_jobQueue;
}

void ossimJobMultiThreadQueue::setJobQueue(std::shared_ptr<ossimJobQueue> q)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   m_jobQueue = q;
   for(idx = 0; idx < m_threadQueueList.size(); ++idx)
   {
      m_threadQueueList[idx]->setJobQueue(m_jobQueue);
   }
}

void ossimJobMultiThreadQueue::setNumberOfThreads(ossim_uint32 nThreads)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   ossim_uint32 queueSize = m_threadQueueList.size();
   
   if(nThreads > queueSize)
   {
      for(idx = queueSize; idx < nThreads;++idx)
      {
         std::shared_ptr<ossimJobThreadQueue> threadQueue = std::make_shared<ossimJobThreadQueue>();
         threadQueue->setJobQueue(m_jobQueue);
         m_threadQueueList.push_back(threadQueue);
      }
   }
   else if(nThreads < queueSize)
   {
      ThreadQueueList::iterator iter = m_threadQueueList.begin()+nThreads;
      while(iter != m_threadQueueList.end())
      {
         (*iter)->cancel();
         iter = m_threadQueueList.erase(iter);
      }
   }
}

ossim_uint32 ossimJobMultiThreadQueue::getNumberOfThreads() const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return static_cast<ossim_uint32>( m_threadQueueList.size() );
}

ossim_uint32 ossimJobMultiThreadQueue::numberOfBusyThreads()const
{
   ossim_uint32 result = 0;
   std::lock_guard<std::mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   ossim_uint32 queueSize = m_threadQueueList.size();
   for(idx = 0; idx < queueSize;++idx)
   {
      if(m_threadQueueList[idx]->isProcessingJob()) ++result;
   }
   return result;
}

bool ossimJobMultiThreadQueue::areAllThreadsBusy()const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   ossim_uint32 queueSize = m_threadQueueList.size();
   for(idx = 0; idx < queueSize;++idx)
   {
      if(!m_threadQueueList[idx]->isProcessingJob()) return false;
   }
   
   return true;
}

bool ossimJobMultiThreadQueue::hasJobsToProcess()const
{
   bool result = false;
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      ossim_uint32 queueSize = m_threadQueueList.size();
      ossim_uint32 idx = 0;
      for(idx = 0; ((idx<queueSize)&&!result);++idx)
      {
         result = m_threadQueueList[idx]->hasJobsToProcess();
      }
   }
   
   return result;
}

void ossimJobMultiThreadQueue::cancel()
{
   for(auto thread:m_threadQueueList)
   {
      thread->cancel();
   }
}

void ossimJobMultiThreadQueue::waitForCompletion()
{
   for(auto thread:m_threadQueueList)
   {
      thread->waitForCompletion();
   }
}

void ossimJobMultiThreadQueue::start()
{
   for(auto thread:m_threadQueueList)
   {
      thread->start();
   }
}

