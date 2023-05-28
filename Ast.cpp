#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include "Ast.h"
#include "parsing.hpp"
#include <string>
#include <vector>
#include "GenerateCode.h"
#include "Utils.hpp"

using namespace std;

stack<llvm::BasicBlock *> Addr_after_loop_stack; //Used to record the address where to jump after break the loop

llvm::Value* Int::GenerateCoder(Environment &Environment){
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(GlobalContext),value,true);
}

llvm::Value* Float::GenerateCoder(Environment &Environment){
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(GlobalContext),value);
}

llvm::Value* Char::GenerateCoder(Environment &Environment){  
    if (this->value.size() == 3) // in the form of 'a'
        return MyIRBuilder.getInt8(this->value.at(1)); //get the a
    else { // match special cases:
        if (this->value.compare("'\\n'") == 0) {
            return MyIRBuilder.getInt8('\n');
        } else if (this->value.compare("'\\\\'") == 0){
            return MyIRBuilder.getInt8('\\');
        } else if (this->value.compare("'\\a'") == 0){
            return MyIRBuilder.getInt8('\a');
        } else if (this->value.compare("'\\b'") == 0){
            return MyIRBuilder.getInt8('\b');
        } else if (this->value.compare("'\\f'") == 0){
            return MyIRBuilder.getInt8('\f');
        } else if (this->value.compare("'\\t'") == 0){
            return MyIRBuilder.getInt8('\t');
        } else if (this->value.compare("'\\v'") == 0){
            return MyIRBuilder.getInt8('\v');
        } else if (this->value.compare("'\\''") == 0){
            return MyIRBuilder.getInt8('\'');
        } else if (this->value.compare("'\\\"'") == 0){
            return MyIRBuilder.getInt8('\"');
        } else if (this->value.compare("'\\0'") == 0){
            return MyIRBuilder.getInt8('\0');
        } else {
            throw logic_error("illegal char!" + this->value);
        }
    }
    return nullptr;
}

llvm::Value* String::GenerateCoder(Environment &Environment) {
    string str = value.substr(1, value.length() - 2); //delete quotation marks
    string carriage = string(1, '\n'); //transform \\n into \n
    int pos = str.find("\\n");
    while(pos != string::npos) {
        str = str.replace(pos, 2, carriage);
        pos = str.find("\\n");
    }
    llvm::Constant *ConstantStr = llvm::ConstantDataArray::getString(GlobalContext, str);
    /*首先，代码使用了 llvm::GlobalVariable 类创建一个全局变量，并将其存储在名为"globalVar"的 Value 对象中。
    然后，代码通过*(emitContext.myModule) 获取当前编译单元 (emitContext.myModule) 的引用。
    接着，代码使用 strConst->getType() 获取字符串常量的类型，该类型应该是一个 String 类型。
    然后，代码将 true 指定为全局变量的符号引用 (symbol reference),表示该变量是全局变量，并且该变量的链接类型为 PrivateLinkage，表示该变量只在其定义的模块中可见, 内部链接，不在符号表中
    最后，代码使用 strConst 和"Const_String"来命名全局变量，并将其存储在名为"globalVar"的 Value 对象中。*/
    llvm::Value *globalVar = new llvm::GlobalVariable(*(Environment.myModule), ConstantStr->getType(), true, llvm::GlobalValue::PrivateLinkage, ConstantStr, "_Const_String_");
    vector<llvm::Value*> indexList; 
    indexList.push_back(MyIRBuilder.getInt32(0));
    indexList.push_back(MyIRBuilder.getInt32(0));
    llvm::Value * StrPointer = MyIRBuilder.CreateInBoundsGEP(globalVar, llvm::ArrayRef<llvm::Value*>(indexList), "tmpstring");
    return StrPointer;
}

