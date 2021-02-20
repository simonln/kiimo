/**
 * @file udp_client.h
 *  @brief UDP client
 *  @author simon
 *  @date 2019-12-06
 *
 */

#ifndef UDP_CLIENT_H_
#define UDP_CLIENT_H_

#include "socket.h"


namespace kiimo {
namespace net {

/*
 *
 */
class UdpClient
{
 public:
  typedef std::shared_ptr<UdpClient> Ptr;

  /// initializes a new instance of UdpClient
  UdpClient();

  /// initializes a new instance of UdpClient and binds it to local port number provided
  UdpClient(int port);

  /// initializes a new instance of UdpClient and establishes a default remote host
  UdpClient(IpEndPoint remote);

  ~UdpClient();

  /// connects remote host
  void Connect(IpEndPoint remote);

  /// send to the default remote host
  int Send(const char *data,int size);

  /// send to the host provided
  int Send(const IpEndPoint remote,const char *data,int size);

  /// receive from default remote host
  int Recv(char *buffer,int max_size);

  /// receive from host
  int Recv(IpEndPoint &remote,char *buffer,int max_size);


  void Close();
  /// Get socket ID
  Socket::Id GetSocketId() const;
  /// Get local host information
  IpEndPoint GetLocalHost() const;

 private:
  bool is_server_;
  Socket client_;


};

} /* namespace net */
} /* namespace kiimo */

#endif /* UDP_CLIENT_H_ */
