#include <ossim/parallel/ossimJobQueue.h>

#include <algorithm> /* for std::find */

ossimJobQueue::ossimJobQueue()
{
}

void ossimJobQueue::add(ossimJob* job, bool guaranteeUniqueFlag)
{
   std::shared_ptr<Callback> cb;
   {
      {
         std::lock_guard<std::mutex> lock(m_jobQueueMutex);
         
         if(guaranteeUniqueFlag)
         {
            if(findByPointer(job) != m_jobQueue.end())
            {
               m_block.set(true);
               return;
            }
         }
         cb = m_callback;
      }
      if(cb) cb->adding(this, job);
      
      job->ready();
      m_jobQueueMutex.lock();
      m_jobQueue.push_back(job);
      m_jobQueueMutex.unlock();
   }
   if(cb)
   {
      cb->added(this, job);
   }
   m_block.set(true);
}

ossimRefPtr<ossimJob> ossimJobQueue::removeByName(const ossimString& name)
{
   ossimRefPtr<ossimJob> result;
   std::shared_ptr<Callback> cb;
   if(name.empty()) return result;
   {
      std::lock_guard<std::mutex> lock(m_jobQueueMutex);
      ossimJob::List::iterator iter = findByName(name);
      if(iter!=m_jobQueue.end())
      {
         result = *iter;
         m_jobQueue.erase(iter);
      }
      cb = m_callback;
   }      
   m_block.set(!m_jobQueue.empty());
   
   if(cb&&result.valid())
   {
      cb->removed(this, result.get());
   }
   return result;
}
ossimRefPtr<ossimJob> ossimJobQueue::removeById(const ossimString& id)
{
   ossimRefPtr<ossimJob> result;
   std::shared_ptr<Callback> cb;
   if(id.empty()) return result;
   {
      std::lock_guard<std::mutex> lock(m_jobQueueMutex);
      ossimJob::List::iterator iter = findById(id);
      if(iter!=m_jobQueue.end())
      {
         result = *iter;
         m_jobQueue.erase(iter);
      }
      cb = m_callback;
      m_block.set(!m_jobQueue.empty());
   }
   if(cb&&result.valid())
   {
      cb->removed(this, result.get());
   }
   return result;
}

void ossimJobQueue::remove(const ossimJob* Job)
{
   ossimRefPtr<ossimJob> removedJob;
   std::shared_ptr<Callback> cb;
   {
      std::lock_guard<std::mutex> lock(m_jobQueueMutex);
      ossimJob::List::iterator iter = std::find(m_jobQueue.begin(), m_jobQueue.end(), Job);
      if(iter!=m_jobQueue.end())
      {
         removedJob = (*iter);
         m_jobQueue.erase(iter);
      }
      cb = m_callback;
   }
   if(cb&&removedJob.valid())
   {
      cb->removed(this, removedJob.get());
   }
}

void ossimJobQueue::removeStoppedJobs()
{
   ossimJob::List removedJobs;
   std::shared_ptr<Callback> cb;
   {
      std::lock_guard<std::mutex> lock(m_jobQueueMutex);
      cb = m_callback;
      ossimJob::List::iterator iter = m_jobQueue.begin();
      while(iter!=m_jobQueue.end())
      {
         if((*iter)->isStopped())
         {
            removedJobs.push_back(*iter);
            iter = m_jobQueue.erase(iter);
         }
         else 
         {
            ++iter;
         }
      }
   }
   if(!removedJobs.empty())
   {
      if(cb)
      {
         ossimJob::List::iterator iter = removedJobs.begin();
         while(iter!=removedJobs.end())
         {
            cb->removed(this, (*iter).get());
            ++iter;
         }
      }
      removedJobs.clear();
   }
}

