#
# usblamp - makefile
# Written 2011 by David Herrmann <dh.herrmann@googlemail.com>
# Dedicated to the Public Domain
#

.PHONY: all clean install

all:
	gcc -Wall -O2 -o usblamp src/main.c src/driver.c `pkg-config --cflags --libs libusb-1.0`

clean:
	rm -f usblamp

install:
	install -m 4755 usblamp /usr/bin

uninstall:
	rm /usr/bin/usblamp
