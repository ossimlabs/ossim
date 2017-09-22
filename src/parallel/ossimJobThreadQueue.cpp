#include <ossim/parallel/ossimJobThreadQueue.h>
#include <cstddef> // for std::nullptr
ossimJobThreadQueue::ossimJobThreadQueue(std::shared_ptr<ossimJobQueue> jqueue)
:m_doneFlag(false)
{
   setJobQueue(jqueue);    
}
void ossimJobThreadQueue::setJobQueue(std::shared_ptr<ossimJobQueue> jqueue)
{
   std::lock_guard<std::mutex> lock(m_threadMutex);
   
   if (m_jobQueue == jqueue) return;
   
   if(isRunning())
   {
      std::shared_ptr<ossimJobQueue> jobQueueTemp = m_jobQueue;
      m_jobQueue = jqueue;
      if(jobQueueTemp)
      {
         jobQueueTemp->releaseBlock();
      }
   }
   else 
   {
      m_jobQueue = jqueue;
   }
   
   if(m_jobQueue) startThreadForQueue();
}

std::shared_ptr<ossimJobQueue> ossimJobThreadQueue::getJobQueue() 
{ 
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return m_jobQueue; 
}

const std::shared_ptr<ossimJobQueue> ossimJobThreadQueue::getJobQueue() const 
{ 
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return m_jobQueue; 
}

std::shared_ptr<ossimJob> ossimJobThreadQueue::currentJob() 
{ 
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return m_currentJob; 
}

void ossimJobThreadQueue::cancelCurrentJob()
{
   std::lock_guard<std::mutex> lock(m_threadMutex);
   if(m_currentJob)
   {
      m_currentJob->cancel();
   }
}
bool ossimJobThreadQueue::isValidQueue()const
{
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return (m_jobQueue!=nullptr);
}

void ossimJobThreadQueue::run()
{
   bool firstTime = true;
   bool validQueue = true;
   std::shared_ptr<ossimJob> job;
   do
   {
      //interrupt();
      // osg::notify(osg::NOTICE)<<"In thread loop "<<this<<std::endl;
      validQueue = isValidQueue();
      job = nextJob();
      if (job&&!m_doneFlag)
      {
         {
            std::lock_guard<std::mutex> lock(m_threadMutex);
            m_currentJob = job;
         }
         
         // if the job is ready to execute
         if(job->isReady())
         {
            job->start();
         }
         {            
            std::lock_guard<std::mutex> lock(m_threadMutex);
            m_currentJob = 0;
         }
         job.reset();
      }
      
      if (firstTime)
      {
         ossim::Thread::yieldCurrentThread();
         firstTime = false;
      }
   } while (!m_doneFlag&&validQueue&&!isInterruptable());
   
   {            
      std::lock_guard<std::mutex> lock(m_threadMutex);
      m_currentJob = 0;
   }
   if(job&&m_doneFlag&&job->isReady())
   {
      job->cancel();
   }
   job = 0;
}

void ossimJobThreadQueue::setDone(bool done)
{
   m_threadMutex.lock();
   if (m_doneFlag==done)
   {
      m_threadMutex.unlock();
      return;
   }
   m_doneFlag = done;
   m_threadMutex.unlock();
   if(done)
   {
      {
         std::lock_guard<std::mutex> lock(m_threadMutex);
         if (m_currentJob)
            m_currentJob->release();
      }
      
      if (m_jobQueue)
         m_jobQueue->releaseBlock();
   }
}

bool ossimJobThreadQueue::isDone() const 
{ 
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return m_doneFlag; 
}

bool ossimJobThreadQueue::isProcessingJob()const
{
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return (m_currentJob!=nullptr);
}

void ossimJobThreadQueue::cancel()
{
   
   if( isRunning() )
   {
      {
         std::lock_guard<std::mutex> lock(m_threadMutex);
         m_doneFlag = true;
         if (m_currentJob)
         {
            m_currentJob->cancel();
         }
         
         if (m_jobQueue) 
         {
            m_jobQueue->releaseBlock();
         }
      }
      // then wait for the the thread to stop running.  Because
      // we can't release our thread we have to keep releasing until
      // we get to our thread that is currently blocked waiting on the
      // queue
      // Because we are destructing and we don't know which
      // order our thread is blocked, we will do a one thread at
      // a time release instead of release all at once all the time
      //
      //
      while(isRunning())
      {
         {
            std::lock_guard<std::mutex> lock(m_threadMutex);
            
            if (m_jobQueue) 
            {
               m_jobQueue->releaseBlock();
            }
         }
         ossim::Thread::yieldCurrentThread();
      }
   }
}

bool ossimJobThreadQueue::isEmpty()const
{
   std::lock_guard<std::mutex> lock(m_threadMutex);
   return m_jobQueue->isEmpty();
}

ossimJobThreadQueue::~ossimJobThreadQueue()
{
   cancel();
}

void ossimJobThreadQueue::startThreadForQueue()
{
   if(m_jobQueue)
   {
      if(!isRunning())
      {
         start();
      }
   }
}

bool ossimJobThreadQueue::hasJobsToProcess()const
{
   bool result = false;
   {
      std::lock_guard<std::mutex> lock(m_threadMutex);
      result = (!m_jobQueue->isEmpty()||m_currentJob);
   }
   
   return result;
}

std::shared_ptr<ossimJob> ossimJobThreadQueue::nextJob()
{
   std::shared_ptr<ossimJob> job;
   m_threadMutex.lock();
   std::shared_ptr<ossimJobQueue> jobQueue = m_jobQueue;
   bool checkIfValid = !m_doneFlag&&jobQueue;
   m_threadMutex.unlock();
   if(checkIfValid)
   {
      job = jobQueue->nextJob(true);
   }
   return job;
}
