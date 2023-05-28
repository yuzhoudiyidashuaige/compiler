#include "fstream"
#include "GenerateCode.h"
#include "Ast.h"
#include <llvm/Support/TargetSelect.h>

extern int yyparse(void);
extern Block* programBlock;

int main(){
    yyparse();
    Environment* generator = new Environment();
    llvm::InitializeNativeTarget();     //  according to the doc
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    std::string jsonString = "";
    programBlock->json(jsonString); //generate Json iteratively
    ofstream fout;
    fout.open("./AST.json");
    fout << jsonString;
    fout.close();

    generator->run(programBlock);//generate IR code
    return 0;
}