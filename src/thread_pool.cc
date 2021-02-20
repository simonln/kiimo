/**
 * @file thread_pool.cc
 *  @brief thread pool implement
 *  @author simon
 *  @date 2019-08-27
 *
 */

#include "thread_pool.h"

#include <functional>
#include <exception>
#include <utility>

#include "logger.h"

using namespace kiimo::base;
using EventLoop = kiimo::net::EventLoop;

ThreadPool::ThreadPool(EventLoop *base,int max_count)
  :next_(0)
{
  loops_.push_back(base);
  shutdown_ = false;
  for(int i = 0; i < max_count; i++)
  {
    EventLoop *loop = new EventLoop();
    threads_.emplace_back(
        //std::make_shared<Pthread>(std::bind(&ThreadPool::ThreadLoop,this,loop)));
        std::make_shared<std::thread>(std::bind(&ThreadPool::ThreadLoop, this, loop)));
    loops_.push_back(loop);
  }
}

//void ThreadPool::AddTask(ThreadPool::Task callback)
//{
//  {
//    LockGuard<Mutex> lock(mtx_);
//    task_list_.push_back(std::move(callback));
//  }
//  cv_.Notify();
//}

//bool ThreadPool::TaskEmpty()
//{
//  bool res = false;
//  {
//  LockGuard<Mutex> lock(mtx_);
//  res = task_list_.empty();
//  }
//  return res;
//}

void* ThreadPool::ThreadLoop(void* param)
{
  try
  {
//    while(true)
//    {
//      //must use while
//      while(!shutdown_ && this->TaskEmpty())
//      {
//        //no task,wait the new task
//        cv_.Wait();
//      }
//      if(!this->TaskEmpty())
//      {
//        Task task;
//        {
//        LockGuard<Mutex> lock(mtx_);
//        task = task_list_.front();
//        task_list_.pop_front();
//        }
//
//        if(task)
//        {
//          task();
//        }
//
//      }
//      else if(shutdown_)
//      {
//        break;
//      }
//    }
    EventLoop *loop = static_cast<EventLoop*>(param);
    loop->Loop();
  }
  catch(const std::exception &ex)
  {
    ErrorL << "exception caught in thread pool " << " reason: " << ex.what();
  }
  return nullptr;
}

EventLoop* ThreadPool::GetNextLoop()
{
  EventLoop *res = loops_[next_++];
  if(threads_.empty() || next_ >= loops_.size())
  {
    next_ = 0;
  }
  return res;
}

void ThreadPool::Shutdown()
{
  if(shutdown_)
  {
    return;
  }
  shutdown_ = true;

  //cv_.NotifyAll();
  cv_.notify_all();

  //wait all thread end
  for(auto &thread : threads_)
  {
    thread->join();
  }
  for(auto loop : loops_)
  {
    delete loop;
  }
}


//#ifdef UNITTEST
#if 0
// sleep_ms function
#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds){ // cross-platform sleep function
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}
#include <gtest/gtest.h>
Mutex mutex;
void Task1()
{
  mutex.Lock();
  std::cout <<"id: " << Pthread::GetCurrentThreadId() <<  " task1" << std::endl;
  mutex.Unlock();
  sleep_ms(500);
 }

void Task2()
{
  mutex.Lock();
  std::cout <<"id: " << Pthread::GetCurrentThreadId() <<  " task2" << std::endl;
  mutex.Unlock();
  sleep_ms(500); //500ms
}

void Task3()
{
  mutex.Lock();
  std::cout <<"id: " << Pthread::GetCurrentThreadId() <<  " task3" << std::endl;
  mutex.Unlock();
  sleep_ms(500);
}

class TestThreadPool
{
 public:
  void Func1()
  {
    mutex.Lock();
    std::cout <<"id: " << Pthread::GetCurrentThreadId() <<  " object task" << std::endl;
    mutex.Unlock();
  }
};

TEST(ThreadPool,Base)
{
 ThreadPool tp(2);
 TestThreadPool ttp;

 for(size_t i = 0 ;i < 2;i++)
 {
   tp.AddTask([]{Task1();});
   tp.AddTask([]{Task2();});
   tp.AddTask([]{Task3();});
 }
 tp.AddTask([&ttp]{ttp.Func1();});

 sleep_ms(500);
 tp.Shutdown();

}
#endif
