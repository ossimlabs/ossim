#include <cstdlib> /* for exit */
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimPolyArea2d.h>

#include <OpenThreads/Mutex>
#include <OpenThreads/Barrier>
#include <OpenThreads/Thread>
OpenThreads::Barrier* startBarrier = 0;
OpenThreads::Barrier* endBarrier = 0;
class Polyarea2dThread : public OpenThreads::Thread
{
public:
   Polyarea2dThread(const ossimString& threadName)
   :m_threadName(threadName)
   {
      m_numberOfPointsToQuery = 1000000;
   }
   void setNumberOfPointsToQuery(ossim_uint32 nPoints)
   {
      m_numberOfPointsToQuery = nPoints;
   }
   virtual void run()
   {
      // let all threads start at the same time
      startBarrier->block();
      ossim_uint32 idx = 0;
      for(idx = 0; idx < m_numberOfPointsToQuery; ++idx)
      {
         ossim_float32 t = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
         ossim_float32 centerx = 10000.0*t; 
         ossim_float32 centery = 10000.0*t;

         ossimPolyArea2d polyArea1(ossimDrect(centerx-1000,centery-1000,centerx+2000,centery+2000));
         ossimPolyArea2d polyArea2(ossimDrect(centerx-500,centery-500,centerx+2000,centery+2000));

         ossimPolyArea2d temp1 = polyArea1&polyArea2;
         ossimPolyArea2d temp2 = polyArea1+polyArea2;

         ossimPolyArea2d temp3(polyArea1);
         ossimPolyArea2d temp4(polyArea2);

         temp3+=temp1;
         temp4&=temp3;

 //        ossimGpt gpt(m_minLat + yt*m_latDelta, m_minLon + xt*m_lonDelta);
      }
      // let all threads end at the same time
      endBarrier->block();
   }
   
   ossimString m_threadName;
   ossim_uint32 m_numberOfPointsToQuery;
};

int main(int argc, char* argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   //ossimInit::instance()->addOptions(argumentParser);
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

   std::vector<Polyarea2dThread*> threadList(threads);
   startBarrier = new OpenThreads::Barrier(threads); // include the main thread for synching
   endBarrier   = new OpenThreads::Barrier(threads+1); //   include main thread for syncing end 
   ossim_uint32 idx = 0;
   for(idx = 0; idx < threads; ++ idx)
   {
      threadList[idx] = new Polyarea2dThread("Thread " + ossimString::toString(idx));
      threadList[idx]->setNumberOfPointsToQuery(nvalues);
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
