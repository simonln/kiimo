/**
 * 	@file parser.cc
 *  @author simon
 *	@version V1.0
 *  @date 2020-04-09
 *  @brief first version
 */
#include "parser.h"

#include <cstring>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "../buffer.h"


using namespace kiimo::net;
using std::vector;
using std::string;

static const char* kNewLine = "\r\n";



static vector<string> StringSplit(const std::string &dest,const std::string &delimiter)
{
 std::vector<std::string> res;

 if(dest.size() <= delimiter.size())
 {
   return res;
 }

 size_t start = 0 , end = dest.size() - delimiter.size();

 // remove delimiter in head
 size_t index = 0;
 while(index == dest.find(delimiter,index))
 {
   index += delimiter.size();
 }
 start = index;

 while(start < end)
 {
   size_t pos = dest.find(delimiter,start);
   if( pos != std::string::npos)
   {
     res.push_back(dest.substr(start,pos - start));
     start = pos + delimiter.size();
   }
   else
   {
     res.push_back(dest.substr(start));
     break;
   }
 }
 return res;
}

HttpParser::ParseRequestStep HttpParser::step_ = ParseRequestStep::kRequestLine;

static bool ParseRequestLine(Slice req_line,HttpRequest &request)
{
  const char *start = req_line.data();
  const char *end = req_line.data() + req_line.size();
  const char *space = std::find(start, end , ' ');
  if(space)
  {
    // request method
    Slice method = Slice(start,space - start);
    if(method == Slice("GET"))
    {
      request.method = RequestMethod::GET;
    }
    else if(method == Slice("POST"))
    {
      request.method = RequestMethod::POST;
    }
    // request path
    start = space + 1;
    space = std::find(start,end,' ');
    if(space)
    {
      request.path = Slice(start,space - start).ToString();

      if((end - space == 8) && (Slice(start,7) == Slice("HTTP/1.")))
      {
        if(*(end - 1) == '0')
        {
          request.http_version = HttpVersion::Ver10;
        }
        else
        {
          request.http_version = HttpVersion::Ver11;
        }

        return true;
      }
    }
  }
  return false;
}

bool HttpParser::Parse(Message &buff, HttpRequest &request)
{
  const char *pos = buff.FindCRLF();
  if(pos && ParseRequestLine(Slice(buff.data(),pos - buff.data()),request))
  {
    buff.Skip(pos -buff.data());
    step_ = ParseRequestStep::kRequestHeader;

  }


  return (step_ == ParseRequestStep::kParseComplete);
}

/**
 *  find the position of \r\n
 *  return -1: none
 *          other: postion
 */

static int FindLineSperator(const std::vector<char> &buff,int start = 0)
{
  int size = buff.size();
  if(size < 2)
  {
    return -1;
  }

  int ret = -1;
  size = size-1;
  for(auto i = start; i < size; ++i)
  {
    if(buff[i + 1] == '\n')
    {
      if(buff[i] == '\r')
      {
        ret = i;
        break;
      }
      else
      {
        i++;
      }
    }
    else if(buff[i+1] == '\r')
    {
      continue;
    }
    else
    {
      i++;
    }
  }
  return ret;
}

