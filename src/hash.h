/**
 *  @file hash.h
 *  @brief HASH class set
 *  @author simon
 *  @date 2019-11-20
 */

#ifndef HASH_H_
#define HASH_H_

#include <string>

#include <cstring>


namespace kiimo {

/**
 *  @brief contains some hash class and logger class ...
 */
namespace base {

  /**
   *  @brief MD5 hash class
   */
  class MD5
  {
   public:
    /**
     *  @brief Constructor
     */
    MD5() = default;
    /**
     *  @brief Update data for MD5 calculator
     *  @param [in] data  raw bytes block
     *  @param num_bytes length of raw bytes block
     *  @note
     *    Usually used for calculating MD5 of file
     *
     */
    void Update(const void *data,size_t num_bytes);
    /**
     *  @brief Translate the hash result to string
     */
    std::string GetHexString();
    /**
     *  @brief Get length of hash result , byte of unit
     */
    static const size_t GetResultLen() {return 16;};
    /**
     *  @brief Get the raw bytes result
     *  @param [out] result  A buffer of saving hash result,size is 16 byte
     */
    void GetVal(uint8_t *result);
   private:
    void ProcessBlock(const void *data,size_t num_bytes);
    void Transform(const void *data);

   private:
    uint8_t  buffer_[64] = {};
    uint32_t state_[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
    uint32_t count_[2] = {};
  };

  /**
   *  @brief SHA256 hash class
   */
  class SHA256
  {
   public:
    /**
     *  @brief Constructor
     */
    SHA256() = default;
    /**
     *  @brief Update data for SHA256 calculator
     *  @param [in] data  raw bytes block
     *  @param size length of raw bytes block
     *  @note
     *    Usually used for calculating SHA256 of file
     */
    void Update(const void *data,size_t size);
    /**
     *  @brief Get length of hash result , byte of unit
     */
    static const size_t GetResultLen(){return 32;};
    /**
     *  @brief Get the raw bytes result
     *  @param [out] result  A buffer of saving hash result,size is 16 byte
     */
    void GetVal(uint8_t *result);
    /**
     *  @brief Translate the hash result to string
     */
    std::string GetHexString();
   private:
    void ProcessFullBlock(const void *data);
    void ProcessFinalBlock(const void *data,size_t size);

   private:
    uint8_t buffer_[64] = {};
    int remain_size_ = 0;
    uint32_t state_[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372,
        0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    uint64_t length_ = 0;
  };

  /**
   *  @brief HMAC of MD5 or SHA256
   */
  template<typename HASH_TYPE>
  class HMAC
  {
   public:
    /**
     *  @brief Constructor
     */
    HMAC()
     {
        hmac_ = new uint8_t[HASH_TYPE::GetResultLen()];
     };
    /**
     *  @brief Destructor
     */
    ~HMAC()
    {
      delete[] hmac_;
    };
    /**
     *  @brief Calculate the HMAC of text
     *  @param [in] key The key
     *  @param key_size The size of key
     *  @param [in] text The context need to be HMAC
     *  @param text_size The size of text
     *  @note
     *    Running  GetVal function will get the result after running this function
     */
    void Cal(const void *key,size_t key_size,const void *text,size_t text_size)
    {
      uint8_t k_ipad[64] = {0};
      uint8_t k_opad[64] = {0};

      size_t real_len = HASH_TYPE::GetResultLen();
      uint8_t cal_key[real_len] = {0};

        memset(k_ipad,0,64);
        memset(k_opad,0,64);

        if(key_size > 64)
        {
          HASH_TYPE hash;
          hash.Update(key,key_size);
          hash.GetVal(cal_key);
        }
        else
        {
          memcpy(cal_key,key,key_size);
        }

        memcpy(k_ipad,cal_key,real_len);
        memcpy(k_opad,cal_key,real_len);
        //XOR
        for(size_t i = 0; i < 64; ++i)
        {
          k_ipad[i] ^= 0x36;
          k_opad[i] ^= 0x5C;
        }
        //inner hash
        uint8_t inner_digest[real_len] = {0};
        HASH_TYPE inner_hash;
        inner_hash.Update(k_ipad,64);
        inner_hash.Update(text,text_size);
        inner_hash.GetVal(inner_digest);

        HASH_TYPE fina_hash;
        fina_hash.Update(k_opad,64);
        fina_hash.Update(inner_digest,sizeof(inner_digest));
        fina_hash.GetVal(hmac_);
    }
    /**
     *  @brief Get length of HMAC result , byte of unit
     */
    size_t GetResultLen()
    {
      return HASH_TYPE::GetResultLen();
    }
    /**
     *  @brief Get the raw bytes result
     *  @param [out] result  A buffer of saving HMAC result,size according to hash type
     */
    void GetVal(uint8_t *result)
    {
      memcpy(result,hmac_,GetResultLen());
    }
    /**
     *  @brief Translate the HMAC result to string
     */
    const std::string GetHexString()
    {
      int len = GetResultLen() * 2 + 1;
      char *buff = new char[len] {0};
      for(size_t i = 0; i < GetResultLen(); ++i)
      {
        sprintf(buff + (i*2),"%02x",hmac_[i]);
      }
      buff[len-1] = '\0';
      std::string str(buff);
      delete buff;
      return str;
    }
   private:
    uint8_t *hmac_;
  };

} /* namespace base */
} /* namespace kiimo */

#endif /* HASH_H_ */
