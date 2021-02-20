/**
 *
 * @file socket.cc
 * @brief IpEndPoint ,Socket class implement
 * @author simon
 * @date 2019-08-02
 *
 */

#include "socket.h"

#include <cstring>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#endif

#ifdef UNITTEST
#include <gtest/gtest.h>
#endif

using namespace kiimo::net;
//using namespace kiimo::base;

const char *IpEndPoint::kAny = "0.0.0.0";

using std::string;

IpEndPoint::IpEndPoint()
{
  ::memset(&host_addr_,0,sizeof(host_addr_));
}

IpEndPoint::IpEndPoint(const std::string ip,const int port)
{

  host_addr_.sin_family = AF_INET;
  if(port <= UINT16_MAX && port > 0)
  {
    host_addr_.sin_port = htons(port);
  }
  else
  {
    host_addr_.sin_port = 0;
  }

#ifdef _WIN32
  host_addr_.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
#else
      host_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
#endif

}

IpEndPoint::IpEndPoint(struct sockaddr* address)
{
  ::memcpy(&host_addr_,address,sizeof(struct sockaddr));
}

string IpEndPoint::GetIp() const
{
  string str = "";
  str += string(inet_ntoa(host_addr_.sin_addr));
  //str += ":" + to_string(ntohs(host_addr_.sin_port));
  return str;
}

int IpEndPoint::GetPort() const
{
  int port = ntohs(host_addr_.sin_port);
  return port;
}
string IpEndPoint::GetPortString() const
{
  string str = "";
  str += std::to_string(ntohs(host_addr_.sin_port));
  return str;
}

string IpEndPoint::ToString() const
{
  string res = "";
  res += this->GetIp();
  res += ":";
  res += this->GetPortString();
  return res;
}

const struct sockaddr* IpEndPoint::GetRawStruct() const
{
  return ((struct sockaddr*)&host_addr_);
}


//socket implement

Socket::Socket()
  :is_server_(false),from_accept_(false),id_(0),kind_(SocketType::kTcpSocket),connected_(false)
{
};

Socket::Socket(Id id,IpEndPoint remote)
  :is_server_(false),from_accept_(true),id_(id),
   remote_(remote),kind_(SocketType::kTcpSocket),connected_(true)
{
};

Socket::Socket(int port)
  :is_server_(true),from_accept_(false),id_(0),kind_(SocketType::kTcpSocket),connected_(false)
{
  local_ = IpEndPoint(IpEndPoint::kAny,port);
};


void Socket::Initialize(SocketType kind)
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,2),&wsa_data);
#endif
    if(kind == SocketType::kTcpSocket)
    {
      id_ = ::socket(AF_INET,SOCK_STREAM,0);
    }
    else
    {
      id_ = ::socket(AF_INET,SOCK_DGRAM,0);
    }
    kind_ = kind;

    if(id_ == 0)
    {
      throw SocketException(SocketErrCode::kInitfailed,"Initialize socket failed");
    }
}

bool Socket::BindToPort()
{
  if(local_.GetPortString() != "0")
  {
    int ret = -1;
    ret = ::bind(id_,local_.GetRawStruct(),sizeof(struct sockaddr));
    if(ret == 0)
    {
      if(kind_ == SocketType::kTcpSocket)
      {
        ret = ::listen(id_,0);
      }
      return ((ret == 0)?true:false);
    }
  }
  return false;
}
bool Socket::BindToPort(int port)
{
//  if(!is_server_)
//  {
//    return false;
//  }

  int ret = -1;
  IpEndPoint info(IpEndPoint::kAny,port);
  ret = ::bind(id_,info.GetRawStruct(),sizeof(struct sockaddr));
  if(ret == 0)
  {
    is_server_ = true;
    local_ = info;
    if(kind_ == SocketType::kTcpSocket)
    {
      ret = ::listen(id_,0);
    }
    return ((ret == 0)?true:false);
  }
  return false;
}



Socket::Ptr Socket::Accept()
{
  if(!is_server_)
  {
    return nullptr;
  }
  struct sockaddr_in addr;
#ifdef _WIN32
  int len = sizeof(addr);
#else
  socklen_t len = sizeof(addr);
#endif
  Id client = ::accept(id_,(struct sockaddr*)&addr,&len);
  remote_ = IpEndPoint((struct sockaddr*)&addr);
  connected_  = true;
  return std::make_shared<Socket>(client,remote_);
}

