#ifndef ossimJobMultiThreadQueue_HEADER
#define ossimJobMultiThreadQueue_HEADER
#include <ossim/parallel/ossimJobThreadQueue.h>
#include <mutex>

/**
* This allocates a thread pool used to listen on a shared job queue
*
* @code
* #include <ossim/base/Thread.h>
* #include <ossim/parallel/ossimJob.h>
* #include <ossim/parallel/ossimJobMultiThreadQueue.h>
* #include <ossim/parallel/ossimJobQueue.h>
* #include <memory>
* #include <iostream>
* 
* class TestJob : public ossimJob
* {
* public:
*    TestJob(){}
* protected:
*    virtual void run()
*    {
*       ossim::Thread::sleepInSeconds(2);
*    }
* };
* class MyCallback : public ossimJobCallback
* {
* public:
*    MyCallback(){}
*    virtual void started(std::shared_ptr<ossimJob> job)  
*    {
*       std::cout << "Started job\n";
*       ossimJobCallback::started(job);
*    }
*    virtual void finished(std::shared_ptr<ossimJob> job) 
*    {
*       std::cout << "Finished job\n";
*       ossimJobCallback::finished(job);
*    }
* };
* 
* int main(int argc, char *argv[])
* {
*    int nThreads = 5;
*    int nJobs = 10;
*    std::shared_ptr<ossimJobQueue> jobQueue = std::make_shared<ossimJobQueue>();
*    std::shared_ptr<ossimJobMultiThreadQueue> jobThreadQueue = std::make_shared<ossimJobMultiThreadQueue>(jobQueue, nThreads);
*    for(int i = 0; i < nJobs; ++i)
*    {
*       std::shared_ptr<TestJob> job = std::make_shared<TestJob>();
*       job->setCallback(std::make_shared<MyCallback>());
*       jobQueue->add(job);
*    }
* 
*    while(jobThreadQueue->hasJobsToProcess())
*    {
*       ossim::Thread::sleepInMilliSeconds(10);
*    }
* 
*    std::cout << "Finished and cancelling thread queue\n";
*    jobThreadQueue->cancel();
*    jobThreadQueue->waitForCompletion();
* 
*    return 0;
* }
* @endcode
*/
class OSSIM_DLL ossimJobMultiThreadQueue
{
public:
   typedef std::vector<std::shared_ptr<ossimJobThreadQueue> > ThreadQueueList;
   
   /**
   * allows one to create a pool of threads with a shared job queue
   */
   ossimJobMultiThreadQueue(std::shared_ptr<ossimJobQueue> q=0, 
                            ossim_uint32 nThreads=0);
   /**
   * Will cancel all threads and wait for completion and clear the thread queue
   * list
   */
   virtual ~ossimJobMultiThreadQueue();

   /**
   * @return the job queue
   */
   std::shared_ptr<ossimJobQueue> getJobQueue();

   /**
   * @return the job queue
   */
   const std::shared_ptr<ossimJobQueue> getJobQueue()const;

   /**
   * set the job queue to all threads
   *
   * @param q the job queue to set
   */
   void setJobQueue(std::shared_ptr<ossimJobQueue> q);

   /**
   * Will set the number of threads
   */
   void setNumberOfThreads(ossim_uint32 nThreads);

   /**
   * @return the number of threads
   */
   ossim_uint32 getNumberOfThreads() const;

   /**
   * @return the number of threads that are busy
   */
   ossim_uint32 numberOfBusyThreads()const;

   /**
   * @return true if all threads are busy and false otherwise
   */
   bool areAllThreadsBusy()const;

   /**
   * @return true if it has jobs that it's processing
   */
   bool hasJobsToProcess()const;

   /**
   * Allows one to cancel all threads
   */
   void cancel();

   /**
   * Allows on to wait for all thread completions.  Usually called after
   * @see cancel
   */
   void waitForCompletion();

protected:
   mutable std::mutex             m_mutex;
   std::shared_ptr<ossimJobQueue> m_jobQueue;
   ThreadQueueList                m_threadQueueList;
};

#endif
