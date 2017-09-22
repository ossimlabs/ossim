#ifndef ossimBarrier_HEADER
#define ossimBarrier_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace ossim{

   /**
   * Barrier is a class used to block threads so we can synchronize and entry point.
   *
   * In this example we show how to block the threads so they all start at the 
   * same time when executing their work.
   * Example:
   *
   * @code
   * #include <ossim/base/Thread.h>
   * #include <ossim/base/Barrier.h>
   * int nThreads = 2;
   * ossim::Barrier barrierStart(nThreads)
   * // one more for main thread
   * ossim::Barrier barrierFinished(nThreads+1);
   *
   * class TestThread : public ossim::Thread
   *  {
   *  public:
   *      TestThread():ossim::Thread(){}
   *      ~TestThread(){
   *         waitForCompletion();
   *      }
   *
   *   protected:
   *      virtual void run()
   *      {
   *         barrierStart.block();
   *         for(int x =0 ; x < 10;++x){
   *            std::cout << "THREAD: " << getCurrentThreadId() << "\n";
   *            sleepInMilliSeconds(10);
   *            interrupt();
   *         }
   *         barrierFinished.block();
   *      }
   *   };
   *
   * int main(int agrc, char* argv[])
   * {
   *    std::vector<std::shared_ptr<TestThread> > threads(nThreads);
   *    for(auto& thread:threads)
   *    {
   *      thread = std::make_shared<TestThread>();
   *      thread->start();
   *    }
   *    // block main until barrier enters their finished state
   *    barrierFinished.block();
   *
   *    // you can also reset the barriers and run again
   *    barrierFinished.reset();
   *    barrierStart.reset();
   *    for(auto& thread:threads)
   *    {
   *      thread->start();
   *    }
   *    barrierFinished.block();
   * }
   *
   * @endcode
   */
   class OSSIM_DLL Barrier 
   {
   public:
      /**
      * Constructor
      * 
      * @param n is the number of threads you wish to block
      */
      Barrier(ossim_int32 n);

      /**
      * Destructor will reset and release all blocked threads.
      */
      ~Barrier();

      /**
      * block will block the thread based on a wait condition.  it will verify 
      * if the thread can be blocked by testing if the number
      * of blocked threads is less than the total number to blocked threads.  If 
      * the total is reached then all threads are notified and woken up and released  
      */
      void block();

      /**
      * Will reset the barrier to the original values.  
      * It will also release all blocked threads and wait for their release
      * before resetting.
      */
      void reset();

      /**
      * Will reset the barrier to a new block count.  
      * It will also release all blocked threads and wait for their release
      * before resetting.
      *
      * @param maxCount is the max number of threads to block
      */
      void reset(ossim_int32 maxCount);

      /**
      * @return the maximum count
      */
      ossim_int32 getMaxCount()const;

      /**
      *  @return block count
      */
      ossim_int32 getBlockedCount()const;

   protected:
      ossim_int32              m_maxCount;
      std::atomic<ossim_int32> m_blockedCount;
      std::atomic<ossim_int32> m_waitCount;
      mutable std::mutex       m_mutex;
      std::condition_variable  m_conditionalBlock;

      /**
      * Will be used for destructing and resetting.
      * resetting should only happen in the main 
      * thread
      */ 
      std::condition_variable m_conditionalWait;
   };
}

#endif