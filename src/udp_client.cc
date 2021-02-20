/**
 * @file udp_client.cc
 *  @brief UDP client
 *  @author simon
 *  @date 2019-12-06
 *
 */

#include "udp_client.h"

using namespace kiimo::net;

UdpClient::UdpClient()
  :is_server_(false)
{
  client_.Initialize(SocketType::kUdpSocket);
}

UdpClient::UdpClient(int port)
  :is_server_(true)
{
  client_.Initialize(SocketType::kUdpSocket);
  client_.BindToPort(port);
}

UdpClient::UdpClient(IpEndPoint remote)
  :is_server_(false)
{
  client_.Initialize(SocketType::kUdpSocket);
  client_.Connect(remote);
}

UdpClient::~UdpClient()
{
}

void UdpClient::Connect(IpEndPoint remote)
{
  client_.Connect(remote);
}

int UdpClient::Send(const char *data,int size)
{
  return client_.Send(data,size);
}

int UdpClient::Send(const IpEndPoint remote,const char *data,int size)
{
  return client_.Send(remote,data,size);
}

int UdpClient::Recv(char *buffer,int max_size)
{
  return client_.Recv(buffer,max_size);
}

int UdpClient::Recv(IpEndPoint &remote,char *buffer,int max_size)
{
  return client_.Recv(remote,buffer,max_size);

}

void UdpClient::Close()
{
  client_.Close();
}

Socket::Id UdpClient::GetSocketId() const
{
  return client_.GetId();
}

IpEndPoint UdpClient::GetLocalHost() const
{
  return client_.GetLocalHost();
}



