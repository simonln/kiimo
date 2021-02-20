/**
 * @file time_queue.cc
 *  @brief time queue implement
 *  @author simon
 *  @date 2019-08-28
 *
 */


#include "time_queue.h"

#include <random>


#ifdef UNITTEST
#include <gtest/gtest.h>
#endif

//#include "utils.h"
//#include "logger.h"

using namespace kiimo::base;
using std::string;


TimerId TimeQueue::AddTimer(string name,TimerCallback func,int period,int times)
{
  Timer tim{name,period,times,func};
  return this->AddTimer(tim);
}

TimerId TimeQueue::AddTimer(Timer time)
{
  if((time.callback == nullptr) || (time.name == "") || (time.period < 0))
  {
    return 0; //invalid
  }
  TimeStamp now = TimeStamp::Now();
  for(auto &item : events_)
  {
    if(item.base.name == time.name)
    {
      item.base = time;
      item.timeout = now + time.period * 1000;
      item.status = TimerStatus::kRunning;
      return item.id;
    }
  }

  //no same name timer exist in events_
  std::mt19937 rd(now.GetMillisecond());
  TimerId timer_id;

  //generate an unique id
  do
  {
    timer_id = std::hash<std::string>{}(time.name + std::to_string(rd()));
  }while(this->GetNameById(timer_id) != "");

  events_.emplace_back(TimerItem{
    time,
    timer_id,
    now + time.period * 1000,
    TimerStatus::kRunning});
  return timer_id;
}

/*
 * anonymous time task
 * func: the task will be run
 * second: time ,unit of second
 * */
TimerId TimeQueue::RunAfter(TimerCallback func,int second)
{
  return RunEvery(func,second,1);
}

TimerId TimeQueue::RunEvery(TimerCallback func,int second ,int times)
{
  std::mt19937 rd(TimeStamp::Now().GetMillisecond());
  string name = "";

  do
  {
    name = "time_" + std::to_string(rd());

  }while(this->GetIdByName(name) != 0);

  return this->AddTimer(name,func,second,times);
}

void TimeQueue::CancelTimer(string name)
{
  for(auto it = events_.begin();it != events_.end();it++)
  {
    if(it->base.name == name)
    {
      it->status = TimerStatus::kStop;
      break;
    }
  }
}

void TimeQueue::CancelTimer(TimerId id)
{
  for(auto it = events_.begin();it != events_.end();it++)
  {
    if(it->id == id)
    {
      it->status = TimerStatus::kStop;
      break;
    }
  }
}

void TimeQueue::PauseTimer(string name)
{
  for(auto &item : events_)
  {
    if(item.base.name == name)
    {
      item.status = TimerStatus::kPause;
      break;
    }
  }
}

void TimeQueue::ResumeTimer(string name)
{
  for(auto &item : events_)
  {
    if(item.base.name == name)
    {
      item.status = TimerStatus::kRunning;
    }
  }
}


bool TimeQueue::HasTimer() const
{
  int count = 0;
  for(auto &item : events_)
  {
    if(item.status != TimerStatus::kStop)
    {
      count++;
    }
  }
  return (count > 0);
}

string TimeQueue::GetNameById(TimerId id) const
{
  string res = "";
  for(auto &item : events_)
  {
    if(item.id == id)
    {
      res = item.base.name;
      break;
    }
  }
  return res;
}

TimerId TimeQueue::GetIdByName(string name) const
{
  TimerId res = 0;
  for(auto &item : events_)
  {
    if(item.base.name == name)
    {
      res = item.id;
      break;
    }
  }
  return res;
}

void TimeQueue::DoFunction()
{
  TimeStamp now = TimeStamp::Now();

  for(auto it  = events_.begin();it != events_.end();)
  {
    if(it->status == TimerStatus::kStop)
    {
      it = events_.erase(it); //only delete timer on here
    }
    else if(it->status == TimerStatus::kPause)
    {
      it++;
    }
    else
    {
      if(it->timeout > now)
      {
        it++;
      }
      else
      {
        //std::cout << "name:" << it->base.name << " timeout:" << it->timeout.ToString() << std::endl;
        //timeout
        if(it->base.callback)
        {
          it->base.callback();
        }
        if(it->base.times > 1)
        {
          it->base.times--;
          it->timeout =  now + it->base.period * 1000;
          it++;
        }
        else if(it->base.times == 1)
        {
          it->status = TimerStatus::kStop; //mark stop
        }
        else
        {
          //aways run until cancle by manualy
          it->timeout = now + it->base.period * 1000;
          it++;
        }
      }
    }

  }

}

#ifdef UNITTEST

TimeQueue tq;
TimerId timer_id  = 0;
std::string result = "";
void Callback1()
{
  //Need resource lock in multithread environment
  result += "1";
}

void Callback2()
{
  result += "2";
}

void Callback3()
{
  tq.CancelTimer("test2");
  result += "3";
}

void Callback4()
{
  result += "4";
}

//TEST(TimeQueue,Add)
//{
//
//  timer_id = tq.AddTimer("test1",std::bind(Callback1),2);
//  EXPECT_EQ(tq.GetIdByName("test1"),timer_id);
//
//  tq.AddTimer("test1",std::bind(Callback1),5);
//  EXPECT_EQ(tq.GetIdByName("test1"),timer_id);
//  EXPECT_EQ(tq.GetNameById(timer_id),"test1");
//
//  Timer timer{"test2",2,3,std::bind(Callback2)};
//  tq.AddTimer(timer);
//
//  tq.AddTimer("test3",std::bind(Callback3),5);
//
//  //3
//  for(size_t i=0;i<3;i++)
//  {
//    timer_id = tq.RunAfter(std::bind(Callback4),1);
//  }
//  //3
//  timer_id = tq.RunAfter(std::bind(Callback4),1);
//  EXPECT_NE(tq.RunAfter(std::bind(Callback4),1),timer_id);
//  EXPECT_NE(tq.RunAfter(std::bind(Callback4),1),timer_id);
//
//  EXPECT_EQ(tq.RunEvery(std::bind(Callback1),-1) , (TimerId)0);
//
//
//  while(true)
//  {
//    tq.DoFunction();
//    if(tq.HasTimer() == false)
//    {
//      break;
//    }
//  }
//  //result 4 4 4 4 4 4 2 2 1 3
//  EXPECT_STREQ(result.c_str(),"4444442213");
//
//}
#endif


