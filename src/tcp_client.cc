/**
 * @file tcp_client.cc
 * @brief TcpClient class implement
 * @author simon
 * @date 2019-09-03
 *
 */
#include "tcp_client.h"

#include <map>

#include "poller.h"


using namespace kiimo::net;
//using namespace kiimo::base;



TcpClient::TcpClient()
  :socket_(new Socket())
{
  socket_->Initialize();
  connected_ = false;
}

TcpClient::TcpClient(Socket::Ptr con)
  :socket_(con),connected_(true)
{
}

TcpClient::TcpClient(std::string remote,int port)
  :socket_(new Socket())
{
  socket_->Initialize();
  socket_->Connect(IpEndPoint(remote,port));
  connected_ = true;
}


void TcpClient::Connect(IpEndPoint remote)
{
  if(!connected_)
  {
    socket_->Connect(remote);
    connected_ = true;
  }

}

void TcpClient::Connect(std::string remote,int port)
{
  if(!connected_)
  {
    socket_->Connect(IpEndPoint(remote,port));
    connected_ = true;
  }
}


bool TcpClient::Connected() const
{
  return connected_;
}

int TcpClient::Avaliable()
{
  return socket_->Avaliable();
}


int TcpClient::Send(const char *data,int size)
{
  return socket_->Send(data,size);
}

int TcpClient::Receive(char *buffer,int max_size)
{
  return socket_->Recv(buffer,max_size);
}

void TcpClient::Close()
{
  socket_->Close();
}

Socket::Id TcpClient::GetSocketId() const
{
  return socket_->GetId();
}

IpEndPoint TcpClient::GetRemoteHost()
{
  if(connected_)
  {
    return socket_->GetRemoteHost();
  }
  else
  {
    return IpEndPoint();
  }
}







