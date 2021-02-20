/**
 * @file hash.cc
 * @brief hash function implement
 * @author simon
 * @date 2019-11-20
 *
 */

#include "hash.h"

#include <cstdio> ///< for sprintf

#ifdef UNITTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace kiimo::base;

/// @name MD5 static function
/// @{
static inline uint32_t rotateLeft (uint32_t x, uint32_t n)      { return (x << n) | (x >> (32 - n)); }

static inline uint32_t F (uint32_t x, uint32_t y, uint32_t z)   { return (x & y) | (~x & z); }
static inline uint32_t G (uint32_t x, uint32_t y, uint32_t z)   { return (x & z) | (y & ~z); }
static inline uint32_t H (uint32_t x, uint32_t y, uint32_t z)   { return x ^ y ^ z; }
static inline uint32_t I (uint32_t x, uint32_t y, uint32_t z)   { return y ^ (x | ~z); }

static void FF (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a = rotateLeft (a + F (b, c, d) + x + ac, s) + b;
}

static void GG (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a = rotateLeft (a + G (b, c, d) + x + ac, s) + b;
}

static void HH (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a = rotateLeft (a + H (b, c, d) + x + ac, s) + b;
}

static void II (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a = rotateLeft (a + I (b, c, d) + x + ac, s) + b;
}

/// @}
void MD5::Update(const void *data,size_t num_bytes)
{
  ProcessBlock(data,num_bytes);
}

void MD5::ProcessBlock(const void *data,size_t num_bytes)
{
  auto bufferPos = ((count_[0] >> 3) & 0x3f);

  count_[0] += (uint32_t) (num_bytes << 3);

  if (count_[0] < ((uint32_t) num_bytes << 3))
  {
    count_[1]++;
  }

  count_[1] += (uint32_t) (num_bytes >> 29);

  auto spaceLeft = (size_t) 64 - (size_t) bufferPos;
  size_t i = 0;

  if (num_bytes >= spaceLeft)
  {
      memcpy (buffer_ + bufferPos, data, spaceLeft);
      Transform (buffer_);

      for (i = spaceLeft; i + 64 <= num_bytes; i += 64)
      {
          Transform (static_cast<const char*> (data) + i);
      }
      bufferPos = 0;
  }

  memcpy (buffer_ + bufferPos, static_cast<const char*> (data) + i, num_bytes - i);
}

