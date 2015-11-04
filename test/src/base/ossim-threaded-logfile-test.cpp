#include <cstdlib> /* for exit */
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/init/ossimInit.h>

#include <OpenThreads/Mutex>
#include <OpenThreads/Barrier>
#include <OpenThreads/Thread>
OpenThreads::Barrier* startBarrier = 0;
OpenThreads::Barrier* endBarrier = 0;
class LogThread : public OpenThreads::Thread
{
public:
   LogThread(const ossimString& threadName)
   :m_threadName(threadName)
   {
      m_numberOfStringsToLog = 1000000;
   }
   void setNumberOfStringsToLog(ossim_uint32 nStrings)
   {
      m_numberOfStringsToLog = nStrings;
   }
   virtual void run()
   {
      // let all threads start at the same time
      startBarrier->block();
      ossim_uint32 idx = 0;
      for(idx = 0; idx < m_numberOfStringsToLog; ++idx)
      {
         ossimNotify() << m_threadName << std::endl;
 //        ossimGpt gpt(m_minLat + yt*m_latDelta, m_minLon + xt*m_lonDelta);
      }
      // let all threads end at the same time
      endBarrier->block();
   }
   
   ossimString m_threadName;
   ossim_uint32 m_numberOfStringsToLog;
};

int main(int argc, char* argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   ossim_uint32 threads = 10;
   ossim_uint32 nvalues = 10;
   ossim_uint32 randomSeed = 0;

   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("--threads","Specify the number of threads to test simultaneus access to elevation");
   argumentParser.getApplicationUsage()->addCommandLineOption("--nvalues","Specify the number of random values");
   if (argumentParser.read("-h") ||
       argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_WARN));
      exit(0);
   }
   if(argumentParser.read("--threads", stringParam))
   {
      threads = tempString.toUInt32();
   }
   if(argumentParser.read("--nvalues", stringParam))
   {
      nvalues = tempString.toUInt32();
   }

   std::vector<LogThread*> threadList(threads);
   startBarrier = new OpenThreads::Barrier(threads); // include the main thread for synching
   endBarrier   = new OpenThreads::Barrier(threads+1); //   include main thread for syncing end 
   ossim_uint32 idx = 0;
   for(idx = 0; idx < threads; ++ idx)
   {
      threadList[idx] = new LogThread("Thread " + ossimString::toString(idx));
      threadList[idx]->setNumberOfStringsToLog(nvalues);
      threadList[idx]->start();
   }
   ossimTimer::Timer_t t1 = ossimTimer::instance()->tick();
   // synch all threads to start at the same time
   std::cout << "Number of threads:         " << threads      << "\n";
//   startBarrier->block();
   endBarrier->block();
   std::cout << "All threads finished\n";
   ossimTimer::Timer_t t2 = ossimTimer::instance()->tick();
   std::cout << "Time elapsed:              " << ossimTimer::instance()->delta_s(t1, t2) << " seconds" << "\n";
   delete startBarrier;
   delete endBarrier;
   return 0;
}
