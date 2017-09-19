#ifndef ossimThread_HEADER
#define ossimThread_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

namespace ossim{

   /**
   * Thread is an abstract class.  It provides a 
   * general purpose thread interface that handles preliminary setup
   * of the std c++11 thread.  It allows one to derive
   * from Thread and override the run method.  Your thread should have calls
   * to interrupt() whenever your thread is in a location that is
   * interruptable.  If cancel is called then any thread that is interruptable will 
   * throw an Interrupt and be caught in the base Thread class and then exit 
   * the thread.
   *
   * Example:
   * @code
   * #include <ossim/base/Thread.h>
   * #include <ossim/base/Barrier.h>
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
   *            // simulate 10 milliseconds of uninterruptable work
   *            sleepInMilliSeconds(10);
   *            interrupt();
   *         }
   *         barrierFinished.block();
   *      }
   *   };
   * int main(int agrc, char* argv[])
   * {
   *    std::vector<std::shared_ptr<TestThread> > threads(nThreads);
   *    for(auto& thread:threads)
   *    {
   *      thread = std::make_shared<TestThread>();
   *      thread->start();
   *    }
   *
   *    // now let's wait for each thread to finish 
   *    // before exiting
   *    for(auto& thread:threads)
   *    {
   *      thread->waitForCompletion();
   *    }
   * }
   * @endCode
   */ 
   class OSSIM_DLL Thread
   {
   public:
      /**
      * This is an Interrupt exception that is thrown if the @see cancel()
      * is called and a call to @see interrupt() is made.
      */
      class Interrupt : public std::exception{
      public:
         Interrupt(const std::string& what=""):m_what(what){}
         virtual const char* what() const _NOEXCEPT{return m_what.c_str();}
      protected:
         std::string m_what;
      };
      /**
      * Constructor for this thread
      */
      Thread();

      /**
      * Destructor for this thread.  It will determine if this thread is joinable
      * to the main thread and if so it will do a join before continuing.  If
      * this is not done then an exeption is thrown by the std.
      */
      virtual ~Thread();

      /**
      * Will actually start the thread and will call the @see internalRun.
      */
      void start();

      /**
      * @return true if the current thread is running and false otherwise.
      */
      bool isRunning()const{return m_running.load(std::memory_order_relaxed);}

      /**
      * This is typically set if @see cancel() is called or if @see setCancel
      * is called with argument set to true.
      *
      * @return true if the thread is interruptable and false otherwise.
      */
      bool isInterruptable()const{return m_interrupt.load(std::memory_order_relaxed);}

      /**
      * This basically requests that the thread be canceled.  @see setCancel.  Note,
      * cancellation is not done immediately and a thread is only cancleed if derived
      * classes call the interrupt().
      *
      * we will make these virtual just in case derived classes want to set conditions
      */
      virtual void cancel(){setCancel(true);}

      /**
      * @param flag if true will enable the thread to be interruptable and if false
      *        the thread is not interruptable.
      */
      virtual void setCancel(bool flag){setInterruptable(flag);}

      /**
      * Convenience to allow one to wait for this thread to finish it's work.
      *
      * Allow this to be overriden.
      */
      virtual void waitForCompletion();

      /**
      * Utility method to allow one to sleep in seconds
      *
      * @param seconds to sleep
      */
      static void sleepInSeconds(ossim_uint64 seconds);

      /**
      * Utility method to allow one to sleep in milliseconds
      *
      * @param millis to sleep
      */
      static void sleepInMilliSeconds(ossim_uint64 millis);

      /**
      * Utility method to allow one to sleep in microseconds
      *
      * @param micros to sleep
      */
      static void sleepInMicroSeconds(ossim_uint64 micros);

      /**
      * Utility method to get the current thread ID
      *
      * @return current thread ID
      */
      static std::thread::id getCurrentThreadId();

      /**
      * Utility to return the number of processors  (concurrent threads)
      */
      static ossim_uint64 getNumberOfProcessors();
      /**
      * Will yield the current thread.
      */
      static void yieldCurrentThread();

   protected:
      /**
      * This method must be overriden and is the main entry
      * point for any work that needs to be done
      */
      virtual void run()=0;

      /**
      * This is the interrupt interface and will cause an internal exception that
      * is caught by @see runInternal
      */
      virtual void interrupt();

      /**
      * runInternal sets up internal flags such as setting m_running to true and checks
      * to make sure it's not interrupted and will then call the @see run() method.
      *
      * runInternal also will trap any Interrupt exceptions.  If the thread is interruptable
      * and the work calls interrupt then an exception is thrown and the work is stopped and 
      * the execution of the thread is marked as not running and returns.
      */
      virtual void runInternal();

   private:
      std::shared_ptr<std::thread>  m_thread;
      std::atomic<bool>             m_running;
      std::atomic<bool>             m_interrupt;
      std::condition_variable       m_runningCondition;
      mutable std::mutex            m_runningMutex;

      /**
      * @see cancel and @see setCancel
      */
      void setInterruptable(bool flag);
   };
}

#endif