void MD5::Transform(const void *data)
{
  auto a = state_[0];
  auto b = state_[1];
  auto c = state_[2];
  auto d = state_[3];

  uint32_t x[16];
  //default little endian so comment below scope
  //copyWithEndiannessConversion (x, bufferToTransform, 64);
  memcpy(x,data,64);

  enum Constants
  {
      S11 = 7, S12 = 12, S13 = 17, S14 = 22, S21 = 5, S22 = 9,  S23 = 14, S24 = 20,
      S31 = 4, S32 = 11, S33 = 16, S34 = 23, S41 = 6, S42 = 10, S43 = 15, S44 = 21
  };

  FF (a, b, c, d, x[ 0], S11, 0xd76aa478);     FF (d, a, b, c, x[ 1], S12, 0xe8c7b756);
  FF (c, d, a, b, x[ 2], S13, 0x242070db);     FF (b, c, d, a, x[ 3], S14, 0xc1bdceee);
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf);     FF (d, a, b, c, x[ 5], S12, 0x4787c62a);
  FF (c, d, a, b, x[ 6], S13, 0xa8304613);     FF (b, c, d, a, x[ 7], S14, 0xfd469501);
  FF (a, b, c, d, x[ 8], S11, 0x698098d8);     FF (d, a, b, c, x[ 9], S12, 0x8b44f7af);
  FF (c, d, a, b, x[10], S13, 0xffff5bb1);     FF (b, c, d, a, x[11], S14, 0x895cd7be);
  FF (a, b, c, d, x[12], S11, 0x6b901122);     FF (d, a, b, c, x[13], S12, 0xfd987193);
  FF (c, d, a, b, x[14], S13, 0xa679438e);     FF (b, c, d, a, x[15], S14, 0x49b40821);

  GG (a, b, c, d, x[ 1], S21, 0xf61e2562);     GG (d, a, b, c, x[ 6], S22, 0xc040b340);
  GG (c, d, a, b, x[11], S23, 0x265e5a51);     GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d);     GG (d, a, b, c, x[10], S22, 0x02441453);
  GG (c, d, a, b, x[15], S23, 0xd8a1e681);     GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6);     GG (d, a, b, c, x[14], S22, 0xc33707d6);
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87);     GG (b, c, d, a, x[ 8], S24, 0x455a14ed);
  GG (a, b, c, d, x[13], S21, 0xa9e3e905);     GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8);
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9);     GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);

  HH (a, b, c, d, x[ 5], S31, 0xfffa3942);     HH (d, a, b, c, x[ 8], S32, 0x8771f681);
  HH (c, d, a, b, x[11], S33, 0x6d9d6122);     HH (b, c, d, a, x[14], S34, 0xfde5380c);
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44);     HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9);
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60);     HH (b, c, d, a, x[10], S34, 0xbebfbc70);
  HH (a, b, c, d, x[13], S31, 0x289b7ec6);     HH (d, a, b, c, x[ 0], S32, 0xeaa127fa);
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085);     HH (b, c, d, a, x[ 6], S34, 0x04881d05);
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039);     HH (d, a, b, c, x[12], S32, 0xe6db99e5);
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8);     HH (b, c, d, a, x[ 2], S34, 0xc4ac5665);

  II (a, b, c, d, x[ 0], S41, 0xf4292244);     II (d, a, b, c, x[ 7], S42, 0x432aff97);
  II (c, d, a, b, x[14], S43, 0xab9423a7);     II (b, c, d, a, x[ 5], S44, 0xfc93a039);
  II (a, b, c, d, x[12], S41, 0x655b59c3);     II (d, a, b, c, x[ 3], S42, 0x8f0ccc92);
  II (c, d, a, b, x[10], S43, 0xffeff47d);     II (b, c, d, a, x[ 1], S44, 0x85845dd1);
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f);     II (d, a, b, c, x[15], S42, 0xfe2ce6e0);
  II (c, d, a, b, x[ 6], S43, 0xa3014314);     II (b, c, d, a, x[13], S44, 0x4e0811a1);
  II (a, b, c, d, x[ 4], S41, 0xf7537e82);     II (d, a, b, c, x[11], S42, 0xbd3af235);
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb);     II (b, c, d, a, x[ 9], S44, 0xeb86d391);

  state_[0] += a;
  state_[1] += b;
  state_[2] += c;
  state_[3] += d;
}

void MD5::GetVal(uint8_t *result)
{
  uint8_t encode_length[8];
  memcpy(encode_length,count_,8);
  // Pad out to 56 mod 64.
  auto index = (count_[0] >> 3) & 0x3f;
  auto paddingLength = (index < 56 ? 56 : 120) - index;

  uint8_t paddingBuffer[64] = { 0x80 }; // first byte is 0x80, remaining bytes are zero.

  ProcessBlock (paddingBuffer, (size_t) paddingLength);
  ProcessBlock (encode_length, 8);
  memcpy(result,state_,16);
}

string MD5::GetHexString()
{
  string str = "";
  uint8_t result[16] = {0};
  char buffer[33] = {0};  //16*2+1
  GetVal(result);
  for(int i = 0 ; i < 4 ; i++)
  {
    sprintf(buffer + (i*8),"%02x%02x%02x%02x",
            result[i*4],result[i*4+1],result[i*4+2],result[i*4+3]);
  }
  buffer[32] = '\0';
  return std::string(buffer);
}

//SHA256

/// @name SHA256 static function
/// @{
static inline uint32_t rotate (uint32_t x, uint32_t y)             { return (x >> y) | (x << (32 - y)); }
static inline uint32_t ch  (uint32_t x, uint32_t y, uint32_t z)    { return z ^ ((y ^ z) & x); }
static inline uint32_t maj (uint32_t x, uint32_t y, uint32_t z)    { return y ^ ((y ^ z) & (x ^ y)); }

static inline uint32_t s0 (uint32_t x)      { return rotate (x, 7)  ^ rotate (x, 18) ^ (x >> 3); }
static inline uint32_t s1 (uint32_t x)      { return rotate (x, 17) ^ rotate (x, 19) ^ (x >> 10); }
static inline uint32_t S0 (uint32_t x)      { return rotate (x, 2)  ^ rotate (x, 13) ^ rotate (x, 22); }
static inline uint32_t S1 (uint32_t x)      { return rotate (x, 6)  ^ rotate (x, 11) ^ rotate (x, 25); }

