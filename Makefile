include Makefile.inc
HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)

CFLAGS=-Wall -pedantic -std=c99 -D_POSIX_C_SOURCE=200112L
LDFLAGS=-lrt -lm -lpthread
EXE=bubble-threaded

.PHONY: clean

all: CFLAGS += -g
all: $(EXE)

release: CFLAGS += -O3
release: $(EXE)
$(EXE): bubble-threaded.o
	@echo "linking $@..."
	@$(CC) -o $@ $^ $(LDFLAGS)

bin/%.o: src/%.c
	@echo "compiling $<..."
	@mkdir -p bin/ 	
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@echo "cleaning..."
	@rm -rf $(EXE) $(OBJ) bin/bubble-threaded.o
	@rm -rf bin/
	@rm -rf doc/

doc: doc/index.html

doc/index.html: Doxyfile $(HEADERS) $(SOURCES)
	@echo "generating doc with doxygen..."
	@doxygen
