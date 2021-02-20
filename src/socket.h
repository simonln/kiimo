/**
 * @file socket.h
 * @brief Wrap socket API
 * @author simon
 * @date 2019-08-02
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <memory>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
//#include <ws2tcpip.h>   //include in winsock2.h
#else
#include <netinet/in.h>
#endif


namespace kiimo{
  namespace net{

    /// socket error code
    enum class SocketErrCode
    {
      kInitfailed,
      kConnectFailed,
      kDisconnect,
      kIpWrong,
      kRemoteFailed,
    };
    class Socket;
    /**
     *  Represent as IP end point,it contains IP address and port.
     */
    class IpEndPoint
    {
     public:
      friend Socket;
      IpEndPoint();
      ~IpEndPoint() = default;
      IpEndPoint(const std::string ip,const int port);
      IpEndPoint(struct sockaddr* address);
      /// Get IP address
      std::string GetIp() const;
      /// Get port number
      int GetPort() const;
      /// Get port number with string format
      std::string GetPortString() const;
      /// Get IP address and port ,format is "x.x.x.x:xxx"
      std::string ToString() const;
      //const struct sockaddr* GetRawStruct() const;
     private:
      const struct sockaddr* GetRawStruct() const;
    public:
      /// local host IP address, "0.0.0.0"
      static const char *kAny;
     private:
      struct sockaddr_in host_addr_;
    };

    /// Socket type, TCP and UDP
    enum class SocketType {kTcpSocket,kUdpSocket,};

    /**
     *  @brief Wrap system socket API
     */
    class Socket
    {
     public:
#ifdef _WIN32
      typedef SOCKET  Id;
#else
      typedef int Id;
#endif

      typedef std::shared_ptr<Socket> Ptr;
      Socket(); ///< default client socket
      Socket(Id id,IpEndPoint remote);  ///< client created from accept
      Socket(int port);  ///< server socket
      ~Socket() = default;

      /**
       *  @brief initialize the socket
       *  @param kind Socket::SocketType
       *  @note
       *     If client socket is created from accept ,this function is \b NOT required
       */
      void Initialize(SocketType kind = SocketType::kTcpSocket);

      /// @name For server
      /// @{
      /**
       * @brief Socket bind to specify port
       */
      bool BindToPort();
      bool BindToPort(int port);

      /**
       *  @brief Wait client be accepted
       *  @return
       *    The pointer of client
       */
      Ptr Accept();
      /// @}

      /**
       *  @brief Connect to remote host
       *  @param remote remote host IP address and port
       */
      void Connect(IpEndPoint remote);
      /**
       *  @brief Receive data from remote host
       *  @param [out] buffer buffer for received data
       *  @param max_size the size of buffer
       *  @note
       *    This function could block the socket IO
       */
      int Recv(char *buffer,int max_size);

      /**
       * @brief receive data from  UDP socket
       * @param remote Specify remote host
       * @param [out] buffer buffer for received data
       * @param max_size the size of buffer
       * @note
       *  This function could block the socket IO
       */
      int Recv(IpEndPoint &remote,char *buffer,int max_size);

      /**
       * Gets the amount of data that has been received from the network and is available to be read.
       * on success >=0 is returned,On error -1 is returned
       */
      int Avaliable();

      /**
       *  Get a value that indicate whether a socket is connected to a remote host
       **/
      bool Connected() const;

      /**
       *  @brief  Send data to remote host
       *  @param [in] buffer The buffer of containing data be send
       *  @param size The size of data
       *  @note
       *    This function would block socket IO
       */
      int Send(const char *buffer,int size);
      /**
       *  @brief  Send data to remote host with UDP socket
       *  @param remote The remote host
       *  @param [in] buffer The data buffer
       *  @param size The size of data
       *  @note
       *    This function would block socket IO
       */
      int Send(const IpEndPoint remote,const char *buffer,int size);

      /**
       *  @brief Close socket
       */
      void Close();

      /**
       *  @brief Get local host information
       *  @return Local host contains IP address and port
       */
      const IpEndPoint GetLocalHost() const;
      /**
       *  @brief Get remote host information
       *  @return Remote host contains IP address and port
       */
      const IpEndPoint GetRemoteHost() const;
      /// Get socket id
      const Id GetId() const;
      /// Get socket type
      SocketType GetType() const;

      /// For std::set container
      bool operator < (const Socket &a) const;

     private:
      bool is_server_;
      bool from_accept_;
      Id id_;
      IpEndPoint local_;
      IpEndPoint remote_;
      SocketType kind_;
      bool connected_;
    };

    /// Socket Exception
    class SocketException :public std::exception
    {
    public:
      /**
       *  @brief Constructor
       *  @param code socket error code
       *  @param msg error message
       */
      SocketException(SocketErrCode code , const std::string &msg)
        :code_(code) , err_msg_(msg)
      {
      }
      /**
       *   Get error message
       */
      virtual const char *what() const noexcept
      {
        return err_msg_.data();
      }
      /**
       *   Get error code
       */
      const SocketErrCode GetErrorCode() const
      {
        return code_;
      }
    private:
      SocketErrCode code_;
      std::string err_msg_;
    };

  } //namespace net
} //namespace kiimo



#endif /* SOCKET_H_ */
