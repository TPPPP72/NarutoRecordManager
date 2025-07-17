project := NarutoRecordManager

# 编译器和资源编译器
CXX := clang++
RC  := windres

# 编译选项
WX_CXXFLAGS := $(shell wx-config --cxxflags)
WX_CPPFLAGS := $(shell wx-config --cppflags)
WX_LIBS     := $(shell wx-config --libs)
PROTO_LIBS  := $(shell pkg-config --cflags --libs protobuf)
CXXFLAGS    := -mwindows -Wall -Wno-ignored-attributes -Wno-unused-command-line-argument -DUNICODE -D_UNICODE -std=c++20 -O2

# 源文件和目标
SRC := main.cpp
OBJ := main.o record_part.pb.o recordlist.pb.o
RES := .\res\res.rc
RES_OBJ := res.o

# 链接
$(project): $(OBJ) $(RES_OBJ)
	$(CXX) $(OBJ) $(RES_OBJ) -o $(project) $(CXXFLAGS) $(WX_CXXFLAGS) $(WX_LIBS) $(PROTO_LIBS)
	rm *.o

# 编译代码文件
%.o : %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(WX_CXXFLAGS) $(WX_LIBS)

%.o: ./proto/%.cc
	pkg-config --cflags protobuf
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(PROTO_LIBS)

# 编译资源文件
$(RES_OBJ): $(RES)
	$(RC) $(WX_CPPFLAGS) $< -O coff -o $@
