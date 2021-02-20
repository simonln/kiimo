/**
 * 	@file server.h
 *  @author simon
 *	@version V1.0
 *  @date 2020-04-10
 *  @brief first version
 */

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <functional>
#include <string>
#include <unordered_map>


#include "request.h"
#include "../tcp_server.h"
namespace kiimo {
namespace net {

/**
 *  HTTP server
 */
class HttpServer {

public:
    typedef std::function<HttpResponse(const HttpRequest&)> Router;
    HttpServer();
    ~HttpServer();
    void RegistRouter(std::string path,const Router &cb);
    void Start();

private:
    void OnConnect(const TcpSession::Ptr &con);
    void OnMessage(const TcpSession::Ptr &con,Message &msg );
    void OnDisconect(const TcpSession::Ptr &con);
private:
     std::unordered_map<std::string,Router> routers_;
     TcpServer *tcp_;
     EventLoop *loop_;
};

} /* namespace net */
} /* namespace kiimo */

#endif /* HTTP_SERVER_H_ */
