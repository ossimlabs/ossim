#ifndef ossimBlock_HEADER
#define ossimBlock_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <mutex>
#include <condition_variable>

namespace ossim{

   class OSSIM_DLL Block
   {
   public:
      Block(bool releaseFlag=false);
      ~Block();
      void set(bool releaseFlag);
      void block();
      void block(ossim_uint64 waitTimeMillis);

      void release();
      void reset();

   private:
      mutable std::mutex      m_mutex;
      std::atomic<bool>       m_release;
      std::condition_variable m_conditionVariable;
   };
}
#endif