llvm::Value* Identifier::GenerateCoder(Environment &Environment){

    llvm::Value* variable = Environment.Get_variable_value(name);
    // if can't find the variable
    if(variable == nullptr){
        throw logic_error("Error! Undeclared variable" + name);
        return nullptr;
    }
    llvm::Type* viariable_type = variable->getType()->getPointerElementType();
    llvm::Value* result = nullptr;
    // if type is array
    if(viariable_type->isArrayTy()) {
        vector<llvm::Value*> indexList;
        indexList.push_back(MyIRBuilder.getInt32(0));
        indexList.push_back(MyIRBuilder.getInt32(0));
        result = MyIRBuilder.CreateInBoundsGEP(variable, indexList, "arrayPtr");
    }
    else { // load the variable
        result = new llvm::LoadInst(viariable_type, variable, "LoadInst", false, MyIRBuilder.GetInsertBlock());
    }
    return result;
}

llvm::Value* Identifier::getAddr(Environment &Environment){ // use for scanf
    llvm::Value* variable = Environment.Get_variable_value(name);
    if(variable == nullptr){
        throw logic_error("Error! Undeclared variable" + name);
        return nullptr;
    }
    return variable;
}

//get address of the array
llvm::Value* getArrayAddr::GenerateCoder(Environment &Environment){
    llvm::Value* arrayValue = Environment.Get_variable_value(identifier.name);
    if(arrayValue == nullptr){
        throw logic_error("Error! Undeclared Array" + identifier.name);
		return nullptr;
    }
    llvm::Value* Arrayindex = index.GenerateCoder(Environment);
    vector<llvm::Value*> indexList;


    /***Pointer***/
    if(arrayValue->getType()->getPointerElementType()->isPointerTy()) {
        arrayValue = MyIRBuilder.CreateLoad(arrayValue->getType()->getPointerElementType(), arrayValue);
        indexList.push_back(Arrayindex);    
    }
    /***Array***/
    else {
        indexList.push_back(MyIRBuilder.getInt32(0));
        indexList.push_back(Arrayindex);    
    }

    llvm::Value* AddrPtr =  MyIRBuilder.CreateInBoundsGEP(arrayValue, llvm::ArrayRef<llvm::Value*>(indexList), "AddrPtr");
    return AddrPtr;
}


// create load of the address of the array[index]
llvm::Value* getArrayElement::GenerateCoder(Environment &Environment){
    llvm::Value* arrayValue = Environment.Get_variable_value(identifier.name);
    if(arrayValue == nullptr){
        throw logic_error("Error! Undeclared Array" + identifier.name);
		return nullptr;
    }

    llvm::Value* Arrayindex = index.GenerateCoder(Environment);
    vector<llvm::Value*> indexList;

    /***Pointer***/
    if(arrayValue->getType()->getPointerElementType()->isPointerTy()) {
        arrayValue = MyIRBuilder.CreateLoad(arrayValue->getType()->getPointerElementType(), arrayValue);
        indexList.push_back(Arrayindex);    
    }
    /***Array***/
    else {
        indexList.push_back(MyIRBuilder.getInt32(0));
        indexList.push_back(Arrayindex);    
    }
    // create the address pointer
    llvm::Value* AddrPtr =  MyIRBuilder.CreateInBoundsGEP(arrayValue, llvm::ArrayRef<llvm::Value*>(indexList), "Arraysection");
    // use the pointer to create load
    return MyIRBuilder.CreateLoad(AddrPtr->getType()->getPointerElementType(), AddrPtr, "loadarray");
}

//  get address of the array[index]    just like above create load process but return the address ptr
llvm::Value* getArrayElement::getAddr(Environment &Environment){
    llvm::Value* arrayValue = Environment.Get_variable_value(identifier.name);
    if(arrayValue == nullptr){
        throw logic_error("Error! Undeclared Array" + identifier.name);
		return nullptr;
    }

    llvm::Value* Arrayindex = index.GenerateCoder(Environment);
    vector<llvm::Value*> indexList;

    /***Pointer***/
    if(arrayValue->getType()->getPointerElementType()->isPointerTy()) {
        arrayValue = MyIRBuilder.CreateLoad(arrayValue->getType()->getPointerElementType(), arrayValue);
        indexList.push_back(Arrayindex);    
    }
    /***Array***/
    else {
        indexList.push_back(MyIRBuilder.getInt32(0));
        indexList.push_back(Arrayindex);    
    }
    // create the address pointer
    llvm::Value* AddrPtr =  MyIRBuilder.CreateInBoundsGEP(arrayValue, llvm::ArrayRef<llvm::Value*>(indexList), "tmparray");
    return AddrPtr;
}

