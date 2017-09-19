#include <ossim/base/Thread.h>

ossim::Thread::Thread()
:m_running(false),
 m_interrupt(false)
{
}

ossim::Thread::~Thread()
{
   if(m_thread)
   {
      // we will wait as a sanity but this should be done in derived Thread
      waitForCompletion();
      if(m_thread->joinable()) m_thread->join();
      m_thread = 0;         
   }
   m_running = false;
}

void ossim::Thread::start()
{
   if(isInterruptable()||isRunning()) return;
   m_running = true;

   // we are managing the thread internal.  If we are not running then we may need to join
   // before allocating a new thread
   if(m_thread)
   {
      if(m_thread->joinable()) m_thread->join();
   }

   m_thread = std::make_shared<std::thread>(&Thread::runInternal, this);
}

void ossim::Thread::waitForCompletion()
{
   if(m_thread)
   {
      std::unique_lock<std::mutex> lock(m_runningMutex);
      m_runningCondition.wait(lock, [&]{return !isRunning();} );
   }
}

void ossim::Thread::sleepInSeconds(ossim_uint64 seconds)
{
   std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void ossim::Thread::sleepInMilliSeconds(ossim_uint64 millis)
{
   std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

void ossim::Thread::sleepInMicroSeconds(ossim_uint64 micros)
{
   std::this_thread::sleep_for(std::chrono::microseconds(micros));
}

std::thread::id ossim::Thread::getCurrentThreadId()
{
   return std::this_thread::get_id();
}

void ossim::Thread::yieldCurrentThread()
{
    std::this_thread::yield();
}

void ossim::Thread::interrupt()
{
   if(m_interrupt)
   {
      throw ossim::Thread::Interrupt();
   }
}

void ossim::Thread::setInterruptable(bool flag)
{
   m_interrupt.store(flag, std::memory_order_relaxed);
}

void ossim::Thread::runInternal()
{
   try
   {
      if(!isInterruptable())
      {
         run();
      }
   }
   catch(Interrupt& e)
   {
   }
   m_running = false;
   m_runningCondition.notify_all();
}

