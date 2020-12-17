VPATH = lib:.

CFLAGS ?= -g #debug
#CFLAGS ?= -O3

# override CFLAGS and make them available for child makefiles
override CFLAGS += -Wall -Wpedantic -Wextra -Iinclude
export CFLAGS

.PHONY: all ex-lib ex-serial ex-files clean

all: ex-lib ex-serial ex-files

clean:
	find lib/ -name '*.so*' -exec rm -v {} \+
	$(MAKE) -C ex-lib clean
	$(MAKE) -C ex-serial clean

# execute make in directory ex-lib
ex-lib: -ltcp
	$(MAKE) -C ex-lib

# execute make in directory ex-serial
ex-serial: -ltcp -lserial
	$(MAKE) -C ex-serial

# execute make in directory ex-files
ex-files: -ltcp -lserial
	$(MAKE) -C ex-files

# TCP LIB
# =======

# make the lib available unversioned
lib/libtcp.so: lib/libtcp.so.2
	ln -sf $(notdir $<) $@

# resolve links (here .so.2 -> .so.2.0)
lib/libtcp.so.2: lib/libtcp.so.2.0
	ldconfig -r lib -n .

# compile the library v2.0
lib/libtcp.so.2.0: lib/tcp-util.c include/tcp-util.h
	$(CC) $(CFLAGS) -Wl,-soname,libtcp.so.2 -shared -fPIC -o $@ $<


# SERIAL LIB
# ==========

# make the lib available unversioned
lib/libserial.so: lib/libserial.so.1
	ln -sf $(notdir $<) $@

# resolve links (here .so.1 -> .so.1.0)
lib/libserial.so.1: lib/libserial.so.1.0
	ldconfig -r lib -n .

# compile the library v1.0
lib/libserial.so.1.0: lib/serial-util.c include/serial-util.h
	$(CC) $(CFLAGS) -Wl,-soname,libserial.so.1 -shared -fPIC -o $@ $<
