#ifndef ossimBlock_HEADER
#define ossimBlock_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace ossim{

   /**
   * This is a very simple block interface.  This allows one to control 
   * how their threads are blocked
   *
   * There is a release state flag that tells the call to block to block the calling
   * thread or release the thread(s) that are currently blocked
   *
   * For a very simple use case we will start a thread and call block and have
   * the main sleep for 2 seconds before releasing the thread
   * @code
   * #include <ossim/base/Block.h>
   * #include <ossim/base/Thread.h>
   * std::shared_ptr<ossim::Block> block = std::make_shared<ossim::Block>();
   * class TestThread : public ossim::Thread
   * {
   * public:
   * 
   * protected:
   *    virtual void run(){
   *      block->block();
   *       std::cout << "STARING!!!!!!!\n";
   *    }
   * };
   * int main(int argc, char *argv[])
   * {
   *    TestThread t1;
   *    t1.start();
   * 
   *    std::cout << "WAITING 2 SECOND to release block\n";
   *    ossim::Thread::sleepInSeconds(2);
   *    block->release();
   *    ossim::Thread::sleepInSeconds(2);
   * }
   * @endcode
   */
   class OSSIM_DLL Block
   {
   public:
      /**
      * Allows one the construct a Block with a release state.
      */
      Block(bool releaseFlag=false);

      /**
      * Destructor
      *
      * Will set internally call release
      */
      ~Block();

      /**
      * Will set the relase flag and wake up all threads to test the condition again.
      */
      void set(bool releaseFlag);

      /**
      * Will block the calling thread based on the internal condition.  If the internal
      * condition is set to release then it will return without blocking.
      */
      void block();

      /**
      * Will block the calling thread base on the internal condition.  If the internal 
      * condition is set to release the it will return without blocking.  If the internal
      * condition is set to not release then it will block for the specified time in 
      * milliseconds
      *
      * @param waitTimeMillis specifies the amount of time to wait for the release
      */
      void block(ossim_uint64 waitTimeMillis);

      /**
      * Releases the threads and will not return until all threads are released
      */
      void release();

      /**
      * Simple reset the values.  Will not do any releasing
      */
      void reset();

   private:
      /**
      * Used by the conditions
      */
      mutable std::mutex      m_mutex;

      /**
      * The release state.
      */
      std::atomic<bool>       m_release;

      /**
      * Condition that tests the release state
      */
      std::condition_variable m_conditionVariable;

      /**
      * Used to count the number of threads blocked or
      * waiting on the condition
      */
      std::atomic<ossim_int32> m_waitCount;

      /**
      * Will be used for destructing and releasing.
      * resetting should only happen in the main 
      * thread
      */ 
      std::condition_variable m_conditionalWait;
   };
}
#endif