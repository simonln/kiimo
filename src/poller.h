/**
 *  @file poller.h
 *  @brief poller set
 *  @author simon
 *  @date 2019-08-05
 */

#ifndef POLLER_H_
#define POLLER_H_

#include <map>
#include <set>
#include <list>
#include <vector>
#include <utility>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include "wepoll/wepoll.h"
#else
#include <poll.h>
#include <sys/epoll.h>
#endif

#include "socket.h"
//#include "utils.h"

namespace kiimo{

namespace net{

    /// Focused event type
    enum EventType
    {
      kNoneEvent,       ///< default
      kReadEvent = 1,
      kWriteEvent = 2,
      kExceptEvent = 4,
      
      kReadAndWrite = 3,
      kReadAndExcept = 5,
      kWriteAndExpect = 6,

      kAllEvent = 7,
    };

    enum EventExist
    {
      kId,        // only id exist
      kEvent,     // id and event exist
      kNone       // not exist
    };

    class IPoller
    {
     public:
        virtual ~IPoller() = default;
        virtual void Add(Socket::Id id, EventType type) = 0;
        virtual void Update(Socket::Id id,EventType type) = 0;
        virtual void Remove(Socket::Id id,EventType type) = 0;
    };

    /// Wait timeout
    const int kTimeout = 100;


#if (linux || _WIN32_WINNT >= 0x0600)
    /// Poll implement
    class Poller:IPoller
    {
     public:
      Poller() = default;
      ~Poller() = default;
      void Add(Socket::Id id, EventType type) override;
      void Update(Socket::Id id,EventType type) override;
      void Remove(Socket::Id id,EventType type) override;
      std::map<Socket::Id,int> Wait(int millsecond = kTimeout);
     private:
      std::vector<struct pollfd>::iterator HasEvent(Socket::Id id);
     private:
      std::vector<struct pollfd> events_;
    };
#endif


    /// epoll implement
    class Epoller:IPoller
    {
      enum Opr
      {
        kAdd, 
        kUpdate,
        kRemove,
        kDefault
      };
      public:
       Epoller();
       ~Epoller();
       /// add a socket with event to listen queue
       void Add(Socket::Id id, EventType type) override;
       /// update the event in listen socket queue 
       void Update(Socket::Id id, EventType type) override;
       /// remove the socket from listen queue
       void Remove(Socket::Id id, EventType type) override;
       std::map<Socket::Id,int> Wait(int millsecond = kTimeout);
      private:
        void UpdateCtrl(Socket::Id, EventType type, Opr op);
      private:
#if _WIN32
        HANDLE epoll_fd_;
#else
        int epoll_fd_;
#endif
        bool epoll_working_;    // epoll fd is vaild
    };



    /**
     *  Wrap socket API \b select function
     */
    class Select
    {
     public:

      Select() = default;
      ~Select() = default;
      /// Update focus list
      void Update(Socket::Id sock,EventType event);
      /// Remove the event from focus list
      void Remove(Socket::Id sock,EventType event);
      /**
       *  @brief Wait focused event happen
       *  @param millsecond wait timeout , 500ms
       */
      std::map<Socket::Id,int> Wait(int millsecond = kTimeout);
     public:
      /// Wait timeout
      static const int kTimeout = 500; //500ms
     private:
      fd_set fdread_;
      fd_set fdwrite_;
      fd_set fdexcept_;
      std::set<Socket::Id> read_set_;
      std::set<Socket::Id> write_set_;
      std::set<Socket::Id> except_set_;
    };

    /**
     *  Special class for Select,used for server socket
     */
    class SingleSelect
    {
     public:
      SingleSelect() = delete;
      ~SingleSelect() = default;
      /**
       *  @brief Constructor
       *  @param id socket id need to focus
       *  @param event item from Select::EventKind,if two or three be selected ,it could use '|'.
       */
      SingleSelect(Socket::Id id,int event);
      void Update(EventType event);
      void Remove(EventType event);
      /**
       *  @brief Wait focused event  happen
       */
      int Wait();

     private:
      Socket::Id wait_id_;
      int wait_events_;
      fd_set fds_[3];
    };

  } //namespace net
} //namespace kiimo


#endif /* POLLER_H_ */
