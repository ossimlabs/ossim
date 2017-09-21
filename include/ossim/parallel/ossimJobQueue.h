//**************************************************************************************************
//                          OSSIM -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
//**************************************************************************************************
//  $Id$
#ifndef ossimJobQueue_HEADER
#define ossimJobQueue_HEADER

#include <ossim/parallel/ossimJob.h>
#include <ossim/base/Block.h>
#include <mutex>
#include <memory>

/**
* This is the base implementation for the job queue.  It allows one to add and remove
* jobs and to call the nextJob and, if specified, block the call if no jobs are on
* the queue.  we derived from std::enable_shared_from_this which allows us access to 
* a safe shared 'this' pointer.  This is used internal to our callbacks.
*
* The job queue is thread safe and can be shared by multiple threads.
*
* Here is a quick code example on how to create a shared queue and to attach
* a thread to it.  In this example we do not block the calling thread for nextJob
* @code
* #include <ossim/base/Thread.h>
* #include <ossim/parallel/ossimJob.h>
* #include <ossim/parallel/ossimJobQueue.h>
* #include <iostream>
* class TestJobQueueThread : public ossim::Thread
* {
* public:
*    TestJobQueueThread(std::shared_ptr<ossimJobQueue> q):m_q(q){}
*    void run()
*    {
*       if(m_q)
*       {
*          while(true)
*          {
*             interrupt();
*             std::shared_ptr<ossimJob> job = m_q->nextJob(false);
*             if(job)
*             {
*                job->start();
*             }
*             yieldCurrentThread();
*             sleepInMilliSeconds(20);
*          }
*       }
*    }
* 
* private:
*   std::shared_ptr<ossimJobQueue> m_q;
* };
* 
* class TestJob : public ossimJob
* {
* public:
*    virtual void run()
*    {
*       std:cout << "Running Job\n";
*       ossim::Thread::sleepInSeconds(2);
*       std::cout << "Finished Running Job\n";
*    }
* };
* int main(int argc, char *argv[])
* {
*    std::shared_ptr<ossimJobQueue> q = std::make_shared<ossimJobQueue>();
*    std::shared_ptr<TestJobQueueThread> jobQueueThread = std::make_shared<TestJobQueueThread>(q);
* 
*    jobQueueThread->start();
* 
*    q->add(std::make_shared<TestJob>());
*    q->add(std::make_shared<TestJob>());
*    q->add(std::make_shared<TestJob>());
* 
*    ossim::Thread::sleepInSeconds(10);
*    jobQueueThread->cancel();
*    jobQueueThread->waitForCompletion();
* }
* @endcode
*/
class OSSIM_DLL ossimJobQueue : public std::enable_shared_from_this<ossimJobQueue>
{
public:
   /**
   * The callback allows one to attach and listen for certain things.  In 
   * the ossimJobQueue it will notify just before adding a job, after adding a job
   * and if a job is removed.
   */
   class OSSIM_DLL Callback
   {
   public:
      Callback(){}

      /**
      * Called just before a job is added
      * 
      * @param q Is a shared_ptr to 'this' job queue
      * @param job Is a shared ptr to the job we are adding
      */
      virtual void adding(std::shared_ptr<ossimJobQueue> /*q*/, 
                          std::shared_ptr<ossimJob> /*job*/){};

      /**
      * Called after a job is added to the queue
      * @param q Is a shared_ptr to 'this' job queue
      * @param job Is a shared ptr to the job we are added     
      */
      virtual void added(std::shared_ptr<ossimJobQueue> /*q*/, 
                         std::shared_ptr<ossimJob> /*job*/){}


      /**
      * Called after a job is removed from the queue
      * @param q Is a shared_ptr to 'this' job queue
      * @param job Is a shared ptr to the job we have removed
      */
      virtual void removed(std::shared_ptr<ossimJobQueue> /*q*/, 
                           std::shared_ptr<ossimJob>/*job*/){}
   };

