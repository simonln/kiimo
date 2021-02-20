/**
 * @file thread_pool.h
 * @brief Thread pool
 * @author simon
 * @date 2019-08-27
 *
 */
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <memory>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

//#include "thread.h"
#include "event_loop.h"

namespace kiimo{
  namespace base{

  /// Thread pool
  class ThreadPool {
    using EventLoop = net::EventLoop;
   public:
    //typedef std::shared_ptr<Pthread> ThreadPtr;
    typedef std::function<void(void)> Task;
    /**
     *  @brief Constructor
     *  @param max_count The thread number except main thread
     *
     */
    ThreadPool(EventLoop *base,int max_count = 0);
    ~ThreadPool() = default;
    /// Add task to thread pool
    //void AddTask(Task callback);
    EventLoop* GetNextLoop();

    /// Shutdown thread pool
    void Shutdown();

   private:
    void* ThreadLoop(void*);
    //bool TaskEmpty();
   private:
    //Condition cv_;
    //Mutex mtx_;
       std::condition_variable cv_;
       std::mutex mtx_;
    bool shutdown_;
    //std::vector<ThreadPtr> threads_;
    std::vector<std::shared_ptr<std::thread>> threads_;
    //std::deque<Task> task_list_;
    std::vector<EventLoop*> loops_;
    size_t next_;

  };



  } //namespace base
} //namespace kiimo

#endif /* THREAD_POOL_H_ */
