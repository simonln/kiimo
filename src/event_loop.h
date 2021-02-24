/**
 * @file event_loop.h
 * @brief Event loop class
 * @author simon
 * @date 2019-12-09
 *
 */

#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <list>

#include "thread.h"
#include "poller.h"
#include "types.h"
#include "time_queue.h"
#include "udp_client.h"
#include "tcp_client.h"
#include "tcp_session.h"

/**
 *  @brief The name space of this library
 */
namespace kiimo {

/**
 * @brief Contain all class of network
 */
namespace net {

  /**
   *  @brief event base-class
   */
  class Event
  {
   public:
    typedef std::function<void(void)> EventCallback; ///< event loop callback type
    Event()
   {
      id = 0;
      wait_event = EventType::kNoneEvent;
      read_handler_ = nullptr;
      write_handler_ = nullptr;
      except_handler_ = nullptr;
   };

    /**
     *  @brief constructor function
     *  @param id socket id
     *  @param kind event type
     */
    Event(Socket::Id id,EventType kind)
    {
      this->id = id;
      wait_event = kind;
      read_handler_ = nullptr;
      write_handler_ = nullptr;
      except_handler_ = nullptr;
    }
    /**
     *  @brief Destructor
     */
    virtual ~Event() = default;

    /**
     *  @brief handler of read event
     */
    virtual void ReadHandler()
    {
      if(read_handler_)
      {
        read_handler_();
      }
    };
    /**
     *  @brief handler of write event
     */
    virtual void WriteHandler()
    {
      if(write_handler_)
      {
        write_handler_();
      }
    };

    /**
     *  @brief handler of except event
     */
    virtual void ExceptHandler()
    {
      if(except_handler_)
      {
        except_handler_();
      }
    };

    /**
     *  @brief Set the callback of read event.
     */
    void SetReadHandler(const EventCallback &cb)
    {
      read_handler_ = cb;
      wait_event |= EventType::kReadEvent;
    }
    /**
     * @brief Set the callback of write event
     */
    void SetWriteHandler(const EventCallback &cb)
    {
      write_handler_ = cb;
      wait_event |= EventType::kWriteEvent;
    }
    /**
     * @brief Set the callback of except event
     */
    void setExceptHandler(EventCallback cb)
    {
      except_handler_ = cb;
      wait_event |= EventType::kExceptEvent;
    }


   public:
    Socket::Id id;  ///< event id
    int wait_event; ///< event type,details in  Select::EventKind
   private:
    EventCallback read_handler_;      ///< read event callback
    EventCallback write_handler_;     ///< write event callback
    EventCallback except_handler_;    ///< except event callback

  };

    /**
     * @brief UDP event inherited from  Event
     */
    class UdpEvent: public Event
    {
     public:
      typedef std::function<void(const IpEndPoint,Message&)> ReadCallback;  ///< UDP read event callback type

      /**
       *  @brief Constructor
       *  @param con The pointer of UdpClient
       */
      UdpEvent(UdpClient::Ptr con);

      /**
       *  @brief Constructor
       *  @param con The pointer of UdpClient.
       *  @param cb Read event callback
       */
      UdpEvent(UdpClient::Ptr con,ReadCallback cb);

      /**
       * @brief Set the callback of read event
       */
      void SetOnMessage(ReadCallback cb);

      /**
       *  @brief Override function of base-class
       */
      virtual void ReadHandler() override;


     private:
      UdpClient::Ptr link_;       // Pointer of  UdpClient
      ReadCallback on_message_;   // The callback of read event

    };

    /**
     *  @brief TCP event inherited from  Event
     */
    class TcpEvent: public Event
    {
     public:
      /**
       * @brief Constructor
       * @param con The pointer of TcpClient
       * @param kind event type
       */
      TcpEvent(TcpSession::Ptr con,EventType kind = EventType::kAllEvent);

      /**
       *  @brief Constructor
       *  @param con The ponint of TcpClient
       *  @param on_connect Connect callback
       *  @param on_disconnect Disconnect callback
       *  @param on_message Callback when message is coming
       */
//      TcpEvent(TcpSession::Ptr con,
//               ConnectCallback on_connect,
//               ConnectCallback on_disconnect,
//               MessageCallback on_message
//               );

      /**
       *  @brief Destructor
       */
      ~TcpEvent() = default;

      /// @name Override base-class function
      /// @{

      void ReadHandler() override;
      void WriteHandler() override;
      void ExceptHandler() override;

      /// @}

      /**
       *  @brief Set the connect callback
       */
//      void SetOnConnect(ConnectCallback cb);
//      /**
//       *  @brief Set the disconnect callback
//       */
//      void SetOnDisconnect(ConnectCallback cb);
//      /**
//       *  @brief Set the callback of message coming
//       */
//      void SetOnMessage(MessageCallback cb);
     private:
      TcpSession::Ptr link_;             // pointer of TcpClient
//      ConnectCallback on_connect_;      // connect callback
//      ConnectCallback on_disconnect_;   // disconnect callback
//      MessageCallback on_message_;      // message callback

    };

  /**
   * @brief  poll some socket I/O
   *
   */
  class EventLoop
  {
   public:
    /**
     *  @brief Constructor
     */
    EventLoop();
    /**
     * @brief Destructor
     */
    ~EventLoop() = default;

#ifdef USE_IOCP
     void InitIocp(const Socket::Id listen_sock);
     void ReleaseIocp();
     Socket::Ptr SpecialAccept();
     int SpecialSend(const Socket::Id id,const std::vector<char> &buff);
     int SpecialRecv(const Socket::Id id,std::vector<char> &buff);
#endif
    /**
     *  @brief Update the event focused
     *  @param event TcpEvent or UdpEvent
     */
    void UpdateEvent(Event *event);

    /**
     *  @brief Is this event in focus event list
     *  @param event TcpEvent or UdpEvent
     *  @return true or false
     */
    //bool HasEvent(Event *event);
    /**
     *  @brief remove this event from focus event list
     *  @param event TcpEvent or UdpEvent
     */
    void RemoveEvent(Event *event);
    /**
     *  @brief Set the function run in loop thread
     */
    void RunInLoop(Function func);

    /// @name For timer
    /// @{
    base::TimerId RunAfter(Function func,int second);
    base::TimerId RunEvery(Function func,int second);
    void Cancle(base::TimerId id);
    /// @}

    /**
     *  @brief event loop.
     *  It is a infinite loop until thread quit
     */
    void Loop();
   private:
    void InsertEvent(Event *event);
    EventExist HasEvent(Event *event);

   private:
    bool quit_;                           // thread loop quit flag
    Epoller select_;
    base::TimeQueue timer_;                // timer run in event loop
    std::map<Socket::Id,Event*> events_;    //focus event list
    std::list<Function> pending_work_;
    //base::Condition cv_;
    //base::Mutex mtx_;
  };

} /* namespace net */
} /* namespace kiimo */

#endif /* EVENT_LOOP_H_ */
