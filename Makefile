CC=gcc
APP_VERSION ?= 0.1

.PHONY: release debug build clean install

release: clean
	CCFLAGS='-O3 -Wall' TARGET=release make build

debug:
	CCFLAGS='-g -Wall' TARGET=debug make build

build:
	mkdir -p bin/$(TARGET) \
		&& mkdir -p build/$(TARGET) \
		&& make bin/$(TARGET)/suid-wrapper

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin \
		&& cp bin/release/suid-wrapper $(DESTDIR)$(PREFIX)/bin/suid-wrapper

clean:
	rm -rf build bin

bin/$(TARGET)/suid-wrapper: build/$(TARGET)/linker.o build/$(TARGET)/runner.o
	$(CC) -o bin/$(TARGET)/suid-wrapper build/$(TARGET)/linker.o build/$(TARGET)/runner.o build/$(TARGET)/wrapper.o

build/$(TARGET)/linker.o: src/linker.c
	$(CC) -DAPP_VERSION=\"$(APP_VERSION)\" $(CCFLAGS) -c src/linker.c -o build/$(TARGET)/linker.o

build/$(TARGET)/runner: src/runner.c build/$(TARGET)/wrapper.o
	$(CC) -DAPP_VERSION=\"$(APP_VERSION)\" $(CCFLAGS) src/runner.c build/$(TARGET)/wrapper.o -o build/$(TARGET)/runner

build/$(TARGET)/runner.o: build/$(TARGET)/runner
	cd build/$(TARGET) \
		&& ld --relocatable --format binary --output runner.o runner

build/$(TARGET)/wrapper.o: src/wrapper.c src/wrapper.h
	$(CC) -DAPP_VERSION=\"$(APP_VERSION)\" $(CCFLAGS) -c src/wrapper.c -o build/$(TARGET)/wrapper.o
