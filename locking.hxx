#ifndef LOCKING_HXX
#define LOCKING_HXX

/**
 * helper class to ensure correct locking and unlocking of
 * the lock placed on a target
 */
template <typename Lock> class LockGuard
{
    Lock &mLock;

public:
    LockGuard(Lock &l)
        : mLock( l )
    {
        l.lock();
    }
    
    ~LockGuard()
    {
        mLock.unlock();
    }
};

/**
 * empty lock type doing no locking at all
 */
struct NullLock
{
    void lock()
    {
    }
    
    void unlock()
    {
    }
};

#endif // LOCKING_HXX