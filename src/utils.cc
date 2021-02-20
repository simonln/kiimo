/**
 * @file utils.cc
 * @brief Some function set
 * @author simon
 * @date 2019-07-31
 *
 *
 */
#include "utils.h"

#include <cstring>
#include <cstdint>
#include <ctime>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <codecvt>
#include <locale>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#endif


#ifdef UNITTEST
#include <gtest/gtest.h>
#endif


//#include "logger.h"

/// Is a number
#ifndef ISNUMBER
#define ISNUMBER(x) (((x) >= '0') && ((x) <= '9'))
#endif
/// Is a high case character
#ifndef ISHIGHCASE
#define ISHIGHCASE(x) (((x) >= 'A') && ((x) <= 'F'))
#endif

using namespace std;
using namespace kiimo::base;

//TimeStamp


TimeStamp::TimeStamp(int second,int millisecond)
{
    tv_.tv_sec = (second < 0)? 0 : second;
    tv_.tv_usec = (millisecond < 0)? 0 : millisecond*1000L;
}

TimeStamp::TimeStamp(struct timeval *val)
{
  if(val != nullptr)
  {
    tv_.tv_sec = (val->tv_sec < 0)? 0 : (val->tv_sec);
    tv_.tv_usec = (val->tv_usec < 0)? 0 : (val->tv_usec);
  }
  else
  {
    tv_.tv_sec = 0;
    tv_.tv_usec = 0;
  }

}

const struct timeval* TimeStamp::GetRawData()
{
    return (&tv_);
}

uint64_t TimeStamp::GetMillisecond() const
{
  return (tv_.tv_sec * 1000ULL + tv_.tv_usec/1000);
}

string TimeStamp::ToString(std::string format) const
{
  string res = "";
  if(format == "d")
  {
    time_t raw_time = tv_.tv_sec;
    struct tm *time_info;
    time_info = ::localtime(&raw_time);
    res += std::to_string(time_info->tm_year + 1900) + "-";
    res += std::to_string(time_info->tm_mon + 1) + "-";
    res += std::to_string(time_info->tm_mday) + " ";
    res += std::to_string(time_info->tm_hour) + ":";
    res += std::to_string(time_info->tm_min) + ":";
    res += std::to_string(time_info->tm_sec);
  }
  else
  {
    res += to_string(tv_.tv_sec);
    res += ".";
    res += to_string(tv_.tv_usec/1000);
  }
  return res;
}


