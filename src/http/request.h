/**
 * 	@file request.h
 *  @author simon
 *	@version V1.0
 *  @date 2020-04-07
 *  @brief first version
 */

#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <string>
#include <vector>
#include <map>

//#include "../socket.h"

namespace kiimo {
namespace net {


enum class RequestMethod
{
  GET,
  POST,
  PUT,
  HEAD,
};

enum class HttpVersion
{
  Ver10,  //1.0
  Ver11,  //1.1
};
/**
 *
 */
struct HttpRequest
{
  // request line
  RequestMethod method;
  std::string path;
  HttpVersion http_version;

  std::map<std::string ,std::string> headers;
  // headers
//  std::string host;
//  std::string user_agent;
//  std::string accept;
//  //std::string connection;
//  std::string accept_encoding;
//  std::string accept_language;
//  bool connection;

  // main body
  std::vector<char> content;

  std::string GetHeaderValue(const std::string &key)
  {
    auto it = headers.find(key);
    if(it != headers.end())
    {
      return it->second;
    }
    else
    {
      return "";
    }
  }
};

struct HttpResponse
{
  int state_code;
  std::string state_desc;

  // headers
  //std::string server;
  std::string content_type;
//  //std::string content_lenght;
//  std::string content_charset;
//  std::string content_encoding;
//  std::string content_language;
  std::map<std::string,std::string> headers;

  std::vector<uint8_t> content;

};

} /* namespace net */
} /* namespace kiimo */

#endif /* HTTP_HTTP_REQUEST_H_ */