llvm::Value* ArrayElementAssign::GenerateCoder(Environment &Environment){
    llvm::Value* arrayValue = Environment.Get_variable_value(identifier.name);
    if(arrayValue == nullptr){
        cerr << "undeclared array " << identifier.name << endl;
		return nullptr;
    }
    llvm::Value* Arrayindex = index.GenerateCoder(Environment);
    vector<llvm::Value*> indexList;

    /***Pointer***/
    if(arrayValue->getType()->getPointerElementType()->isPointerTy()) {
        arrayValue = MyIRBuilder.CreateLoad(arrayValue->getType()->getPointerElementType(), arrayValue);
        indexList.push_back(Arrayindex);    
    }
    /***Array***/ 
    else {
        indexList.push_back(MyIRBuilder.getInt32(0));
        indexList.push_back(Arrayindex);    
    }
    llvm::Value* address_pointer =  MyIRBuilder.CreateInBoundsGEP(arrayValue, llvm::ArrayRef<llvm::Value*>(indexList), "tmpvar");
    llvm::Value *assign_value = value_Exp.GenerateCoder(Environment);
    //type cast assignvalue to array type
    if (assign_value->getType() != address_pointer->getType()->getPointerElementType())
        assign_value = typeCast(assign_value, address_pointer->getType()->getPointerElementType());
    // create store IR code
    return MyIRBuilder.CreateStore(assign_value, address_pointer);
}


llvm::Value* FunctionCall::GenerateCoder(Environment &Environment){
    llvm:: Value* gkf = GenerateKeyFunctions(Environment, args, identifier.name); //if call scanf/printf/gets
    if(gkf!=nullptr) return gkf;

    //find the function named ifentifier.name in myModule
    llvm::Function *func = Environment.myModule->getFunction(identifier.name.c_str());
    if (func == NULL) {
		throw logic_error("Error! Undeclared function" + identifier.name);
	}

    vector<llvm::Value*> *call_args=new vector<llvm::Value *>;
    for(auto i : args){  //generate IRcode for each argument
        call_args->push_back((*i).GenerateCoder(Environment));
    }
	return MyIRBuilder.CreateCall(func, *call_args, "");; 
}