   /**
   * Default constructor
   */
   ossimJobQueue();
  
   /**
   * This is the safe way to create a std::shared_ptr for 'this'.  Calls the derived
   * method shared_from_this
   */  
   std::shared_ptr<ossimJobQueue> getSharedFromThis(){
      return shared_from_this();
   }

   /**
   * This is the safe way to create a std::shared_ptr for 'this'.  Calls the derived
   * method shared_from_this
   */  
   std::shared_ptr<const ossimJobQueue> getSharedFromThis()const{
      return shared_from_this();
   }

   /**
   * Will add a job to the queue and if the guaranteeUniqueFlag is set it will
   * scan and make sure the job is not on the queue before adding
   *
   * @param job The job to add to the queue.
   * @param guaranteeUniqueFlag if set to true will force a find to make sure the job
   *        does not already exist
   */
   virtual void add(std::shared_ptr<ossimJob> job, bool guaranteeUniqueFlag=true);
   
   /**
   * Allows one to remove a job passing in it's name.
   *
   * @param name The job name
   * @return a shared_ptr to the job.  This will be nullptr if not found.
   */
   virtual std::shared_ptr<ossimJob> removeByName(const ossimString& name);

   /**
   * Allows one to remove a job passing in it's id.
   *
   * @param id The job id
   * @return a shared_ptr to the job.  This will be nullptr if not found.
   */
   virtual std::shared_ptr<ossimJob> removeById(const ossimString& id);

   /**
   * Allows one to pass in a job pointer to remove
   *
   * @param job the job you wish to remove from the list
   */
   virtual void remove(const std::shared_ptr<ossimJob> Job);

   /**
   * Will remove any stopped jobs from the queue
   */
   virtual void removeStoppedJobs();

   /**
   * Will clear the queue
   */
   virtual void clear();

   /**
   * Will grab the next job on the list and will return the job or 
   * a null shared_ptr.  You can block the caller if the queueis empty forcing it
   * to wait for more jobs to come onto the queue
   *
   * @param blockIfEmptyFlag If true it will block the calling thread until more jobs appear
   *        on the queue.  If false, it will return without blocking
   * @return a shared pointer to a job
   */
   virtual std::shared_ptr<ossimJob> nextJob(bool blockIfEmptyFlag=true);

   /**
   * will release the block and have any blocked threads continue
   */
   virtual void releaseBlock();

   /**
   * @return true if the queue is empty false otherwise
   */
   bool isEmpty()const;

   /**
   * @return the number of jobs on the queue
   */
   ossim_uint32 size();

   /**
   *  Allows one to set the callback to the list
   *
   * @param c shared_ptr to a callback
   */
   void setCallback(std::shared_ptr<Callback> c);

   /**
   * @return the callback
   */
   std::shared_ptr<Callback> callback();
   
protected:
   /**
   * Internal method that returns an iterator
   *
   * @param the id of the job to search for
   * @return the iterator
   */
   ossimJob::List::iterator findById(const ossimString& id);

   /**
   * Internal method that returns an iterator
   *
   * @param name the name of the job to search for
   * @return the iterator
   */
   ossimJob::List::iterator findByName(const ossimString& name);

   /**
   * Internal method that returns an iterator
   *
   * @param job the job to search for
   * @return the iterator
   */
   ossimJob::List::iterator findByPointer(const std::shared_ptr<ossimJob> job);

   /**
   * Internal method that returns an iterator
   * 
   * @param job it will find by the name or by the pointer
   * @return the iterator
   */
   ossimJob::List::iterator findByNameOrPointer(const std::shared_ptr<ossimJob> job);

   /**
   * Internal method that determines if we have the job
   * 
   * @param job the job you wish to search for
   */
   bool hasJob(std::shared_ptr<ossimJob> job);
   
   mutable std::mutex m_jobQueueMutex;
   ossim::Block m_block;
   ossimJob::List m_jobQueue;
   std::shared_ptr<Callback> m_callback;
};

#endif
