#include <atomic>

class Spinlock {
    std::atomic_flag _flag;
public:
    Spinlock() : _flag{ATOMIC_FLAG_INIT} {
    }

    Spinlock(Spinlock const&) = delete;
    Spinlock& operator=(Spinlock const&) = delete;

    //Locks the mutex. If another thread has already locked the mutex, a call to lock will block execution until the lock is acquired.
    void lock() {
        while(_flag.test_and_set(std::memory_order_acquire))
            ;   // spin
    }
    //Tries to lock the mutex. Returns immediately. On successful lock acquisition returns true, otherwise returns false.
    bool try_lock() {
        return !_flag.test_and_set(std::memory_order_acquire);
    }
    //Unlocks the mutex.
    //The mutex must be locked by the current thread of execution, otherwise, the behavior is undefined.
    void unlock() {
        _flag.clear(std::memory_order_release);
    }
};