llvm::Value* BinaryOp::GenerateCoder(Environment &Environment){
    llvm::Value* left_llvmvalue = left_arg.GenerateCoder(Environment);
    llvm::Value* right_llvmvalue = right_arg.GenerateCoder(Environment);
    llvm::Instruction::BinaryOps bi_op;

    if(op == PLUS || op == MINUS || op == MUL || op == DIV){ // + - * /
        bi_cast(left_llvmvalue,right_llvmvalue); //type cast
        if(op == PLUS){
            bi_op = left_llvmvalue->getType()->isFloatTy() ? llvm::Instruction::FAdd : llvm::Instruction::Add;
        }
        else if(op == MINUS){
            bi_op = left_llvmvalue->getType()->isFloatTy() ? llvm::Instruction::FSub : llvm::Instruction::Sub;
        }
        else if(op == MUL){
            bi_op = left_llvmvalue->getType()->isFloatTy() ? llvm::Instruction::FMul : llvm::Instruction::Mul;
        }
        else if(op == DIV){
            bi_op = left_llvmvalue->getType()->isFloatTy() ? llvm::Instruction::FDiv : llvm::Instruction::SDiv;
        }
        return llvm::BinaryOperator::Create(bi_op,left_llvmvalue,right_llvmvalue,"", MyIRBuilder.GetInsertBlock());
    }
    else if(op == AND){
        if (left_llvmvalue->getType() != llvm::Type::getInt1Ty(GlobalContext) || right_llvmvalue->getType() != llvm::Type::getInt1Ty(GlobalContext)) {
            throw logic_error("cannot use types other than bool in and exp");
        }
        return MyIRBuilder.CreateAnd(left_llvmvalue, right_llvmvalue, "tmpAnd");
    }
    else if (op == OR) {
        if (left_llvmvalue->getType() != llvm::Type::getInt1Ty(GlobalContext) || right_llvmvalue->getType() != llvm::Type::getInt1Ty(GlobalContext)) {
            throw logic_error("cannot use types other than bool in and exp");
        }
        return MyIRBuilder.CreateOr(left_llvmvalue, right_llvmvalue, "tmpOR");
    }
    else{  //< > = != <= >=
        bi_cast(left_llvmvalue,right_llvmvalue);
        if (op == EQ) {
            return (left_llvmvalue->getType() == llvm::Type::getFloatTy(GlobalContext)) ? MyIRBuilder.CreateFCmpOEQ(left_llvmvalue, right_llvmvalue, "fcmptmp") : MyIRBuilder.CreateICmpEQ(left_llvmvalue, right_llvmvalue, "icmptmp");
        }
        else if (op == NEQ) {
            return (left_llvmvalue->getType() == llvm::Type::getFloatTy(GlobalContext)) ? MyIRBuilder.CreateFCmpONE(left_llvmvalue, right_llvmvalue, "fcmptmp") : MyIRBuilder.CreateICmpNE(left_llvmvalue, right_llvmvalue, "icmptmp");
        }
        else if (op == GT) {
            return (left_llvmvalue->getType() == llvm::Type::getFloatTy(GlobalContext)) ? MyIRBuilder.CreateFCmpOGT(left_llvmvalue, right_llvmvalue, "fcmptmp") : MyIRBuilder.CreateICmpSGT(left_llvmvalue, right_llvmvalue, "icmptmp");
        }
        else if (op == LT) {
            return (left_llvmvalue->getType() == llvm::Type::getFloatTy(GlobalContext)) ? MyIRBuilder.CreateFCmpOLT(left_llvmvalue, right_llvmvalue, "fcmptmp") : MyIRBuilder.CreateICmpSLT(left_llvmvalue, right_llvmvalue, "icmptmp");
        }
        else if (op == GEQ) {
            return (left_llvmvalue->getType() == llvm::Type::getFloatTy(GlobalContext)) ? MyIRBuilder.CreateFCmpOGE(left_llvmvalue, right_llvmvalue, "fcmptmp") : MyIRBuilder.CreateICmpSGE(left_llvmvalue, right_llvmvalue, "icmptmp");
        }
        else if (op == LEQ) {
            return (left_llvmvalue->getType() == llvm::Type::getFloatTy(GlobalContext)) ? MyIRBuilder.CreateFCmpOLE(left_llvmvalue, right_llvmvalue, "fcmptmp") : MyIRBuilder.CreateICmpSLE(left_llvmvalue, right_llvmvalue, "icmptmp");
        }
        return NULL;
    }
}

llvm::Value* getAddr::GenerateCoder(Environment &Environment){
    // check environment symboltable or global variable
    llvm::Value* value_addr = Environment.Get_variable_value(identifier.name);
    if(value_addr == nullptr){ //can't find the variable
        throw logic_error("Undeclared variable:"+identifier.name);
		return nullptr;
    }
    return value_addr;
}

llvm::Value* Assignment::GenerateCoder(Environment &Environment){
    // check environment symboltable or global variable
    llvm::Value* value_addr = Environment.Get_variable_value(id.name);
    if(value_addr == nullptr){ //can't find the variable
        throw logic_error("Undeclared variable:"+id.name);
		return nullptr;
    }

    llvm::Value* assign_value = exp_value.GenerateCoder(Environment);
    // get insert block
    auto CurrentBlock = MyIRBuilder.GetInsertBlock();
    
    if (assign_value->getType() != value_addr->getType()->getPointerElementType())
        assign_value = typeCast(assign_value, value_addr->getType()->getPointerElementType());

    return new llvm::StoreInst(assign_value, value_addr, false, CurrentBlock);
}

