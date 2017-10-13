#include <ossim/base/RWLock.h>

ossim::RWLock::RWLock() :
        m_refCounter(0),
        MIN_INT(std::numeric_limits<int>::min())
{

}

void ossim::RWLock::lockWrite()
{
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

bool ossim::RWLock::tryLockWrite() 
{
    int expected = 0;
    return m_refCounter.compare_exchange_strong(expected, MIN_INT,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed);
}

void ossim::RWLock::unlockWrite() 
{
    m_refCounter.store(0, std::memory_order_release);
    m_waitConditional.notify_all();
}

void ossim::RWLock::lockRead() 
{
    if(m_refCounter.fetch_add(1, std::memory_order_acquire) < 0){
        m_refCounter.fetch_sub(1, std::memory_order_release);

        std::unique_lock<std::mutex> lk(m_waitMutex);
        m_waitConditional.wait(lk, [this]{
            return m_refCounter.fetch_add(1, std::memory_order_acquire) >= 0;
        });
        lk.unlock();
    }
}

bool ossim::RWLock::tryLockRead() 
{
    return m_refCounter.fetch_add(1, std::memory_order_acquire) >= 0;
}

void ossim::RWLock::unlockRead() 
{
    m_refCounter.fetch_sub(1, std::memory_order_release);
    m_waitConditional.notify_one();
}
