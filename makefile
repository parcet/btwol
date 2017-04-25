CC=gcc
CFLAGS=-c -fpermissive 
LIBS=-lbluetooth

all: btwold installer

btwold: btwold.o
	$(CC) $(LIBS) btwold.o -o btwold

btwold.o: btwold.cpp
	$(CC) $(CFLAGS) btwold.cpp

installer:
	cp btwold ./btwold-1.0/usr/sbin
	dpkg -b ./btwold-1.0 btwold-1.0-armhf.deb
clean:
	rm -f *o btwold
	rm -f btwold-1.0-armhf.deb

install:
	cp btwold /usr/sbin
	cp btwol.sh /etc/init.d
	cp btwol /etc/default

