/**
 * @file sqlite_connector.cc
 * @brief Sqlite connector
 * @author simon
 * @date 2019-09-24
 *
 */

#if 0
#include "sqlite_connector.h"

#ifdef UNITTEST
#include <gtest/gtest.h>
#include <vector>
#endif

using namespace kiimo::base;
//using namespace std;


SqliteConnector::SqliteConnector(std::string file_name)
  :db_handle_(nullptr)
{
  if(!sqlite3_open(file_name.c_str(),&db_handle_))
  {
    is_connect_ = true;
  }
  else
  {
    sqlite3_close(db_handle_);
    throw SqlException("Sqlite3 Open",sqlite3_errmsg(db_handle_));
  }
}

SqliteConnector::~SqliteConnector()
{
  if(is_connect_)
  {
    sqlite3_close(db_handle_);
  }
}

bool SqliteConnector::Query(std::string query)
{

  if(!sqlite3_exec(db_handle_,query.c_str(),nullptr,nullptr,nullptr))
  {
    return true;
  }
  else
  {
    return false;
  }

}

void SqliteConnector::Close()
{
  if(is_connect_)
  {
    sqlite3_close(db_handle_);
  }
}

#ifdef UNITTEST
struct UpdateInfo
{
  UpdateInfo(std::vector<std::string> &val)
 {
    if(val.size() >= 4)
    {
      sn = val[0];
      files = val[1];
      date = val[2];
      log = val[3];
    }
 }
 std::string sn;
 std::string files;
 std::string date;
 std::string log;
};


const char drop_table[] = "DROP TABLE IF EXISTS wireless_update";
const char create_table[] = "CREATE TABLE wireless_update ("
    "id INT AUTO_INCREMENT,"
    "sn VARCHAR(10),"
    "files VARCHAR(255) NOT NULL,"
    "log TEXT,"
    "date DATE,"
    "PRIMARY KEY (id,sn)"
    ")";
const char insert_data[] = "INSERT INTO wireless_update (sn,files,date,log) VALUES "
    "('1.0.0','[\"Wireless.exe\",\"Arthas.dll\"]','2019-07-13','first release'),"
    "('1.1.0','[\"Wireless.exe\",\"Arthas.dll\"]','2019-08-14','fix some bug'),"
    "('1.2.0','[\"Wireless.exe\",\"Arthas.dll\"]','2019-08-15','change the size of main window'),"
    "('1.2.2','[\"Wireless.exe\",\"Arthas.dll\"]','2019-08-20','fix the command of send button'),"
    "('1.3.0','[\"Wireless.exe\",\"Arthas.dll\"]','2019-08-22','add auto update features'),"
    "('1.3.1','[\"Wireless.exe\",\"Arthas.dll\"]','2019-08-29','fix the show way of log')";

TEST(SqliteConnector,Base)
{
  SqliteConnector con("test.db");
  ASSERT_TRUE(con.IsConnect());

  //create the table
  con.Query(drop_table);
  con.Query(create_table);
  con.Query(insert_data);

  auto others_val = con.Query<UpdateInfo>(
      "SELECT sn,files,date,log FROM wireless_update ORDER BY sn DESC LIMIT 1");
  ASSERT_FALSE(others_val == nullptr);
  EXPECT_TRUE(others_val->size() == 1);
  EXPECT_EQ(others_val->at(0).sn ,"1.3.1");

}
#endif

#endif