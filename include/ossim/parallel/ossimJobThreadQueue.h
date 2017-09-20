#ifndef ossimJobThreadQueue_HEADER
#define ossimJobThreadQueue_HEADER
#include <ossim/parallel/ossimJobQueue.h>
#include <ossim/base/Thread.h>
#include <mutex>
class OSSIM_DLL ossimJobThreadQueue : public ossim::Thread
{
public:
   ossimJobThreadQueue(std::shared_ptr<ossimJobQueue> jqueue=0);
   virtual ~ossimJobThreadQueue();
   void setJobQueue(std::shared_ptr<ossimJobQueue> jqueue);
   
   std::shared_ptr<ossimJobQueue> getJobQueue();
   
   const std::shared_ptr<ossimJobQueue> getJobQueue() const; 
   
   std::shared_ptr<ossimJob> currentJob();
   
   void cancelCurrentJob();
   bool isValidQueue()const;
   
   virtual void run();
   
   void setDone(bool done);
   
   bool isDone()const;
   virtual void cancel();
   bool isEmpty()const;
   
   bool isProcessingJob()const;
   
   bool hasJobsToProcess()const;
   
protected:
   
   void startThreadForQueue();
   virtual std::shared_ptr<ossimJob> nextJob();
   
   bool                           m_doneFlag;
   mutable std::mutex             m_threadMutex;
   std::shared_ptr<ossimJobQueue> m_jobQueue;
   std::shared_ptr<ossimJob>      m_currentJob;
   
};

#endif
