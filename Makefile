
CXX = g++

#CFLAGS := -std=c11
GTEST_INCLUDE := "D:\work\src\googletest\googletest\include"
GTEST_LIB := gtest

CXXFLAGS := -Wall -std=c++11 -static 

CXXFLAGS += -I"D:\\work\\src\\fmt\\include" -I"C:\\Program Files (x86)\\MinGW\\opt\\include"



LDLIBS := -lpthread
ifneq ("`make -ver | grep mingw`","") 
	LDLIBS += -lWs2_32
endif



OUTPUT := Debug
SRCS := $(wildcard src/*.cc)
OBJS := $(patsubst %.cc,%.o,$(SRCS))

all: kiimo doc

kiimo: $(OBJS)
	$(CXX) -g0 -O2 $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDLIBS)	

doc: doxyfile
	doxygen doxyfile
#gtest

	
unittest: prepare_test
	@echo 'unittest'
	@echo $(CXXFLAGS)
#	@echo $(LDLIBS)
		

.PHONY:clean doc

prepare_test:
	CXXFLAGS = $(CXXFLAGS) -I$(GTEST_INCLUDE)
#	@LDLIBS += -l$(GTEST_LIB)
	
clean:
	rm $(OBJS)
	@if [ -f "kiimo" ];then rm kiimo; fi
	@if [ -d "doc/html" ];then rm -rf doc/*;fi
