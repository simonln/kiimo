/**
 * @file poller.cc
 * @brief Select ,SingleSelect implement
 * @author simon
 * @date 2019-08-05
 *
 */

#include "poller.h"
#include <cstring>
#ifdef linux
#include <unistd.h>
#endif

#ifdef _DEBUG
#include "logger.h"
#include "utils.h"
using namespace kiimo::base;
#endif
using namespace kiimo::net;


#if (linux || _WIN32_WINNT >= 0x0600)
auto Poller::HasEvent(Socket::Id id) -> decltype(events_.begin())
{
  for(auto it = events_.begin(); it != events_.end(); it++)
  {
    if(it->fd == id)
    {
      return it;
    }
  }
  return events_.end();

}
void Poller::Add(Socket::Id id, EventType type)
{
  auto it = HasEvent(id);
  if (it != events_.end())
  {
    return;
  }
  struct pollfd new_event {id, 0, 0};
  if(type & EventType::kReadEvent)
  {
    new_event.events |= POLLRDNORM;
  }
  if(type & EventType::kWriteEvent)
  {
    new_event.events |= POLLWRNORM;
  }
  events_.emplace_back(new_event);

}

void Poller::Update(Socket::Id id,EventType type)
{
  auto it = HasEvent(id);
  if(it != events_.end())
  {
    if((type & EventType::kReadEvent) && !(it->events & POLLRDNORM))
    {
      it->events |= POLLRDNORM;
    }
    if((type & EventType::kWriteEvent) && !(it->events & POLLWRNORM))
    {
      it->events |= POLLWRNORM;
    }
  }
}

void Poller::Remove(Socket::Id id,EventType type)
{
  auto it = HasEvent(id);
  if(it != events_.end())
  {
    if((type & EventType::kReadEvent) && (it->events & POLLRDNORM))
    {
      it->events &= ~POLLRDNORM;
    }
    if((type & EventType::kWriteEvent) && (it->events & POLLWRNORM))
    {
      it->events &= ~POLLWRNORM;
    }
    if(!(it->events))
    {
      events_.erase(it);
    }
  }
}
std::map<Socket::Id,int> Poller::Wait(int timeout)
{
  std::map<Socket::Id,int> actives;
  int active_size = 0;
#ifdef _WIN32
  active_size = WSAPoll(events_.data(),events_.size(),timeout);
#else
  active_size = poll(events_.data(),events_.size(),timeout);
#endif
  if(active_size > 0)
  {
    for(auto &it : events_)
    {
      if(it.revents)
      {
        int event = EventType::kNoneEvent;
        //actives.emplace(it.fd,EventType::kNoneEvent);

        if((it.revents & POLLRDNORM))
        {
          event |= EventType::kReadEvent;
        }
        if(it.revents & POLLWRNORM)
        {
          event |= EventType::kWriteEvent;
        }
        if (it.revents & POLLERR || it.revents & POLLHUP)
        {
            //if (it.revents & POLLHUP)
            {
                //shutdown,continue read stop write
                //event = EventType::kReadEvent;
                //Remove(it.fd, EventType::kReadEvent);
            }
            //else
            {
                event |= EventType::kExceptEvent;
            }
            //DebugL << "socket id: " << it.fd << " event: " << it.events << " revent: " << it.revents << " errro: " << GetLastError();
            //DebugL << "socket id: " << it.fd << " event: " << it.events << " revent: " << it.revents;

        }
        actives.emplace(it.fd,event);
      }
    }
  }
  return actives;
}
#endif


Epoller::Epoller()
  : epoll_working_(false)
{
  epoll_fd_ = epoll_create(1); // 1 is meanless
  if (epoll_fd_ > 0)
  {
    epoll_working_ = true;
  }
}

Epoller::~Epoller()
{
  if (epoll_working_)
  {
  #if _WIN32
    epoll_close(epoll_fd_);
  #else
    close(epoll_fd_);
  #endif
  }
}

void Epoller::UpdateCtrl(Socket::Id id, EventType type, Opr op)
{
  if (!epoll_working_)
  {
    return;
  }
  struct epoll_event ev {0, 0};
  if (type & kReadEvent)
  {
    ev.events |= EPOLLIN;
  }
  if (type & kWriteEvent)
  {
    ev.events |= EPOLLOUT;
  }
  if (type & kExceptEvent)
  {
    ev.events |= EPOLLERR;
    ev.events |= EPOLLHUP;
  }
  ev.data.fd = id ;
  int epoll_op = 0;
  switch (op)
  {
  case kAdd:
    {
      epoll_op = EPOLL_CTL_ADD;
    }
    break;
  case kUpdate:
    {
      epoll_op = EPOLL_CTL_MOD;
    }
    break;
  case kRemove:
    {
      epoll_op = EPOLL_CTL_DEL;
    }
    break;
  default:
    break;
  }
  int ret = epoll_ctl(epoll_fd_, epoll_op, id, &ev);
  //DebugL << Fmt::Format("ret: {0} error: {1}", ret, errno);
}
void Epoller::Add(Socket::Id sock, EventType event)
{
  UpdateCtrl(sock, event, kAdd);
}
void Epoller::Update(Socket::Id sock, EventType event)
{
  UpdateCtrl(sock, event, kUpdate);
}

void Epoller::Remove(Socket::Id sock, EventType event)
{
  UpdateCtrl(sock, event, kRemove);
}

