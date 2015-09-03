#include <cstdlib> /* for exit */
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimAppFixedTileCache.h>
#include <ossim/imaging/ossimImageData.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/Barrier>
#include <OpenThreads/Thread>
OpenThreads::Barrier* startBarrier = 0;
OpenThreads::Barrier* endBarrier = 0;
class TileCacheThread : public OpenThreads::Thread
{
public:
   TileCacheThread(const ossimString& threadName="",
                   const ossimIpt& regionSize=ossimIpt(4096,4096),
                   const ossimIpt& tileSize=ossimIpt(64,64))
   :m_threadName(threadName),
   m_regionSize(regionSize),
   m_tileSize(tileSize),
   m_cacheId(-1)
   {
   }
   virtual void run()
   {
      startBarrier->block();
      m_cacheId = ossimAppFixedTileCache::instance()->newTileCache();
      ossimAppFixedTileCache::instance()->setRect(m_cacheId,ossimIrect(0,0,m_regionSize.x-1, m_regionSize.y-1));
      ossimAppFixedTileCache::instance()->setTileSize(m_cacheId, m_tileSize);      
      ossimIpt origin;
      ossim_int32 x;
      ossim_int32 y;
      
      for(y = 0; y < m_regionSize.y;y+=m_tileSize.y)
      {
         for(x = 0; x < m_regionSize.x;x+=m_tileSize.x)
         {
            ossimRefPtr<ossimImageData> data = new ossimImageData(0, OSSIM_UINT8, 3, m_tileSize.x, m_tileSize.y);
            data->setImageRectangle(ossimIrect(x,y,x+(m_tileSize.x-1), y+(m_tileSize.y-1)));
            data->initialize();
            ossimAppFixedTileCache::instance()->addTile(m_cacheId,
                                                        data.get());
         }
      }
      for(y = 0; y < m_regionSize.y;y+=m_tileSize.y)
      {
         for(x = 0; x < m_regionSize.x;x+=m_tileSize.x)
         {
            origin.x = x;
            origin.y = y;
            ossimRefPtr<ossimImageData> tempTile = ossimAppFixedTileCache::instance()->getTile(m_cacheId,
                                                                                               origin);
            if(!tempTile.valid())
            {
               std::cout << "TILE not found in cache,  THIS MESSAGE SHOULD NEVER HAPPEN!!!" << std::endl;
            }
         }
      }
      
      ossimAppFixedTileCache::instance()->deleteCache(m_cacheId);
      // let all threads end at the same time
      std::cout << "THREAD FINISHED: " << m_threadName << std::endl;
      endBarrier->block();
      m_cacheId = -1;
   }
   
   ossimString m_threadName;
   ossimIpt    m_regionSize;
   ossimIpt    m_tileSize;
   ossimAppFixedTileCache::ossimAppFixedCacheId m_cacheId;
};

int main(int argc, char* argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   //ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   ossimAppFixedTileCache::instance()->setMaxCacheSize(1024*1024*1024);      
   ossim_uint32 threads = 10;
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("--threads","Specify the number of threads to test simultaneus access to elevation");
//   argumentParser.getApplicationUsage()->addCommandLineOption("--random-seed", "value to use as the seed for the random elevation post generator");
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
   std::vector<TileCacheThread*> threadList(threads);
   startBarrier = new OpenThreads::Barrier(threads+1); // include the main thread for synching
   endBarrier = new OpenThreads::Barrier(threads+1); //   include main thread for syncing end 
   ossim_uint32 idx = 0;
   for(idx = 0; idx < threads; ++ idx)
   {
      threadList[idx] = new TileCacheThread("Thread " + ossimString::toString(idx));
      threadList[idx]->start();
   }
   ossimTimer::Timer_t t1 = ossimTimer::instance()->tick();
   // synch all threads to start at the same time
   startBarrier->block();
   std::cout << "Number of threads:         " << threads      << "\n";
   endBarrier->block();
   std::cout << "All threads finished\n";
   ossimTimer::Timer_t t2 = ossimTimer::instance()->tick();
   std::cout << "Time elapsed:              " << ossimTimer::instance()->delta_s(t1, t2) << " seconds" << "\n";
   delete endBarrier;
   return 0;
}
