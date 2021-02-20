/**
 * 	@file server.cc
 *  @author simon
 *	@version V1.0
 *  @date 2020-04-10
 *  @brief first version
 */

#include "server.h"

#include "parser.h"
#include "../logger.h"

using namespace std::placeholders;
using namespace kiimo::net;
using namespace kiimo::base;

HttpServer::HttpServer()
  :tcp_(nullptr),loop_(nullptr)
{
  loop_ = new EventLoop();
  tcp_ = new TcpServer(loop_);

  tcp_->SetOnConnect(std::bind(&HttpServer::OnConnect,this,_1));
  tcp_->SetOnMessage(std::bind(&HttpServer::OnMessage,this,_1,_2));
  tcp_->SetDisconnect(std::bind(&HttpServer::OnDisconect,this,_1));
}

HttpServer::~HttpServer()
{
  if(loop_ != nullptr)
  {
    delete loop_;
  }
  if(tcp_ != nullptr)
  {
    delete tcp_;
  }
}

void HttpServer::RegistRouter(std::string path,const Router &cb)
{
  if(routers_.count(path)!= 1)
  {
    routers_.emplace(path,cb);
  }
  else
  {
    routers_[path] = cb;
  }
}

void HttpServer::Start()
{
  tcp_->Start(5);  // 5 thread in release
  loop_->Loop();

  tcp_->Stop();
}

void HttpServer::OnConnect(const TcpSession::Ptr &con)
{
  DebugL << con->GetSocket()->GetRemoteHost().ToString() << " Connected";
}
void HttpServer::OnMessage(const TcpSession::Ptr &con,Message& msg )
{
//  std::vector<char> msg;
//  con->GetData(msg);
  //std::string str(msg.begin(),msg.end());
  HttpRequest request;
  HttpParser::Parse(msg,request);
  HttpResponse response;

  //DebugL << Fmt::Format("\r\nPath: {0}\r\nAccept: {1}\r\nUser-Agent: {2}",request.path,request.GetHeaderValue("Accept"),request.GetHeaderValue("User-Agent"));

  auto it = routers_.find(request.path);
  if(it != routers_.end())
//  if(routers_.count(request.path) == 1)
  {
    response = it->second(request);
  }
  else
  {
   response.state_code = 404;    //Not Found
   response.content.clear();
  }
  //response = routers_.begin()->second(request);
  std::vector<char> rsp;
  HttpParser::Deparse(response,rsp);
  con->Send(rsp);
//  if(!request.connection)
//  {
//    con->ShutDown();
//  }
}
void HttpServer::OnDisconect(const TcpSession::Ptr &con)
{
  DebugL << con->GetSocket()->GetRemoteHost().ToString() << " Close";
}



