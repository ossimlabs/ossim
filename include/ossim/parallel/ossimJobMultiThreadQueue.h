#ifndef ossimJobMultiThreadQueue_HEADER
#define ossimJobMultiThreadQueue_HEADER
#include <ossim/parallel/ossimJobThreadQueue.h>
#include <mutex>

class OSSIM_DLL ossimJobMultiThreadQueue
{
public:
   typedef std::vector<std::shared_ptr<ossimJobThreadQueue> > ThreadQueueList;
   
   ossimJobMultiThreadQueue(std::shared_ptr<ossimJobQueue> q=0, 
                            ossim_uint32 nThreads=0);
   std::shared_ptr<ossimJobQueue> getJobQueue();
   const std::shared_ptr<ossimJobQueue> getJobQueue()const;
   void setQueue(std::shared_ptr<ossimJobQueue> q);
   void setNumberOfThreads(ossim_uint32 nThreads);
   ossim_uint32 getNumberOfThreads() const;
   ossim_uint32 numberOfBusyThreads()const;
   bool areAllThreadsBusy()const;
   
   bool hasJobsToProcess()const;

protected:
   mutable std::mutex  m_mutex;
   std::shared_ptr<ossimJobQueue> m_jobQueue;
   ThreadQueueList m_threadQueueList;
};

#endif
