/**
 * @file utils.h
 * @brief Some function set
 * @author simon
 * @date 2019-07-31
 *
 *
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <map>
#include <initializer_list>
//#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#endif


namespace kiimo{
  namespace base{


    /// Time stamp in UTC, in microseconds resolution.
    class TimeStamp
    {
     public:
      TimeStamp(int second,int millisecond);
      TimeStamp(struct timeval *val);
      const struct timeval* GetRawData();
      uint64_t GetMillisecond() const;
      /**
       * @brief time string
       * @param format
       *        s: second style, second.usecond
       *        d: date style, year-month-day hour:minute:second
       */
      std::string ToString(std::string format = "s") const;

      bool operator==(uint64_t millisecond);
      bool operator==(TimeStamp time);
      bool operator>(TimeStamp time);
      TimeStamp operator+(TimeStamp time);
      TimeStamp operator+(uint64_t millisecond);

      /// Get system time now
      static TimeStamp Now();

     private:
      TimeStamp();
      struct timeval tv_;
    };
    /// File processing
    class File
    {
     public:
      static bool Exist(std::string path);
      static bool Delete(std::string path);
      static size_t Size(std::string file_name);
      static size_t Size(std::fstream &file);
      static std::string GetBaseName(std::string path);
    };

    /// Hex2Bin ,BASE64
    class Utils
    {
     public:

      /// @name hex bin
      /// @{
      static std::string Bin2Hex(std::vector<char> array);
      static std::string Bin2Hex(long num);
      static std::vector<char> Hex2Bin(std::string &str);
      /// @}

      /// @name bin base64
      /// @{
      static std::string Base64Encode(std::vector<char> array);
      static std::vector<char> Base64Decode(std::string &str);
      /// @}

     private:
      static std::string Char2Hex(char c);
    };

    /// CRC32
    class Crc32
    {
    public:
      Crc32(uint32_t init_val = 0xFFFFFFFF);
      ~Crc32();
      void Update(char *buffer, size_t size);
      void Update(std::string file);
      void Update(std::fstream &file);
      //void UpdateForHugeFile(char *buffer,size_t size);
      uint32_t GetValue();
    private:
      const size_t kBufferSize = 1024;
      uint32_t val_;
      char *buffer_;
    };

    /// Format output to console
    class Fmt
    {
      using string = std::string;
      using Strings = std::vector<string>;
     public:
        static string Trim(const string &);

        template<typename... Args>
        static const string Format(const string& format,const Args&... args )
        {
          Strings formats;
          std::map<size_t,FormatToken> maps;

          Sort(format,formats,maps);
          if((sizeof...(args) == 0) || maps.empty())
          {
            return format;
          }
          Strings params;
          ParseParam(maps,params,args...);

          // mapping
          for(auto it : maps)
          {
            if(it.second.index < params.size())
            {
              formats[it.first] = params[it.second.index];
            }
          }
          string res;
          for(auto &it : formats)
          {
            res.append(it);
          }
          return res;
        }
     private:
        struct FormatToken
        {
          size_t index;
          int align;
          string format_str;
        };
        inline static std::pair<size_t,size_t> FindBraces(const string foramt,size_t pos = 0)
        {
          size_t end = foramt.find('}',pos);
          size_t start = string::npos;
          if(end != string::npos)
          {
            start = foramt.rfind('{', end);
            if(start < pos)
            {
              start= string::npos;
            }
          }
          return std::make_pair(start,end);
        }
        inline static bool HasSomething(std::pair<size_t,size_t> braces)
        {
          if((braces.first < braces.second) && (braces.first + 1 < braces.second))
          {
            return true;
          }
          else
          {
            return false;
          }
        }

        // fixme
        // alignment will be implement
        /// { index[,alignment][:formatString]}
        static bool TryParseFormat(const string &str,FormatToken &token)
        {
          bool res = false;
          try
          {
            //string tmp = str.substr(1,str.size() - 2);  // rm braces
            string tmp(str,1,str.size() - 2);

            size_t d = tmp.find(',',0);
            size_t m = tmp.find(':', (d==string::npos?0:d));  // ':' MUST after ','
            if( d == string::npos && m == string::npos)
            {
              token.index = std::stoul(tmp);
            }
            else if(d == string::npos && m != string::npos)
            {
              token.index = std::stoul(tmp.substr(0 , m));
              token.format_str = Trim(tmp.substr(m + 1));
            }
            else if(d != string::npos && m == string::npos)
            {
              token.index = std::stoul(tmp.substr(0 , d));
              token.align = std::stoi(tmp.substr(d + 1));
            }
            else
            {
              token.index = std::stoul(tmp.substr(0 , d));
              token.align = std::stoi(tmp.substr(d + 1, m - d - 1));
              token.format_str = Trim(tmp.substr(m + 1));
            }

            res = true;
          }
          catch(...)
          {
            res = false;
          }
          return res;
        }
        static void Sort(const string &format,Strings &list,std::map<size_t,FormatToken> &maps)
        {
          size_t pos = 0;
          std::pair<size_t,size_t> last_braces {0,-1};
          while(pos < format.size())
          {
            auto braces = FindBraces(format,pos);
            if(HasSomething(braces))
            {
              FormatToken token {size_t(-1)};
              string one_format = format.substr(braces.first,braces.second - braces.first + 1);
              if(TryParseFormat(one_format,token))
              {
                list.push_back(format.substr(last_braces.second + 1,braces.first - (1 + last_braces.second)));  // the string before braces
                list.push_back(one_format);  // padding
                maps.emplace(list.size() - 1,token);  // mapping
                last_braces = braces;
              }
            }
            pos = (braces.second != string::npos)? (braces.second + 1) : braces.second;
          }
          if(pos != format.size())
          {
            // copy rear string
            list.push_back(format.substr(last_braces.second + 1));
          }
        }

        static string GetFormatStr(const std::map<size_t,FormatToken> &maps,size_t index)
        {
          for(auto &it : maps)
          {
            if(it.second.index == index)
            {
              return it.second.format_str;
            }
          }
          return ("");
        }
        template<typename T>
        static string GetString(const string &format_str,const T &t)
        {
          std::stringstream str;
          if(std::is_integral<T>::value)
          {
            if(format_str == "X")
            {
              str << std::uppercase << std::hex;
            }
            else if(format_str == "x")
            {
              str << std::hex;
            }
            str << t;
          }
          else
          {
            str << t;
          }
          return str.str();
        }
        template<typename... Args>
        static void ParseParam(const std::map<size_t,FormatToken> &maps,Strings &params,const Args&... args)
        {
          size_t index = 0;
          std::initializer_list<int>{(params.push_back(GetString(GetFormatStr(maps,index++),args)),0)...};
        }

    };



    class Char
    {
    public:
      Char();
      Char(int utf8); //for multichar
      Char(char c);
      Char(wchar_t c);
      Char(const Char &c);
      Char& operator=(char c);
      Char& operator=(wchar_t wc);
      Char& operator=(int utf8);    //for multichar

      bool operator==(int utf8);  //for multichar
      bool operator==(char c);
      bool operator==(wchar_t wc);
      bool operator==(Char right) const;

      bool operator!=(Char right) const;
      bool operator<(Char right) const;

      int operator-(Char right) const;

    private:
        wchar_t ch_;     // UTF16 coder almost equal unicode coder
    };

    class String
    {
     public:
        static const size_t npos = -1;
        friend String operator+(const String &,const String &);
        String();
        String(const char *s);
        String(const wchar_t *ws);
        String(const Char *s,size_t size);
        String(const String &s);
        String(String &&str) noexcept;
        ~String();



        String& operator=(const String &s);
        String& operator=(const char *s);
        String& operator=(const wchar_t *ws);
        String& operator=(String &&str) noexcept;

        size_t Size() const;
        size_t Capacity() const;

        Char& operator[](size_t pos);
        const Char& operator[](size_t pos) const;
        Char At(size_t pos) const;

        String& operator+=(const char *s);
        String& operator+=(const wchar_t *ws);
        String& operator+=(const String &str);

        std::string ConvertToBaseString();
        std::wstring ConvertToBaseWstring();

        String Substring(size_t pos = 0,size_t len = 0) const;

        int Compare(const String &str) const;

        size_t Find(Char c,size_t pos = 0) const;
        size_t Find(const char *s,size_t pos = 0) const;
        size_t Find(const wchar_t *ws,size_t pos = 0) const;
        size_t Find(const String &str,size_t pos = 0) const;

        size_t Rfind(Char c,size_t pos = 0) const;
        size_t Rfind(const String &patten,size_t pos = 0) const;

        String& Erase(size_t pos = 0,size_t len = npos);

        // fixme
        //  20200623: only basic function,NO check arguement ...
        template<typename ... Args>
        static const String Format(const String &format,const Args... args)
        {
          // expand parameter pack
          std::vector<String> list;
          std::initializer_list<int>{(list.push_back(PrintArgs(args)),0)...};
          //[&]{std::initializer_list<int>{(list.push_back(PrintArgs(args)),0)...};}();
          String str;
          size_t start = format.Find('{');
          size_t end = format.Find('}',start);
          size_t index = 0;
          size_t last_end = 0;
          while(start != String::npos && end != String::npos)
          {
            if((end - start) > 1)
            {
              int i = std::stoi((format.Substring(start + 1,end - start)).ConvertToBaseString());
              str += format.Substring(last_end,start - last_end + 1);
              str += list[i];
            }
            last_end = end + 1;
            index++;
            start = format.Find('{',last_end);
            end = format.Find('}',start);
          }

          return str;
        }

     private:
        void CopyUtf8String(const char *s,size_t start = 0);
        void CopyUtf16String(const wchar_t *ws,size_t start = 0);
        void CopyString(const String &str,size_t start = 0);
        Char& Indexof(size_t pos) const;
        template<typename T> static String PrintArgs(T t)
        {
          std::stringstream sstr("");
          sstr << t;
          return {sstr.str().c_str()};
        }
     private:
       size_t size_;
       size_t capacity_;
       Char *ustring_;
    };

  } //namespace base
} //namespace kiimo


#endif /* UTILS_H_ */
