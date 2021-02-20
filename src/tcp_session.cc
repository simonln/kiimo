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

using namespace kiimo::net;

TcpSession::TcpSession(IpEndPoint server,EventLoop *loop,Socket::Ptr con)
  :server_(server),loop_(loop),socket_(con),status_(Status::kConnected)
   //on_connect_(nullptr),on_disconnect_(nullptr),on_message_(nullptr)
{
  //recv_buff_.reserve(2048);  //value near by MTU(1500)
  //send_buff_.reserve(2048);
  on_connect_ = [](const TcpSession::Ptr&){};
  on_message_ = [](const TcpSession::Ptr&,const Message&){};
  on_disconnect_ = [](const TcpSession::Ptr&){};
}

//TcpClient::Ptr Connection::GetTcpClientPtr()
//{
// return client_;
//}

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

int TcpSession::Getdata(char *buffer,size_t max_size)
{
  int read_size = 0;
//  if(max_size > recv_buff_.size())
//  {
//    read_size = recv_buff_.size();
//  }
//  memcpy(buffer,recv_buff_.data(),read_size * sizeof(char));
//  recv_buff_.erase(recv_buff_.begin(),recv_buff_.begin() + read_size);
  return read_size;
}

int TcpSession::GetData(std::vector<char> &buffer)
{
//  buffer.assign(recv_buff_.begin(), recv_buff_.end());
//  recv_buff_.clear();
  return buffer.size();
}


void TcpSession::Shutdown()
{
  on_disconnect_(shared_from_this());
  socket_->Close();
  auto* event = new Event(socket_->GetId(), EventType::kAllEvent);
  loop_->RemoveEvent(event);
  delete event;
  
}
void TcpSession::AppendToBuffer(const char *buffer,int size)
{
//  recv_buff_.insert(recv_buff_.end(),buffer,buffer + size);
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
#ifdef USE_IOCP
  int size = loop_->SpecialRecv(socket_->GetId(), recv_buff_);
  if(size <= 0 )
  {
    if(on_disconnect_)
    {
      on_disconnect_(shared_from_this());
    }
    socket_->Close();
  }
  else
  {
    if(on_message_)
    {
      on_message_(shared_from_this(),recv_buff_);
    }
  }
#else

  //int size = socket_->Avaliable();
    int err = 0;
    size_t size = recv_buff_.ReadFromFD(socket_->GetId(), &err);
  if(size <= 0)
  {
      //if(on_disconnect_)
      //{
      //  on_disconnect_(shared_from_this());
      //}
      //socket_->Close();
      Shutdown();
  }
  else
  {

    //if(on_message_)
    {
      on_message_(shared_from_this(),recv_buff_);
    }
  }
#endif
}
void TcpSession::HandleWrite()
{
  if(send_buff_.size() > 0)
  {
#ifdef USE_IOCP
    loop_->SpecialSend(socket_->GetId(), send_buff_);
#else
    int n = socket_->Send(send_buff_.data(),send_buff_.size());
#endif
    send_buff_.Skip(n);

  }
  else
  {
    Event *event = new Event(socket_->GetId(),EventType::kWriteEvent);
    loop_->RemoveEvent(event);
    delete event;
  }
}
void TcpSession::HandleExcept()
{
  //if(on_disconnect_ != nullptr)
  //{
  //  on_disconnect_(shared_from_this());
  //}
  //socket_->Close();
    Shutdown();
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




