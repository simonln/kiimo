/**
 * @file types.h
 * @brief Some typedefs
 * @author simon
 * @date 2019-08-03
 *
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <vector>
#include <functional>
#include <memory>

namespace kiimo{
  namespace net{
    class TcpSession;
    typedef std::shared_ptr<TcpSession> TcpSessionPtr;
    //typedef std::vector<char> Message;
    class Buffer;
    typedef Buffer Message;
    typedef std::function<void(const TcpSessionPtr&)> ConnectCallback;
    typedef std::function<void(const TcpSessionPtr& ,Message &msg)> MessageCallback;
    typedef std::function<void(void)> Function;

  }
}




#endif /* TYPES_H_ */
