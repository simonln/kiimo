/**
 * @file connection.cc
 * @brief Represent connection
 * @author simon
 * @date 2019-09-09
 *
 */

#include "tcp_session.h"

#include <cstring>

#include "event_loop.h"

#include "logger.h" // for log
using namespace kiimo::base;

using namespace kiimo::net;

TcpSession::TcpSession(IpEndPoint server,EventLoop *loop,Socket::Ptr con)
  :server_(server),loop_(loop),socket_(con),status_(Status::kConnected)
{
  // set default value for callback function
  on_connect_ = [](const TcpSession::Ptr&){};
  on_message_ = [](const TcpSession::Ptr&,const Message&){};
  on_disconnect_ = [](const TcpSession::Ptr&){};
}

int TcpSession::Send(const char *buffer , int size)
{
  if(size <= 0)
  {
    return 0;
  }
  else
  {
    send_buff_.Append(buffer, size);
    Event *event = new Event(socket_->GetId(),EventType::kWriteEvent);
    loop_->UpdateEvent(event);
    delete event;
    return size;
    //return socket_->Send(buffer,size);
  }

}

int TcpSession::Send(std::vector<char> &buffer)
{
  return Send(buffer.data(),buffer.size());
}

void TcpSession::Shutdown()
{
  if (recv_buff_.size() > 0)
  {
    status_ = Status::kDisconnect;
  }
}

void TcpSession::Termination()
{
  on_disconnect_(shared_from_this());
  
  auto* event = new Event(socket_->GetId(), EventType::kAllEvent);
  loop_->RemoveEvent(event);
  delete event;
  socket_->Close();

}

Socket::Id TcpSession::GetSocketId() const
{
  return socket_->GetId();
}
Socket::Ptr TcpSession::GetSocket()
{
  return socket_;
}

void TcpSession::HandleRead()
{
  int err = 0;
  size_t size = recv_buff_.ReadFromFD(socket_->GetId(), &err);
  if(size > 0)
  {
    on_message_(shared_from_this(),recv_buff_);
      
  }
  else if (size == 0)
  {
      Termination();
  }
  else
  {
    ErrorL << "read socket error: " << errno;
  }
}
void TcpSession::HandleWrite()
{
  int n = socket_->Send(send_buff_.data(),send_buff_.size());
  if(n > 0)
  {
    send_buff_.Retrieve(n);
    if(send_buff_.size() == 0)
    {
      DisableWriting();
      if (status_ == Status::kDisconnect)
      {
        on_disconnect_(shared_from_this());
        socket_->Close();
      }
    }
  }
}
void TcpSession::HandleExcept()
{
    Termination();
}

void TcpSession::SetOnConnect(const ConnectCallback &cb)
{
  if(cb != nullptr)
  {
    on_connect_ = cb;
  }
}
void TcpSession::SetOnClose(const ConnectCallback &cb)
{
  if(cb != nullptr)
  {
    on_disconnect_ = cb;
  }
}
void TcpSession::SetOnMessage(const MessageCallback &cb)
{
  if(cb != nullptr)
  {
    on_message_ = cb;
  }
}

void TcpSession::DisableWriting()
{
    Event *event = new Event(socket_->GetId(),EventType::kWriteEvent);
    loop_->RemoveEvent(event);
    delete event;
}


