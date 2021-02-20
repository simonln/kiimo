/**
 * @file logger.h
 * @brief logger library.
 *
 * MIT License
 *
 * Copyright (c) 2016 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <cstdio>
#include <cstring>
#include <map>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>

#ifdef _WIN32
#include <time.h>
#include <Winsock2.h>  //struct timeval
//#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif


//#include "thread.h"   //pthread wrapped in the MINGWIN platform


/**
 *  @brief Logger singleton
 */
#define INSTANCE_IMP(class_name,...)\
  class_name &class_name::Instance(){\
  static std::shared_ptr<class_name> s_instance(new class_name(__VA_ARGS__));\
  static class_name &s_instance_ref = *s_instance;\
  return s_instance_ref;\
}


namespace kiimo
{
  namespace base{

  /**
   *  @brief No copy base-class
   */
  class NonCopyable
  {
    protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    private:
    NonCopyable(const NonCopyable &that) = delete;
    NonCopyable(NonCopyable &&that) = delete;
    NonCopyable &operator=(const NonCopyable &that) = delete;
    NonCopyable &operator=(NonCopyable &&that) = delete;
  };

  /**
   * @brief logger level
   */
  typedef enum {
    LTrace,       ///< trace
    LDebug,       ///< debug
    LInfo,        ///< information
    LWarn,        ///< warning
    LError,       ///< error
  } LogLevel;

  class LogContext;
  class LogChannel;
  class LogWriter;
  typedef std::shared_ptr<LogContext> LogContextPtr;  ///< rename the smart pointer of LogContext

  /**
   * @brief Log class
   */
  class Logger : public std::enable_shared_from_this<Logger> , public NonCopyable {
  public:
      friend class AsyncLogWriter;
      friend class LogContextCapturer;
      typedef std::shared_ptr<Logger> Ptr;

      /**
       * 获取日志单例
       * @return
       */
      static Logger &Instance();


    Logger(const std::string &loggerName);
      ~Logger();

      /**
       * 添加日志通道，非线程安全的
       * @param channel log通道
       */
      void add(const std::shared_ptr<LogChannel> &channel);

    /**
       * 删除日志通道，非线程安全的
       * @param name log通道名
       */
      void del(const std::string &name);

      /**
       * 获取日志通道，非线程安全的
       * @param name log通道名
       * @return 线程通道
       */
      std::shared_ptr<LogChannel> get(const std::string &name);

      /**
       * 设置写log器，非线程安全的
       * @param writer 写log器
       */
      void setWriter(const std::shared_ptr<LogWriter> &writer);

      /**
       * 设置所有日志通道的log等级
       * @param level log等级
       */
      void setLevel(LogLevel level);

      /**
       * 获取logger名
       * @return
       */
      const std::string &getName() const;
  private:
      void writeChannels(const LogContextPtr &stream);
      void write(const LogContextPtr &stream);
  private:
      std::map<std::string, std::shared_ptr<LogChannel> > _channels;
      std::shared_ptr<LogWriter> _writer;
      std::string _loggerName;
      //static Mutex mutex_;
      //static std::mutex  mutex_;
      static Logger *instance_ref_;
  };

  ///////////////////LogContext///////////////////
  /**
   * @brief Log context
   *
   * 日志上下文
   */
  class LogContext : public std::ostringstream{
  public:
      friend class LogContextCapturer;
  public:
      LogLevel _level;
      int _line;
      const char *_file;
      const char *_function;
      struct timeval _tv;
  private:
      LogContext(LogLevel level,const char *file,const char *function,int line);
  };

  /**
   * @brief Log context capture
   *
   * 日志上下文捕获器
   */
  class LogContextCapturer {
  public:
    typedef std::shared_ptr<LogContextCapturer> Ptr;
      LogContextCapturer(Logger &logger,LogLevel level, const char *file, const char *function, int line);
      LogContextCapturer(const LogContextCapturer &that);

      ~LogContextCapturer();

      /**
       * 输入std::endl(回车符)立即输出日志
       * @param f std::endl(回车符)
       * @return 自身引用
       */
    LogContextCapturer &operator << (std::ostream &(*f)(std::ostream &));