/// @}

void SHA256::ProcessFullBlock(const void *data)
{
  const uint32_t constants[] =
  {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  };

  uint32_t block[16], s[8];
  memcpy (s, state_, sizeof (s));

  auto d = static_cast<const uint8_t*> (data);

  for (auto& b : block)
  {
      b = (uint32_t (d[0]) << 24) | (uint32_t (d[1]) << 16) | (uint32_t (d[2]) << 8) | d[3];
      d += 4;
  }

  auto convolve = [&] (uint32_t i, uint32_t j)
  {
      s[(7 - i) & 7] += S1 (s[(4 - i) & 7]) + ch (s[(4 - i) & 7], s[(5 - i) & 7], s[(6 - i) & 7]) + constants[i + j]
                           + (j != 0 ? (block[i & 15] += s1 (block[(i - 2) & 15]) + block[(i - 7) & 15] + s0 (block[(i - 15) & 15]))
                                     : block[i]);
      s[(3 - i) & 7] += s[(7 - i) & 7];
      s[(7 - i) & 7] += S0 (s[(0 - i) & 7]) + maj (s[(0 - i) & 7], s[(1 - i) & 7], s[(2 - i) & 7]);
  };

  for (uint32_t j = 0; j < 64; j += 16)
  {
    for (uint32_t i = 0; i < 16; ++i)
    {
      convolve (i, j);
    }

  }


  for (int i = 0; i < 8; ++i)
  {
      state_[i] += s[i];
  }

  length_ += 64;
}

void SHA256::ProcessFinalBlock(const void *data,size_t num_bytes)
{
  length_ += num_bytes;
  length_ *= 8; // (the length is stored as a count of bits, not bytes)

  uint8_t finalBlocks[128];

  memcpy (finalBlocks, data, num_bytes);
  finalBlocks[num_bytes++] = 128; // append a '1' bit

  while (num_bytes != 56 && num_bytes < 64 + 56)
      finalBlocks[num_bytes++] = 0; // pad with zeros..

  for (int i = 8; --i >= 0;)
      finalBlocks[num_bytes++] = (uint8_t) (length_ >> (i * 8)); // append the length.



  ProcessFullBlock(finalBlocks);

  if (num_bytes > 64)
  {
      ProcessFullBlock (finalBlocks + 64);
  }
}

void SHA256::Update(const void *data , size_t num_bytes)
{
  auto *ptr = static_cast<const uint8_t*>(data);
  auto count = remain_size_ + num_bytes;
  if(count < 64)
  {
    if(num_bytes > 0)
    {
      memcpy(buffer_ + remain_size_,ptr,num_bytes);
      remain_size_ += num_bytes;
    }
    else
    {
      if(length_ == 0)
      {
        //first run
        ProcessFinalBlock(data,0);
      }
    }
  }
  else if(count == 64)
  {
    uint8_t *temp = new uint8_t[64];
    memcpy(temp,buffer_,remain_size_);
    memcpy(temp + remain_size_,ptr,num_bytes);
    ProcessFullBlock(temp);
    remain_size_ = 0;
    delete[] temp;
  }
  else
  {
    uint8_t *temp = new uint8_t[64];
    memcpy(temp,buffer_,remain_size_);
    memcpy(temp + remain_size_,ptr,64 - remain_size_);
    ProcessFullBlock(temp);
    delete[] temp;

    size_t i = 64 - remain_size_;
    for(;(i + 64) < num_bytes; i += 64)
    {
      ProcessFullBlock(ptr + i);
    }
    remain_size_ = 0;
    if( i < num_bytes)
    {
      //ProcessFinalBlock(ptr + i,num_bytes - i);
      memcpy(buffer_,ptr + i,num_bytes - i);
      remain_size_ = num_bytes - i;
    }
  }

}

void SHA256::GetVal(uint8_t *result)
{
  if(remain_size_ > 0)
  {
    ProcessFinalBlock(buffer_,remain_size_);
    remain_size_ = 0;
  }

  for(auto s : state_)
  {
    *result++ = uint8_t(s >> 24);
    *result++ = uint8_t(s >> 16);
    *result++ = uint8_t(s >> 8);
    *result++ = uint8_t(s);
  }
}

