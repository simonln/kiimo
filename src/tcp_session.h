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

      /// close the link  right now
      void Termination();
      /// close the link after all data is send over
      void Shutdown();

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
      void DisableWriting();

     private:
      enum class Status{kConnected,kReceive,kSend,kDisconnect,};
      const IpEndPoint server_;
      EventLoop *loop_;
      Socket::Ptr socket_;
      Status status_;
      Buffer recv_buff_;
      Buffer send_buff_;

      ConnectCallback on_connect_;
      ConnectCallback on_disconnect_;
      MessageCallback on_message_;
    };

  } /* namespace net */
} /* namespace kiimo */
#endif /* TCP_SESSION_H_ */