bool TimeStamp::operator ==(uint64_t millisecond)
{
  uint64_t val = tv_.tv_sec * 1000ULL + tv_.tv_usec/1000;
  if(val == millisecond)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool TimeStamp::operator ==(TimeStamp time)
{
  if((time.GetRawData()->tv_sec == tv_.tv_sec) &&
                    (time.GetRawData()->tv_usec == tv_.tv_usec))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool TimeStamp::operator >(TimeStamp time)
{
  if(tv_.tv_sec == time.GetRawData()->tv_sec)
  {
    return (tv_.tv_usec > time.GetRawData()->tv_usec);
  }
  else
  {
    return (tv_.tv_sec > time.GetRawData()->tv_sec);
  }

}

TimeStamp TimeStamp::operator+(TimeStamp time)
{
  struct timeval val;
  val.tv_sec = tv_.tv_sec + time.GetRawData()->tv_sec;
  val.tv_usec = tv_.tv_usec + time.GetRawData()->tv_usec;
  return {&val};
}
TimeStamp TimeStamp::operator+(uint64_t millisecond)
{
  struct timeval val;
  val.tv_sec = tv_.tv_sec + millisecond/1000;
  val.tv_usec = tv_.tv_usec + (millisecond%1000)*1000;
  return {&val};
}

#ifdef _WIN32
int gettimeofday(struct timeval * tp, struct timezone * /*tzp*/)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif
TimeStamp TimeStamp::Now()
{
  struct timeval tv;
  gettimeofday(&tv,nullptr);
  return {&tv};
}

bool File::Exist(string path)
{
#ifdef _WIN32
  auto ret = GetFileAttributesA(path.c_str());
  if((ret != INVALID_FILE_ATTRIBUTES) && !(ret & FILE_ATTRIBUTE_DIRECTORY))
  {
    return true;
  }
#else
  struct stat buffer;
  if(0 == stat(path.c_str(),&buffer))
  {
    return true;
  }
#endif
  return false;
}

bool File::Delete(string path)
{
#ifdef _WIN32
  if(DeleteFileA(path.c_str()))
  {
    return true;
  }
#endif
  return false;
}


size_t File::Size(string file_name)
{
  size_t res = 0;
  fstream fs;
  try
  {
    fs.open(file_name,ios::in | ios::binary);
    if(fs.is_open())
    {
      fs.seekg(0,ios::end);
      res = fs.tellg();
      fs.close();
    }

  }
  catch(...)
  {
    if(fs.is_open())
    {
      fs.close();
    }
  }
  return res;
}

size_t File::Size(std::fstream &file)
{
  size_t res = 0;
  if(file.is_open())
  {
    file.seekg(0,std::ios::end);
    res = file.tellg();
    file.seekg(0,std::ios::beg);
  }
  return res;
}

std::string File::GetBaseName(std::string path)
{
  auto pos = path.find_last_of("/\\");
  if (pos != std::string::npos)
  {
    return path.substr(pos+1);
  }
  else
  {
    return path;
  }
}

string Utils::Bin2Hex(vector<char> array)
{
  string res = "";
  for(auto it = array.begin();it != array.end();it++)
  {
    res += Char2Hex(*it);
  }
  return res;
}

string Utils::Bin2Hex(long num)
{
  string res = "";
  char temp;
  for(int i = sizeof(long) - 1;i >= 0;i--)
  {
    temp = num >> (i*8);
    if(temp != 0)
    {
      res += Char2Hex(temp);
    }
  }
  return res;
}



vector<char> Utils::Hex2Bin(string &data)
{
  vector<char> array;
  unsigned char temp = 0;
  if(data.size()%2)
  {
    return array;
  }
  for(size_t i = 0;i < data.size();i += 2)
  {
    temp = 0;
    if(ISNUMBER(data[i]))
    {
      //number
      temp += (data[i] - '0') * 16;
      if(ISNUMBER(data[i+1]))
      {
        temp += data[i+1] - '0';
      }
      else if(ISHIGHCASE(data[i+1]))
      {
        temp += data[i+1] - 'A' + 10;
      }
      else
      {
        continue;
      }

    }
    else if(ISHIGHCASE(data[i]))
    {
      //Aalpm
      temp += (data[i] - 'A'  + 10) * 16;
      if(ISNUMBER(data[i+1]))
      {
        temp += data[i+1] - '0';
      }
      else if(ISHIGHCASE(data[i+1]))
      {
        temp += data[i+1] - 'A' + 10;
      }
      else
      {
        continue;
      }
    }
    else
    {
      continue;
    }
    array.push_back(temp);
  }
  return array;
}

/**
 * return the hex format of a char
 * if the value less 16,pad zero in the head
 * the length of result is 2
 */
string Utils::Char2Hex(const char c)
{
  string res = "";
  char temp;

  for(int i = 0;i < 2;i++)
  {
    //get 4 bit
    temp = (c >> (4*(1-i))) & 0x0F;
    if(temp > 9)
    {
      res += 'A' + (temp - 10);
    }
    else
    {
      res += '0' + temp;
    }
  }
  return res;
}

/// BASE64 encode sheet
static const char kEncodeChars[64] =
{
    'A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X',
    'Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3',
    '4','5','6','7','8','9','+','/'
};

/// encode union for BASE64
union EncodeUnion
{
#pragma pack(4)
  uint32_t value;
  uint8_t raw_data[4];
  struct
  {
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
  };
#pragma pack()
};
string Utils::Base64Encode(vector<char> array)
{
  string res = "";
  EncodeUnion encoder;
  size_t i = 0;
  size_t array_size = array.size();

  for(;i < array_size/3;i++)
  {
    encoder.value = 0;
    //copy data to union
    for(int j = 0;j < 3;j++)
    {
      encoder.raw_data[2-j] = array[3*i+j];
    }
    for(int j = 3;j >= 0;j--)
    {
      res += kEncodeChars[(encoder.value >> (6*j)) & 0x3F];
    }
  }
  i = i*3;
  if(array_size%3 == 2)
  {
    encoder.value = 0;
    encoder.raw_data[2] = array[i];
    encoder.raw_data[1] = array[i+1];

    res += kEncodeChars[(encoder.value >> 18)&0x3F];
    res += kEncodeChars[(encoder.value >> 12)&0x3F];
    res += kEncodeChars[(encoder.value >> 6)&0x3F];
    res += '=';
  }
  else if(array_size%3 == 1)
  {
    encoder.value = 0;
    encoder.raw_data[2] = array[i];

    res += kEncodeChars[(encoder.value >> 18)&0x3F];
    res += kEncodeChars[(encoder.value >> 12)&0x3F];
    res += "==";
  }
  return res;
}
/// Get index of character in Encode sheet
static char GetIndex(const char ch)
{
  if(ch >= 'A' && ch <= 'Z')
  {
    return (ch - 'A');
  }
  else if(ch >= 'a' && ch <= 'z')
  {
    return (ch - 'a' + 26);
  }
  else if(ch >= '0' && ch <= '9')
  {
    return (ch - '0' + 52);
  }
  else
  {
    if(ch == '+')
    {
      return 62;
    }
    else if(ch == '/')
    {
      return 63;
    }
    else
    {
      return 0;
    }
  }

}

vector<char> Utils::Base64Decode(string &str)
{
  size_t i = 0;
  EncodeUnion decoder;
  vector<char> res;
  size_t str_size = str.size();

  if(str_size % 4)
  {
    return res;
  }
  for(;(i + 3 < str_size) && (str[i + 3] != '='); i += 4)
  {
      decoder.value = 0;
      for(auto j = 0;j < 4; ++j )
      {
        decoder.value += GetIndex(str.data()[j + i]) << (6 * (3 - j));
      }
      res.push_back(decoder.byte3);
      res.push_back(decoder.byte2);
      res.push_back(decoder.byte1);
  }

  if(str[str_size - 2] == '=')
  {
    //two =
    decoder.value = 0;
    decoder.value += GetIndex(str.data()[i++]) << 18;
    decoder.value += GetIndex(str.data()[i++]) << 12;
    res.push_back(decoder.byte3);
  }
  else if(str[str_size - 1] == '=')
  {
    //one =
    decoder.value = 0;
    decoder.value += GetIndex(str.data()[i++]) << 18;
    decoder.value += GetIndex(str.data()[i++]) << 12;
    decoder.value += GetIndex(str.data()[i++]) << 6;
    res.push_back(decoder.byte3);
    res.push_back(decoder.byte2);
  }
  else
  {

  }
  return res;

}



//CRC32

/// CRC32 accelerate calculate sheet
static uint32_t  crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

Crc32::Crc32(uint32_t init_val)
  :val_(init_val),buffer_(nullptr)
{
   buffer_ = new char[kBufferSize];
}
Crc32::~Crc32()
{
  if(buffer_ != nullptr)
  {
    delete buffer_;
  }
}

void Crc32::Update(char *buffer,size_t size)
{
  for (size_t i = 0;i < size; ++i)
  {
    val_ = (crc_32_tab[((val_) ^ ((uint8_t)buffer[i])) & 0xff] ^ ((val_) >> 8));
  }
}

void Crc32::Update(std::string file)
{
    fstream fs;
    char *buff = buffer_;
    val_ = 0xFFFFFFFF;
    try
    {
      fs.open(file,std::ios::binary | std::ios::in);
      while(!fs.eof())
      {
        fs.read(buff,kBufferSize);
        this->Update(buff,fs.gcount());
      }
      fs.close();
    }
    catch(std::exception &ex)
    {
      if(fs.is_open())
      {
        fs.close();
      }
      throw ex;
    }
}

void Crc32::Update(std::fstream &file)
{
  //char *buffer = new char[kBufferSize];
  char *buff = buffer_;
  val_ = 0xFFFFFFFF;
  while(!file.eof())
  {
    file.read(buff,kBufferSize);
    this->Update(buff,file.gcount());
  }

  file.clear();
  file.seekg(0,std::ios::beg);

  //delete[] buffer;
}


uint32_t Crc32::GetValue()
{
  uint32_t ret = ~val_;
  val_ = 0xFFFFFFFF;
  return ret;
}



std::string Fmt::Trim(const std::string &str)
{
  /*
  std::string tmp = str;
  int pos= 0;
  while(tmp[pos] == ' ' || tmp[pos] == '\t')   // space or tab
  {
    pos++;
  }
  tmp.erase(0,pos);
  pos = tmp.size() - 1;
  while(tmp[pos] == ' ' || tmp[pos] == '\t')
  {
    pos--;
  }
  tmp  = tmp.substr(0,pos + 1);
  return tmp;
  */
  if(str.size() == 0)
  {
    return ("");
  }
  size_t start = 0;
  while((start < str.size()) && (!isgraph(str[start])))
  {
    start++;
  }
  size_t end = str.size();
  while((end > 0) && (!isgraph(str[--end])));   // find last visible character
  if(end >= start)
  {
    return str.substr(start,end - start + 1);
  }
  else
  {
    return ("");
  }

}


//static uint32_t Utf16ToUtf8(wchar_t wc)
//{
//  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
//  std::string mbs = convert.to_bytes(wc);
//  uint32_t utf8 = 0;
//  for(size_t i = 0; i < mbs.size() ; ++i)
//  {
//    utf8 <<= 8;
//    utf8 += (uint8_t)mbs[i];
//  }
//  return utf8;
//}

static wchar_t Utf8ToUtf16(int utf8)
{
  wchar_t ch = 0;
  if((utf8 & 0xF8000000) == 0xF0000000)
  {
    ch = utf8 & 0x3F;
    ch += (utf8 & 0x3F00) >> 2;
    ch += (utf8 & 0x3F0000) >> 4;
    ch += (utf8 & 0x7000000) >> 6;
  }
  else if((utf8 & 0xF00000) == 0xE00000)
  {
    ch  = utf8 & 0x3F;
    ch += (utf8 & 0x3F00) >> 2;
    ch += (utf8 & 0xF0000) >> 4;
  }
  else if((utf8 & 0xE000) == 0xC000)
  {
    ch = utf8 & 0x3F;
    ch += (utf8 & 0x1F00) >> 2;
  }
  else if(utf8 <= 0x7F)
  {
    ch = utf8 & 0x7F;
  }
  else
  {
    ch = 0;
  }
  return ch;
}

// Char implement
Char::Char()
  :ch_(0)
{
}

Char::Char(int utf8)
  :ch_(Utf8ToUtf16(utf8))
{
}
Char::Char(char c)
  :ch_(c & 0x7F)
{
}

Char::Char(wchar_t c)
  :ch_(c)
{
}

Char::Char(const Char &c)
  :ch_(c.ch_)
{
}

Char& Char::operator =(char c)
{
  ch_ = c & 0x7F;
  return *this;
}

Char& Char::operator =(wchar_t c)
{
  ch_ = c;
  return *this;
}
Char& Char::operator=(int utf8)
{
  ch_ = Utf8ToUtf16(utf8);
  return *this;
}

bool Char::operator==(int utf8)
{
  wchar_t wc = Utf8ToUtf16(utf8);
  return (wc == ch_);
}

bool Char::operator==(char c)
{
  return (ch_ == c);
}

bool Char::operator ==(wchar_t wc)
{
  return (ch_ == wc);
}

bool Char::operator==(Char right) const
{
  return (ch_ == right.ch_);
}

bool Char::operator !=(Char right) const
{
  return (ch_ != right.ch_);
}

bool Char::operator<(Char right) const
{
  return (ch_ < right.ch_);
}

int Char::operator-(Char right) const
{
  return (ch_ - right.ch_);
}

static size_t RoundUp2Power(size_t raw_size)
{
    size_t num = 1;
    while(num < raw_size)
    {
        num <<= 1;
    }
    return num;
}


/// String implement
const size_t String::npos;
String::String()
  :size_(0),capacity_(128),ustring_(new Char[capacity_])
{
  ustring_[0] = L'\0';
}


void String::CopyUtf8String(const char *s,size_t start)
{
  size_t len = ::strlen(s);
  if((start + len) > capacity_)
  {
    Char *oldptr = ustring_;
    ustring_ = new Char[start + len + 1];

    //copy old dara
    //::memcpy(ustring_,oldptr,size_ * sizeof(Char));
    for(size_t i = 0 ; i < size_; ++i)
    {
      ustring_[i] = oldptr[i];
    }

    capacity_ = start + len + 1;
    delete[] oldptr;
  }
  //size_ = ::mbstowcs((wchar_t*)ustring_,s,len);
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;
  std::wstring ws = cvt.from_bytes(s);
  size_ = start +  ws.size();
  for(size_t i = 0 ; i < ws.size(); ++i)
  {
    ustring_[start + i] = ws[i];
  }
  ustring_[size_] = L'\0';
}

String::String(const char *s)
  :String()
{
  CopyUtf8String(s);
}


void String::CopyUtf16String(const wchar_t *ws,size_t start)
{
  size_t len = ::wcslen(ws);
  if((start + len) > capacity_)
  {
    Char *oldptr = ustring_;
    size_t new_size = RoundUp2Power(start + len);
    ustring_ = new Char[new_size];

    //copy old data
    for(size_t i = 0 ; i < size_; ++i)
    {
      ustring_[i] = oldptr[i];
    }

    capacity_ = new_size;
    delete[] oldptr;
  }
  size_ = start + len;
  //::wcscpy((wchar_t *)ustring_,ws);
  for(size_t i = 0; i < len; ++i)
  {
    ustring_[start + i] = ws[i];
  }
  ustring_[size_] = L'\0';
}

String::String(const wchar_t *ws)
  :String()
{
  CopyUtf16String(ws);
}

String::String(const String &s)
  :size_(s.size_),capacity_(s.capacity_),ustring_(new Char[capacity_])
{
  //::memcpy(ustring_,s.ustring_,sizeof(Char) * s.Size());
  for(size_t i = 0 ; i < s.Size();++i)
  {
    ustring_[i] = s.ustring_[i];
  }
  ustring_[size_] = L'\0';
}

String::String(const Char *s,size_t size)
  :String()
{
  if(size > capacity_)
  {
    Char *oldptr = ustring_;
    size_t new_size = RoundUp2Power(size);
    ustring_ = new Char[new_size];
    capacity_ = new_size;
    delete[] oldptr;
  }
  for(size_t i = 0; i < size; ++i)
  {
    ustring_[i] = s[i];
  }
  size_ = size;
}

String::String(String &&str) noexcept
{
  if(&str != this)
  {
    size_ = str.size_;
    capacity_ = str.capacity_;
    ustring_ = str.ustring_;
    str.ustring_ =  nullptr;
  }
}

String::~String()
{
  if(ustring_ != nullptr)
  {
    delete[] ustring_;
  }
}

String& String::operator =(const char *s)
{
  CopyUtf8String(s);
  return *this;
}

String& String::operator =(const wchar_t *ws)
{
  CopyUtf16String(ws);
  return *this;
}

void String::CopyString(const String &str,size_t start)
{
  if((start + str.size_) > capacity_)
  {
    Char *oldptr = ustring_;
    size_t new_size = RoundUp2Power(start + str.size_ );
    ustring_ = new Char[new_size];

    // copy old data
    for(size_t i = 0 ; i < size_; ++i)
    {
      ustring_[i] = oldptr[i];
    }
    capacity_ = new_size;
    delete[] oldptr;
  }
  size_ = start + str.size_;

  for(size_t i = 0 ; i < str.size_;++i)
  {
    ustring_[start + i] = str.ustring_[i];
  }
  ustring_[size_] = L'\0';
}
String& String::operator =(const String &s)
{
  CopyString(s);
  return *this;
}

String& String::operator=(String &&str) noexcept
{
  if(&str != this)
  {
    delete[] ustring_;
    size_ = str.size_;
    capacity_ = str.capacity_;
    ustring_ = str.ustring_;
    str.ustring_ = nullptr;
  }
  return *this;
}

size_t String::Size() const
{
  return size_;
}

size_t String::Capacity() const
{
  return capacity_;
}

Char& String::Indexof(size_t pos) const
{
    if(pos < 0 || pos > size_)
    {
      return ustring_[size_];
    }
    else
    {
      return ustring_[pos];
    }
}

Char& String::operator[](size_t pos)
{
  return Indexof(pos);
}


const Char& String::operator[](size_t pos) const
{
  return Indexof(pos);
}

Char String::At(size_t pos) const
{
  return Indexof(pos);
}


String& String::operator +=(const char *s)
{
  CopyUtf8String(s,size_);
  return *this;
}

String& String::operator +=(const wchar_t *ws)
{
  CopyUtf16String(ws,size_);
  return *this;
}

String& String::operator +=(const String &str)
{
  CopyString(str,size_);
  return *this;
}

std::string String::ConvertToBaseString()
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;
  return cvt.to_bytes((wchar_t*)ustring_);
}

std::wstring String::ConvertToBaseWstring()
{
  return std::wstring((wchar_t*)ustring_);
}


String String::Substring(size_t pos,size_t len) const
{
  if(pos >= size_ || len == 0)
  {
    return {""};
  }
  else
  {
    if((pos + len) > size_)
    {
      return {ustring_ + pos, size_ - pos};
    }
    else
    {
      return {ustring_ + pos,len};
    }
  }
}

int String::Compare(const String &str) const
{
  if(size_ != str.size_)
  {
    return (str.size_ - size_);
  }
  for(size_t i = 0; i < size_; ++i)
  {
    if(str[i] != ustring_[i])
    {
      return (str[i] - ustring_[i]);
    }
  }
  return 0;
}

size_t String::Find(Char c,size_t pos) const
{
  for(size_t i = pos; i < size_; ++i)
  {
    if(ustring_[i] == c)
    {
      return i;
    }
  }
  return npos;
}

size_t String::Find(const String &patten ,size_t pos) const
{
  size_t  i = pos;
  size_t j = 0;

  while( (i < size_) && (j < patten.size_))
  {
    if(patten[j] != ustring_[i])
    {
      size_t rear = i + (patten.size_ - j);
      if(rear < size_)
      {
        j = 0;
        size_t index = patten.Rfind(ustring_[rear]);
        if(index != patten.npos)
        {
          i = rear - index;
        }
        else
        {
          i = rear + 1;
        }
      }
      else
      {
        break;
      }
    }
    else
    {
      ++i;
      ++j;
    }
  }
  if(j == patten.size_)
  {
    return (i - patten.size_);
  }
  else
  {
    return npos;
  }
}

// Sunday algorithm
size_t String::Find(const char *s,size_t pos) const
{
  String pat(s);
  return Find(pat,pos);
}

size_t String::Find(const wchar_t *ws,size_t pos) const
{
  String pat(ws);
  return Find(pat,pos);
}

size_t String::Rfind(Char c,size_t pos) const
{
  if(pos >= size_)
  {
    return npos;
  }

  if(pos == 0)
  {
    pos = size_ - 1;
  }
  for(size_t i = 0; i <= pos; ++i)
  {
    if(ustring_[pos - i] == c)
    {
      return (pos - i);
    }
  }
  return npos;
}

size_t String::Rfind(const String &patten,size_t pos) const
{
  if((pos >= size_) || (patten.size_ > size_))
  {
    return npos;
  }

  if(pos == 0)
  {
    pos = size_ - 1;
  }
  int i = pos, j = patten.size_ - 1;
  while((i >=0) && (j >= 0))
  {
    if(ustring_[i] != patten[j])
    {

      int head = i - j - 1;
      if(head >= 0)
      {
        j = patten.size_ - 1;
        size_t index = patten.Find(ustring_[head]);
        if(index != patten.npos)
        {
          i = head + patten.size_ - index - 1;
        }
        else
        {
          i = head - 1;
        }
      }
      else
      {
        break;
      }
    }
    else
    {
      --i;
      --j;
    }
  }
  if(j == -1)
  {
    return (i+1);
  }
  else
  {
    return npos;
  }
}

String& String::Erase(size_t pos,size_t len)
{
  // pos and len invalid
  if( pos >= size_ || len == 0)
  {

  }
  // 1.clear all
  else if((pos == 0) && (len > size_))
  {
    Char *oldptr = ustring_;
    ustring_ = new Char[128] {0};
    size_ = 0;
    capacity_ = 128;
    delete[] oldptr;
  }
  // 2.cut rear
  else if((len == npos) || (pos + len == size_))
  {
    if((pos * 2) < size_)
    {
      Char *oldptr = ustring_;
      size_t new_size = RoundUp2Power(size_ - pos);
      ustring_ = new Char[new_size] {0};
      for(size_t i = 0; i < pos; ++i)
      {
        ustring_[i] = oldptr[i];
      }
      capacity_ = new_size;
      delete[] oldptr;
    }
    size_ = pos;
  }
  // 3.cut middle include of cutting head only
  else
  {
    if((len * 2 ) > size_)
    {
      Char *oldptr = ustring_;
      size_t new_size = RoundUp2Power(size_ - len);
      ustring_ = new Char[new_size] {0};
      //copy old data
      for(size_t i = 0; i < pos; ++i)
      {
        ustring_[i] = oldptr[i];
      }
      for(size_t i = pos + len; i < size_; ++i)
      {
        ustring_[i - len] = oldptr[i];
      }
      size_ = size_ - len;
      capacity_ = new_size;
      delete[] oldptr;
    }
    else
    {
      for(size_t i = pos + len; i < size_; ++i)
      {
        ustring_[i - len] = ustring_[i];
      }
      size_ = size_ - len;
    }
  }
  return *this;
}


#ifdef UNITTEST
// sleep_ms function
#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds){ // cross-platform sleep function
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}

