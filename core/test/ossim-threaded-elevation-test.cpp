#include <cstdlib> /* for exit */
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/elevation/ossimDtedElevationDatabase.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/init/ossimInit.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/Barrier>
#include <OpenThreads/Thread>

OpenThreads::Barrier* startBarrier = 0;
OpenThreads::Barrier* endBarrier = 0;
OpenThreads::Mutex accumMutex;
ossimRefPtr<ossimDtedElevationDatabase> dtedDatabase = new ossimDtedElevationDatabase;
double accumH = 0.0;

class ElevationThread : public OpenThreads::Thread
{
public:
   ElevationThread(const ossimString& threadName="",
                   double minLon=-180.0, 
                   double minLat=-90.0,
                   double maxLon=180.0,
                   double maxLat=90.0)
   :m_threadName(threadName),
   m_minLat(minLat),
   m_minLon(minLon),
   m_maxLat(maxLat),
   m_maxLon(maxLon)
   {
      m_numberOfPointsToQuery = 1000;
      m_lonDelta = m_maxLon-m_minLon;
      m_latDelta = m_maxLat-m_minLat;
      m_mslTest = false;
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
      double hs=0.0;
      for(idx = 0; idx < m_numberOfPointsToQuery; ++idx)
      {
         ossim_float32 xt = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
         ossim_float32 yt = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
         ossimGpt gpt(m_minLat + yt*m_latDelta, m_minLon + xt*m_lonDelta);
         double h;

#if 0
         h = ossimElevManager::instance()->getHeightAboveEllipsoid(gpt);// << "==" << ossimElevManager::instance()->getHeightAboveMSL(gpt) << "\n";
         std::cout << h << "\n";
#else
         if(m_mslTest)
         {
            h = ossimElevManager::instance()->getHeightAboveMSL(gpt);
         }
         else
         {
            h = ossimElevManager::instance()->getHeightAboveEllipsoid(gpt);
         }
#endif
         hs += h;
      }

      accumMutex.lock();
      accumH += hs / m_numberOfPointsToQuery;
      accumMutex.unlock();

      // let all threads end at the same time
      endBarrier->block();
   }
   
   ossimString m_threadName;
   ossim_uint32 m_numberOfPointsToQuery;
   double m_minLat;
   double m_minLon;
   double m_maxLat;
   double m_maxLon;
   double m_latDelta;
   double m_lonDelta;
   
   bool m_mslTest;
};

int main(int argc, char* argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   //ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   ossim_uint32 num_threads = 10;
   ossim_uint32 round_robin = 10;
   ossim_uint32 num_posts = 1000;
   ossim_uint32 randomSeed = 0;
   double minLon = -110.0;
   double minLat = -90.0;
   double maxLon = -80.0;
   double maxLat = 90.0;
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("--threads","<int> Specify the number of threads to test simultaneus access to elevation");
   argumentParser.getApplicationUsage()->addCommandLineOption("--elev-post-queries","<int> Number of elevation post queries per thread");
   argumentParser.getApplicationUsage()->addCommandLineOption("--posts","Same as elev-post-queries option");
   argumentParser.getApplicationUsage()->addCommandLineOption("--random-seed", "<int> Value to use as the seed for the random elevation post generator");
   argumentParser.getApplicationUsage()->addCommandLineOption("--use-geoid-if-null", "<true|false> Specify true or false to use the geoid if the elevation post is null");
   argumentParser.getApplicationUsage()->addCommandLineOption("--bbox", "<min-lat, min-lon, max-lat, max-lon> Ground rect to constrain the random values to");
   argumentParser.getApplicationUsage()->addCommandLineOption("--round-robin", "<int> Specifies size of round robin list used by elevation manager.");
   if (argumentParser.read("-h") ||
       argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_WARN));
      exit(0);
   }
   if(argumentParser.read("--threads", stringParam))
   {
      num_threads = tempString.toUInt32();
      round_robin = num_threads;
   }
   if(argumentParser.read("--round-robin", stringParam))
   {
      round_robin = tempString.toUInt32();
   }
   if (argumentParser.read("--elev-post-queries", stringParam) ||
       argumentParser.read("--posts", stringParam))
   {
      num_posts = tempString.toUInt32();
   }
   if(argumentParser.read("--random-seed", stringParam))
   {
      randomSeed = tempString.toUInt32();
   }
   if(argumentParser.read("--use-geoid-if-null", stringParam))
   {
      ossimElevManager::instance()->setUseGeoidIfNullFlag(tempString.toBool());
   }
   if(argumentParser.read("--bbox", stringParam))
   {
      std::vector<ossimString> splitArray;
      tempString.split(splitArray, ",");
      if(splitArray.size()==4)
      {
         minLon = splitArray[0].toDouble();
         minLat = splitArray[1].toDouble();
         maxLon = splitArray[2].toDouble();
         maxLat = splitArray[3].toDouble();
         
         std::cout  << "minlon = " << minLon << "\n";
         std::cout  << "minlat = " << minLat << "\n";
         std::cout  << "maxlon = " << maxLon << "\n";
         std::cout  << "maxlat = " << maxLat << "\n";
      }
   }

   ossimElevManager::instance()->setRoundRobinMaxSize(round_robin);

   ossimKeywordlist kwl;
//   kwl.add("connection_string", "/data/elevation/dted/1k");
//   kwl.add("type", "ossimDtedElevationDatabase");
  // std::cout << "Loaded database? " << dtedDatabase->loadState(kwl) << std::endl;
   srand(randomSeed);
   std::vector<ElevationThread*> threadList(num_threads);
   startBarrier = new OpenThreads::Barrier(num_threads); // include the main thread for synching
   endBarrier = new OpenThreads::Barrier(num_threads+1); //   include main thread for syncing end
   ossim_uint32 idx = 0;
   for(idx = 0; idx < num_threads; ++ idx)
   {
      threadList[idx] = new ElevationThread("Thread " + ossimString::toString(idx), minLon, minLat, maxLon, maxLat);
      threadList[idx]->setNumberOfPointsToQuery(num_posts);
      threadList[idx]->start();
   }
   ossimTimer::Timer_t t1 = ossimTimer::instance()->tick();

   // synch all threads to start at the same time
   std::cout << "Number of threads:         " << num_threads      << "\n";
   std::cout << "Posts to query per thread: " << num_posts << "\n";
   //   startBarrier->block();
   endBarrier->block();
   std::cout << "All threads finished\n";
   ossimTimer::Timer_t t2 = ossimTimer::instance()->tick();
   std::cout << "Time elapsed:              " << ossimTimer::instance()->delta_s(t1, t2) << " seconds" << "\n";
   std::cout << "mean elevation:              " << accumH/num_threads << "\n";
   delete startBarrier;
   delete endBarrier;
   return 0;
}
