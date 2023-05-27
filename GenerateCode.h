#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <map>
#include <stack>
#include <string>
#include <typeinfo>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include "Ast.h"

using namespace std;

extern llvm::LLVMContext GlobalContext; //定义全局context
extern llvm::IRBuilder<> myBuilder; //定义全局IRbuilder

struct Symboltable{ 
    map<string, llvm::Value*> local_var; //局部变量map
    map<string, llvm::Type*> local_var_type;//局部变量string-llvm::type的map
};


class Environment{
public:
    vector<Symboltable *> Symboltable_stack; //符号栈
    llvm::Module *myModule; 
    vector<llvm::Function*> keyfunctions;
    //llvm::Function *printf,*scanf, *gets;
    llvm::Function* Caller_function;
    llvm::BasicBlock* returnaddr;
    llvm::Value* returnvalue;
    bool generate_code_for_args; // if it is generating IR code for arguments now. Used when the array is an argument of function
    bool Return_or_break; //if return or break, it should stop generate IR code and jump to next block
    map<string, llvm::Value*>& Get_localvar_values();
    map<string, llvm::Type*>& Get_localvar_types();
    
    Environment();
    void PushSymboltable();
    void PopSymboltable();
    llvm::Value* Get_variable_value(string variable);
    vector<llvm::Function*> Getkeyfunctions(); //得到llvm形式的printf函数
    // llvm::Function* getScanf();  //得到llvm形式的scanf函数
    // llvm::Function* getGets();   //得到llvm形式的gets函数
    void run(BlockNode* rootblock);   //From top to bottom parsing

};