std::string SHA256::GetHexString()
{
  uint8_t result[32] = {0};
  char buffer[65] = {0};  //32*2+1
  GetVal(result);

  for(int i = 0 ;i < 8;++i)
  {
    sprintf(buffer + (8*i),"%02x%02x%02x%02x",
            result[i*4],result[i*4+1],result[i*4+2],result[i*4+3]);
  }
  buffer[64] = '\0';
  return std::string(buffer);
}

#ifdef UNITTEST
TEST(SHA256,base)
{
  string str1 = "";
  string str2 = "The quick brown fox jumps over the lazy dog";
  string str3 = "The quick brown fox jumps over the lazy dog.";
  SHA256 hash;
  hash.Update(str1.c_str(),str1.size());
  EXPECT_EQ(hash.GetHexString(),"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

  SHA256 hash2;
  hash2.Update(str2.c_str(),str2.size());
  EXPECT_EQ(hash2.GetHexString(),"d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");

  SHA256 hash3;
  hash3.Update(str3.c_str(),str3.size());
  EXPECT_EQ(hash3.GetHexString(),"ef537f25c895bfa782526529a9b63d97aa631564d5d789c2b765448c8635fb6c");
}

TEST(SHA256,continuous)
{
  string str = "The quick brown fox jumps over the lazy dog";
  SHA256 hash;
  hash.Update(str.c_str(),10);
  hash.Update(str.data() + 10,str.size() - 10);
  EXPECT_EQ(hash.GetHexString(),"d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
}

TEST(MD5,base)
{
  string str1 = "";
  string str2 = "The quick brown fox jumps over the lazy dog";
  string str3 = "The quick brown fox jumps over the lazy dog.";
  MD5 hash1;
  hash1.Update(str1.c_str(),str1.size());
  EXPECT_EQ(hash1.GetHexString(),"d41d8cd98f00b204e9800998ecf8427e");

  MD5 hash2;
  hash2.Update(str2.c_str(),str2.size());
  EXPECT_EQ(hash2.GetHexString(),"9e107d9d372bb6826bd81d3542a419d6");

  MD5 hash3;
  hash3.Update(str3.c_str(),str3.size());
  EXPECT_EQ(hash3.GetHexString(),"e4d909c290d0fb1ca068ffaddf22cbd0");
}

TEST(MD5,continuous)
{
  string str = "The quick brown fox jumps over the lazy dog";
  MD5 hash;
  hash.Update(str.c_str(),10);
  hash.Update(str.data() + 10,str.size() - 10);
  EXPECT_EQ(hash.GetHexString(),"9e107d9d372bb6826bd81d3542a419d6");
}

TEST(MD5,performance)
{
  string str = "The quick brown fox jumps over the lazy dog";
  MD5 hash;
  for(int i =0;i < 10000;++i)
  {
    hash.Update(str.c_str(),str.size());
  }
  EXPECT_EQ(hash.GetHexString(),"104c116ea86921cb4415902f823d3871");
}


TEST(HMAC,MD5)
{
  uint8_t key[] = {0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b};
  uint8_t data[] = "Hi There";
  HMAC<MD5> hmac;
  hmac.Cal(key,sizeof(key),data,8);
  EXPECT_TRUE(hmac.GetResultLen() == 16);
  EXPECT_EQ(hmac.GetHexString(),"9294727a3638bb1c13f48ef8158bfc9d");

  string key1 = "Jefe";
  string text1 = "what do ya want for nothing?";
  HMAC<MD5> hmac1;
  hmac1.Cal(key1.c_str(),key1.size(),text1.c_str(),text1.size());
  EXPECT_EQ(hmac1.GetHexString(),"750c783e6ab0b503eaa86e310a5db738");
}

TEST(HMAC,SHA256)
{
  uint8_t key[20]; //= {0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b};
  uint8_t data[] = "Hi There";
  memset(key,0x0b,20);
  HMAC<SHA256> hmac;
  hmac.Cal(key,sizeof(key),data,8);
  EXPECT_TRUE(hmac.GetResultLen() == 32);
  EXPECT_EQ(hmac.GetHexString(),
            "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");

  string key1 = "Jefe";
  string text1 = "what do ya want for nothing?";
  HMAC<SHA256> hmac1;
  hmac1.Cal(key1.c_str(),key1.size(),text1.c_str(),text1.size());
  EXPECT_EQ(hmac1.GetHexString(),
            "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
}
#endif



