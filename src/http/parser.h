/**
 * 	@file parser.h
 *  @author simon
 *	@version V1.0
 *  @date 2020-04-09
 *  @brief first version
 */

#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

#include <string>
#include <stdexcept>

#include "../types.h"
#include "request.h"

namespace kiimo {
namespace net {

/**
 *
 */
class HttpParser
{
 public:
  enum class ParseRequestStep
  {
    kRequestLine,
    kRequestHeader,
    kContent,
    kParseComplete
  };
public:
  static bool Parse(Message& buff,HttpRequest &request);
  static bool Parse(const std::vector<char> &request_str,HttpRequest &request);
  static void Deparse(const HttpResponse &response,std::vector<char> &raw_rsp);
private:
  static ParseRequestStep step_;
};

/**
 * Exception in Http
 *
 */
class HttpException:std::exception
{
public:
  HttpException(std::string msg)
      :msg_(msg)
  {
  }
  const char* what() const noexcept override
  {
    return msg_.data();
  }
private:
    std::string msg_;
};


} /* namespace net */
} /* namespace kiimo */

#endif /* HTTP_PARSER_H_ */
