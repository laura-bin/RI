CC ?= gcc
CFLAGS ?= -g #debug
#CFLAGS ?= -O3
VPATH = lib:.

# add warnings & add inc directory to the include path
override CFLAGS += -Wall -Wpedantic -Wextra -Iinc

.PHONY: all clean

all: echo-client echo-server

clean:
	find . -name '*.so*' -exec rm -v {} \+
	rm -fv echo-client echo-server

# compile c files with the lib dependency
echo-client echo-server: echo-%: src/echo-%.c -ltcp
	$(CC) $(CFLAGS) -Llib -Wl,-rpath='$$ORIGIN/lib' -o $@ $^

# make the lib available unversioned
lib/libtcp.so: lib/libtcp.so.1
	ln -sf $(notdir $<) $@

# resolve links (here .so.1 -> .so.1.0)
lib/libtcp.so.1: lib/libtcp.so.1.0
	ldconfig -r lib -n .

# compile the library v1.0
lib/libtcp.so.1.0: lib/tcp-util.c inc/tcp-util.h
	$(CC) $(CFLAGS) -Wl,-soname,libtcp.so.1 -shared -fPIC -o $@ $<
