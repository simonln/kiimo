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

#if linux
Epoller::Epoller()
  :epoll_fd_(-1), epoll_working_(false)
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
    close(epoll_fd_);
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
    //DebugL << Fmt::Format("socket id: {0} event: {1}", events[i].data.fd, events[i].events);
  }
  return actives;

}
#endif

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

#if ( _WIN32 && USE_IOCP)
bool Iocp::Init(Socket::Id listen_sock)
{
  int res = true;
  listen_state_ = new CompletionState {listen_sock,CompletionOp::kAccepting,0,new char[128]};
  port_ = CreateIoCompletionPort((HANDLE)listen_sock,NULL,(ULONG_PTR)listen_state_,0);
  if(port_ == nullptr)
  {
    res = false;
  }
  // load acceptex
  DWORD bytes = 0;
  GUID accept_guid = WSAID_ACCEPTEX;

  // get acceptex function pointer
  WSAIoctl(listen_sock,SIO_GET_EXTENSION_FUNCTION_POINTER,&accept_guid,sizeof(accept_guid),&acceptex_,sizeof(acceptex_),
           &bytes,NULL,NULL);
  // get getacceptexsockaddr function pointer
  GUID getaddr_guid = WSAID_GETACCEPTEXSOCKADDRS;
  WSAIoctl(listen_sock,SIO_GET_EXTENSION_FUNCTION_POINTER,&getaddr_guid,sizeof(getaddr_guid),&getaddrex_,sizeof(getaddrex_),
           &bytes,NULL,NULL);

//  Socket::Id accept_sock = ::socket(AF_INET,SOCK_STREAM,0);
//
//  listen_state_->id = accept_sock;
//  //listen_state_->op = CompletionOp::kAccept;
//  ::memset(&listen_ovl_, 0,sizeof(listen_ovl_));
//  DWORD expect_len = sizeof(struct sockaddr_in) + 16;
//
//  if(!acceptex_(listen_sock,accept_sock,addr_buff_, 0,expect_len,expect_len,nullptr,&listen_ovl_))
//  {
//    res = false;
//  }
  listen_sock_ = listen_sock;
  if(!StartAccept())
  {
    res = false;
  }
  //DebugL << "init result: " << res;
  //accept_sock_= accept_sock;
  return res;
}
void Iocp::Update(Socket::Id id,EventType type)
{
  auto it = HasLink(id);
  if(it != links_.end())
  {
    // first add read event
    if((type & EventType::kReadEvent) && ((*it)->focus & EventType::kReadEvent) == 0)
    {
      StartReading(*it);
    }
    (*it)->focus |= type;
  }
}

void Iocp::Remove(Socket::Id id,EventType type)
{
  auto it = HasLink(id);
    if(it != links_.end())
    {
      (*it)->focus &= ~type;
    }
}

//int Iocp::Wait(CompletionState *state)
//{
//  DWORD length;
//  auto res = GetQueuedCompletionStatus(port_,&length,(PULONG_PTR)&state,&res_ovl_,kTimeout);
//  state->length = length;
//  return res;
//}

std::pair<Socket::Id,int> Iocp::Wait(int millsecond)
{
  DWORD length = 0 ;
  CompletionState *state = nullptr;
  WSAOVERLAPPED *ovl = NULL;
  std::pair<Socket::Id,int> ret;
  auto res =  GetQueuedCompletionStatus(port_,&length,(PULONG_PTR)&state,&ovl,kTimeout);
  if((res == TRUE) && (state != nullptr) && (ovl != NULL))
  {
    switch(state->op)
    {
      case CompletionOp::kAccepting:
      {
        state->op = CompletionOp::kAccepted;
        state->length = length;
        // update context
        setsockopt(state->id,SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT,(char*)&listen_sock_,sizeof(listen_sock_));
        CompletionState *client_state = new CompletionState {state->id,CompletionOp::kNone,0,new char[kBufferSize * 2]};
        CreateIoCompletionPort((HANDLE)client_state->id, port_ ,(ULONG_PTR)client_state, 0);
        links_.push_back(client_state);
        ret = std::make_pair(listen_sock_,EventType::kReadEvent);  // listen socket read event
        //start accept;
        //StartAccept();
        //DebugL << "Accepting socket id:" << state->id;
      }
      break;
      case CompletionOp::kReading:
      {
        state->op = CompletionOp::kRead;
        if(state->focus & EventType::kReadEvent)
        {
          state->length = length;
          ret = std::make_pair(state->id,EventType::kReadEvent);
          if(length != 0)
           {
              FixReceiveBuffer(state);
              StartReading(state);
           }
        }
        // close socket
        if(length == 0)
        {
          links_.remove(state);
          delete state->buffer;
          delete state;
        }
      }
      break;
      case CompletionOp::kWriting:
      {
        state->op = CompletionOp::kWrited;
        if(state->focus & EventType::kWriteEvent)
        {
          FixSendBuffer(state);
          ret = std::make_pair(state->id,EventType::kWriteEvent);
        }
      }
      break;
      default:
        break;
    }
  }
  else
  {
    int err = GetLastError();
    if(err != WAIT_TIMEOUT)
    {
      //closesocket(state->id);
    DebugL << "wait completion packet error: " << err;
    }
  }
  return ret;

}

