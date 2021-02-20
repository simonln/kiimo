/**
 * @file event_loop.cc
 * @brief event loop implement
 * @author simon
 * @date 2019-12-09
 *
 */

#include "event_loop.h"


using namespace kiimo::base;
using namespace kiimo::net;


//UdpEvent implement
UdpEvent::UdpEvent(UdpClient::Ptr con)
  :Event(con->GetSocketId(),EventType::kNoneEvent),link_(con),on_message_(nullptr)
{
}

UdpEvent::UdpEvent(UdpClient::Ptr con,ReadCallback cb)
  :Event(con->GetSocketId(),EventType::kReadEvent),link_(con),on_message_(cb)
{
}

void UdpEvent::SetOnMessage(ReadCallback cb)
{
  on_message_ = cb;
  this->wait_event |= EventType::kReadEvent;
}

void UdpEvent::ReadHandler()
{
  char *buffer = new char[1024];
  IpEndPoint remote;
  int recv_size = link_->Recv(remote,buffer,1024);
  if(on_message_ != nullptr && recv_size > 0)
  {
    //Message msg(buffer,buffer + recv_size);
    //on_message_(link_->GetLocalHost(),msg);
  }
  delete[] buffer;
}

//TcpEvent implement

TcpEvent::TcpEvent(TcpSession::Ptr con,EventType kind)
  :Event(con->GetSocketId(),kind)
{

  link_ = con;
//  on_connect_ = nullptr;
//  on_disconnect_ = nullptr;
//  on_message_ = nullptr;

}

//TcpEvent::TcpEvent(TcpSession::Ptr con,
//                   ConnectCallback on_connect,
//                   ConnectCallback on_disconnect,
//                   MessageCallback on_message)
//  :Event(con->GetSocketId(),Select::kReadEvent)
//{
//  this->wait_event |= Select::kExceptEvent;
//  link_ = con;
//  on_connect_ = on_connect;
//  on_disconnect_ = on_disconnect;
//  on_message_ = on_message;
//}

//void TcpEvent::SetOnConnect(ConnectCallback cb)
//{
//  on_connect_ = cb;
//  this->wait_event = Select::kReadEvent;
//}
//void TcpEvent::SetOnDisconnect(ConnectCallback cb)
//{
//  on_disconnect_ = cb;
//  this->wait_event = Select::kReadEvent | Select::kExceptEvent;
//}
//void TcpEvent::SetOnMessage(MessageCallback cb)
//{
//  on_message_ = cb;
//  this->wait_event = Select::kReadEvent;
//}

void TcpEvent::ReadHandler()
{
  link_->HandleRead();
}

void TcpEvent::WriteHandler()
{
  link_->HandleWrite();
}

void TcpEvent::ExceptHandler()
{
  link_->HandleExcept();
}


//EventLoop implement

EventLoop::EventLoop()
  :quit_(false)
{
}

#ifdef USE_IOCP
void EventLoop::InitIocp(const Socket::Id id)
{
  select_.Init(id);
}
void EventLoop::ReleaseIocp()
{
  select_.Release();
}
Socket::Ptr EventLoop::SpecialAccept()
{
  return select_.SpecialAccept();
}
int EventLoop::SpecialSend(const Socket::Id id,const std::vector<char> &buff)
{
  return select_.SpecialSend(id,buff);
}
int EventLoop::SpecialRecv(const Socket::Id id,std::vector<char> &buff)
{
  return select_.SpecialRecv(id,buff);
}
#endif

#if 0
bool EventLoop::HasEvent(Event *event)
{
  for(auto it = events_.begin();it != events_.end();++it)
  {
    if((it->first == event->id) && (it->second->wait_event & event->wait_event))
    {
      return true;
    }
  }
  return false;
}
#endif
EventExist EventLoop::HasEvent(Event *event)
{
  auto it = events_.find(event->id);
  if (it != events_.end())
  {
    if (it->second->wait_event & event->wait_event)
    {
      return EventExist::kEvent;
    }
    else
    {
      return EventExist::kId;
    }
  }
  return EventExist::kNone;
}
void EventLoop::UpdateEvent(Event *event)
{
  EventExist state = HasEvent(event);
  if (state == EventExist::kEvent)
  {
    // already exist
    return;
  }
  
  if (state == EventExist::kNone)
  {
    // insert event
    select_.Add(event->id, EventType(event->wait_event));
  }
  else
  {
    // id exist
    select_.Update(event->id, EventType(event->wait_event));
  }
  InsertEvent(event);

}

void EventLoop::InsertEvent(Event *event)
{
  auto it = events_.find(event->id);
  if (it != events_.end())
  {
      if(event->wait_event & EventType::kReadEvent)
      {
        it->second->wait_event |= EventType::kReadEvent;
      }
      if(event->wait_event & EventType::kWriteEvent)
      {
        it->second->wait_event |= EventType::kWriteEvent;
      }
      if(event->wait_event & EventType::kExceptEvent)
      {
        it->second->wait_event |= EventType::kExceptEvent;
      }
      return;
  }
  events_.emplace(event->id,event);
}

void EventLoop::RemoveEvent(Event *event)
{
  for(auto it = events_.begin();it != events_.end();++it)
  {
    if(event->id == it->first)
    {
      it->second->wait_event &=  ~event->wait_event; //erase event
      if(it->second->wait_event != EventType::kNoneEvent)
      {
        select_.Update(it->first, EventType(it->second->wait_event));
      }
      else
      {  
        //no wait event ,delete this event from list
        delete (it->second);
        events_.erase(it);
        break;
      }
    }
  }
}

void EventLoop::RunInLoop(Function cb)
{
  //timer_.RunAfter(cb,0);
  if(cb != nullptr)
  {
    pending_work_.push_back(cb);
  }

}

TimerId EventLoop::RunAfter(Function func,int second)
{
  return timer_.RunAfter(func,second);
}
TimerId EventLoop::RunEvery(Function func,int second)
{
  return timer_.RunEvery(func,second);
}

void EventLoop::Cancle(TimerId id)
{
  timer_.CancelTimer(id);
}

void EventLoop::Loop()
{
  quit_ = false;
//  if(events_.size() <= 0)
//  {
//    return;
//  }
#ifdef USE_IOCP
  std::pair<Socket::Id,int> actives;
#else
  std::map<Socket::Id,int> actives;
#endif
  while(!quit_)
  {
    actives = select_.Wait();
    //if(actives.size() > 0)
    {
#ifdef USE_IOCP
      auto it = events_.find(actives.first);
      if(it != events_.end())
      {
        if(actives.second & EventType::kReadEvent)
        {
          it->second->ReadHandler();
        }
        if(actives.second & EventType::kWriteEvent)
        {
            it->second->WriteHandler();
        }
        if(actives.second & EventType::kExceptEvent)
        {
            it->second->ExceptHandler();
        }
      }
#else
      for(auto it = actives.begin() ; it != actives.end() ; ++it)
      {


        if(it->second & EventType::kReadEvent)
        {
            events_[it->first]->ReadHandler();
        }

        if(it->second & EventType::kWriteEvent)
        {
            events_[it->first]->WriteHandler();
        }

        if (it->second & EventType::kExceptEvent)
        {
            events_[it->first]->ExceptHandler();
        }


      }
#endif
    }

    timer_.DoFunction();  //500ms

    if(pending_work_.size() > 0)
    {
      for(auto it : pending_work_)
      {
        it();
      }
      pending_work_.clear();
    }

  }
}




