#ifndef _UTILS_H_
#define _UTILS_H_
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
#include <string>
#include <vector>
#include "Ast.h"
#include "parsing.hpp"

llvm::Type* getLLvmType_var(string type){ //transform type into llvm type
    if(type == "int"){return llvm::Type::getInt32Ty(GlobalContext);}
    else if(type == "float"){return llvm::Type::getFloatTy(GlobalContext);}
    else if(type == "char"){return llvm::Type::getInt8Ty(GlobalContext);}
    return llvm::Type::getVoidTy(GlobalContext);
}

llvm::Type* getLLvmType_var_Ptr(string type){ ////transform pointer's type into llvm type
    if(type == "int"){return llvm::Type::getInt32PtrTy(GlobalContext);}
    else if(type == "float"){return llvm::Type::getFloatPtrTy(GlobalContext);}
    else if(type == "char"){return llvm::Type::getInt8PtrTy(GlobalContext);}
    return llvm::Type::getVoidTy(GlobalContext);
}

llvm::Type* getLLvmType_Array(string type,int size){ ////transform array's type into llvm type
    if(type == "int"){ 
        return llvm::ArrayType::get(llvm::Type::getInt32Ty(GlobalContext), size);
    }
    else if(type == "float"){
        return llvm::ArrayType::get(llvm::Type::getFloatTy(GlobalContext), size);
    }
    else if(type == "char"){
        return llvm::ArrayType::get(llvm::Type::getInt8Ty(GlobalContext), size);
    }
    else{return nullptr;}
}

llvm::Instruction::CastOps getCastInst(llvm::Type* src, llvm::Type* dst) {
    if (src == llvm::Type::getFloatTy(GlobalContext) && dst == llvm::Type::getInt32Ty(GlobalContext)) { 
        return llvm::Instruction::FPToSI;  
    }
    else if (src == llvm::Type::getInt32Ty(GlobalContext) && dst == llvm::Type::getFloatTy(GlobalContext)) { 
        return llvm::Instruction::SIToFP;
    }
    else if (src == llvm::Type::getInt8Ty(GlobalContext) && dst == llvm::Type::getFloatTy(GlobalContext)) {
        return llvm::Instruction::UIToFP;
    }
    else if (src == llvm::Type::getInt8Ty(GlobalContext) && dst == llvm::Type::getInt32Ty(GlobalContext)) {
        return llvm::Instruction::ZExt;
    }
    else if (src == llvm::Type::getInt32Ty(GlobalContext) && dst == llvm::Type::getInt8Ty(GlobalContext)) {
        return llvm::Instruction::Trunc;
    }
    else {
        throw logic_error("[ERROR] Wrong typecast");
    }
}

llvm::Value* typeCast(llvm::Value* src, llvm::Type* dst) {
    llvm::Instruction::CastOps op = getCastInst(src->getType(), dst);
    return MyIRBuilder.CreateCast(op, src, dst, "typecast");
}

void bi_cast(llvm::Value* &left,llvm::Value* &right){
    if (left->getType() != right->getType()) {
        /***float***/
        if (left->getType() == llvm::Type::getFloatTy(GlobalContext)) {
            right = typeCast(right, llvm::Type::getFloatTy(GlobalContext));
        } 
        /***int***/
        else {
            if (right->getType() == llvm::Type::getFloatTy(GlobalContext)) {
                left = typeCast(left, llvm::Type::getFloatTy(GlobalContext));
            } 
            else {
                if (left->getType() == llvm::Type::getInt32Ty(GlobalContext)) {
                    right = typeCast(right, llvm::Type::getInt32Ty(GlobalContext));
                } 
                else if(right->getType() == llvm::Type::getInt32Ty(GlobalContext)) {
                    left = typeCast(left, llvm::Type::getInt32Ty(GlobalContext));
                } 
                else {
                    throw logic_error("Wrong variable type bool");
                }
            }
        }
    }
}

llvm:: Value* GenerateKeyFunctions(Environment &Environment,vector<Exp*> args,string type){
    vector<llvm::Value *> *arguments = new vector<llvm::Value *>;
    for(auto it: args){
        llvm::Value* i = it->GenerateCoder(Environment);
        if (i->getType() == llvm::Type::getFloatTy(GlobalContext))
            i = MyIRBuilder.CreateFPExt(i, llvm::Type::getDoubleTy(GlobalContext), "tmpdouble");
        arguments->push_back(i);
    }  

    if(type == "printf"){
        return MyIRBuilder.CreateCall(Environment.keyfunctions[0], *arguments, "printf");
    }
    else if(type == "scanf"){
        return MyIRBuilder.CreateCall(Environment.keyfunctions[1], *arguments, "scanf");
    }
    else if(type == "gets"){
        return MyIRBuilder.CreateCall(Environment.keyfunctions[2], *arguments, "gets");
    }
    else return nullptr;
}

#endif 