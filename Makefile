all:
	mkdir -p bin
	gcc src/*.c -o bin/main

installdebug:
	sudo install bin/main /usr/bin/kesslang

install:
	sudo install bin/main /usr/bin/kesslang
	sudo cp -r klstd /usr/include/klstd
