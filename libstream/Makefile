MAJOR=1
MINOR=0
VERSION=$(MAJOR).$(MINOR)

#TARGETAR=libstream.a
TARGETSO=libstream.so

CC=gcc
CXX=g++ -g
LD=ld
AR=ar

CFLAGS= -DNDEBUG -D_UNIX -D_LINUX -D_REENTRANT -Wall -fPIC  -I. -I/usr/local/include  -D_FILE_OFFSET_BITS=64
CXXFLAGS= -DNDEBUG -D_UNIX -D_LINUX -D_REENTRANT -Wall -fPIC -I. -I/usr/local/include -D_FILE_OFFSET_BITS=64
ARFLAGS=-rcs
LDFLAGS=$(ARCH) -shared -Wl,-soname,$(TARGETSO).$(MAJOR) -lpthread  -L/usr/local/lib -o 
SYSLIBS=-lpthread 
LIBS= 

ARCH=
RANLIB=ranlib
LINK=ln -sf

#OBJ_DIR=.obj/

SRCS= $(wildcard *.c) $(wildcard *.cpp)
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCS)))

TARGET_OBJS=$(patsubst %.c,$(OBJ_DIR)%.o,$(patsubst %.cpp,$(OBJ_DIR)%.o,$(SRCS)))


.SUFFIXES: .c .cpp

.c.o:
	$(CC) $(CFLAGS) -c -o $(OBJ_DIR)$@ $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)$@ $<

.PHONY : all clean

all: $(TARGETAR) $(TARGETSO)

#$(TARGETAR): $(OBJS)
#	$(AR) $(ARFLAGS) $@ $(OBJS)
#	$(RANLIB) $(TARGETAR)

$(TARGETSO): $(OBJS)
	$(CXX) $(LDFLAGS) $(TARGETSO).$(VERSION) $(TARGET_OBJS)
	$(LINK) $(TARGETSO).$(VERSION) $(TARGETSO).$(MAJOR)
	$(LINK) $(TARGETSO).$(MAJOR) $(TARGETSO)


clean:
	rm -f $(TARGETAR) $(TARGETSO) $(TARGETSO).$(VERSION) $(TARGETSO).$(MAJOR) $(TARGET_OBJS) core


