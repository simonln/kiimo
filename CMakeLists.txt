#
#	usage: 
#	MinGW on windows: mkdir build && cd build && cmake -G "MinGW Makefiles" .. && make
#	gcc on linux: mkdir build && cd build && cmake .. && cmake
#	
#	test on linux: cd build && cmake -DCMAKE_BUILD_TYPE=UnitTest .. && ./kiimo
#


#最低要求
cmake_minimum_required(VERSION 3.10)

#设置构建类型
if (NOT CMAKE_BUILD_TYPE)
	    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Set build type to Debug o Release (default Release)" FORCE)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;UnitTest" CACHE STRING "Limited configurations" FORCE)
message("Build type: Debug Release UnitTest, current is ${CMAKE_BUILD_TYPE}")

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
	add_definitions(-D_DEBUG)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL UnitTest)
	# windows
	# set(GTEST_PATH "D:\\work\\src\\googletest")
	set(GTEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../googletest")
	# message(${GTEST_PATH})
	include_directories(${GTEST_PATH}/googletest/include)
	include_directories(${GTEST_PATH}/googlemock/include)
	link_directories(${GTEST_PATH}/lib)
	link_libraries("gtest")
	add_definitions(-DUNITTEST)
endif()

#设置编译参数
#set (CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -g -O0 -Wall")
#set (CMAKE_EXE_LINKER_FLAGS_RELEASE  "-std=c++11 -static -g0 -O2")

# 禁止多字节编译警告
add_definitions(-Wno-multichar)

#环境
if (${CMAKE_GENERATOR} MATCHES "MinGW")
	set(CMAKE_MAKE_PROGRAM "C:\\Program Files\\MinGW\\bin\\make")
	set(SQLITES_PATH "C:\\Program Files\\MinGW\\opt")
	include_directories(${SQLITES_PATH}/include)
	link_directories(${SQLITES_PATH}/lib)
	#set windows SDK version for poll function
	add_definitions(-D_WIN32_WINNT=0x0600)
	#target_link_libraries(kiimo "pthread" "sqlite3")
elseif(${CMAKE_GENERATOR} MATCHES "MSVC")
	add_definitions(/std:c++11 /D "_DEBUG")
endif()

#项目名称
project(kiimo)

if (WIN32)
	link_libraries("Ws2_32")
endif()


#添加include 目录 
#include_directories(src/databases ${SQLITES_PATH}/include ${FMT_PATH}/include)
include_directories(src/databases  src/http)

#aux_source_directory( . DIR_SRCS)
#add_executable(kiimo ${DIR_SRCS})

#列出当前项目下所有的.cc文件
file(GLOB_RECURSE ALL_SRCS LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cc)

#库文件搜索目录
#link_directories(${SQLITES_PATH}/lib ${FMT_PATH})


add_executable(kiimo ${ALL_SRCS})

# only for gcc
target_link_libraries(kiimo "pthread" "sqlite3")