TEST(TimeStamp,Construct)
{
  TimeStamp tim(-1,0);
  TimeStamp tim1(0,0);
  struct timeval tv = {1,2589};
  TimeStamp tim2(&tv);

  EXPECT_EQ(tim.GetRawData()->tv_sec,0);
  EXPECT_EQ(tim.GetRawData()->tv_usec,0);
  EXPECT_TRUE(tim == 0);

  EXPECT_EQ(tim1.GetRawData()->tv_usec,0);
  EXPECT_NE(tim1.GetRawData()->tv_usec,2);
  EXPECT_FALSE(tim1 == 1);

  EXPECT_EQ(tim2.GetRawData()->tv_sec,1);
  EXPECT_EQ(tim2.GetRawData()->tv_usec,2589);
  EXPECT_TRUE(tim2 == 1002);

}

TEST(TimeStamp,OverrideOperator)
{
  TimeStamp tim1(12,11);
  EXPECT_TRUE(tim1 == 12011);
  EXPECT_EQ(tim1.GetMillisecond(),12011ULL);

  TimeStamp tim2(12,13);
  EXPECT_FALSE(tim1 > tim2);
  EXPECT_FALSE(tim1 == tim2);

  TimeStamp tim3 = tim1 + tim2;
  EXPECT_EQ(tim3.GetMillisecond(),24024ULL);
  EXPECT_TRUE((tim1 + 12013) == tim3);

}

