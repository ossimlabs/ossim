/**
* This code was derived from https://gist.github.com/mshockwave
*
*/

#ifndef ossimRWm_lockHEADER
#define ossimRWm_lockHEADER

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <limits>

namespace ossim {

    enum WaitGenre {
        SPIN=0,
        SLEEP=1
    };
    /**
    * Code was derived from https://gist.github.com/mshockwave
    *
    * Has a pure c++11 implementation for read/write locks
    * allowing one to choose the locking technique to use.
    *
    * You currently can choose between a SLEEP or SPIN technique
    * for locking
    *
    * At the bottom we added typedefs so you do not have
    * to specify the template values.
    *
    *
    * Example:
    * @code
    *  ossim::RWSleepLock mutex;
    *  // enter a section that just needs read only access 
    *  {
    *       ossim::ScopeReadSleepLock  lock(mutex);
    *  }
    *  // enter some section that requires write
    *  {
    *      ossim::ScopeWriteSleepLock  lock(mutex); 
    *  }
    * @endcode
    */
    template <WaitGenre WAIT_GENRE>
    class RWLock {

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

    template <WaitGenre WAIT_GENRE>
    RWLock<WAIT_GENRE>::RWLock() :
            m_refCounter(0),
            MIN_INT(std::numeric_limits<int>::min()){}

    template <>
    void RWLock<SPIN>::lockWrite() {

        int expected = 0;
        while(!m_refCounter.compare_exchange_weak(expected, MIN_INT,
                                                  std::memory_order_acquire,
                                                  std::memory_order_relaxed)){
            expected = 0;
        }
    }
    template <>
    void RWLock<SLEEP>::lockWrite(){
        int expected = 0;
        if(!m_refCounter.compare_exchange_strong(expected, MIN_INT,
                                                 std::memory_order_acquire,
                                                 std::memory_order_relaxed)){
            expected = 0;

            std::unique_lock<std::mutex> lk(m_waitMutex);
            m_waitConditional.wait(lk, [this,&expected] {
                if(!m_refCounter.compare_exchange_strong(expected, MIN_INT,
                                                         std::memory_order_acquire,
                                                         std::memory_order_relaxed)){
                    expected = 0;
                    return false;
                }
                return true;
            });
            lk.unlock();
        }
    }

    template <WaitGenre WAIT_GENRE>
    bool RWLock<WAIT_GENRE>::tryLockWrite() {
        int expected = 0;
        return m_refCounter.compare_exchange_strong(expected, MIN_INT,
                                                    std::memory_order_acquire,
                                                    std::memory_order_relaxed);
    }

    template <WaitGenre WAIT_GENRE>
    void RWLock<WAIT_GENRE>::unlockWrite() {
        m_refCounter.store(0, std::memory_order_release);
        if(WAIT_GENRE == SLEEP)
            m_waitConditional.notify_all();
    }

    template <>
    void RWLock<SPIN>::lockRead() {
        while(m_refCounter.fetch_add(1, std::memory_order_acquire) < 0){
            m_refCounter.fetch_sub(1, std::memory_order_release);
        }
    }
    template <>
    void RWLock<SLEEP>::lockRead() {
        if(m_refCounter.fetch_add(1, std::memory_order_acquire) < 0){
            m_refCounter.fetch_sub(1, std::memory_order_release);

            std::unique_lock<std::mutex> lk(m_waitMutex);
            m_waitConditional.wait(lk, [this]{
                return m_refCounter.fetch_add(1, std::memory_order_acquire) >= 0;
            });
            lk.unlock();
        }
    }

    template <WaitGenre WAIT_GENRE>
    bool RWLock<WAIT_GENRE>::tryLockRead() {
        return m_refCounter.fetch_add(1, std::memory_order_acquire) >= 0;
    }

    template <WaitGenre WAIT_GENRE>
    void RWLock<WAIT_GENRE>::unlockRead() {
        m_refCounter.fetch_sub(1, std::memory_order_release);
        if(WAIT_GENRE == SLEEP)
            m_waitConditional.notify_one();
    }

    template <WaitGenre WAIT_GENRE>
    class ScopeReadLock {
        RWLock<WAIT_GENRE> &m_lock;
    public:
        ScopeReadLock(RWLock<WAIT_GENRE> &lock) : m_lock(lock) {
            m_lock.lockRead();
        }

        ~ScopeReadLock() {
            m_lock.unlockRead();
        }
    };

    template <WaitGenre WAIT_GENRE>
    class ScopeWriteLock {
        RWLock<WAIT_GENRE> &m_lock;
    public:
        ScopeWriteLock(RWLock<WAIT_GENRE> &lock) : m_lock(lock) {
            m_lock.lockWrite();
        }

        ~ScopeWriteLock() {
            m_lock.unlockWrite();
        }
    };

    typedef RWLock<SLEEP> RWSleepLock;
    typedef RWLock<SPIN>  RWSpinLock;
    typedef ScopeWriteLock<SLEEP> ScopeWriteSleepLock;
    typedef ScopeWriteLock<SPIN>  ScopeWriteSpinLock;
    typedef ScopeReadLock<SLEEP> ScopeReadSleepLock;
    typedef ScopeReadLock<SPIN>  ScopeReadSpinLock;

};

#endif