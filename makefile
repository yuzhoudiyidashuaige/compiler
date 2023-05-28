CC = g++
EXE = Compiler.exe
OBJS = GenerateCode.o main.o parsing.o token.o Ast.o GenerateJson.o

LLVM_LIBS = $(shell llvm-config --libs)

all:EXE

parsing.cpp: parsing.y
	bison -d parsing.y

token.cpp: token.l parsing.cpp
	flex -o token.cpp token.l

.cpp.o:
	g++ $< -c -o $@ -g

EXE: $(OBJS)
	$(CC) -o Compiler.exe $(OBJS) -g -lLLVM-10
clean:
	rm *.o


		