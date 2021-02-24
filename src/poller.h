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
       void Add(Socket::Id id, EventType type) override;
       void Update(Socket::Id id, EventType type) override;
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
#ifdef _WIN32
    enum class CompletionOp
    {
      kNone,
      kAccepting,
      kAccepted,
      kReading,
      kRead,
      kWriting,
      kWrited,
    };
    struct CompletionState
    {
      Socket::Id id;
      CompletionOp op;
      int length;
      char *buffer;
      int focus;
      int recv_len;
      int async_recv_start;
      int send_len;
      int async_send_end;

      /// default constructor
      CompletionState(Socket::Id sock,CompletionOp operation,
                      int len = 0,char *buff=nullptr,int foc = EventType::kNoneEvent)
        :id(sock),op(operation),length(len),buffer(buff),focus(foc),
         recv_len(0),async_recv_start(0),send_len(0),async_send_end(0)
      {
      }
    };
    /**
     *  IO completion port in Windows
     */
    class Iocp
    {
     public:
      bool Init(Socket::Id listen_sock);
      void Release();
      void Update(Socket::Id id,EventType event);
      void Remove(Socket::Id id,EventType event);
      //int Wait(CompletionState *state);
      std::pair<Socket::Id,int> Wait(int millsecond = kTimeout);

      Socket::Ptr SpecialAccept();
      int SpecialSend(const Socket::Id id,const std::vector<char> &buff);
      int SpecialRecv(const Socket::Id id,std::vector<char> &buff);
     private:
      bool StartAccept();
      std::list<CompletionState*>::iterator HasLink(Socket::Id id);
      bool StartReading(CompletionState *state);
      void FixReceiveBuffer(CompletionState *state);
      void FixSendBuffer(CompletionState *state);
     public:
       static const int kBufferSize = 2048;
     private:
      Socket::Id listen_sock_;
      //Socket::Id accept_sock_;
      CompletionState *listen_state_;
      //std::map<Socket::Id,int> event_;
      HANDLE port_;
      LPFN_ACCEPTEX acceptex_;
      LPFN_GETACCEPTEXSOCKADDRS getaddrex_;
      WSAOVERLAPPED listen_ovl_;
      WSAOVERLAPPED res_ovl_;
      //char addr_buff_[128];   //save remote address info and local address info only
      std::list<CompletionState*> links_;
    };
#endif

  } //namespace net
} //namespace kiimo


#endif /* POLLER_H_ */
