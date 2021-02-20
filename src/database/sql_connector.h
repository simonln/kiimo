/**
 * @file sql_connector.h
 * @brief SQL connector
 * @author simon
 * @date 2019-09-20
 *
 */

#ifndef SQL_CONNECTOR_H_
#define SQL_CONNECTOR_H_

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

namespace kiimo {
namespace base {

  /// SQL exception
  class SqlException:public std::exception
  {
   public:
    /**
     *
     * @brief Constructor
     * @param sql SQL statement
     * @param err Error string
     */
    SqlException(const std::string &sql,const std::string &err)
   {
      sql_ = sql;
      err_ = err;
   }
    virtual const char *what() const noexcept
    {
      return err_.data();
    }
    const std::string &GetSql() const
    {
      return sql_;
    }
   private:
    std::string sql_;
    std::string err_;
  };

  /**
   * SQL connector
   *
   */
  class SqlConnector
  {
  public:
    SqlConnector()
      :is_connect_(false)
    {};

    virtual ~SqlConnector(){};

    bool IsConnect() const
    {
      return is_connect_;
    }
    /// Ignore the result after SQL executing
    virtual bool Query(std::string) = 0;

    /// Get the name of column after query
    std::vector<std::string>& GetQueryFields()
    {
      return fields_;
    }
    virtual void Close() = 0;

  protected:
    bool is_connect_;
    std::vector<std::string> fields_;

  };

} /* namespace base */
} /* namespace kiimo */

#endif /* SQL_CONNECTOR_H_ */
