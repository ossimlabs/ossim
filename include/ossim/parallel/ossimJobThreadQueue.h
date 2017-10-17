#ifndef ossimJobThreadQueue_HEADER
#define ossimJobThreadQueue_HEADER
#include <ossim/parallel/ossimJobQueue.h>
#include <ossim/base/Thread.h>
#include <mutex>

/**
* ossimJobThreadQueue allows one to instantiate a thread with a shared
* queue. the thread will block if the queue is empty and will continue
* to pop jobs off the queue calling the start method on the job.  Once it
* finishes the job it is disguarded and then the next job will be popped off the 
* queue.
*  
* @code
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
* int main(int argc, char *argv[])
* {
*    std::shared_ptr<ossimJobQueue> jobQueue = std::make_shared<ossimJobQueue>();
*    std::shared_ptr<ossimJobThreadQueue> jobThreadQueue = std::make_shared<ossimJobThreadQueue>(jobQueue);
*    jobThreadQueue->start();
*    std::shared_ptr<TestJob> job = std::make_shared<TestJob>();
*    job->setCallback(std::make_shared<MyCallback>());
*    jobQueue->add(job);
*    std::cout << "Waiting 5 seconds before terminating\n";
*    ossim::Thread::sleepInSeconds(5);
*    jobThreadQueue->cancel();
*    jobThreadQueue->waitForCompletion();
* 
*    return 0;
* }
* @endcode
*/
class OSSIM_DLL ossimJobThreadQueue : public ossim::Thread
{
public:
   /**
   * constructor that allows one to instantiat the thread with 
   * a shared job queue.
   *
   * @param jqueue shared job queue
   */
   ossimJobThreadQueue(std::shared_ptr<ossimJobQueue> jqueue=0);
   
   /**
   * destructor.  Will terminate the thread and stop current jobs
   */
   virtual ~ossimJobThreadQueue();

   /**
   *
   * Sets the shared queue that this thread will be pulling jobs from
   *
   * @param jqueue the shared job queue to set
   */
   void setJobQueue(std::shared_ptr<ossimJobQueue> jqueue);
   
   /**
   * @return the current shared job queue
   */
   std::shared_ptr<ossimJobQueue> getJobQueue();
   
   /**
   * @return the current shared job queue
   */
   const std::shared_ptr<ossimJobQueue> getJobQueue() const; 
   
   /**
   * @return the current job that is being handled.
   */
   std::shared_ptr<ossimJob> currentJob();
   
   /**
   * Will cancel the current job
   */
   void cancelCurrentJob();

   /**
   * @return is the queue valid
   */
   bool isValidQueue()const;
   
   /**
   * This is method is overriden from the base thread class and is
   * the main entry point of the thread
   */
   virtual void run();
   
   /**
   * Sets the done flag.
   *
   * @param done the value to set
   */
   void setDone(bool done);
   
   /**
   * @return if the done flag is set
   */
   bool isDone()const;

   /**
   * Cancels the thread
   */
   virtual void cancel();

   /**
   * @return true if the queue is empty
   *         false otherwise.
   */
   bool isEmpty()const;
   
   /**
   * @return true if a job is currently being processed
   *         false otherwise.
   */
   bool isProcessingJob()const;
   
   /**
   * @return true if there are still jobs to be processed
   *         false otherwise.
   */
   bool hasJobsToProcess()const;
   
protected:
   /**
   * Internal method.  If setJobQueue is set on this thread
   * it will auto start this thread.
   */
   void startThreadForQueue();

   /**
   * Will return the next job on the queue
   */
   virtual std::shared_ptr<ossimJob> nextJob();
   
   bool                           m_doneFlag;
   mutable std::mutex             m_threadMutex;
   std::shared_ptr<ossimJobQueue> m_jobQueue;
   std::shared_ptr<ossimJob>      m_currentJob;
   
};

#endif
