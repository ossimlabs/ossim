/**
* This code was derived from https://gist.github.com/mshockwave
*
*/
#ifndef ossimRWLockHEADER
#define ossimRWLockHEADER 1

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <limits>

namespace ossim {

    /**
    * Code was derived from https://gist.github.com/mshockwave
    *
    * Has a pure c++11 implementation for read/write locks
    * allowing one to choose the locking technique to use.
    *
    * At the bottom we added typedefs so you do not have
    * to specify the template values.
    *
    *
    * Example:
    * @code
    *  ossim::RWLock mutex;
    *  // enter a section that just needs read only access 
    *  {
    *       ossim::ScopeReadLock  lock(mutex);
    *  }
    *  // enter some section that requires write
    *  {
    *      ossim::ScopeWriteLock  lock(mutex); 
    *  }
    * @endcode
    */
    class RWLock {
    private:
        std::mutex m_waitMutex;
        std::condition_variable m_waitConditional;
        std::atomic_int m_refCounter;
        const int MIN_INT;

    public:

        RWLock();

        void lockWrite();

        bool tryLockWrite();

        void unlockWrite();

        void lockRead();

        bool tryLockRead();

        void unlockRead();
    };


    class ScopeReadLock {
        RWLock& m_lock;
    public:
        ScopeReadLock(RWLock &lock) : m_lock(lock) {
            m_lock.lockRead();
        }

        ~ScopeReadLock() {
            m_lock.unlockRead();
        }
    };

    class ScopeWriteLock {
        RWLock& m_lock;
    public:
        ScopeWriteLock(RWLock& lock) : m_lock(lock) {
            m_lock.lockWrite();
        }

        ~ScopeWriteLock() {
            m_lock.unlockWrite();
        }
    };
};

#endif