void Socket::Connect(IpEndPoint remote)
{
  if(is_server_)
  {
    return;
    //return false;
  }
  if(::connect(id_,remote.GetRawStruct(),sizeof(struct sockaddr)) < 0)
  {
    throw SocketException(SocketErrCode::kConnectFailed,"Connect remote failed");
    //return false;
  }
  else
  {
    remote_ = remote;
    connected_ = true;
  }
}

// get the size of receive buffer in system
int Socket::Avaliable()
{
  unsigned long size = 0;
#ifdef _WIN32
  // this is normally the same as the total amount of data queued on the socket (since a data stream is byte-oriented, this is not guaranteed).
  // https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-ioctls
  if(0 == ::ioctlsocket(id_,FIONREAD,&size))
#else
  if(0 == ::ioctl(id_,FIONREAD,&size))
#endif  
  {
    return size;
  }
  else
  {
    return -1;  // failed
  }
}

int Socket::Recv(char *buffer,int max_size)
{
  if(!connected_)
  {
    throw SocketException(SocketErrCode::kDisconnect,"Receive failed,Disconnect with Remote");
  }
  int ret = 0;
  ret = ::recv(id_,buffer,max_size,0);
  return ret;
}

int Socket::Recv(IpEndPoint &remote,char *buffer,int max_size)
{
 struct sockaddr addr;
#ifdef _WIN32
 int len = sizeof(addr);
#else
 socklen_t len = sizeof(addr);
#endif
 int ret = 0;
 ret = ::recvfrom(id_,buffer,max_size,0,&addr,&len);
 remote = IpEndPoint(&addr);
 return ret;
}

int Socket::Send(const char *buffer,int size)
{
  if(!connected_)
  {
    throw SocketException(SocketErrCode::kDisconnect,"Send failed,Disconnect with Remote");
  }
  return ::send(id_,buffer,size,0);
}

int Socket::Send(const IpEndPoint remote,const char* buffer,int size)
{

  return ::sendto(id_,buffer,size,0,
                  remote.GetRawStruct(),sizeof(struct sockaddr));
}

void Socket::Close()
{
  if(id_ != 0 )
  {
#ifdef _WIN32
    ::closesocket(id_);
#else
    ::close(id_);
#endif
    connected_ = false;
  }
  if(!from_accept_)
  {
#ifdef _WIN32
  //WSACleanup();
#endif
  }
}

bool Socket::Connected() const
{
  return connected_;
}

const IpEndPoint Socket::GetLocalHost() const
{
  return local_;
}

const IpEndPoint Socket::GetRemoteHost() const
{
  return remote_;
}
const Socket::Id Socket::GetId() const
{
  return id_;
}

SocketType Socket::GetType() const
{
  return kind_;
}


bool Socket::operator < (const Socket &a) const
{
  return (this->id_ < a.GetId());
}


#if UNITTEST

TEST(IpEndPoint,Construct)
{
  IpEndPoint ip;
  EXPECT_EQ(ip.GetIp(),"0.0.0.0");
  EXPECT_EQ(ip.GetPortString(),"0");

  IpEndPoint ip1("",0);
  EXPECT_EQ(ip1.GetIp(),"255.255.255.255");
  EXPECT_EQ(ip1.GetPortString(),"0");

  IpEndPoint ip2("255.255.255.255",100000);

  EXPECT_EQ(ip2.GetIp(),"255.255.255.255");
  EXPECT_EQ(ip2.GetPortString(),"0");
//  struct sockaddr_in *temp = (struct sockaddr_in*)(ip2.GetRawStruct());
//  ASSERT_TRUE(temp != nullptr);
//  EXPECT_STREQ(inet_ntoa(temp->sin_addr),"255.255.255.255");
//  EXPECT_EQ(ntohs(temp->sin_port),0);
}
TEST(Socket,Constructor)
{
  Socket socket;
  EXPECT_EQ(socket.GetType(),SocketType::kTcpSocket);
  EXPECT_TRUE(socket.GetId() == 0);
  EXPECT_FALSE(socket.BindToPort());

  Socket socket2(3,IpEndPoint("0.0.0.1",1234));
  EXPECT_EQ(socket2.GetType(),SocketType::kTcpSocket);
  EXPECT_TRUE(socket2.GetId() == 3);
  EXPECT_EQ(socket2.GetRemoteHost().GetIp(),"0.0.0.1");
  EXPECT_EQ(socket2.GetRemoteHost().GetPortString(),"1234");
  EXPECT_FALSE(socket2.BindToPort());

  Socket socket3(0);
  EXPECT_TRUE(socket3.GetId() == 0);
  socket3.Initialize();
  EXPECT_FALSE(socket3.GetId() == 0);

}
#endif

