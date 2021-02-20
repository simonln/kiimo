// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

// Modified : zieckey (zieckey at gmail dot com)


#include "buffer.h"


#include <errno.h>

#ifdef _DEBUG
#include "logger.h"
#include "utils.h"
using namespace kiimo::base;
#endif

#ifdef _WIN32
#define iovec _WSABUF
#define iov_base buf
#define iov_len len
#else
#include <sys/uio.h>
#endif

namespace kiimo
{
namespace net
{

  const char Buffer::kCRLF[] = "\r\n";

  const size_t Buffer::kCheapPrependSize = 8;
  const size_t Buffer::kInitialSize  = 1024;

#ifdef __WIN32
  int readv(int sockfd, struct iovec* iov, int iovcnt)
  {
      DWORD readn = 0;
      DWORD flags = 0;

      if (::WSARecv(sockfd, iov, iovcnt, &readn, &flags, nullptr, nullptr) == 0)
      {
          return readn;
      }
      return -1;
  }
#endif
  size_t Buffer::ReadFromFD(int fd, int* err)
  {
      // saved an ioctl()/FIONREAD call to tell how much to read
      char extrabuf[65536];
      struct iovec vec[2];
      const size_t writable = WritableBytes();
      vec[0].iov_base = begin() + write_index_;
      vec[0].iov_len = writable;
      vec[1].iov_base = extrabuf;
      vec[1].iov_len = sizeof(extrabuf);
      // when there is enough space in this buffer, don't read into extrabuf.
      // when extrabuf is used, we read 64k bytes at most.
#ifdef _WIN32
      DWORD n = 0;
      DWORD flags = 0;
      int ret = WSARecv(fd, vec, 2, &n, &flags, nullptr, nullptr);
      //DebugL << "WSARecv size: " << n;
      if (ret != 0 || n < 0)
      {
          *err = GetLastError();
#ifdef _DEBUG
          DebugL << "WSARecv error : " << *err;
#endif
      }
#else
      const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
      const ssize_t n = ::readv(fd, vec, iovcnt);
      if (n < 0)
      {
          *err = errno;
      }
#endif
      else if (static_cast<size_t>(n) <= writable) 
      {
          write_index_ += n;

      }
      else 
      {
          write_index_ = capacity_;
          Append(extrabuf, n - writable);
      }

      return n;
  }

}
}
