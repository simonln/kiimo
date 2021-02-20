/*
 * main.cc
 *
 *  Created on: 2019-08-01
 *      Author: simon
 */

#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <limits>

#ifdef UNITTEST
#include <gtest/gtest.h>
#endif

#ifndef _WIN32
#include <signal.h>
#endif

#include "types.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "logger.h"
#include "database/sqlite_connector.h"
#include "http/server.h"

using namespace std;
using namespace placeholders;
using namespace kiimo::base;
using namespace kiimo::net;


class Updater
{
public:
    Updater()
      :server_(nullptr)
    {
      server_ = new HttpServer();
    }
  void Start()
  {
    server_->RegistRouter("/",std::bind(&Updater::RootRouter,this,_1));
    server_->RegistRouter("/file",std::bind(&Updater::BinaryRouter,this,_1));

    server_->Start();
  }
private:
   HttpResponse RootRouter(const HttpRequest &request)
   {
     HttpResponse rsp = {200,"OK","text/html"};
     /*
     char format[] = "<html><head><script type=\"text/JavaScript\">function AutoRefresh(t){setTimeout(\"location.reload(true);\",t);}</script> \
     </head><body onload=\"JavaScript:AutoRefresh(5000);\"><p><b style=\"color:blue\">hello world!</b></p><p>{0}</p></body></html>";
     */
//     std::string content = Fmt::Format(string(format),TimeStamp::Now().ToString("d"));
     std::string content = "<html><p><b style=\"color:blue\">hello world!</b></p><p>"+TimeStamp::Now().ToString("d") + "</p></html>";
     //rsp.content = std::vector<uint8_t>(content.begin(),content.end());
     rsp.content.assign(content.begin(), content.end());
     return rsp;
   }
   HttpResponse BinaryRouter(const HttpRequest &request)
   {
     HttpResponse rsp = {200,"OK","application/txt"};
     std::string content = Fmt::Format("<html><p><b style=\"color:blue\">hello world!</b></p><p>{0}</p></html>",TimeStamp::Now().ToString("d"));
     //std::string content = "<html><p><b style=\"color:blue\">hello world!</b></p><p>"+TimeStamp::Now().ToString("d") + "</p></html>";
     //rsp.content = std::vector<uint8_t>(content.begin(),content.end());
     rsp.content.assign(content.begin(), content.end());;
     return rsp;
   }
private:
  HttpServer *server_;
};


#ifdef UNITTEST
int main(int argc,char **argv)
{
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();

}
#else
int main()
{
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif
#ifdef _DEBUG
  Logger::Instance().add(make_shared<ConsoleChannel>()); // @suppress("Invalid arguments")
#else
  Logger::Instance().add(make_shared<FileChannel>());
#endif
  Logger::Instance().setWriter(make_shared<AsyncLogWriter>()); // @suppress("Invalid arguments")

  Updater updater;
  updater.Start();

  return 0;
}
#endif
