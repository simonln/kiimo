/**
 * @file sqlite_connector.h
 * @brief Sqlite connector
 * @author simon
 * @date 2019-09-24
 *
 */

#if 0
#ifndef DATABASE_SQLITE_CONNECTOR_H_
#define DATABASE_SQLITE_CONNECTOR_H_


#include <sqlite3.h>
#include <iostream>

#include "sql_connector.h"


namespace kiimo {
namespace base {

/**
 *  Connector for sqlite
 */
class SqliteConnector:public SqlConnector
{
 public:
  SqliteConnector(std::string file_name);
  ~SqliteConnector();

  /**
   * @brief Execute SQL primitive in database
   * @param query SQL primitive
   *
   */
  template <typename T>
  std::shared_ptr<std::vector<T>> Query(std::string query)
  {
    int num_row = 0;
    int num_column = 0;
    char *error_msg = nullptr;
    char **result = nullptr;
    std::vector<T> val;

    if(sqlite3_get_table(db_handle_,query.c_str(),&result,&num_row,&num_column,&error_msg))
    {
      std::string err_str(error_msg);
      sqlite3_free_table(result);
      sqlite3_free(error_msg);
      //return nullptr;
      throw SqlException(query,err_str);
    }

    fields_.clear();
    for(auto i = 0;i < num_column ; ++i)
    {
      //get the name of clomun
      fields_.push_back(result[i]);
    }

    std::vector<std::string> row_val;
    for(auto i = 1 ; i < num_row+1; ++i)
    {
      row_val.clear();
      for(auto j = 0;j < num_column ; ++j)
      {
        row_val.push_back(result[i*num_column + j]);
      }
      val.push_back(T(row_val));
    }
    sqlite3_free_table(result);
    return std::make_shared<std::vector<T>>(val);
  }

  virtual bool Query(std::string query);
  virtual void Close();
private:
  sqlite3 *db_handle_;
};

} /* namespace base */
} /* namespace kiimo */

#endif /* DATABASE_SQLITE_CONNECTOR_H_ */

#endif