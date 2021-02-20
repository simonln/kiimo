/**
 * @file thread.h
 * @brief Pthread warp
 * @author simon
 * @date 2019-07-23
 *
 */

#ifdef __GNUC__ 
#ifndef PTHREAD_H_
#define PTHREAD_H_
#include <pthread.h>
#include <functional>


namespace kiimo{
  namespace base{

    typedef pthread_t ThreadId;

    /// pthread wrap
    class Pthread
    {
     public:
      typedef std::function<void*(void*)> Func;
      typedef void* ThreadParam;
      /// parameter structure in thread
      struct Param
      {
        Func function;
        ThreadParam param;
      };
      /// Get current thread ID of running this function
      static ThreadId GetCurrentThreadId();
      Pthread(Func func,ThreadParam param);
      Pthread(Func func);
      ~Pthread() = default;
      /// Get current thread ID
      ThreadId GetThreadId();
      void Join();
      void Detach();
      void Exit();

     private:
      Pthread();
    private:
      pthread_t tid_;
    };

    /// pthread mutex wrap
    class Mutex
    {
     public:
      Mutex();
      virtual ~Mutex();
      /// lock the mutex,block the context
      void Lock();
      /**
       * @brief Try lock the mutex
       * @note
       *  This function return immediately
       * @retval true success
       * @retval false failed
       *
       */
      bool TryLock();
      void Unlock();
      Mutex(const Mutex&) = delete;
      /// forbidden copy
      void operator=(const Mutex&) = delete;
    private:
      pthread_mutex_t mtx_ ;/*= nullptr;  invaild in linux */
    };

    /// pthread condition wrap
    class Condition
    {
      public:
      Condition();
      ~Condition();
      /// Wait the condition meet
      void Wait();
      /// Notify another thread,the condition happen
      void Notify();
      /// Notify others thread ,the condition happen
      void NotifyAll();

      private:
      pthread_mutex_t mutex_;
      pthread_cond_t thread_hold_;
    };

    /// pthread read write lock wrap
    class ReadWriteLock
    {
     public:
      ReadWriteLock();
      ~ReadWriteLock();
      void ReadLock();
      void WriteLock();
      void Unlock();
     private:
       pthread_rwlock_t rwlock_;
    };

    /// c++ standard library lock_gurad wrap
    template<class MutexType>
    class LockGuard
    {
     public:
      explicit LockGuard(MutexType &mutex)
      {
        mutex_ = &mutex;
        mutex_->Lock();
      }
      ~LockGuard()
      {
        mutex_->Unlock();
      }
      MutexType *InternalMutex()
      {
        return mutex_;
      }

      /// Cannot be copied
      LockGuard(const LockGuard &) = delete;

      /// Disable the = operator
      void operator=(LockGuard&) = delete;

     private:
      MutexType *mutex_;

    };


  } //namespace base
} //namespace kiimo

#endif /* PTHREAD_H_ */

#endif
