/**
 *  @file tcp_server.cc
 *  @brief TcpServer implement
 *  @author simon
 *  @date 2019-08-01
 *
 */
#include "tcp_server.h"

#include <utility>
#include <list>
#include <map>
#include <functional>

#include "poller.h"
#include "thread_pool.h"
#include "logger.h"


using namespace std;
using namespace placeholders;
using namespace kiimo::base;
using namespace kiimo::net;

TcpServer::TcpServer(EventLoop *loop,int port)
  :base_loop_(loop),event_(nullptr),thread_pool_(nullptr)
{
  on_connecting_ = [](const TcpSession::Ptr&){};
  on_message_ = [](const TcpSession::Ptr&,const Message&){};
  on_disconnect_ = [](const TcpSession::Ptr&){};

  socket_ = Socket(port);

}


TcpServer::~TcpServer()
{
  //wait all the thread to quit
  if(thread_pool_)
  {
    thread_pool_->Shutdown();
  }

  socket_.Close();
}


void TcpServer::Start(int max_thread)
{
    socket_.Initialize(SocketType::kTcpSocket);
    if(!socket_.BindToPort())
    {
      ErrorL << "bind to port: failed!" <<endl;
      return;
    }
    else
    {
      InfoL << "listen port: "  << socket_.GetLocalHost().GetPortString() << endl;
    }
    //add to event loop
    event_ = new Event(socket_.GetId(),(EventType)(EventType::kReadEvent | EventType::kExceptEvent));
    event_->SetReadHandler(std::bind(&TcpServer::Accepter,this)); // @suppress("Invalid arguments")
    event_->setExceptHandler(std::bind(&TcpServer::ExceptHappened,this)); // @suppress("Invalid arguments")
    base_loop_->UpdateEvent(event_);

    //if(max_thread > 0)
    {
      //create the thread pool
      thread_pool_ = new ThreadPool(base_loop_,max_thread);
    }
}



void TcpServer::Accepter()
{
  //accpt new connection

  Socket::Ptr con = socket_.Accept();
  
  if(con->GetId() >= 0)
  {
    //TcpClient::Ptr client = make_shared<TcpClient>(con);
    EventLoop *loop = thread_pool_->GetNextLoop();

    loop->RunInLoop([=]{
        TcpSession::Ptr session = make_shared<TcpSession>(socket_.GetLocalHost(),loop,con);
        //session->SetOnConnect(on_connecting_);
        session->SetOnMessage(on_message_);
        session->SetOnClose(on_disconnect_);
        on_connecting_(session);
        TcpEvent *event = new TcpEvent(session,EventType::kReadEvent);
        loop->UpdateEvent(event);
        
    });
  }
  else
  {
    con->Close();
  }
}

void TcpServer::ExceptHappened()
{
  //TcpSever error
  this->Stop();
}



void TcpServer::RunInThreadPool(const Function &func)
{
    func();
}





void TcpServer::SetOnConnect(const ConnectCallback &cb)
{
  if(cb != nullptr)
  {
    on_connecting_ = cb;
  }

}

void TcpServer::SetOnMessage(const MessageCallback &cb)
{
  if(cb != nullptr)
  {
    on_message_ = cb;
  }

}

void TcpServer::SetDisconnect(const ConnectCallback &cb)
{
  if(cb != nullptr)
  {
    on_disconnect_ = cb;
  }
}

void TcpServer::Stop()
{
  base_loop_->RemoveEvent(event_);
  //if(event_)
  {
    delete event_;
  }
  //if(thread_pool_)
  {
    thread_pool_->Shutdown();
  }
  socket_.Close();

}




void  TcpServer::EstablishConnect(const TcpSession::Ptr &con)
{
//  if(on_connecting_)
  {
    on_connecting_(con);
  }
  //DebugL << "Thread: " << Pthread::GetCurrentThreadId() << endl;
}

void TcpServer::ClientMsgCome(const TcpSession::Ptr &con,Message &msg)
{
    //RunInThreadPool([&]{ /*if(on_message_) */on_message_(con,msg);});
  RunInThreadPool(std::bind(on_message_,con,msg));
}


//may run in another thread
void TcpServer::StopConnect(const TcpSession::Ptr &con)
{
//  if(on_disconnect_)
  {
    on_disconnect_(con);
  }

  base_loop_->RunInLoop(std::bind(&TcpServer::RemoveClientInLoop,this,con));
}

void TcpServer::RemoveClientInLoop(const TcpSession::Ptr &con)
{
   auto it = links_.find(con);
   if(it != links_.end())
   {
     base_loop_->RemoveEvent(it->second);
     links_.erase(it);
   }
 //std::cout << "Thread: " << Pthread::GetCurrentThreadId() << " Client size: " << clients_.size() << endl;
//   DebugL << "Client size: " << links_.size();
}


Socket::Id TcpServer::GetSocketId() const
{
  return socket_.GetId();
}



