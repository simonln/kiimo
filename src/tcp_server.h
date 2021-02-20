/**
 * @file tcp_server.h
 * @brief TCP server
 * @author simon
 * @date 2019-08-01
 *
 */

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_
#include <functional>
#include <set>
#include <list>
#include <unordered_map>

#include "types.h"
#include "socket.h"
#include "tcp_client.h"
#include "time_queue.h"
#include "thread_pool.h"
#include "event_loop.h"
#include "tcp_session.h"

namespace kiimo{
  namespace net{

    /// default server listen port
    static const int kListenPort = 1234;

    /**
     *  @brief TCP server
     */
    class TcpServer
    {
     public:
      /**
       *  @brief Constructor
       *  @param loop event loop
       *  @param port listen port
       */
      TcpServer(EventLoop *loop,int port = kListenPort);
      ~TcpServer();

      /**
       *  @brief start the listen process
       *  @param max_thread The thread number of server client
       *
       */
      void Start(int max_thread = 0);

      void Stop();

      /// @name For client
      /// @{
      void SetOnConnect(const ConnectCallback &cb);
      void SetOnMessage(const MessageCallback &cb);
      void SetDisconnect(const ConnectCallback &cb);
      /// @}



    private:
      TcpServer() = delete;

      void Accepter();
      void ExceptHappened();

      void RunInThreadPool(const Function &func);


      void EstablishConnect(const TcpSession::Ptr &con);
      void ClientMsgCome(const TcpSession::Ptr &con,Message &msg);
      void StopConnect(const TcpSession::Ptr &con);

      void  RemoveClientInLoop(const TcpSession::Ptr &con);

      /// Get the socket ID
      Socket::Id GetSocketId() const;

     private:
      EventLoop *base_loop_;
      Socket socket_;
      Event *event_;

      std::unordered_map<TcpSession::Ptr,TcpEvent*> links_;
      base::ThreadPool *thread_pool_;

      //use in client
      ConnectCallback on_connecting_;
      MessageCallback on_message_;
      ConnectCallback on_disconnect_;
    };


  } //namespace net
} // namespace kiimo

#endif /* TCP_SERVER_H_ */