void Block::GenerateCoder(Environment &Environment){
    for(auto i : StmtList){
        (*i).GenerateCoder(Environment);
        // if we return or break, we should skip following Stmts
        if(Environment.Return_or_break == true)
            break;
    }
}

void ExpStmt::GenerateCoder(Environment &Environment){
	exp.GenerateCoder(Environment);
}

void BreakStmt::GenerateCoder(Environment &Environment){
    Environment.Return_or_break = true;
    //branch to the block after loop
    MyIRBuilder.CreateBr(Addr_after_loop_stack.top());
}

void IfStmt::GenerateCoder(Environment &Environment){
    llvm::Value *br_flag = exp.GenerateCoder(Environment);
    // create compare whether to branch
    br_flag = MyIRBuilder.CreateICmpNE(br_flag, llvm::ConstantInt::get(llvm::Type::getInt1Ty(GlobalContext), 0, true), "ifCond");
    llvm::BasicBlock *Ifblock = llvm::BasicBlock::Create(GlobalContext, "if", Environment.Caller_function);
    llvm::BasicBlock *Afterifblock = llvm::BasicBlock::Create(GlobalContext, "then_if",Environment.Caller_function);
    MyIRBuilder.CreateCondBr(br_flag, Ifblock, Afterifblock);

    MyIRBuilder.SetInsertPoint(Ifblock);
    // push if block's symboltable
    Environment.PushSymboltable();
    ifBlock.GenerateCoder(Environment);
    // pop if block's symboltable
    Environment.PopSymboltable();

    if(Environment.Return_or_break) //just break
        Environment.Return_or_break = false;
    else //branch to Afterifblock
        MyIRBuilder.CreateBr(Afterifblock);

    MyIRBuilder.SetInsertPoint(Afterifblock);    
}

void IfElseStmt::GenerateCoder(Environment &Environment){  
    // 跳转判断语句
    llvm::Value *br_flag = exp.GenerateCoder(Environment), *thenValue = nullptr, *elseValue = nullptr;
    br_flag = MyIRBuilder.CreateICmpNE(br_flag, llvm::ConstantInt::get(llvm::Type::getInt1Ty(GlobalContext), 0, true), "ifCond");
    llvm::BasicBlock *Ifblock = llvm::BasicBlock::Create(GlobalContext, "if", Environment.Caller_function);
    llvm::BasicBlock *Elseblock = llvm::BasicBlock::Create(GlobalContext, "else",Environment.Caller_function);
    llvm::BasicBlock *After_ifelse_block = llvm::BasicBlock::Create(GlobalContext, "then_else",Environment.Caller_function);
    MyIRBuilder.CreateCondBr(br_flag, Ifblock, Elseblock);
    MyIRBuilder.SetInsertPoint(Ifblock);
    // push if block's symboltable
    Environment.PushSymboltable();
    ifBlock.GenerateCoder(Environment);
    // pop if block's symboltable
    Environment.PopSymboltable();

    if(Environment.Return_or_break) //just break
        Environment.Return_or_break = false;
    else //branch to After_ifelse_block
        MyIRBuilder.CreateBr(After_ifelse_block);

    MyIRBuilder.SetInsertPoint(Elseblock);
    // push else block's symboltable
    Environment.PushSymboltable();
    elseBlock.GenerateCoder(Environment);
    // pop else block's symboltable
    Environment.PopSymboltable();

    if(Environment.Return_or_break) //just break
        Environment.Return_or_break = false;
    else //branch to After_ifelse_block
        MyIRBuilder.CreateBr(After_ifelse_block);

    MyIRBuilder.SetInsertPoint(After_ifelse_block);    
}