TEST(TimeStamp,Now)
{
  TimeStamp now = TimeStamp::Now();
  EXPECT_TRUE(TimeStamp::Now() == now);
//    << "Now: " << now.GetMillisecond();
  sleep_ms(1000);
  EXPECT_TRUE(TimeStamp::Now() > (now+1000));
//    << "Now: " << TimeStamp::Now().GetMillisecond();

}

TEST(File,Path)
{
  std::string  path = "../src/main.cc";
  std::string path1 = "../UnitTest/src/main.o";
  std::string path2 = "../.project";
  std::string path3 = "../main.h";

  EXPECT_TRUE(File::Exist(path));
  EXPECT_FALSE(File::Exist(path1));
  EXPECT_TRUE(File::Exist(path2));
  EXPECT_FALSE(File::Exist(path3));

}

TEST(File,Size)
{
  std::string file = "../Doxyfile";
  //EXPECT_EQ(File::Size(file),2147483648UL);
  EXPECT_TRUE(File::Size(file) == 63634L);
}


TEST(Hex2Bin,Edge)
{
  unsigned char test[] = {0x55,0xAA,0x45};
  EXPECT_EQ(Utils::Bin2Hex(std::vector<char>(test,test+3)),"55AA45");
  EXPECT_EQ(Utils::Bin2Hex(0x14568F1),"014568F1");

  std::string str = "5F";
  char temp = Utils::Hex2Bin(str).at(0);
  EXPECT_EQ(temp,0x5F);

  str = "1G";
  EXPECT_TRUE(Utils::Hex2Bin(str).size() == 0);

  str = "1G5F";
  ASSERT_TRUE(Utils::Hex2Bin(str).size() == 1 );
  EXPECT_EQ(Utils::Hex2Bin(str).at(0),0x5F);

}