      template<typename T>
      LogContextCapturer &operator<<(T &&data) {
        if (!_logContext) {
            return *this;
        }
        (*_logContext) << std::forward<T>(data);

        return *this;
      }

      void clear();
  private:
      LogContextPtr _logContext;
    Logger &_logger;
  };


  ///////////////////LogWriter///////////////////
  /**
   * @brief Log writer
   *
   * 写日志器 ，默认选择，线程不安全
   */
  class LogWriter : public NonCopyable {
  public:
    LogWriter() {}
    virtual ~LogWriter() {}
    virtual void write(const LogContextPtr &stream) = 0;
  };

  /**
   * @brief Asynchronous writer
   *
   * 写日志器，异步的方式写入，所以保证线程安全
   */
  class AsyncLogWriter: public LogWriter{
   public:
      AsyncLogWriter(Logger &logger = Logger::Instance());
      ~AsyncLogWriter();
  private:
      void *run(void*);
      void flushAll();
      void write(const LogContextPtr &stream) override;
  private:
      bool exit_flag_;
      //std::shared_ptr<Pthread> thread_;
      std::shared_ptr<std::thread> thread_;
      std::deque<LogContextPtr> pending_;
      //Mutex mutex_;
      //Condition sem_;
      std::mutex mutex_;
      std::condition_variable cond_;
      Logger &logger_;

  };

  ///////////////////LogChannel///////////////////
  /**
   * @brief Log channel
   *
   * 日志通道
   */
  class LogChannel : public NonCopyable{
  public:
    LogChannel(const std::string& name, LogLevel level = LTrace);
    virtual ~LogChannel();
    virtual void write(const Logger &logger,const LogContextPtr & stream) = 0;
    const std::string &name() const ;
    void setLevel(LogLevel level);

    static std::string printTime(const struct timeval &tv);
  protected:
    /**
      * 打印日志至输出流
      * @param logger Logger对象
      * @param stream 日志上下文
      * @param ost 输出流
      * @param enableColor 是否请用颜色
      * @param enableDetail 是否打印细节(函数名、源码文件名、源码行)
      */
    virtual void format(const Logger &logger,
              std::ostream &ost,
              const LogContextPtr & stream,
              bool enableColor = true,
              bool enableDetail = true);
  protected:
    std::string _name;
    LogLevel _level;
  };

  /**
   * @brief Output log stream to console
   *
   * 输出日志至终端，支持输出日志至android logcat
   */
  class ConsoleChannel : public LogChannel {
  public:
      ConsoleChannel(const std::string &name = "ConsoleChannel" , LogLevel level = LTrace) ;
      ~ConsoleChannel();
      void write(const Logger &logger , const LogContextPtr &logContext) override;
  };

  /**
   * @brief Output log stream to file
   *
   * 输出日志至文件
   */
  class FileChannel : public LogChannel {
  public:
      FileChannel(const std::string &name = "FileChannel",const std::string &path = std::string("./run") + ".log", LogLevel level = LTrace);
      ~FileChannel();

      void write(const Logger &logger , const std::shared_ptr<LogContext> &stream) override;
      void setPath(const std::string &path);
      const std::string &path() const;
  protected:
      virtual void open();
      virtual void close();
  protected:
      std::ofstream _fstream;
      std::string _path;
  };

  #define TraceL LogContextCapturer(Logger::Instance(), LTrace, __FILE__,__FUNCTION__, __LINE__)
  #define DebugL LogContextCapturer(Logger::Instance(),LDebug, __FILE__,__FUNCTION__, __LINE__)
  #define InfoL LogContextCapturer(Logger::Instance(),LInfo, __FILE__,__FUNCTION__, __LINE__)
  #define WarnL LogContextCapturer(Logger::Instance(),LWarn,__FILE__, __FUNCTION__, __LINE__)
  #define ErrorL LogContextCapturer(Logger::Instance(),LError,__FILE__, __FUNCTION__, __LINE__)
  #define WriteL(level) LogContextCapturer(Logger::Instance(),level,__FILE__, __FUNCTION__, __LINE__)


  } //namespace utils
} //namespace kiimo

#endif /* LOGGER_H_ */
