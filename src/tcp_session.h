/**
 * @file connection.h
 * @brief Represent connection
 * @author simon
 * @date 2019-09-09
 *
 */
#ifndef TCP_SESSION_H_
#define TCP_SESSION_H_

#include <memory>
#include <vector>

#include "types.h"
#include "socket.h"
#include "buffer.h"
//#include "event_loop.h"

namespace kiimo {
  namespace net {
    class EventLoop;
    /**
     * TCP session
     */
    class TcpSession: public std::enable_shared_from_this<TcpSession>
    {
     public:
      typedef std::shared_ptr<TcpSession> Ptr;
      TcpSession(IpEndPoint server, EventLoop *loop,Socket::Ptr con);
      ~TcpSession() = default;

      int Send(const char *buffer,int size);
      int Send(std::vector<char> &buffer);

      int Getdata(char *buffer,size_t max_size);
      int GetData(std::vector<char> &buffer);

      void Shutdown();

      //void SendTo(const char *buffer,int size);
      void AppendToBuffer(const char *buffer,int size);


      //TcpClient::Ptr GetTcpClientPtr();
      Socket::Id GetSocketId() const;
      Socket::Ptr GetSocket();

      void SetOnConnect(const ConnectCallback &cb);
      void SetOnClose(const ConnectCallback &cb);
      void SetOnMessage(const MessageCallback &cb);

      /// @name For EventLoop
      /// @{
      void HandleRead();
      void HandleWrite();
      void HandleExcept();
      /// @}

     private:
      enum class Status{kConnected,kReceive,kSend,kDisconnect,};
      const IpEndPoint server_;
      EventLoop *loop_;
      Socket::Ptr socket_;
      Status status_;
      //std::vector<char> recv_buff_;
      Buffer recv_buff_;
      //std::vector<char> send_buff_;
      Buffer send_buff_;

      ConnectCallback on_connect_;
      ConnectCallback on_disconnect_;
      MessageCallback on_message_;
    };

  } /* namespace net */
} /* namespace kiimo */
#endif /* TCP_SESSION_H_ */