std::map<Socket::Id, int> Epoller::Wait(int timeout)
{
#define MAX_EPOLL_EVENTS (1024)
  struct epoll_event events[MAX_EPOLL_EVENTS];
  int nfd = 0;
  std::map<Socket::Id,int> actives;

  nfd = epoll_wait(epoll_fd_, events, MAX_EPOLL_EVENTS, timeout);
  if (nfd <= 0)
  {
    return actives;
  }
  for(int i = 0; i < nfd; ++i)
  {
    int event = EventType::kNoneEvent;

    if(events[i].events & EPOLLIN)
    {
      event |= EventType::kReadEvent;
    }
    if(events[i].events & EPOLLOUT)
    {
      event |= EventType::kWriteEvent;
    }
    if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
    {
        
      event |= EventType::kExceptEvent;
        
      //DebugL << "socket id: " << it.fd << " event: " << it.events << " revent: " << it.revents << " errro: " << GetLastError();
      //DebugL << "socket id: " << events[i].data.fd << " event: " << it.events << " revent: " << it.revents;

    }
    Socket::Id sock = events[i].data.fd;
    actives.emplace(sock, event);
#ifdef _DEBUG
    //DebugL << Fmt::Format("socket id: {0} event: {1}", events[i].data.fd, events[i].events);
#endif
  }
  return actives;

}

void Select::Update(Socket::Id sock,EventType event)
{
  switch(event)
  {
    case kReadEvent:
    {
      if(read_set_.find(sock) == read_set_.end())
      {
        read_set_.emplace(sock);
      }
    }
    break;
    case kWriteEvent:
    {
      if(write_set_.find(sock) == write_set_.end())
      {
        write_set_.emplace(sock);
      }
    }
    break;
    case kExceptEvent:
    {
      if(except_set_.find(sock) == except_set_.end())
      {
        except_set_.emplace(sock);
      }
    }
    break;
    default:
      break;
  }
}

void Select::Remove(Socket::Id sock,EventType event)
{
  switch(event)
  {
    case kReadEvent:
    {
      if(read_set_.find(sock) != read_set_.end())
      {
        read_set_.erase(sock);
      }
    }
    break;
    case kWriteEvent:
    {
      if(write_set_.find(sock) != write_set_.end())
      {
        write_set_.erase(sock);
      }
    }
    break;
    case kExceptEvent:
    {
      if(except_set_.find(sock) != except_set_.end())
      {
        except_set_.erase(sock);
      }
    }
    break;
    default:
      break;
  }
}

std::map<Socket::Id,int> Select::Wait(int millsecond)
{
  std::map<Socket::Id,int> res;
  struct timeval tv {0,millsecond*1000};
  Socket::Id max_id = -1;

  FD_ZERO(&fdread_);
  for(auto id : read_set_)
  {
    FD_SET(id,&fdread_);
#ifndef _WIN32
    //for linux
    if(id > max_id)
    {
      max_id = id;
    }
#endif
  }

  FD_ZERO(&fdwrite_);
  for(auto id : write_set_)
  {
    FD_SET(id,&fdwrite_);
#ifndef _WIN32
    //for linux
    if(id > max_id)
    {
      max_id = id;
    }
#endif
  }

  FD_ZERO(&fdexcept_);
  for(auto id : except_set_)
  {
    FD_SET(id,&fdexcept_);
#ifndef _WIN32
    //for linux
    if(id > max_id)
    {
      max_id = id;
    }
#endif
  }
  res.clear();
  if(::select(max_id + 1,&fdread_,&fdwrite_,&fdexcept_,&tv) > 0)
  {
    for(auto id : read_set_)
    {
      if(FD_ISSET(id,&fdread_))
      {
        res.emplace(id,kReadEvent);
      }
    }

    for(auto id : write_set_)
    {
      if(FD_ISSET(id,&fdwrite_))
      {
        auto ptr = res.find(id);
        if(ptr != res.end())
        {
          ptr->second |= kWriteEvent;
        }
        else
        {
        res.emplace(id,kWriteEvent);
      }

    }
    }

    for(auto id : except_set_)
    {
      if(FD_ISSET(id,&fdexcept_))
      {
        auto ptr = res.find(id);
        if(ptr != res.end())
        {
          ptr->second |= kExceptEvent;
        }
        else
        {
          res.emplace(id,kExceptEvent);
        }

      }
    }
  }
  return res;
}


SingleSelect::SingleSelect(Socket::Id id,int event)
{
  wait_id_ = id;
  wait_events_ = event;
}

void SingleSelect::Update(EventType event)
{
  wait_events_ |= event;
}

void SingleSelect::Remove(EventType event)
{
  wait_events_ &=  ~event;
}

int SingleSelect::Wait()
{
  struct timeval timeout = {0 , kTimeout};
  int res = EventType::kNoneEvent;
  for(auto &fd : fds_)
  {
    FD_ZERO(&fd);
  }
  if(wait_events_ & EventType::kReadEvent)
  {
    FD_SET(wait_id_,&fds_[0]);
  }
  if(wait_events_ & EventType::kWriteEvent)
  {
    FD_SET(wait_id_,&fds_[1]);
  }
  if(wait_events_ & EventType::kExceptEvent)
  {
    FD_SET(wait_id_,&fds_[2]);
  }
  if(::select(wait_id_ + 1,&fds_[0],&fds_[1],&fds_[2],&timeout) > 0)
  {
    if(FD_ISSET(wait_id_,&fds_[0]))
    {
      res |= EventType::kReadEvent;
    }
    if(FD_ISSET(wait_id_,&fds_[1]))
    {
      res |= EventType::kWriteEvent;
    }
    if(FD_ISSET(wait_id_,&fds_[2]))
    {
      res |= EventType::kExceptEvent;
    }
  }
  return res;
}