TEST(Base64Enocde,Base)
{
  char temp[] = {'M','a','n'};
  EXPECT_EQ(Utils::Base64Encode(std::vector<char>(temp,temp+3)),"TWFu");

  EXPECT_EQ(Utils::Base64Encode(std::vector<char>(temp,temp+2)),"TWE=");

  EXPECT_EQ(Utils::Base64Encode(std::vector<char>(temp,temp+1)),"TQ==");

}

TEST(Base64Decode,Base)
{
  std::string str = "TQ==";
  ASSERT_TRUE(Utils::Base64Decode(str).size() == 1);
  EXPECT_EQ(Utils::Base64Decode(str).at(0),'M');

  str = "TWFu";
  ASSERT_TRUE(Utils::Base64Decode(str).size() == 3);
  EXPECT_EQ(Utils::Base64Decode(str).at(2),'n');

  str = "FPucA9l+";
  ASSERT_TRUE(Utils::Base64Decode(str).size() == 6);
  EXPECT_TRUE(Utils::Base64Decode(str).at(5) == 0x7e);
}


TEST(Crc32,Buffer)
{
  char buffer[1024];
  memset(buffer,0x11,1024);
  Crc32 crc;
  crc.Update(buffer,1024);
  EXPECT_TRUE(crc.GetValue() == 1921749356);

  crc.Update(buffer,1024);
  EXPECT_TRUE(crc.GetValue() == 1921749356);

}

