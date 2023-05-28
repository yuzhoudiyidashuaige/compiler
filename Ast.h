#ifndef _AST_H_
#define _AST_H_

#include "llvm/ADT/STLExtras.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <llvm/IR/Value.h>
#include <string>
#include <vector>


using namespace std;

class Environment;

class Ast {
public:
  Ast(int lineNo) : lineNo(lineNo) {}
  virtual ~Ast() {}
  virtual void json(string &s) =0;
public:
  int lineNo;
};

class Exp : public Ast {
public:
  Exp(int lineNo) : Ast(lineNo) {}
  virtual llvm::Value *getAddr(Environment &Environment) {
    return nullptr;
  }
  virtual llvm::Value *GenerateCoder(Environment &Environment)=0;
};

class Stmt : public Ast {
public:
  Stmt(int lineNo) : Ast(lineNo) {}
  virtual void GenerateCoder(Environment &Environment) = 0;
};

class Int : public Exp {
public:
  Int(int value, int lineNo) : Exp(lineNo), value(value) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  int value;
};

class Float : public Exp {
public:
  Float(float value, int lineNo) : Exp(lineNo), value(value) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  float value;
};

class Char : public Exp {
public:
  Char(string &value, int lineNo) : Exp(lineNo), value(value) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  string &value;
};

class String : public Exp {
public:
  String(string &value, int lineNo) : Exp(lineNo), value(value) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  string &value;
};

class Identifier : public Exp {
public:
  Identifier(string &name, int lineNo) : Exp(lineNo), name(name) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  llvm::Value *getAddr(Environment &Environment);
  void json(string &s);
public:
  string name;
};

class getArrayElement : public Exp {   
public:
  getArrayElement(Identifier& identifier, Exp &index, int lineNo) : Exp(lineNo), identifier(identifier), index(index) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  llvm::Value *getAddr(Environment &Environment);
  void json(string &s);
public:
  Identifier  &identifier;
  Exp  &index;
};

class ArrayElementAssign : public Exp {   
public:
  ArrayElementAssign(Identifier &identifier, Exp &index, Exp &value_Exp, int lineNo) : Exp(lineNo), identifier(identifier), index(index), value_Exp(value_Exp) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Identifier& identifier;
  Exp &index;
  Exp &value_Exp;
};

class FunctionCall : public Exp {
public:
  FunctionCall(Identifier &identifier, vector<Exp*> &args, int lineNo) : Exp(lineNo), identifier(identifier), args(args) {}
  FunctionCall(Identifier &identifier, int lineNo) : Exp(lineNo), identifier(identifier) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Identifier& identifier;
  vector<Exp*> args;
};

class BinaryOp : public Exp {  
public:
  BinaryOp(int op, Exp &left_arg, Exp &right_arg, int lineNo) : Exp(lineNo), op(op), left_arg(left_arg), right_arg(right_arg) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  int op;
  Exp& left_arg;
  Exp& right_arg;
};

class getAddr : public Exp {  //&
public:
  getAddr(Identifier &identifier, int lineNo) : Exp(lineNo), identifier(identifier) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Identifier& identifier;
};

class getArrayAddr : public Exp {  
public:
  getArrayAddr(Identifier &identifier, Exp &index, int lineNo) : Exp(lineNo), index(index), identifier(identifier) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Identifier& identifier;
  Exp& index;
};

class Assignment : public Exp {
public:
  Assignment(Identifier &id, Exp &exp_value, int lineNo) : Exp(lineNo), id(id), exp_value(exp_value) {}
  llvm::Value *GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Identifier& id;
  Exp& exp_value;
};

class Block : public Stmt {
public:
  Block(int lineNo) : Stmt(lineNo) {}
  Block(vector<Stmt*> StmtList, int lineNo) : Stmt(lineNo),  StmtList(StmtList) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  vector<Stmt*> StmtList;
};

class ExpStmt : public Stmt {  
public:
	Exp &exp;
	ExpStmt(Exp& exp, int lineNo) : Stmt(lineNo),  exp(exp) {}
	void GenerateCoder(Environment &Environment);
  void json(string &s);
};

class BreakStmt : public Stmt {
public:
	BreakStmt(int lineNo) : Stmt(lineNo) {}
	void GenerateCoder(Environment &Environment);
  void json(string &s);
};

class IfElseStmt : public Stmt {
public:
  IfElseStmt(Exp &exp, Block &ifBlock, Block &elseBlock, int lineNo)
    : Stmt(lineNo), exp(exp), ifBlock(ifBlock), elseBlock(elseBlock) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Exp &exp;
  Block &ifBlock;
  Block &elseBlock;
};

class IfStmt : public Stmt {
public:
  IfStmt(Exp &exp, Block &ifBlock, int lineNo)
    : Stmt(lineNo), exp(exp), ifBlock(ifBlock) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Exp &exp;
  Block &ifBlock;
};

class WhileStmt : public Stmt {
public:
  WhileStmt(Exp &exp, Block &block, int lineNo)
    : Stmt(lineNo), exp(exp), block(block) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Exp &exp;
  Block &block;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt(Exp &exp, int lineNo) : Stmt(lineNo), exp(exp) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Exp &exp;
};

class ReturnVoid : public Stmt {
public:
  ReturnVoid(int lineNo) : Stmt(lineNo) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
};

class VariableDeclaration : public Stmt {  
public:
  VariableDeclaration(Identifier &var_type, Identifier &identifier, int lineNo) : Stmt(lineNo), var_type(var_type), identifier(identifier), array_size(0), assignmentExp(nullptr) {}

  VariableDeclaration(Identifier &var_type, Identifier &identifier, 
      int array_size, int lineNo) : 
        Stmt(lineNo), var_type(var_type), identifier(identifier), array_size(array_size), assignmentExp(nullptr) {}

  VariableDeclaration(Identifier& var_type, Identifier& identifier, 
    Exp *assignmentExp, int lineNo) : 
        Stmt(lineNo), var_type(var_type), identifier(identifier), assignmentExp(assignmentExp) {
        }

  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  int array_size;
  Identifier &var_type;
  Identifier &identifier;
  Exp *assignmentExp;
};

class FunctionDeclaration : public Stmt {
public:
  FunctionDeclaration(Identifier &func_type, Identifier &identifier, 
    vector<VariableDeclaration*> args, Block& block, int lineNo) : Stmt(lineNo), func_type(func_type), identifier(identifier), args(args), block(block) {}
  void GenerateCoder(Environment &Environment);
  void json(string &s);
public:
  Identifier &func_type;
  Identifier &identifier;
  Block &block;
  vector<VariableDeclaration*> args;
};


#endif 