void ossimJobQueue::clear()
{
   ossimJob::List removedJobs(m_jobQueue);
   std::shared_ptr<Callback> cb;
   {
      std::lock_guard<std::mutex> lock(m_jobQueueMutex);
      m_jobQueue.clear();
      cb = m_callback;
   }
   if(cb)
   {
      // ossim_uint32 idx = 0;
      for(ossimJob::List::iterator iter=removedJobs.begin();iter!=removedJobs.end();++iter)
      {
         cb->removed(this, (*iter).get());
      }
   }
}

ossimRefPtr<ossimJob> ossimJobQueue::nextJob(bool blockIfEmptyFlag)
{
   m_jobQueueMutex.lock();
   bool emptyFlag = m_jobQueue.empty();
   m_jobQueueMutex.unlock();
   if (blockIfEmptyFlag && emptyFlag)
   {
      m_block.block();
   }
   
   ossimRefPtr<ossimJob> result;
   std::lock_guard<std::mutex> lock(m_jobQueueMutex);
   
   if (m_jobQueue.empty())
   {
      m_block.set(false);
      return result;
   }
   
   ossimJob::List::iterator iter= m_jobQueue.begin();
   while((iter != m_jobQueue.end())&&
         (((*iter)->isCanceled())))
   {
      (*iter)->finished(); // mark the ob as being finished 
      iter = m_jobQueue.erase(iter);
   }
   if(iter != m_jobQueue.end())
   {
      result = *iter;
      m_jobQueue.erase(iter);
   }
   m_block.set(!m_jobQueue.empty());
   return result;
}
void ossimJobQueue::releaseBlock()
{
   m_block.release();
}
bool ossimJobQueue::isEmpty()const
{
   // std::lock_guard<std::mutex> lock(m_jobQueueMutex);
   // return m_jobQueue.empty();
   m_jobQueueMutex.lock();
   bool result =  m_jobQueue.empty();
   m_jobQueueMutex.unlock();
   return result;
}

ossim_uint32 ossimJobQueue::size()
{
   std::lock_guard<std::mutex> lock(m_jobQueueMutex);
   return (ossim_uint32) m_jobQueue.size();
}

ossimJob::List::iterator ossimJobQueue::findById(const ossimString& id)
{
   if(id.empty()) return m_jobQueue.end();
   ossimJob::List::iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if(id == (*iter)->id())
      {
         return iter;
      }
      ++iter;
   }  
   return m_jobQueue.end();
}

ossimJob::List::iterator ossimJobQueue::findByName(const ossimString& name)
{
   if(name.empty()) return m_jobQueue.end();
   ossimJob::List::iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if(name == (*iter)->name())
      {
         return iter;
      }
      ++iter;
   }  
   return m_jobQueue.end();
}

ossimJob::List::iterator ossimJobQueue::findByPointer(const ossimJob* job)
{
   return std::find(m_jobQueue.begin(),
                    m_jobQueue.end(),
                    job);
}

ossimJob::List::iterator ossimJobQueue::findByNameOrPointer(const ossimJob* job)
{
   ossimString n = job->name();
   ossimJob::List::iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if((*iter).get() == job)
      {
         return iter;
      }
      else if((!n.empty())&&
              (job->name() == (*iter)->name()))
      {
         return iter;
      }
      ++iter;
   }  
   
   return m_jobQueue.end();
}

bool ossimJobQueue::hasJob(ossimJob* job)
{
   ossimJob::List::const_iterator iter = m_jobQueue.begin();
   while(iter != m_jobQueue.end())
   {
      if(job == (*iter).get())
      {
         return true;
      }
      ++iter;
   }
   
   return false;
}

void ossimJobQueue::setCallback(std::shared_ptr<Callback> c)
{
   std::lock_guard<std::mutex> lock(m_jobQueueMutex);
   m_callback = c;
}

std::shared_ptr<ossimJobQueue::Callback> ossimJobQueue::callback()
{
   std::lock_guard<std::mutex> lock(m_jobQueueMutex);
   return m_callback;
}
