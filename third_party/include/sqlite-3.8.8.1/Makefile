# makefile for building Static Libary

#默认都打开debuginfo,optimize，GCC这点和WINDOWS完全不一样
debuginfo ?=1
optimize ?= 1

#Build with thread support.
threads ?= 1

#交叉编译 ==32，指定编译32位的程序，==64，编译64位程序
buildbits ?= 0

# 这个赋值?=是表示没有定义就赋值
AR        ?= ar

CC        ?= gcc
CXX       ?= g++
DLD       := $(CXX)
LD        := $(CXX)
RM        ?= rm -f
MKDIR     ?= mkdir -p
RMDIR     ?= rm -rf


INCLDIRS ?=
DEFFLAGS ?=

ifeq ($(buildbits),32)
CFLAGS += -m32
CCFLAGS += -m32
LDFLAGS  += -m32
endif
ifeq ($(buildbits),64)
CFLAGS += -m64
CCFLAGS += -m32
LDFLAGS  += -m64
endif

#ARFLAGS好像外部有定义，诡异
ARFLAGS   = rsuv

#编译选项
#编译选项
CFLAGS += -W -Wall -Wpointer-arith 
CCFLAGS += -std=c++0x -W -Wall -Wpointer-arith 

#如果有优化
ifeq ($(optimize),1)
CFLAGS  += -O2
CCFLAGS += -O2
endif # optimize

#-ggdb这个调试开关打开后调试信息应该好于-g,但是不能使用其他调试器调试了
#如果需要调试信息，linux的版本一般还是带上了调试信息，无论版本
ifeq ($(debuginfo),1)
  CFLAGS  += -ggdb
  CCFLAGS += -ggdb
endif # debuginfo

LIBARYS += -ldl

ifeq ($(threads),1)
  CFLAGS += -lpthread
  CCFLAGS += -lpthread
  LIBARYS += -lpthread -lrt
endif

DEFFLAGS += -D_GNU_SOURCE

C_FLIE   = $(wildcard ./*.c ) 
CPP_FILE = $(wildcard ./*.cpp )

O_CFILE   = $(patsubst ./%.c, ./%.o, $(C_FLIE))
O_CPPFILE = $(patsubst ./%.cpp, ./%.o, $(CPP_FILE))
#这个根据情况调整，
O_FILE  = $(O_CFILE) 

COMPILE.c  = $(CC)  $(CFLAGS)  $(DEFFLAGS) -c
COMPILE.cc = $(CXX) $(CCFLAGS) $(DEFFLAGS) -c

OUTFILE = libsqlite.a

.PHONY :all clean

all : $(OUTFILE) 

#这个实用GCC编译
$(OUTFILE): $(O_FILE)
	$(AR) $(ARFLAGS) $(OUTFILE) $(O_FILE)
	
./%.o : ./%.c
	$(COMPILE.c) $(INCLDIRS) $< -o $@ 

#$(OUTFILE): $(O_FILE)
#	$(AR) $(ARFLAGS) $(OUTFILE) $(O_FILE)
#%.o : %.cpp
#	$(COMPILE.cc) $(INCLDIRS) $< -o $@ 

clean:
	$(RM)  $(OUTFILE) $(O_FILE)
