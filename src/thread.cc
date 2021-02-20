/**
 * @file thread.cc
 * @brief Pthread warp
 * @author simon
 * @date 2019-07-23
 *
 */

#ifdef __GNUC__ 

#include "thread.h"

#include <stdexcept>
#include <memory>
#include <utility>

using namespace kiimo::base;

/// Running in sub-thread
static void *RunInThread(void *param)
{
  Pthread::Param *ptr = static_cast<Pthread::Param*>(param);
  if(ptr->function)
  {
    ptr->function(static_cast<void*>(ptr->param));
  }
  delete ptr;
  return nullptr;
}

Pthread::Pthread(Func func,ThreadParam param)
{
  Param *ptr = new Param{func,param};

  pthread_create(&tid_,NULL,&RunInThread,ptr);
}

Pthread::Pthread(Func func)
  :Pthread(func,nullptr)
{
}

ThreadId Pthread::GetThreadId()
{
  return tid_;
}

ThreadId Pthread::GetCurrentThreadId()
{
  return pthread_self();
}


void Pthread::Join()
{
  pthread_join(tid_,NULL);
}

void Pthread::Detach()
{
  pthread_detach(tid_);
}

void Pthread::Exit()
{
  pthread_exit(NULL);
}

Mutex::Mutex()
{
  if(0 != pthread_mutex_init(&mtx_,nullptr))
  {
    throw std::runtime_error("In constructor of Mutex failed");
  }
}

Mutex::~Mutex()
{
  pthread_mutex_destroy(&mtx_);
}

void Mutex::Lock()
{
  if(0 != pthread_mutex_lock(&mtx_))
  {
    throw std::runtime_error("Mutex lock failed");
  }
}

bool Mutex::TryLock()
{
  bool status = false;
  auto res = pthread_mutex_trylock(&mtx_);
  if(res == 0)
  {
    status = true;
  }
  else if(res == EBUSY)
  {
    status = false; //mutex is held by other thread
  }
  else
  {
    throw std::runtime_error("Mutex try lock failed,already locked");
  }
  return status;
}
void Mutex::Unlock()
{

  if(0 != pthread_mutex_unlock(&mtx_))
  {
    throw std::runtime_error("Mutex unlock failed");
  }
}


Condition::Condition()
{
  pthread_mutex_init(&mutex_,NULL);
  pthread_cond_init(&thread_hold_,NULL);
}

Condition::~Condition()
{
  pthread_mutex_destroy(&mutex_);
  pthread_cond_destroy(&thread_hold_);
}

void Condition::Wait()
{
  pthread_mutex_lock(&mutex_);
  pthread_cond_wait(&thread_hold_,&mutex_);
  pthread_mutex_unlock(&mutex_);
}

void Condition::Notify()
{
  pthread_mutex_lock(&mutex_);
  pthread_cond_signal(&thread_hold_);
  pthread_mutex_unlock(&mutex_);
}

void Condition::NotifyAll()
{
  pthread_mutex_lock(&mutex_);
  pthread_cond_broadcast(&thread_hold_);
  pthread_mutex_unlock(&mutex_);
}


/*
 * Read Write lock implement
 *
 * */

ReadWriteLock::ReadWriteLock()
{
  pthread_rwlock_init(&rwlock_,nullptr);
}

ReadWriteLock::~ReadWriteLock()
{
  pthread_rwlock_destroy(&rwlock_);
}

void ReadWriteLock::ReadLock()
{
  pthread_rwlock_rdlock(&rwlock_);
}

void ReadWriteLock::WriteLock()
{
  pthread_rwlock_wrlock(&rwlock_);
}

void ReadWriteLock::Unlock()
{
  pthread_rwlock_unlock(&rwlock_);
}



#endif