/*
TEST(Crc32,File)
{
  //1807156556
  std::string file = "../kiimo/TestFiles/test140m.upload";
  std::fstream fs;
  Crc32 crc;
  fs.open(file,std::ios::binary |std::ios::in);
  ASSERT_TRUE(fs.is_open());
  crc.Update(fs);
  EXPECT_EQ(crc.GetValue() , (uint32_t)1807156556);
  fs.close();

  crc.Update(file);
  EXPECT_TRUE(crc.GetValue() == 1807156556);

}
*/


TEST(Fmt,Trim)
{
  string dest = "       \t    \t      Happy new \t year               ";
  string res = Fmt::Trim(dest);
  EXPECT_EQ(res,"Happy new \t year");
  dest = " x ";
  res = Fmt::Trim(dest);
  EXPECT_EQ(res,"x");

}

TEST(Fmt,Format)
{

  std::string res = Fmt::Format("Number {1} is not { 2 } equal number { {0} }.",3,5);
  EXPECT_EQ(res,"Number 5 is not { 2 } equal number { 3 }.");

  res = Fmt::Format("test for {invalid} statement",'x','y');
  EXPECT_EQ(res,"test for {invalid} statement");

  res = Fmt::Format("test for { { 0 } statement",'x');
  EXPECT_EQ(res,"test for { x statement");

  res = Fmt::Format("{1} hello world {0}",'x');
  EXPECT_EQ(res,"{1} hello world x");

  res = Fmt::Format("hello world!",'x');
  EXPECT_EQ(res,"hello world!");

  res = Fmt::Format("hello { xyz } world {0}",'x');
  EXPECT_EQ(res,"hello { xyz } world x");

  res = Fmt::Format("Number {0} is not equal number {1}.",32,5,'7',9);
  EXPECT_EQ(res,"Number 32 is not equal number 5.");

  res = Fmt::Format("Three Number is : {0} or { 1 : x } or {2:X}.",35,228,228);
  EXPECT_EQ(res,"Three Number is : 35 or e4 or E4.");

}



