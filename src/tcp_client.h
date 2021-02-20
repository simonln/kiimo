/**
 * @file tcp_client.h
 * @brief TCP client
 * @author simon
 * @date 2019-09-03
 *
 */

#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <memory>
#include <string>

#include "types.h"
#include "socket.h"

namespace kiimo{
  namespace net{

    /// Sending and receiving buffer size in TCP transmit
    static const int kBufferMaxSize = 1024;
    /// TCP client class
    class TcpClient
    {
     public:
      typedef std::shared_ptr<TcpClient> Ptr;

      /// initializes a new TcpClient instance ,default
      TcpClient();

      /// initializes a new TcpClient instance from TcpSever
      TcpClient(Socket::Ptr con);

      /// initializes a new TcpClient instance and connects to specified on specified host
      TcpClient(std::string remote,int port);

      ~TcpClient() = default;

      /**
       *  Connect to remote host
       */
      void Connect(IpEndPoint remote);
      void Connect(std::string remote,int port);

      /**
       *  @brief Indicate the connecting status
       *  @retval true connected
       *  @retval false disconnect
       */
      bool Connected() const;

      /**
       *  Get the size of receive buffer,byte of unit
       *
       */
      int Avaliable();

      /**
       * @brief Send data to remote host
       * @param [in] data The data would be send
       * @param size The size of data
       *
       */
      int Send(const char *data,int size);
      /**
       * @brief Receive data from remote host
       * @param [out] buffer receive data buffer
       * @param max_size The max size of buffer
       *
       */
      int Receive(char *buffer,int max_size);

      /// Close the connection
      void Close();

      /// Get the socket ID
      Socket::Id GetSocketId() const;
      /// Get the remote host information
      IpEndPoint GetRemoteHost();

     private:
      Socket::Ptr socket_;
      bool connected_;
    };
  } //namespace net
} //namespace kiimo



#endif /* TCP_CLIENT_H_ */
