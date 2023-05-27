make 
./Compiler.exe < ../test/course/course.c > /dev/null
llvm-as-10 test.ll 
llc-10 test.bc
clang-10 -c test.s
clang-10 test.o -o test
../tester/advisor/advisor-linux-amd64 ./test

./Compiler.exe < ../test/quicksort/quicksort.c > /dev/null
llvm-as-10 test.ll
llc-10 test.bc
clang-10 -c test.s
clang-10 test.o -o test
../tester/quicksort/quicksort-linux-amd64 ./test

./Compiler.exe < ../test/multiplication/multiply.c > /dev/null
llvm-as-10 test.ll
llc-10 test.bc
clang-10 -c test.s
clang-10 test.o -o test
../tester/matrix/matrix-linux-amd64 ./test