TEST(Utf8AndUtf16,Base)
{
  //EXPECT_EQ(Utf16ToUtf8(L'你') , (uint32_t)0x00E4BDA0);
  EXPECT_EQ(Utf8ToUtf16('你'), L'你');
  EXPECT_EQ(Utf8ToUtf16(0x112233),L'\0');

}

TEST(Char,Base)
{
  Char ch1('A');
  Char ch2 = '你';
  Char ch3 = L'A';
  Char ch4(L'你');

  EXPECT_EQ(ch1,ch3);
  EXPECT_EQ(ch2,ch4);
  EXPECT_TRUE(ch2 == '你');
  EXPECT_TRUE(ch2 == L'你');
  EXPECT_TRUE(ch1 == 'A');

}

TEST(String,Base)
{
  String str1;
  EXPECT_EQ(str1.Size(),(size_t)0);
  EXPECT_GT(str1.Capacity(),(size_t)1);

  String str2("this is a utf8 string,你好");
  ASSERT_EQ(str2.Size(),(size_t)24);
  EXPECT_TRUE(str2[22] == '你');
  EXPECT_TRUE(str2[3] == 's');

  String str3(L"this is a chinese string,你好");
  ASSERT_EQ(str3.Size(),(size_t)27);
  EXPECT_TRUE(str3[26] == '好');
  EXPECT_TRUE(str3[3] == 's');

  String str4(str3);
  ASSERT_EQ(str4.Size(),(size_t)27);
  EXPECT_TRUE(str4[25] == L'你');
  EXPECT_TRUE(str4[3] == 's');

}

TEST(String , Assignment)
{
  String str2 = "this is a utf8 string,你好";
  ASSERT_EQ(str2.Size(),(size_t)24);
  EXPECT_TRUE(str2[22] == '你');
  EXPECT_TRUE(str2[3] == 's');

  String str3 = L"this is a chinese string,你好";
  ASSERT_EQ(str3.Size(),(size_t)27);
  EXPECT_TRUE(str3[26] == '好');
  EXPECT_TRUE(str3[3] == 's');

  String str4 = str3;
  ASSERT_EQ(str4.Size(),(size_t)27);
  EXPECT_TRUE(str4[25] == L'你');
  EXPECT_TRUE(str4[3] == 's');

  str4[25] = '我';
  str4[26] = L'是';
  EXPECT_TRUE(str4.At(25) == L'我');
  EXPECT_TRUE(str4.At(26) == '是');
}

TEST(String , Plus)
{
  String str1("this is a utf8 string,");
  str1 += "你好";
  ASSERT_EQ(str1.Size(),(size_t)24);
  EXPECT_TRUE(str1[22] == '你');
  EXPECT_TRUE(str1[3] == 's');

  String str2(L"this is a chinese string,");
  str2 += "你好";
  str2 += "1. 推送本地分支到远端 git push origin test_branch:test_breanch。git push origin --delete test_branch 4. 将文件移出版本管理系统，并保留本地副本    git rm --f test_file";
  ASSERT_EQ(str2.Size(),(size_t)164);
  EXPECT_GT(str2.Capacity(),(size_t)164);
  EXPECT_TRUE(str2[26] == '好');
  EXPECT_TRUE(str2[3] == 's');

}

TEST(String , Convert)
{
  String str1 = "this is a utf8 string,你好";
  std::string str2 = str1.ConvertToBaseString();
  EXPECT_EQ(str2.size(),(size_t)28);
  EXPECT_TRUE(str2[3] == 's');

  std::wstring str3 = str1.ConvertToBaseWstring();
  EXPECT_EQ(str3.size(),(size_t)24);
  EXPECT_TRUE(str3[22] == L'你');

}

