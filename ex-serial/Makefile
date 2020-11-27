VPATH = ../lib

# add warnings & add inc directory to the include path
CFLAGS += -I../include

.PHONY: all clean

all: client server

clean:
	find . -name '*.o' -exec rm -v {} \+
	rm -fv client server

# compile c files with the lib dependency
client server: %: src/%.c src/data.o -ltcp -lserial
	$(CC) $(CFLAGS) -Wl,-rpath='$$ORIGIN/../lib' -o $@ $^

src/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c -o $@ $<