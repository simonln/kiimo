/**
 * @file time_queue.h
 *  @brief time queue implement
 *  @author simon
 *  @date 2019-08-28
 *
 */
#ifndef TIME_QUEUE_H_
#define TIME_QUEUE_H_
#include <list>
#include <functional>
#include <utility>
#include <string>

//#include "types.h"
#include "utils.h"

namespace kiimo{
  namespace base{
    typedef size_t TimerId;
    typedef std::function<void(void)> TimerCallback;
    struct Timer
    {
      /// name ,just for print or change the parameter
      std::string name;

      /// unit second
      int period;

      /// -1 :aways ,default once
      int times;

      /// call this function when is timeout
      TimerCallback callback;

    };

    /// Time queue
    class TimeQueue
    {
    public:
      TimeQueue() = default;
      ~TimeQueue() = default;
      /**
       * @brief Add timer to timer queue
       * @param name The name of timer
       * @param func Run this function after timer expired
       * @param period The period of timer
       * @param times The times of expire repeating
       *
       */
      TimerId  AddTimer(std::string name,TimerCallback func,int period,int times = 1);
      /// Add a timer
      TimerId  AddTimer(Timer time);
      /// Add a anonymous timer
      TimerId RunEvery(TimerCallback func,int second,int times = -1);
      /// Add a once timer
      TimerId  RunAfter(TimerCallback func,int second);
      /// Cancel the timer by name
      void CancelTimer(std::string name);
      /// Cancel the timer by id
      void CancelTimer(TimerId id);
      /// Pause the timer
      void PauseTimer(std::string name);
      /// Resume the timer be paused
      void ResumeTimer(std::string name);
      /**
       * @brief has timer in timer queue
       * @retval true someone timer in timer queue
       * @retval false no timer
       *
       */
      bool HasTimer() const;
      /**
       * @brief Get name by ID
       *
       */
      std::string GetNameById(TimerId id) const;
      /**
       *
       * @brief Get ID by name
       */
      TimerId GetIdByName(std::string name) const;

      /// called in loop
      void DoFunction();
    private:
      enum class TimerStatus{kRunning,kPause,kStop,};
      struct TimerItem
      {
        /// base timer
        Timer base;

        /// hash code
        TimerId id;

        /// when is timeout in system time
        TimeStamp timeout;

        /// status
        TimerStatus status;
      };

      std::list<TimerItem> events_;

    };



  } //namespace base
} //namespace kiimo

#endif /* TIME_QUEUE_H_ */