TEST(String,Find)
{
  String str1 = "googletest is a testing framework developed by the Testing Technology team with Google s specific requirements and constraints in mind. Whether you work on Linux, Windows, or a Mac, if you write C++ code, googletest can help you. And it supports any kind of tests, not just unit tests.So what makes a good test, and how does googletest fit in? We believe:Tests should be independent and repeatable. It s a pain to debug a test that succeeds or fails as a result of other tests. googletest isolates the tests by running each of them on a different object. When a test fails, googletest allows you to run it in isolation for quick debugging.Tests should be well organized and reflect the structure of the tested code. googletest groups related tests into test suites that can share data and subroutines. This common pattern is easy to recognize and makes tests easy to maintain. Such consistency is especially helpful when people switch projects and start to work on a new code base.Tests should be portable and reusable. Google has a lot of code that is platform-neutral; its tests should also be platform-neutral. googletest works on different OSes, with different compilers, with or without exceptions, so googletest tests can work with a variety of configurations.When tests fail, they should provide as much information about the problem as possible. googletest doesn t stop at the first test failure. Instead, it only stops the current test and continues with the next. You can also set up tests that report non-fatal failures after which the current test continues. Thus, you can detect and fix multiple bugs in a single run-edit-compile cycle.The testing framework should liberate test writers from housekeeping chores and let them focus on the test content. googletest automatically keeps track of all tests defined, and doesn t require the user to enumerate them in order to run them.Tests should be fast. With googletest, you can reuse shared resources across tests and pay for the set-up/tear-down only once, without making tests depend on each other.Since googletest is based on the popular xUnit architecture, you ll feel right at home if you ve used JUnit or PyUnit before. If not, it will take you about 10 minutes to learn the basics and get started. So let s go!";
  char patten1[] = "run-edit-compile cycle.The testing framework should liberate test xriters from housekeeping chores and let them focus on the test content. googletest automatically keeps track of all tests defined, and doesn t require the user to enumerate them in order to run them.Tests should be fast. With googletest, you can reuse shared resources across tests and pay for the set-up/tear-down";
  char patten2[] = "is easy to recognize and makes tests easy to maintain. Such consistency is especially helpful when people switch projects and start to work on a new code base.Tests should be portable and reusable. Google has a lot of code that is platform-n";
  char patten3[] = "platform-neutral";
  EXPECT_EQ(str1.Find(patten1),str1.npos);
  EXPECT_NE(str1.Find(patten2),str1.npos);
  EXPECT_TRUE(str1.Find(patten3) == 1053);
  EXPECT_TRUE(str1.Find(patten3,1054) == 1096);
  EXPECT_EQ(str1.Rfind(patten3) ,(size_t)1096);
  EXPECT_EQ(str1.Rfind(patten3,1096) ,(size_t)1053);

  String str2 = "UTF-16 与 UTF-8 类似，使用变长编码。不同的是 UTF-16 使用一个或者两个 16bit 大小的编码单元，这样单个 Unicode 字符在 UTF-16 编码下字节长度为 2 或者 4。常用汉字在 UTF-16 中均可使用 2 个字节表示，但数字和字母也是 2 个字节。换句话说，如果是英文较多，使用 UTF-16 会产生较大的浪费；如果是中文较多，相较于 UTF-8 会节约不少空间。坏消息是 UTF-16 与 ASCII 码不兼容，应用上不如 UTF-8 广泛";
  wchar_t patten4[] = L"如果是英文较多，使用 UTF-16";
  wchar_t patten5[] = L" UTF-16 ";
  wchar_t patten6[] = L"常用汉字在 UTF-16 中均可使用 2 个字节表示，但数字和字母也是 2 个字节。选择话说，如果是英文较多，使用 UTF-16 会产生较大的浪费";
  EXPECT_NE(str2.Find(patten4),str2.npos);
  EXPECT_NE(str2.Find(patten5),str2.npos);
  EXPECT_EQ(str2.Find(patten6),str2.npos);


}

TEST(String,Erase)
{
  char str1[] = "这样单个 Unicode 字符在 UTF-16 编码下字节长度为 2 或者 4。ASCII 码不兼容，应用上不如 UTF-8 广泛";
  char str2[] = "这样单个符在 UTF-16 编码下字节长度为 2 或者 4。ASCII 码不兼容，应用上不如 UTF-8 广泛";
  char str3[] = "这样单个符在 UTF-16 编码下字节长度为 2 或者 4。ASCII 码不兼容，应用上不如";
  String str(str1);
  EXPECT_EQ(str.Erase(100).Compare(str1),0);  //invalid
  EXPECT_EQ(str.Erase(4,10).Compare(str2),0);  // cut middle
  EXPECT_EQ(str.Erase(46).Compare(str3),0);   //cut rear

}

TEST(String,Format)
{
  String res = String::Format("hello {0} !","world");
  EXPECT_TRUE(res.Compare("hello world !"));

  res = String::Format("Number {0} is not equal number {1}.",3,5);
  EXPECT_TRUE(res.Compare("Number 3 is not equal number 5."));

  res = String::Format("Number {1} is not equal number {0}.",3,5);
  EXPECT_TRUE(res.Compare("Number 5 is not equal number 3."));
}



#endif