void WhileStmt::GenerateCoder(Environment &Environment){

    llvm::BasicBlock *whilecondition_block = llvm::BasicBlock::Create(GlobalContext, "cond", Environment.Caller_function);
    llvm::BasicBlock *loop_block = llvm::BasicBlock::Create(GlobalContext, "loop", Environment.Caller_function);
    llvm::BasicBlock *After_while_block = llvm::BasicBlock::Create(GlobalContext, "afterLoop", Environment.Caller_function);
    Addr_after_loop_stack.push(After_while_block);
    MyIRBuilder.CreateBr(whilecondition_block);
    MyIRBuilder.SetInsertPoint(whilecondition_block);

    llvm::Value *if_loop = exp.GenerateCoder(Environment);
    if_loop = MyIRBuilder.CreateICmpNE(if_loop, llvm::ConstantInt::get(llvm::Type::getInt1Ty(GlobalContext), 0, true), "whileCond");
    MyIRBuilder.CreateCondBr(if_loop, loop_block, After_while_block);
    whilecondition_block = MyIRBuilder.GetInsertBlock();

    MyIRBuilder.SetInsertPoint(loop_block);
    // push loop block's symboltable
    Environment.PushSymboltable();
    block.GenerateCoder(Environment);
    if(Environment.Return_or_break) //break while
        Environment.Return_or_break = false;
    else //branch back to whilecondition
        MyIRBuilder.CreateBr(whilecondition_block);

    // pop loop block's symboltable
    Environment.PopSymboltable();
    MyIRBuilder.SetInsertPoint(After_while_block);
    Addr_after_loop_stack.pop();
}

void ReturnStmt::GenerateCoder(Environment &Environment){
	llvm::Value *return_value = exp.GenerateCoder(Environment);
    if (return_value->getType() != Environment.returnvalue->getType()->getPointerElementType()){
        return_value = typeCast(return_value, Environment.returnvalue->getType()->getPointerElementType());
    }
    MyIRBuilder.CreateStore(return_value, Environment.returnvalue);
    Environment.Return_or_break = true;
    MyIRBuilder.CreateBr(Environment.returnaddr);
}

void ReturnVoid::GenerateCoder(Environment &Environment){
    Environment.Return_or_break = true;
    MyIRBuilder.CreateBr(Environment.returnaddr);
}

void VariableDeclaration::GenerateCoder(Environment &Environment) {  
    if(array_size == 0){ //if not an array
        llvm::Type* llvmType = getLLvmType_var(var_type.name);
        // if there is no Caller_function, this varaible is global variable
        if(Environment.Caller_function == nullptr) {
            llvm::Value *find_var = Environment.myModule->getGlobalVariable(identifier.name, true); //check whether redefined global variable
            if(find_var != nullptr){
                throw logic_error("Redefined Global Variable: " + identifier.name);
            }
            //create global variable and initialize
            llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(*(Environment.myModule), llvmType, false, llvm::GlobalValue::PrivateLinkage, 0, identifier.name);
            globalVar->setInitializer(llvm::ConstantInt::get(llvmType, 0));
        } 
        else {
            if(Environment.Get_localvar_values().count(identifier.name) != 0) {
                //check whether redefined variable in this environment
                throw logic_error("Redefined Local Variable: " + identifier.name);
            }
            auto *newblock = MyIRBuilder.GetInsertBlock();
            llvm::Value *mem_addr = new llvm::AllocaInst(llvmType,newblock->getParent()->getParent()->getDataLayout().getAllocaAddrSpace(),(identifier.name.c_str()), newblock);

            // update symbol table
            Environment.Get_localvar_types()[identifier.name] = llvmType;
            Environment.Get_localvar_values()[identifier.name] = mem_addr;
            if (assignmentExp != NULL) {
                Assignment assignment(identifier, *assignmentExp,lineNo);
                assignment.GenerateCoder(Environment);
            }
        }
    }
    else{ //array with arraysize not 0
        llvm::Type* llvmType = getLLvmType_Array(var_type.name, array_size); 
        // if there is no Caller_function, this varaible is global variable
        if(Environment.Caller_function == nullptr) { 
            llvm::Value *find_var = Environment.myModule->getGlobalVariable(identifier.name, true);
            if(find_var != nullptr){
                throw logic_error("Redefined Global Array: " + identifier.name);
            }
            //create global variable and initialize
            llvm::GlobalVariable* globalVar = new llvm::GlobalVariable(*(Environment.myModule), llvmType, false, llvm::GlobalValue::PrivateLinkage, 0, identifier.name);           
            std::vector<llvm::Constant*> initarray;
            llvm::Constant* element_0 = llvm::ConstantInt::get(llvmType->getArrayElementType(), 0);
            for (int i = 0; i < llvmType->getArrayNumElements(); i++) {
                initarray.push_back(element_0);
            }
            llvm::Constant* constArray = llvm::ConstantArray::get(llvm::ArrayType::get(llvmType->getArrayElementType(), llvmType->getArrayNumElements()), element_0);
            globalVar->setInitializer(constArray);
        }
        else { //if local variable
            if(Environment.Get_localvar_values().count(identifier.name) != 0) { // check redefined local variable               
                throw logic_error("Redefined Local Variable: " + identifier.name);
            }
            if(Environment.generate_code_for_args) {
                // if the array is declared in function arguments, we should pass the pointer of the array
                llvmType = getLLvmType_var_Ptr(var_type.name);
            }
            Environment.Get_localvar_types()[identifier.name] = llvmType;
            auto *newblock = MyIRBuilder.GetInsertBlock();
            llvm::Value *array_ptr = new llvm::AllocaInst(llvmType,newblock->getParent()->getParent()->getDataLayout().getAllocaAddrSpace(),(identifier.name.c_str()), newblock);
            Environment.Get_localvar_values()[identifier.name] = array_ptr;
        }
    }

}

