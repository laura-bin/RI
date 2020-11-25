VPATH = ../lib

# add warnings & add inc directory to the include path
CFLAGS += -I../include

.PHONY: all clean

all: echo-client echo-server

clean:
	rm -fv echo-client echo-server

# compile c files with the lib dependency
echo-client echo-server: echo-%: src/echo-%.c -ltcp
	$(CC) $(CFLAGS) -Llib -Wl,-rpath='$$ORIGIN/../lib' -o $@ $^