void Iocp::Release()
{
  delete listen_state_;
}

Socket::Ptr Iocp::SpecialAccept()
{
  Socket::Ptr client = nullptr;
  if(listen_state_->op == CompletionOp::kAccepted)
  {
    struct sockaddr *local;
    struct sockaddr *remote;
    int sock_len = sizeof(local);
    DWORD expect_len = sizeof(struct sockaddr_in) + 16;
    getaddrex_(&listen_state_,listen_state_->length,expect_len,expect_len,
                         &local,&sock_len,&remote,&sock_len);
    IpEndPoint remote_addr {(struct sockaddr *)&remote};
    client = std::make_shared<Socket>(listen_state_->id,remote_addr);
    StartAccept();
  }
  return client;
}
// run after write event
int Iocp::SpecialSend(const Socket::Id id , const std::vector<char> &buff)
{
  int ret = -1;
  auto it= HasLink(id);
  if(it != links_.end())
  {
    CompletionState *state = *it;
      // save data to cache
    int remain = kBufferSize - state->send_len;
    ret = (buff.size() > (size_t)remain)?remain:buff.size();
    memcpy(state->buffer + kBufferSize + state->send_len,buff.data(),ret);

    // async send all data
    state->async_send_end = ret + state->send_len;
    WSABUF wsabuf = { (ULONG)(state->async_send_end) , state->buffer + kBufferSize};
    memset(&res_ovl_,0,sizeof(WSAOVERLAPPED));
    state->op = CompletionOp::kWriting;
    if(WSASend(state->id,&wsabuf,1,NULL,0,&res_ovl_,NULL) == SOCKET_ERROR)
    {
      ret = -1;
    }
  }
  return ret;
}

// run after read event
int Iocp::SpecialRecv(const Socket::Id id,std::vector<char> &buff)
{
  int ret = 0;  //default socket close
  auto it = HasLink(id);
  if(it != links_.end())
  {
    CompletionState *state = *it;
//    if(state->op == CompletionOp::kReading)
//    {
//      if(state->length <= 0)
//      {
//        buff.clear();
//        ret = 0;
//      }
//      else
//      {
//        ret = state->length;
//        buff.resize(ret);
//        memcpy(buff.data(),state->buffer,ret);
//        state->op = CompletionOp::kRead;  //receive over
//      }

//    }
    ret = state->recv_len;
    buff.resize(ret);
    memcpy(buff.data(),state->buffer,ret);
    state->recv_len = 0;
  }
  return ret;
}


bool Iocp::StartAccept()
{
  Socket::Id accept_sock = ::socket(AF_INET,SOCK_STREAM,0);
  //Socket::Id accept_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

  listen_state_->id = accept_sock;
  listen_state_->op = CompletionOp::kAccepting;
  ::memset(&listen_ovl_, 0,sizeof(listen_ovl_));
  DWORD expect_len = sizeof(struct sockaddr_in) + 16;

  if(!acceptex_(listen_sock_,accept_sock,listen_state_->buffer, 0 ,expect_len,expect_len,NULL,&listen_ovl_))
  {
    int err = WSAGetLastError();
    if(err != ERROR_IO_PENDING)
    {
      DebugL << "AcceptEx function failed : " << err;
      return false;
    }
  }
  return true;
}

auto Iocp::HasLink(Socket::Id id) -> decltype(links_.begin())
{
  for(auto it = links_.begin() ;it != links_.end(); it++)
  {
    if((*it)->id == id)
    {
      return it;
    }
  }
  return links_.end();
}

bool Iocp::StartReading(CompletionState *state)
{
  DWORD flags = 0;
  WSABUF wsabuf = {ULONG(kBufferSize - state->recv_len),state->buffer + state->async_recv_start};
  memset(&res_ovl_,0,sizeof(WSAOVERLAPPED));
  state->op = CompletionOp::kReading;
  //state->length = -1;
  if(WSARecv(state->id,&wsabuf,1,NULL,&flags,&res_ovl_,NULL) == SOCKET_ERROR)
  {
    int err = GetLastError();
    if(err != WSA_IO_PENDING)
    {
      DebugL << "Run WSARecv failed: " << err;
      return false;
    }
  }
  return true;
}

void Iocp::FixReceiveBuffer(CompletionState *state)
{
  if(state->length <= 0)
  {
    return;
  }
  if(state->recv_len != state->async_recv_start)
  {
    memmove(state->buffer + state->recv_len,state->buffer + state->async_recv_start,state->length);
  }
  state->recv_len += state->length;
  state->async_recv_start = state->recv_len;
}
void Iocp::FixSendBuffer(CompletionState *state)
{
  if(state->length <= 0)
  {
    return;
  }
  if(state->length < state->async_send_end)
  {
    char *send_buff_start = state->buffer + kBufferSize;
    memmove(send_buff_start,send_buff_start + state->length,state->async_send_end - state->length);
  }
  state->recv_len = state->async_send_end - state->length;
}

#endif


