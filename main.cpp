#include "fstream"
#include "GenerateCode.h"
#include "Ast.h"
#include <llvm/Support/TargetSelect.h>

extern int yyparse(void);
extern BlockNode* programBlock;

int main(){
    yyparse();
    //BlockNode* Root;
    Environment* generator = new Environment();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    cout<<"program begin"<<endl;

    // jsonize AST
    string jsonString("");
    programBlock->generateJson(jsonString);
    ofstream fout;
    fout.open("./AST.json");
    fout << jsonString;
    fout.close();

    generator->run(programBlock);

    return 0;

}