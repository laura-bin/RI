VPATH = ../lib

# add warnings & add inc directory to the include path
CFLAGS += -I../include

.PHONY: all clean

all: client server

clean:
	find . -name '*.o' -exec rm -v {} \+
	rm -fv client server
	rmdir out

# compile c files with the lib dependency
client server: %: src/%.c out/data.o -ltcp -lserial
	$(CC) $(CFLAGS) -Wl,-rpath='$$ORIGIN/../lib' -o $@ $^

out/%.o: src/%.c include/%.h out
	$(CC) $(CFLAGS) -c -o $@ $<

out:
	mkdir out