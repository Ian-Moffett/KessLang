all:
	mkdir -p bin
	gcc src/*.c -o bin/main

install:
	sudo install bin/main /usr/bin/kesslang