bool HttpParser::Parse(const std::vector<char> &buff, HttpRequest &request)
{
  bool res = false;
  int pos = -1;
  int last_pos = -1;
  pos = FindLineSperator(buff);

  if(pos != -1)
  {
    auto start = buff.begin();
    auto end = buff.begin() + pos;
    auto space = std::find(buff.begin(), end, ' ');
    if(space != end)
    {
      string method(buff.begin(),space);
      if(method == "GET")
      {
        request.method = RequestMethod::GET;
      }
      else if(method == "POST")
      {
        request.method = RequestMethod::POST;
      }
      else
      {

      }
      start = space + 1;
      space = std::find(start,end, ' ');
      if(space != end)
      {
        request.path = string(start ,space);
        start = space + 1;
        if((end - start == 8) && std::equal(start, end-1, "HTTP/1."))
        {
          if(*(end - 1) == '0')
          {
            request.http_version = HttpVersion::Ver10;
          }
          else
          {
            request.http_version = HttpVersion::Ver11;
          }
        }
        else
        {
          res = false;
        }
      }
    }
    last_pos = pos + 2;
    pos = FindLineSperator(buff,last_pos);
    while((pos != -1) && (pos != last_pos))
    {
//      auto item = StringSplit(string(buff.data() + last_pos,buff.data() + pos),": ");
//      if(item.size() > 1)
//      {
//        request.headers.emplace(item[0],item[1]);
//      }
      auto start = buff.begin() + last_pos;
      auto end = buff.begin() + pos;
      auto colo = std::find(start,end,':');
      if(colo != end)
      {
        request.headers.emplace(string(start,colo),string(colo + 2,end));
      }
      last_pos = pos + 2;
      pos = FindLineSperator(buff,last_pos);
    }
    if(pos == last_pos)
    {
      request.content.assign(buff.begin() + (pos + 2),buff.end() );
    }
  }
  return res;
}

void HttpParser::Deparse(const HttpResponse &response,std::vector<char> &raw_rsp)
{
  // first response line
//  std::string headers = "HTTP/1.1 ";
//  headers += std::to_string(response.state_code) + " " + response.state_desc + kNewLine;
//
//  //headers
//  headers += "Server: Windows 10\r\n";
//  headers += "Content-Type: " + response.content_type + kNewLine;
//  headers += "Content-Length: " + std::to_string(response.content.size()) + kNewLine;
//
//  //main body
//  headers += kNewLine;
//
//  // copy to array
////  raw_rsp.clear();
////  raw_rsp.insert(raw_rsp.end(),headers.begin(),headers.end());
////  raw_rsp.insert(raw_rsp.end(),response.content.begin(),response.content.end());
//  raw_rsp.resize(headers.size() + response.content.size());
//  memcpy(raw_rsp.data(),headers.data(),headers.size());
//  memcpy(raw_rsp.data() + headers.size(),response.content.data(),response.content.size());
  char buff[128];
  int n = snprintf(buff,sizeof(buff),"HTTP/1.1 %d ",response.state_code);
  std::string headers;
  headers.append(buff,n);
  headers.append(response.state_desc);
  headers.append(kNewLine);

  headers.append("Server: Windows 10\r\n");
  headers.append("Content-Type: ");
  headers.append(response.content_type);
  headers.append(kNewLine);
  n = snprintf(buff,sizeof(buff),"Content-Length: %zd\r\n",response.content.size());
  headers.append(buff,n);

  for(auto &it :response.headers)
  {
    headers.append(it.first);
    headers.append(": ");
    headers.append(it.second);
    headers.append(kNewLine);
  }
  headers.append(kNewLine);   // header end
  raw_rsp.resize(headers.size() + response.content.size());
  memcpy(raw_rsp.data(),headers.data(),headers.size());
  memcpy(raw_rsp.data() + headers.size(),response.content.data(),response.content.size());
}

#ifdef UNITTEST
#include <gtest/gtest.h>
TEST(StringSplit,Base)
{
  string dest = "hello world !\r\nIt is a simple comment";

  vector<string> v1 = StringSplit(dest," ");
  EXPECT_TRUE(v1.size() == 7);
  EXPECT_TRUE(v1[1] == "world");

  vector<string> v2 = StringSplit(dest,"\r\n");
  EXPECT_TRUE(v2.size() == 2);

  vector<string> v3 = StringSplit(dest,"zero");
  EXPECT_FALSE(v3.size() == 2);

  string dest1 = "\r\n\r\n\r\n\r\n\r\n\r\n hello\r\nworld";
  vector<string> v4 = StringSplit(dest1,"\r\n");
  EXPECT_TRUE(v4.size() == 2);
  EXPECT_EQ(v4[1],string("world"));
}

TEST(FindLineSperator,Base)
{
  char dest[] = "hello world !\r\nIt is a simple comment\r\n";
  std::vector<char> buff(dest,dest + sizeof(dest));

  EXPECT_EQ(FindLineSperator(buff),13);
  EXPECT_EQ(FindLineSperator(buff,14),37);

}


#endif
