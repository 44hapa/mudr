CXXFLAGS =  -O2 -g -Wall -fmessage-length=0

OBJS = wsmd.o
#LIBS = -lpthread
#LIBS = -liconv -L/usr/local/lib
#INCLUDE = -I/usr/local/include
TARGET = wsmd

TARGET =   ../bin/wsmd

$(TARGET):  $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(CXXFLAGS) $(INCLUDE)

all:    $(TARGET)
rmudd.o:
	$(CC) -O2 $(CXXFLAGS) -c wsmd.c $(INCLUDE)

clean:
	rm -f $(OBJS) $(TARGET)

