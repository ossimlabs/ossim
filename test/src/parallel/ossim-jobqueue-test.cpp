#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobQueue.h>
#include <ossim/parallel/ossimJobThreadQueue.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/init/ossimInit.h>
#include <OpenThreads/Thread>
#include <iostream>
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
   virtual void start()
   {
   }
};

class ossimTestJobCallback : public ossimJobCallback
{
public:
   ossimTestJobCallback()
   {
   }
   virtual void started(ossimJob* job)
   {
      std::cout << "I HAVE STARTED THE Job " << job->name() << "\n";
   }
   virtual void finished(ossimJob* job)
   {
      std::cout << "I HAVE FINISHED THE Job " << job->name() << "\n";
   }
   virtual void canceled(ossimJob* job)
   {
      std::cout << "I HAVE CANCELED THE Job " << job->name() << "\n";
   }
   
};

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   
   ossimRefPtr<ossimJobQueue> q = new ossimJobQueue();
   ossimRefPtr<ossimJobMultiThreadQueue> threadQueue = new ossimJobMultiThreadQueue(q.get(), INITIAL_THREADS);
   ossimRefPtr<ossimTestJobCallback> callback = new ossimTestJobCallback();
   ossim_uint32 idx = 0;
   for(idx = 0; idx < INITIAL_JOBS; ++idx)
   {
      ossimRefPtr<ossimTestJob> job = new ossimTestJob(ossimString::toString(idx+1));
      job->setCallback(callback.get());
      job->ready();
      q->add(job.get());
   }
   
   // FOREVER loop until all jobs are completed.
   while(true)
   {
      if ( OpenThreads::Thread::microSleep(250) == 0 )
      {
         if ( threadQueue->hasJobsToProcess() == false )
         {
            break;
         }
      }
   }
   
   return 0;
}
