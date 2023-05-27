#include <llvm-10/llvm/IR/GlobalVariable.h>
#include <llvm-10/llvm/IR/Type.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>
#include "GenerateCode.h"

llvm::LLVMContext GlobalContext; 
llvm::IRBuilder<> myBuilder(GlobalContext); //IRbuilder


Environment::Environment(){
    this->myModule = new llvm::Module("main",GlobalContext);
    vector<llvm::Function*> keyfunctions = Getkeyfunctions();
    this ->keyfunctions = keyfunctions;
    this->Return_or_break = false;
    this->generate_code_for_args = false;
}

map<string, llvm::Value*>& Environment::Get_localvar_values() {  // get current field's symboltable
    return Symboltable_stack.back()-> local_var; 
    }
map<string, llvm::Type*>& Environment::Get_localvar_types(){
    return Symboltable_stack.back()->local_var_type;
}

void Environment::PushSymboltable(){
    Symboltable_stack.push_back(new Symboltable);
}

void Environment::PopSymboltable(){
    Symboltable *tmp = Symboltable_stack.back();
    Symboltable_stack.pop_back();
    delete tmp;
}

llvm::Value* Environment::Get_variable_value(string variable) {
    
    // first find if it is local variable
    for (auto iter = Symboltable_stack.rbegin();iter != Symboltable_stack.rend();++iter){
        if((*iter)->local_var.find(variable) != (*iter)->local_var.end())  return (*iter)->local_var[variable];
    }

    // if it is not local variable, then find it in global variables
    return this->myModule->getGlobalVariable(variable, true); //it will return nullptr if the variable does't exi
}

vector<llvm::Function *> Environment::Getkeyfunctions()
{
    vector<llvm::Function *> keyfunctions;
    /*****printf*****/
    vector<llvm::Type*> printf_arg_types;printf_arg_types.push_back(myBuilder.getInt8PtrTy()); //according to zhihu examples
    llvm::FunctionType* printf_type = llvm::FunctionType::get(myBuilder.getInt32Ty(),printf_arg_types,true);
    llvm::Function* printf_func = llvm::Function::Create(printf_type,llvm::Function::ExternalLinkage,llvm::Twine("printf"),this->myModule);
    printf_func->setCallingConv(llvm::CallingConv::C);
    keyfunctions.push_back(printf_func);

    llvm::FunctionType* scanf_or_gets_type = llvm::FunctionType::get(myBuilder.getInt32Ty(),true);
    /*****scanf*****/
    llvm::Function* scanf_func = llvm::Function::Create(scanf_or_gets_type,llvm::Function::ExternalLinkage,llvm::Twine("scanf"),this->myModule);
    scanf_func->setCallingConv(llvm::CallingConv::C);
    keyfunctions.push_back(scanf_func);

    /*****gets*****/
    llvm::Function* gets_func = llvm::Function::Create(scanf_or_gets_type,llvm::Function::ExternalLinkage,llvm::Twine("gets"),this->myModule);
    gets_func->setCallingConv(llvm::CallingConv::C);
    keyfunctions.push_back(gets_func);

    return keyfunctions;
}

void Environment::run(BlockNode* rootblock){ 
    rootblock->GenerateCoder(*this);    //generate IR code iteratively
    llvm::verifyModule(*this->myModule, &llvm::outs());
    std::error_code EC;
    llvm::raw_fd_ostream Out("test.ll",EC); //redirect output to test.ll
    this->myModule->print(Out, nullptr);
}

