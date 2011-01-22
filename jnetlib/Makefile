# freebsd3 makefile
default: jnl.a

LDFLAGS = -pthread -lstdc++
CFLAGS = -s -O2 -Wall -DTHREAD_SAFE -D_THREAD_SAFE -D_REENTRANT
CC = gcc
CPP = g++
CXX = g++

asyncdns.o: asyncdns.cpp
	$(CC) ${CFLAGS} -c asyncdns.cpp
connection.o: connection.cpp
	$(CC) ${CFLAGS} -c connection.cpp
httpget.o: httpget.cpp
	$(CC) ${CFLAGS} -c httpget.cpp
httppost.o: httppost.cpp
	$(CC) ${CFLAGS} -c httppost.cpp
httpserv.o: httpserv.cpp
	$(CC) ${CFLAGS} -c httpserv.cpp
ircclient.o: ircclient.cpp
	$(CC) ${CFLAGS} -c ircclient.cpp
listen.o: listen.cpp
	$(CC) ${CFLAGS} -c listen.cpp
sslconnection.o: sslconnection.cpp
	$(CC) ${CFLAGS} -c sslconnection.cpp
ssllisten.o: ssllisten.cpp
	$(CC) ${CFLAGS} -c ssllisten.cpp
util.o: util.cpp
	$(CC) ${CFLAGS} -c util.cpp
webserver.o: webserver.cpp
	$(CC) ${CFLAGS} -c webserver.cpp

OBJS = asyncdns.o connection.o httpget.o httppost.o httpserv.o ircclient.o listen.o sslconnection.o ssllisten.o util.o webserver.o

jnl.a: ${OBJS}
	-rm -f jnl.a
	ar rcs jnl.a ${OBJS}

test: ${OBJS} test.o
	$(CC) ${CFLAGS} -o test test.o ${OBJS} ${LDFLAGS}

clean:
	-rm -f ${OBJS} test jnl.a test.o