void FunctionDeclaration::GenerateCoder(Environment &Environment){
    vector<llvm::Type*> Argument_types;
    for(auto it : args){
        if(it->array_size == 0) //not array we push the type 
            Argument_types.push_back(getLLvmType_var(it->var_type.name));
        else { //if array we push the pointer's type
            Argument_types.push_back(getLLvmType_var_Ptr(it->var_type.name));
        }
    }
    //use llvm api to create function's IR code
	llvm::FunctionType *function_type = llvm::FunctionType::get(getLLvmType_var(func_type.name), makeArrayRef(Argument_types), false);
	llvm::Function *function = llvm::Function::Create(function_type, llvm::GlobalValue::ExternalLinkage, identifier.name.c_str(), Environment.myModule);
	llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(GlobalContext, "entry", function, 0);
    MyIRBuilder.SetInsertPoint(entry_block);
    Environment.Caller_function = function;
    Environment.returnaddr = llvm::BasicBlock::Create(GlobalContext, "return", function, 0);
    // use return value to record the value
    if(func_type.name.compare("void") != 0) {
        Environment.returnvalue = new llvm::AllocaInst(getLLvmType_var(func_type.name), entry_block->getParent()->getParent()->getDataLayout().getAllocaAddrSpace(), "", entry_block);
    }
    //push function's symboltable
	Environment.PushSymboltable();

	llvm::Function::arg_iterator func_argvalue_iter = function->arg_begin();
    llvm::Value* store_value;

    //generate IR code for function arguments
    Environment.generate_code_for_args = true;
    for(auto it : args){
        (*it).GenerateCoder(Environment);
        store_value = &*func_argvalue_iter++;
        store_value->setName((it)->identifier.name.c_str());
        llvm::StoreInst *inst = new llvm::StoreInst(store_value, Environment.Get_localvar_values()[(it)->identifier.name], false, entry_block);
	}
    Environment.generate_code_for_args = false;
	
    //generate IR code for function's inner Stmts
	block.GenerateCoder(Environment);
    Environment.Return_or_break = false;

    MyIRBuilder.SetInsertPoint(Environment.returnaddr);
    if(func_type.name == "void") {
        MyIRBuilder.CreateRetVoid();
    } else {
        llvm::Value* return_value = MyIRBuilder.CreateLoad(getLLvmType_var(func_type.name), Environment.returnvalue, "");
        MyIRBuilder.CreateRet(return_value);
    }

	Environment.PopSymboltable();
    Environment.Caller_function = nullptr;
}
