ifndef CFG
CFG = Debug
endif

INC_PATH = -I../src -I. -I"../../deps/ACE-6.0.0.0" -I"../../deps/protobuf-2.3.0/include" -I../../deps/libcurl/includes
LIB_PATH = -L"../../deps/ACE-6.0.0.0/lib" -L../../deps/libcurl/lib -L"../../deps/protobuf-2.3.0/Libs"

CC = gcc
CFLAGS = -fpic

CXX = g++
CXXFLAGS = $(CFLAGS)

LD = g++
LDFLAGS = -fpic -static -pthread $(LIB_PATH)
#LIBS = -lACE -lprotobuf -lprotobuf-lite -lprotoc -lcurl -lidn -lrt -lssh2 -lm
LIBS = -lACE -lprotobuf -lprotobuf-lite -lprotoc -lcurl -lrt -lm -ldl


ifeq "$(CFG)" "Debug"
WARNING_FLAGS = -W
OPTIMIZATION_FLAGS = -O0
DEBUG_FLAGS = -g -D_DEBUG
endif

ifeq "$(CFG)" "Release"
WARNING_FLAGS = -W
OPTIMIZATION_FLAGS = -O2
DEBUG_FLAGS = -g -DNDEBUG
endif

CFLAGS += $(WARNING_FLAGS) $(OPTIMIZATION_FLAGS) $(DEBUG_FLAGS) $(INC_PATH)

BIZ = \
	../src/biz/AppConfig.cpp \
	../src/biz/AppConfig.h \
	../src/biz/PeerManager.cpp \
	../src/biz/PeerManager.h \
	../src/biz/PluginManager.cpp \
	../src/biz/PluginManager.h

NETWORK = \
	../src/network/PeerHandler.cpp \
	../src/network/PeerHandler.h

MESSAGE = \
	../src/message/MsgID.h \
	../src/message/MsgPacket.cpp \
	../src/message/MsgPacket.h \
	../src/message/OPPMsg_CPW.pb.cc \
	../src/message/OPPMsg_CPW.pb.h

TINYXML = \
	../src/tinyxml/tinystr.cpp \
	../src/tinyxml/tinystr.h \
	../src/tinyxml/tinyxml.cpp \
	../src/tinyxml/tinyxml.h \
	../src/tinyxml/tinyxmlerror.cpp \
	../src/tinyxml/tinyxmlparser.cpp

UTIL = \
	../src/util/CLock.h \
	../src/util/CMutex.h \
	../src/util/CMutex.cpp \
	../src/util/CSharedMem.h \
	../src/util/CSharedMem.cpp \
	../src/util/EasyFTPWrapper.cpp \
	../src/util/EasyFTPWrapper.h \
	../src/util/FileSysUtil.cpp \
	../src/util/FileSysUtil.h \
	../src/util/HTTPDownload.cpp \
	../src/util/HTTPDownload.h \
	../src/util/IDGenerator.h \
	../src/util/Logger.cpp \
	../src/util/Logger.h \
	../src/util/minidump.h \
	../src/util/Util.cpp \
	../src/util/Util.h \
	../src/util/Updater.cpp \
	../src/util/Updater.h

COMMON = \
	../src/common/Common.h \
	../src/common/Linux.h \
	../src/common/Win32.h

MAIN = \
	../src/main/Application.cpp \
	../src/main/Application.h \

ZLIB = \
	../src/zip/zlib/adler32.c \
	../src/zip/zlib/compress.c \
	../src/zip/zlib/crc32.c \
	../src/zip/zlib/crc32.h \
	../src/zip/zlib/deflate.c \
	../src/zip/zlib/deflate.h \
	../src/zip/zlib/gzclose.c \
	../src/zip/zlib/gzguts.h \
	../src/zip/zlib/gzlib.c \
	../src/zip/zlib/gzread.c \
	../src/zip/zlib/gzwrite.c \
	../src/zip/zlib/infback.c \
	../src/zip/zlib/inffast.c \
	../src/zip/zlib/inffast.h \
	../src/zip/zlib/inffixed.h \
	../src/zip/zlib/inflate.c \
	../src/zip/zlib/inflate.h \
	../src/zip/zlib/inftrees.c \
	../src/zip/zlib/inftrees.h \
	../src/zip/zlib/trees.c \
	../src/zip/zlib/trees.h \
	../src/zip/zlib/uncompr.c \
	../src/zip/zlib/zconf.h \
	../src/zip/zlib/zlib.h \
	../src/zip/zlib/zutil.c \
	../src/zip/zlib/zutil.h

SRCS = $(BIZ) $(NETWORK) $(MESSAGE) $(TINYXML) $(UTIL) $(COMMON) $(MAIN) $(PLUGIN) $(ZLIB) 

OBJS = $(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS))))))

ifndef TARGET
TARGET = OPPWatchdog
endif

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cxx
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	-rm -f -v $(OBJS) $(TARGET) OPPWatchdog.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > OPPWatchdog.dep

-include OPPWatchdog.dep



