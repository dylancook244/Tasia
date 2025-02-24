

make &&
./bin/Tasia hello.sia &&
clang runner.c hello.o -o hello_program &&
./hello_program