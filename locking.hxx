#ifndef LOCKING_HXX
#define LOCKING_HXX

/**
 * empty lock type doing no locking at all
 * Use this lock in situations, where only a single thread will access the logger
 * and you want to save the extra effort of locking a mutex.
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