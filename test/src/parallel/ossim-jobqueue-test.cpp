#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobQueue.h>
#include <ossim/parallel/ossimJobThreadQueue.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/init/ossimInit.h>
#include <iostream>
#include <ossim/base/Thread.h>

static const int INITIAL_THREADS = 10;
static const int INITIAL_JOBS = 20;
class ossimTestJob : public ossimJob
{
public:
   ossimTestJob(const ossimString& name)
   :ossimJob()
   {
      setName(name);
   }
protected:
   virtual void run()
   {
   }
};

class ossimTestJobCallback : public ossimJobCallback
{
public:
   ossimTestJobCallback()
   {
   }
   virtual void started(std::shared_ptr<ossimJob> job)
   {
      std::cout << "I HAVE STARTED THE Job " << job->name() << "\n";
   }
   virtual void finished(std::shared_ptr<ossimJob> job)
   {
      std::cout << "I HAVE FINISHED THE Job " << job->name() << "\n";
   }
   virtual void canceled(std::shared_ptr<ossimJob> job)
   {
      std::cout << "I HAVE CANCELED THE Job " << job->name() << "\n";
   }
   
};

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   
   std::shared_ptr<ossimJobQueue> q = std::make_shared<ossimJobQueue>();
   std::shared_ptr<ossimJobMultiThreadQueue> threadQueue = std::make_shared<ossimJobMultiThreadQueue>(q, INITIAL_THREADS);
   std::shared_ptr<ossimTestJobCallback> callback = std::make_shared<ossimTestJobCallback>();
   ossim_uint32 idx = 0;
   for(idx = 0; idx < INITIAL_JOBS; ++idx)
   {
      std::shared_ptr<ossimTestJob> job = std::make_shared<ossimTestJob>(ossimString::toString(idx+1));
      job->setCallback(callback);
      job->ready();
      q->add(job);
   }
   
   // FOREVER loop until all jobs are completed.
   while(true)
   {
      ossim::Thread::sleepInMicroSeconds(250);
      if ( threadQueue->hasJobsToProcess() == false )
      {
         break;
      }
      ossim::Thread::yieldCurrentThread();
   }
   
   return 0;
}
