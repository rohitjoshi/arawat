VENDOR_HOME= $(DEV_HOME)/vendor
OPENSOURCE_HOME = $(VENDOR_HOME)/opensource
V_SMPPCXX_INC=-I$(OPENSOURCE_HOME)/smppcxx/include
V_SMPPCXX_LIB=-L$(OPENSOURCE_HOME)/smppcxx/lib -lsmppcxx

V_BOOST_VER=v1.46.1
V_BOOST_INC=-I$(OPENSOURCE_HOME)/boost/$(V_BOOST_VER)/include
V_BOOST_LIB=-L$(OPENSOURCE_HOME)/boost/$(V_BOOST_VER)/lib -lboost_system -lboost_filesystem -lboost_thread -lboost_signals -lboost_log_setup -lboost_log

V_MSGPACK_INC=-I$(OPENSOURCE_HOME)/msgpack/include -I$(OPENSOURCE_HOME)/mpio/include
V_MSGPACK_LIB=-L$(OPENSOURCE_HOME)/msgpack/lib -lmsgpack -lmsgpack-rpc -L$(OPENSOURCE_HOME)/mpio/lib -lmpio

V_QPID_INC=-I$(OPENSOURCE_HOME)/qpidc/include
V_QPID_LIB=-L$(OPENSOURCE_HOME)/qpidc/lib -lqpidcommon -lqpidmessaging -lqpidclient -lqpidtypes -lqpidbroker

V_GTEST_HOME=$(OPENSOURCE_HOME)/gtest
V_GTEST_INC=-I$(V_GTEST_HOME)/include
V_GTEST_LIB=-L$(V_GTEST_HOME)/lib -lgtest

V_LUA_HOME=$(OPENSOURCE_HOME)/lua
V_LUA_INC=-I$(V_LUA_HOME)/include/lua
V_LUA_LIB=-L$(V_LUA_HOME)/lib -llua -lluabind


VENDOR_INC=$(V_BOOST_INC) $(V_MSGPACK_INC) $(V_SMPPCXX_INC) $(V_GTEST_INC) $(V_QPID_INC) $(V_LUA_INC)
VENDOR_LIB=-lpthread -lm $(V_BOOST_LIB) $(V_MSGPACK_LIB) $(V_SMPPCXX_LIB) $(V_GTEST_LIB) $(V_QPID_LIB) $(V_LUA_LIB)

ARAWAT_DEV=$(DEV_HOME)/arawat
UTIL_HOME=$(ARAWAT_DEV)/util
UTIL_INC=-I$(UTIL_HOME)/include

PROTOCOL_HOME=$(ARAWAT_DEV)/protocols
PROTOCOL_CMN_INC=-I$(PROTOCOL_HOME)/common/include

SMPP_HOME=$(PROTOCOL_HOME)/smpp
SMPP_INC=-I$(SMPP_HOME)/include

QPID_HOME=$(PROTOCOL_HOME)/qpid
QPID_INC=-I$(QPID_HOME)/include

LIB_PREFIX= Arwt
BIN_PREFIX = Arwt
LIB_INSTALL = $(ARAWAT_HOME)/lib
BIN_INSTALL = $(ARAWAT_HOME)/bin
BUILD_DIR = .build
CC = g++

CFLAGS += -fPIC -g -Wall
LIBS=-L$(LIB_INSTALL) $(VENDOR_LIB) -L$(ARAWAT_HOME)/lib

ifdef release
CFLAGS += -O2
else
CFLAGS += -ggdb -O0 --coverage -fprofile-arcs -ftest-coverage
LIBS += -lgcov
endif

INCLUDES= $(UTIL_INC) $(PROTOCOL_CMN_INC)  $(VENDOR_INC)



all: lall rall

clean: lclean rclean

test: ltest rtest

install: linstall rinstall

$(BUILD_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<
