#ifndef ossimJobThreadQueue_HEADER
#define ossimJobThreadQueue_HEADER
#include <ossim/parallel/ossimJobQueue.h>
#include <ossim/base/Thread.h>
#include <mutex>
class OSSIM_DLL ossimJobThreadQueue : public ossimReferenced, 
                                      public ossim::Thread
{
public:
   ossimJobThreadQueue(ossimJobQueue* jqueue=0);
   void setJobQueue(ossimJobQueue* jqueue);
   
   ossimJobQueue* getJobQueue();
   
   const ossimJobQueue* getJobQueue() const; 
   
   ossimRefPtr<ossimJob> currentJob();
   
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
   virtual ~ossimJobThreadQueue();
   
   void startThreadForQueue();
   virtual ossimRefPtr<ossimJob> nextJob();
   
   bool                       m_doneFlag;
   mutable std::mutex m_threadMutex;
   ossimRefPtr<ossimJobQueue> m_jobQueue;
   ossimRefPtr<ossimJob>      m_currentJob;
   
};

#endif
